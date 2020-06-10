#include "SldMacros.h"

#include "SldMetadataParser.h"

#include "SldCompare.h"
#include "SldCSSDataManager.h"
#include "SldDictionary.h"
#include "SldMetadataManager.h"
#include "SldUnicode.h"

#define U16(_TEXT_) SldU16StringLiteral(_TEXT_)

// bit counts used for storing merged dict index and a per dictionary css block index
// inside the css block indices handed out to the users
// that's good enough for 64 dictionaries and ~67mil css blocks per dictionary
enum : unsigned { MergedDictIndexBits = 6u, MergedCSSBlockIndexBits = 32u - MergedDictIndexBits };

// Парсит строку которая подразумевает html цвет
static ESldError ParseHtmlColorParam(const UInt16 *aStr, UInt32 aLen, UInt32 *aColor)
{
	if (aLen != 6 && aLen != 8)
		return eMetadataErrorParsingTextMetadata;

	ESldError error = CSldCompare::StrToUInt32(aStr, 16, aColor);
	if (error != eOK)
		return error;

	// make colors without alpha channel fully opaque
	if (aLen == 6)
		*aColor = (*aColor << 8) | 0xff;
	return eOK;
}

static ESldError ParseUIntParam(const UInt16 *aText, UInt32 aRadix, UInt32 *aNumber)
{
	return CSldCompare::StrToUInt32(aText, aRadix, aNumber);
}

static ESldError ParseUIntParam(const UInt16 *aText, UInt32 aRadix, UInt16 *aNumber)
{
	UInt32 number;
	ESldError error = CSldCompare::StrToUInt32(aText, aRadix, &number);
	if (error != eOK || number > 0xffff)
		return eMetadataErrorToUIntConvertion;

	*aNumber = (UInt16)number;
	return eOK;
}

static ESldError ParseUIntParam(const UInt16 *aText, UInt32 aRadix, UInt8 *aNumber)
{
	UInt32 number;
	ESldError error = CSldCompare::StrToUInt32(aText, aRadix, &number);
	if (error != eOK || number > 0xff)
		return eMetadataErrorToUIntConvertion;

	*aNumber = (UInt8)number;
	return eOK;
}

// local stoul analog
static UInt32 stou32(SldU16StringRef aStr, UInt32 aRadix = 10, UInt32 *aLength = nullptr)
{
	UInt32 number = 0;
	if (aLength != nullptr)
		*aLength = 0;

	for (UInt16 chr : aStr)
	{
		if ((UInt32)(chr - '0') <= 9)
			number = aRadix * number + (chr - '0');
		else if ((UInt32)((chr | ' ') - 'a') <= 5 && aRadix == 16)
			number = aRadix * number + ((chr | ' ') - 'a' + 10);
		else // return the number on first non digit
			return number;

		if (aLength != nullptr)
			(*aLength)++;
	}

	return number;
}

static SldU16StringRef ParseDataIndex(SldU16StringRef aStr, UInt16 aChr, UInt32 *aIndex)
{
	if (aStr.empty() || aStr.front() != aChr)
		return aStr;

	UInt32 len;
	const UInt32 index = stou32(aStr.substr(1), 16, &len);
	if (len != 0)
		*aIndex = index;
	return aStr.substr(len + 1);
}

// Парсит индексы бинарных метаданных и css блока
CSldMetadataParser::CommonInfo CSldMetadataParser::ParseCommonInfo(SldU16StringRef &aStr) const
{
	CommonInfo info{ false, InvalidDataIndex, InvalidDataIndex };

	info.isClosing = aStr == SldU16StringRefLiteral("close");
	if (info.isClosing)
		return info;

	// for "binary" metadata we expect the string to be of the form:
	//   @<id>
	// where <id> is the index of the binary metadata (in base-16)
	aStr = ParseDataIndex(aStr, '@', &info.metadataIdx);

	// css property "blocks" are expected to be denoted by:
	//   #<id>
	// where <id> is the index of the css prop block (in base-16)
	aStr = ParseDataIndex(aStr, '#', &info.cssBlockIdx);

	if (info.cssBlockIdx != InvalidDataIndex && m_dictionary)
	{
		UInt32 dictIdx = m_dictionary->GetMergedDictIndex();
		// sld_assert(dictIdx <= (1u << MergedDictIndexBits), "are you mad?");
		// sld_assert(info.cssBlockIdx < (1u << MergedCSSBlockIndexBits), "not enough room for a css block index");
		info.cssBlockIdx |= (dictIdx << MergedCSSBlockIndexBits);
	}

	return info;
}

// Functions for working with CSldMetadataParser::TStrings
namespace {

// initalizes the strings structure for textual strings
static void init(CSldMetadataParser::TStrings &strings, UInt16 *aMem, UInt16 aMemSize)
{
	if (aMem && aMemSize)
	{
		strings.owned = false;
		strings.data = aMem;
		strings.capacity = aMemSize;
	}
	else
	{
		strings.owned = true;
		strings.data = NULL;
		strings.capacity = 0;
	}
	strings.current = 0;
}

// deinitalizer
static void clear(CSldMetadataParser::TStrings &strings)
{
	strings.current = 0;
	//sldMemZero(strings.data, strings.capacity * sizeof(strings.data[0]));
}

// returns a string at the given "index"
static SldU16StringRef get(const CSldMetadataParser::TStrings &strings, UInt16 aIndex)
{
	const UInt16 *string = aIndex < strings.capacity ? strings.data + aIndex : nullptr;
	return string ? SldU16StringRef(string + 1, *string) : nullptr;
}

// ensures that we have enough room for a string with @aLen length
static bool ensureCapacity(CSldMetadataParser::TStrings &strings, const UInt32 aLen)
{
	const UInt32 newCapacity = strings.current + aLen;
	if (newCapacity <= strings.capacity)
		return true;

	// we can't grow non-owned string storage, nor can we store strings longer than 2^16
	if (!strings.owned || newCapacity > 0xffff)
		return false;

	UInt16 *data = sldMemReallocT(strings.data, newCapacity);
	if (!data)
		return false;

	strings.data = data;
	strings.capacity = static_cast<UInt16>(newCapacity);
	return true;
}

/**
 * "allocates" a string
 *
 * @param[in]    strings  - the strings object where to push the strings
 * @param[in]    aLen     - length of the string
 * @param[out]   aString  - the metadata string reference that will be set
 *                          "pointing" to the created string
 *
 * @return pointer to the beginning of the string (guaranteed to be able to "take" aLen + 1 chars)
 *         or nullptr in case of oom
 */
static UInt16* allocate(CSldMetadataParser::TStrings &strings, UInt32 aLen, TMetadataString *aString)
{
	// we use a single uint16 to encode length
	if (aLen > 0xffff)
		return nullptr;

	// make room for a null-terminator & string length
	aLen += 2;

	if (!ensureCapacity(strings, aLen))
		return nullptr;

	const UInt16 index = strings.current;
	UInt16 *string = strings.data + index;
	strings.current += static_cast<UInt16>(aLen);

	aString->type = eMetadataStringSource_Memory;
	aString->data = index;

	// store the length (without the nul-terminator & length itself) in the first uint16
	*string = (UInt16)aLen - 2;
	return string + 1;
}

} // namespace

/// Инициализация
CSldMetadataParser::CSldMetadataParser(CSldDictionary *aDict, CSldCSSUrlResolver *aResolver, UInt16 *aMem, UInt16 aMemSize)
	: m_dictionary(aDict), m_metadataManager(aDict ? aDict->GetMetadataManager() : nullptr), m_resolver(aResolver),
	  m_mergeInfo(aDict ? aDict->GetMergeInfo() : nullptr),
	  m_mergedDictIdx(m_mergeInfo ? aDict->GetMergedDictIndex() : 0)
{
	init(m_strings, aMem, aMemSize);
}

/// Деструктор
CSldMetadataParser::~CSldMetadataParser()
{
	if (m_strings.owned && m_strings.data)
		sldMemFree(m_strings.data);
}

/**
 * Возвращает строку по переданной "ссылке"
 *
 * @param[in]  aString  - ссылка на строку
 *
 * @return "ссылка" на строку
 *         ВАЖНО! строка (контент) актуальна *только* до следующего вызова GetString()
 *                т.е. если надо сохранять и т.п. - необходимо копировать
 */
SldU16StringRef CSldMetadataParser::GetStringRef(const TMetadataString &aString) const
{
	switch (aString.type)
	{
	case eMetadataStringSource_Empty:
	{
		return SldU16StringRefLiteral("");
	}
	case eMetadataStringSource_List:
	{
		// we can safely cache this if needed
		const Int32 listIndex = m_dictionary->GetMetadataStringsListIndex();
		if (listIndex == SLD_DEFAULT_LIST_INDEX)
			return nullptr;

		UInt16 *word;
		ESldError error = m_dictionary->GetWordByGlobalIndex(listIndex, aString.data, 0, &word);
		return error == eOK ? word : nullptr;
	}
	// embedded strings use the same format & storage as the "legacy" ones
	case eMetadataStringSource_Memory:
	case eMetadataStringSource_Embedded:
	{
		return get(m_strings, aString.data);
	}
	case eMetadataStringSource_StringStore:
	{
		if (!m_metadataManager)
			return nullptr;

		SldU16StringRef string;
		ESldError error = m_metadataManager->GetString(aString.data, string);
		return error == eOK ? string : nullptr;
	}
	default:
		// XXX: this really should be either an abort() or some hard error about invalid data
		//assert(!"Invalid string type.");
		return nullptr;
	}
	return nullptr;
}

// copies & fixes up embedded metadata strings into the internal buffer
ESldError CSldMetadataParser::LoadEmbeddedString(SldU16StringRef aStr)
{
	// substrings in a single embedded metadata string are delimited with ascii STX & ETX
	// we copy the string into our internal buffer fixing up these special codes:
	// - STX becomes the substring length
	// - ETX becomes '\0' as we have cstring returning apis
	// this way the format of the stored strings matches the one used for the legacy ones
	// and we can use the same codepaths throughout

	if (aStr.front() != 0x2 || aStr.back() != 0x3)
		return eMetadataErrorParsingTextMetadata;

	clear(m_strings);
	if (!ensureCapacity(m_strings, aStr.size()))
		return eMemoryNotEnoughMemory;

	UInt16 *const string = m_strings.data;

	// TODO: benchmark copying end -> start as it's little bit less code
	UInt32 length = 0;
	UInt32 lengthPos = 0;
	for (UInt32 i = 0; i < aStr.size(); i++)
	{
		switch (aStr[i])
		{
		case 0x2: // STX - start of a substring
			lengthPos = i;
			break;
		case 0x3: // ETX - end of a substring
			string[lengthPos] = length;
			string[i] = '\0';
			length = 0;
			break;
		default:
			string[i] = aStr[i];
			length++;
			break;
		}
	}
	return eOK;
}

// Функция проверяющая символ на "стандартный разделитель" атрибутов метаданных
static inline bool isMetadataDelim(UInt16 aChr) { return aChr == ';'; }

/** *********************************************************************
* Получает следующую пару "параметр-значение" из строки
*
* @param[in]	aStr			- указатель на строку, из которой нужно получить пару "параметр-значение"
* @param[out]	aName			- сюда сохраняется имя параметра
* @param[out]	aVal			- сюда сохраняется значение параметра
* @param[in]	aIsDelimiter	- коллбэк определяющий является ли символ разделителем
*
* @return указатель на строку `после` текущего параметра или пустая строка
************************************************************************/
static bool GetNextParam(SldU16StringRef &aStr, UInt16 *aName, UInt16 *aVal, bool(*const aIsDelimiter)(UInt16) = isMetadataDelim)
{
	if (aStr.empty())
		return false;

	UInt32 i = 0;
	for (; i < aStr.size() && aStr[i] != '='; i++)
		*aName++ = aStr[i];
	*aName = '\0';

	i++; // skip '='
	if (i >= aStr.size() || aStr[i] != '"')
		return false;

	i++; // skip '"'
	for (; i < aStr.size() && aStr[i] != '"'; i++)
		*aVal++ = aStr[i];
	*aVal = '\0';

	i++; // skip '"'
	while (i < aStr.size() && aIsDelimiter(aStr[i]))
		i++;

	aStr = aStr.substr(i);
	return true;
}

// The metadata parsing "context" used internally by the metadata parser
struct CSldMetadataParser::ParseContext
{
	CSldMetadataParser &p;

	ParseContext(CSldMetadataParser &parser) : p(parser)
	{
		clear(p.m_strings);
	}

	// pushes a new string to the string cache "as is"
	ESldError PushString(SldU16StringRef aStr, TMetadataString *aString)
	{
		if (aStr.empty())
			return eOK;

		UInt16 *str = allocate(p.m_strings, aStr.size(), aString);
		if (!str)
			return eMemoryNotEnoughMemory;

		sld2::StrNCopy(str, aStr.data(), aStr.size());
		str[aStr.size()] = '\0';
		return eOK;
	}

	// pushes a new string to the string cache decoding it
	ESldError PushDecodedString(SldU16StringRef aStr, TMetadataString *aString)
	{
		if (aStr.empty())
			return eOK;

		UInt16 *str = allocate(p.m_strings, aStr.size(), aString);
		if (!str)
			return eMemoryNotEnoughMemory;

		// decode loop
		static const UInt16 codeMap[] = { 0, '=', '"', ';' };
		for (UInt16 chr : aStr)
			*str++ = chr < sld2::array_size(codeMap) ? codeMap[chr] : chr;
		*str = 0;

		return eOK;
	}
};

// local typedef of ParseContext
namespace { typedef CSldMetadataParser::ParseContext ParseContext; }

namespace {

/**
 *  Parses a metadata string into a metadata struct
 *
 *  @param [in]  parser       - pointer to CSldMetadataParser
 *  @param [in]  aStr         - the string to be parsed
 *  @param [out] aMetadata    - pointer to the metadata struct
 *  @param [in]  aCallback    - metadata attributes parsing "callback" for textual metadata
 *
 *  @return error code
 */
template <typename T, typename Callback>
static ESldError parse(CSldMetadataParser *parser, SldU16StringRef aStr,
					   T *aMetadata, Callback&& aCallback)
{
	ParseContext ctx(*parser);
	return aCallback(&ctx, aStr, aMetadata);
}

// callback used for parsing metadata attributes
template <typename T>
using ParseAttribCallback = ESldError(*)(ParseContext*, const UInt16 *key, const UInt16 *val, T *metaData);

/**
 *  Parses a metadata string into a metadata struct
 *
 *  @param [in]  parser       - pointer to CSldMetadataParser
 *  @param [in]  aStr         - the string to be parsed
 *  @param [out] aMetadata    - pointer to the metadata struct
 *  @param [in]  aHandler     - the metadata attributes "callback" for textual metadata
 *
 *  @return error code
 */
template <typename T>
static ESldError parse(CSldMetadataParser *parser, SldU16StringRef aStr, T *aMetadata,
					   const ParseAttribCallback<T> aHandler)
{
	const struct {
		const ParseAttribCallback<T> cb;
		ESldError operator()(ParseContext *ctx, SldU16StringRef aStr, T *aMetadata) const {
			// TODO: Introduce string_refs... we *can* avoid all this copying
			UInt16 key[MetaParamMaxNameSize] = { 0 };
			UInt16 val[MetaParamMaxValSize] = { 0 };
			while (GetNextParam(aStr, key, val))
			{
				// mangle the return value of the metadata parsing functions
				// there is no point in this zoo of different error codes when
				// all we possibly care about is if have some error or not...
				ESldError error = cb(ctx, key, val, aMetadata);
				if (error != eOK)
					return eMetadataErrorParsingTextMetadata;
			}
			return eOK;
		}
	} op = { aHandler };
	return parse(parser, aStr, aMetadata, op);
}

}

/// Парсит ссылку на внешнюю SDC базу
static ESldError ParseExtKey(ParseContext *ctx,
                             const UInt16 *aName, const UInt16 *aVal, TMetadataExtKey *aExtKey,
                             const UInt16 *const aDictIdName = U16("dictid"),
                             const UInt16 *const aListIdxName = U16("listidx"),
                             const UInt16 *const aKeyName = U16("key"))
{
	if (CSldCompare::StrCmp(aName, aDictIdName) == 0)
		return ctx->PushDecodedString(aVal, &aExtKey->DictId);

	else if (CSldCompare::StrCmp(aName, aListIdxName) == 0)
		return CSldCompare::StrToInt32(aVal, 10, &aExtKey->ListIdx);

	else if (CSldCompare::StrCmp(aName, aKeyName) == 0)
		return ctx->PushDecodedString(aVal, &aExtKey->Key);

	return eOK;
}

// Обработчики текстовых метаданных
namespace {
namespace handlers
{
// NOTE: the handlers must have a signature of ParseAttribCallback<T>

static ESldError AbstractResource(ParseContext *ctx, const UInt16 *key, const UInt16 *val, TMetadataAbstractResource *aMetadata)
{
	ESldError error = eOK;
	if (CSldCompare::StrCmp(key, U16("abstract_item_idx")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->ItemIdx);
	}
	else if (CSldCompare::StrCmp(key, U16("type")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->Type);
	}
	else
	{
		error = ParseExtKey(ctx, key, val, &aMetadata->ExtKey);
	}
	return error;
}

static ESldError AtomicObject(ParseContext *ctx, const UInt16 *key, const UInt16 *val, TMetadataAtomicObject *aMetadata)
{
	ESldError error = eOK;
	if (CSldCompare::StrCmp(key, U16("atomicIndex")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->Index);
	}
	else if (CSldCompare::StrCmp(key, U16("content_type")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->ContentType);
	}
	else if (CSldCompare::StrCmp(key, U16("logical_type")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->LogicalType);
	}
	else if (CSldCompare::StrCmp(key, U16("activity_type")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->ActivityType);
	}
	return error;
}

static ESldError BackgroundImage(ParseContext *ctx, const UInt16 *key, const UInt16 *val, TMetadataBackgroundImage *aMetadata)
{
	ESldError error = eOK;
	if (CSldCompare::StrCmp(key, U16("img_idx")) == 0)
	{
		error = CSldCompare::StrToInt32(val, 16, &aMetadata->PictureIndex);
	}
	else if (CSldCompare::StrCmp(key, U16("show_width")) == 0)
	{
		aMetadata->ShowWidth = TSizeValue::FromString(val, eMetadataUnitType_px);
	}
	else if (CSldCompare::StrCmp(key, U16("show_height")) == 0)
	{
		aMetadata->ShowHeight = TSizeValue::FromString(val, eMetadataUnitType_px);
	}
	else if (CSldCompare::StrCmp(key, U16("level")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->Level);
	}
	return error;
}

static ESldError Caption(ParseContext *ctx, const UInt16 *key, const UInt16 *val, TMetadataCaption *aMetadata)
{
	ESldError error = eOK;
	if (CSldCompare::StrCmp(key, U16("hide")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->Hide);
	}
	else if (CSldCompare::StrCmp(key, U16("editable")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->Editable);
	}
	else if (CSldCompare::StrCmp(key, U16("type")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->Type);
	}
	return error;
}

static ESldError CrosswordItem(ParseContext *ctx, const UInt16 *key, const UInt16 *val, TMetadataCrosswordItem *aMetadata)
{
	ESldError error = eOK;
	if (CSldCompare::StrCmp(key, U16("x")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->X);
	}
	else if (CSldCompare::StrCmp(key, U16("y")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->Y);
	}
	else if (CSldCompare::StrCmp(key, U16("direction")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->Direction);
	}
	else if (CSldCompare::StrCmp(key, U16("answer")) == 0)
	{
		error = ctx->PushDecodedString(val, &aMetadata->Answer);
	}
	else if (CSldCompare::StrCmp(key, U16("init")) == 0)
	{
		error = ctx->PushDecodedString(val, &aMetadata->Init);
	}
	return error;
}

static ESldError ExternArticle(ParseContext *ctx, const UInt16 *key, const UInt16 *val, TMetadataExternArticle *aMetadata)
{
	ESldError error = eOK;
	if (CSldCompare::StrCmp(key, U16("list_idx")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->ListIndex);
	}
	else if (CSldCompare::StrCmp(key, U16("entry_idx")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->EntryIndex);
	}
	else if (CSldCompare::StrCmp(key, U16("article_id")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->ArticleIndex);
	}
	else if (CSldCompare::StrCmp(key, U16("block_start")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->BlockStart);
	}
	else if (CSldCompare::StrCmp(key, U16("block_end")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->BlockEnd);
	}
	else
	{
		error = ParseExtKey(ctx, key, val, &aMetadata->ExtKey);
	}
	return error;
}

static ESldError FlashCardsLink(ParseContext *ctx, const UInt16 *key, const UInt16 *val, TMetadataFlashCardsLink *aMetadata)
{
	ESldError error = eOK;
	if (CSldCompare::StrCmp(key, U16("front_list_idx")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->FrontListId);
	}
	else if (CSldCompare::StrCmp(key, U16("front_entry_idx")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->FrontListEntryId);
	}
	else if (CSldCompare::StrCmp(key, U16("back_list_idx")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->BackListId);
	}
	else if (CSldCompare::StrCmp(key, U16("back_entry_idx")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->BackListEntryId);
	}
	return error;
}

static ESldError Formula(ParseContext *ctx, const UInt16 *key, const UInt16 *val, TMetadataFormula *aMetadata)
{
	ESldError error = eOK;
	if (CSldCompare::StrCmp(key, U16("abstract_item_idx")) == 0)
	{
		error = ParseUIntParam(val, 16, &aMetadata->AbstractItemIdx);
	}
	return error;
}

static ESldError Hide(ParseContext *ctx, const UInt16 *key, const UInt16 *val, TMetadataHide *aMetadata)
{
	ESldError error = eOK;
	if (CSldCompare::StrCmp(key, U16("label")) == 0)
	{
		error = ctx->PushString(val, &aMetadata->Label);
	}
	else if (CSldCompare::StrCmp(key, U16("control")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->HasControl);
	}
	return error;
}

static ESldError Image(ParseContext *ctx, const UInt16 *key, const UInt16 *val, TMetadataImage *aMetadata)
{
	ESldError error = eOK;
	if (CSldCompare::StrCmp(key, U16("img_idx")) == 0)
	{
		error = CSldCompare::StrToInt32(val, 16, &aMetadata->PictureIndex);
	}
	else if (CSldCompare::StrCmp(key, U16("full_img_idx")) == 0)
	{
		error = CSldCompare::StrToInt32(val, 16, &aMetadata->FullPictureIndex);
	}
	else if (CSldCompare::StrCmp(key, U16("show_width")) == 0)
	{
		aMetadata->ShowWidth = TSizeValue::FromString(val, eMetadataUnitType_px);
	}
	else if (CSldCompare::StrCmp(key, U16("show_height")) == 0)
	{
		aMetadata->ShowHeight = TSizeValue::FromString(val, eMetadataUnitType_px);
	}
	else if (CSldCompare::StrCmp(key, U16("scalable")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->Scalable);
	}
	else if (CSldCompare::StrCmp(key, U16("system")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->System);
	}
	else if (CSldCompare::StrCmp(key, U16("show_width_f")) == 0)
	{
		aMetadata->FullShowWidth = TSizeValue::FromString(val, eMetadataUnitType_px);
	}
	else if (CSldCompare::StrCmp(key, U16("show_height_f")) == 0)
	{
		aMetadata->FullShowHeight = TSizeValue::FromString(val, eMetadataUnitType_px);
	}
	else
	{
		error = ParseExtKey(ctx, key, val, &aMetadata->ExtKey);
		if (error == eOK)
			error = ParseExtKey(ctx, key, val, &aMetadata->FullExtKey, U16("dictid_f"), U16("listidx_f"), U16("key_f"));
	}
	return error;
}

static ESldError ImageArea(ParseContext *ctx, const UInt16 *key, const UInt16 *val, TMetadataImageArea *aMetadata)
{
	ESldError error = eOK;
	if (!CSldCompare::StrCmp(key, U16("id")))
	{
		error = ctx->PushString(val, &aMetadata->Id);
	}
	else if (!CSldCompare::StrCmp(key, U16("left")))
	{
		error = ParseUIntParam(val, 10, &aMetadata->Left);
	}
	else if (!CSldCompare::StrCmp(key, U16("top")))
	{
		error = ParseUIntParam(val, 10, &aMetadata->Top);
	}
	else if (!CSldCompare::StrCmp(key, U16("width")))
	{
		error = ParseUIntParam(val, 10, &aMetadata->Width);
	}
	else if (!CSldCompare::StrCmp(key, U16("height")))
	{
		error = ParseUIntParam(val, 10, &aMetadata->Height);
	}
	else if (!CSldCompare::StrCmp(key, U16("type")))
	{
		error = ParseUIntParam(val, 10, &aMetadata->Type);
	}
	else if (!CSldCompare::StrCmp(key, U16("action")))
	{
		error = ctx->PushString(val, &aMetadata->ActionScript);
	}
	else if (!CSldCompare::StrCmp(key, U16("percent")))
	{
		error = ParseUIntParam(val, 10, &aMetadata->Percent);
	}
	else if (!CSldCompare::StrCmp(key, U16("coords")))
	{
		error = ctx->PushString(val, &aMetadata->Coords);
	}
	else if (!CSldCompare::StrCmp(key, U16("mask")))
	{
		error = ParseUIntParam(val, 10, &aMetadata->Mask);
	}
	return error;
}

static ESldError InfoBlock(ParseContext *ctx, const UInt16 *key, const UInt16 *val, TMetadataInfoBlock *aMetadata)
{
	ESldError error = eOK;
	if (CSldCompare::StrCmp(key, U16("type")) == 0)
	{
		error = ctx->PushString(val, &aMetadata->Type);
	}
	return error;
}

static ESldError InteractiveObject(ParseContext *ctx, const UInt16 *key, const UInt16 *val, TMetadataInteractiveObject *aMetadata)
{
	ESldError error = eOK;
	if (CSldCompare::StrCmp(key, U16("id")) == 0)
	{
		error = ctx->PushDecodedString(val, &aMetadata->Id);
	}
	else if (CSldCompare::StrCmp(key, U16("shape")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->Shape);
	}
	else if (CSldCompare::StrCmp(key, U16("starting_pos")) == 0)
	{
		error = ctx->PushDecodedString(val, &aMetadata->StartingPos);
	}
	else if (CSldCompare::StrCmp(key, U16("starting_size")) == 0)
	{
		error = ctx->PushDecodedString(val, &aMetadata->StartingSize);
	}
	else if (CSldCompare::StrCmp(key, U16("starting_angle")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->StartingAngle);
	}
	else if (CSldCompare::StrCmp(key, U16("draggable")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->Draggable);
	}
	else if (CSldCompare::StrCmp(key, U16("rotatable")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->Rotatable);
	}
	else if (CSldCompare::StrCmp(key, U16("scalable")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->Scalable);
	}
	return error;
}

static ESldError Label(ParseContext *ctx, const UInt16 *key, const UInt16 *val, TMetadataLabel *aMetadata)
{
	ESldError error = eOK;
	if (CSldCompare::StrCmp(key, U16("data")) == 0)
	{
		error = ctx->PushString(val, &aMetadata->Data);
	}
	else if (CSldCompare::StrCmp(key, U16("type")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->Type);
	}
	return error;
}

static ESldError LegendItem(ParseContext *ctx, const UInt16 *key, const UInt16 *val, TMetadataLegendItem *aMetadata)
{
	ESldError error = eOK;
	if (CSldCompare::StrCmp(key, U16("type")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->Type);
	}
	return error;
}

static ESldError Link(ParseContext *ctx, const UInt16 *key, const UInt16 *val, TMetadataLink *aMetadata)
{
	ESldError error = eOK;
	if (CSldCompare::StrCmp(key, U16("list_idx")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->ListIndex);
	}
	else if (CSldCompare::StrCmp(key, U16("entry_idx")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->EntryIndex);
	}
	else if (CSldCompare::StrCmp(key, U16("title")) == 0)
	{
		error = ctx->PushDecodedString(val, &aMetadata->Title);
	}
	else if (CSldCompare::StrCmp(key, U16("link_type")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->Type);
	}
	else if (CSldCompare::StrCmp(key, U16("label")) == 0)
	{
		error = ctx->PushString(val, &aMetadata->Label);
	}
	else if (CSldCompare::StrCmp(key, U16("self")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->Self);
	}
	else
	{
		error = ParseExtKey(ctx, key, val, &aMetadata->ExtKey);
	}
	return error;
}

static ESldError List(ParseContext *ctx, const UInt16 *key, const UInt16 *val, TMetadataList *aMetadata)
{
	ESldError error = eOK;
	if (CSldCompare::StrCmp(key, U16("style")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->Style);
		if (error == eOK)
			aMetadata->Style++;
	}
	else if (CSldCompare::StrCmp(key, U16("type")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->Type);
	}
	else if (CSldCompare::StrCmp(key, U16("marker")) == 0)
	{
		error = ctx->PushDecodedString(val, &aMetadata->Marker);
	}
	else if (CSldCompare::StrCmp(key, U16("number")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->Number);
	}
	return error;
}

static ESldError ManagedSwitch(ParseContext *ctx, SldU16StringRef aStr, TMetadataManagedSwitch *aMetadata)
{
	return ctx->PushString(aStr, &aMetadata->Label);
}

static ESldError Map(ParseContext *ctx, const UInt16 *key, const UInt16 *val, TMetadataMap *aMetadata)
{
	ESldError error = eOK;
	if (CSldCompare::StrCmp(key, U16("type")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->Type);
	}
	else if (CSldCompare::StrCmp(key, U16("base_image_idx")) == 0)
	{
		error = ParseUIntParam(val, 16, &aMetadata->PictureIndex);
	}
	else if (CSldCompare::StrCmp(key, U16("top_image_idx")) == 0)
	{
		error = ParseUIntParam(val, 16, &aMetadata->TopPictureIndex);
	}
	return error;
}

static ESldError MapElement(ParseContext *ctx, const UInt16 *key, const UInt16 *val, TMetadataMapElement *aMetadata)
{
	ESldError error = eOK;
	if (CSldCompare::StrCmp(key, U16("name")) == 0)
	{
		error = ctx->PushString(val, &aMetadata->Name);
	}
	return error;
}

static ESldError MediaContainer(ParseContext *ctx, const UInt16 *key, const UInt16 *val, TMetadataMediaContainer *aMetadata)
{
	ESldError error = eOK;
	if (CSldCompare::StrCmp(key, U16("type")) == 0)
	{
		error = ParseUIntParam(val, 16, &aMetadata->Type);
	}
	else if (CSldCompare::StrCmp(key, U16("colour")) == 0)
	{
		error = ParseUIntParam(val, 16, &aMetadata->Colour);
	}
	else if (CSldCompare::StrCmp(key, U16("colour_begin")) == 0)
	{
		error = ParseUIntParam(val, 16, &aMetadata->ColourBegin);
	}
	else if (CSldCompare::StrCmp(key, U16("colour_end")) == 0)
	{
		error = ParseUIntParam(val, 16, &aMetadata->ColourEnd);
	}
	else if (CSldCompare::StrCmp(key, U16("gradient")) == 0)
	{
		error = ParseUIntParam(val, 16, &aMetadata->Gradient);
	}
	else if (CSldCompare::StrCmp(key, U16("background")) == 0)
	{
		error = ParseUIntParam(val, 16, &aMetadata->Background);
	}
	else if (CSldCompare::StrCmp(key, U16("tiling")) == 0)
	{
		error = ParseUIntParam(val, 16, &aMetadata->Tiling);
	}
	else if (CSldCompare::StrCmp(key, U16("flow")) == 0)
	{
		error = ParseUIntParam(val, 16, &aMetadata->Flow);
	}
	else if (CSldCompare::StrCmp(key, U16("display")) == 0)
	{
		error = ParseUIntParam(val, 16, &aMetadata->Display);
	}
	else if (CSldCompare::StrCmp(key, U16("float")) == 0)
	{
		error = ParseUIntParam(val, 16, &aMetadata->Float);
	}
	else if (CSldCompare::StrCmp(key, U16("width")) == 0)
	{
		aMetadata->Width = TSizeValue::FromString(val);
	}
	else if (CSldCompare::StrCmp(key, U16("height")) == 0)
	{
		aMetadata->Height = TSizeValue::FromString(val);
	}
	else if (CSldCompare::StrCmp(key, U16("clear")) == 0)
	{
		error = ParseUIntParam(val, 16, &aMetadata->ClearType);
	}
	else if (CSldCompare::StrCmp(key, U16("expansion")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->Expansion);
	}
	else if (CSldCompare::StrCmp(key, U16("zoom_to_full")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->ZoomToFull);
	}
	else if (CSldCompare::StrCmp(key, U16("interactivity")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->Interactivity);
	}
	else if (CSldCompare::StrCmp(key, U16("align_vertical")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->AlignVertical);
	}
	else if (CSldCompare::StrCmp(key, U16("align_horizontal")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->AlignHorizont);
	}
	else if (CSldCompare::StrCmp(key, U16("margin_top")) == 0)
	{
		aMetadata->MarginTop = TSizeValue::FromString(val, eMetadataUnitType_px);
	}
	else if (CSldCompare::StrCmp(key, U16("margin_right")) == 0)
	{
		aMetadata->MarginRight = TSizeValue::FromString(val, eMetadataUnitType_px);
	}
	else if (CSldCompare::StrCmp(key, U16("margin_bottom")) == 0)
	{
		aMetadata->MarginBottom = TSizeValue::FromString(val, eMetadataUnitType_px);
	}
	else if (CSldCompare::StrCmp(key, U16("margin_left")) == 0)
	{
		aMetadata->MarginLeft = TSizeValue::FromString(val, eMetadataUnitType_px);
	}
	else if (CSldCompare::StrCmp(key, U16("padding_top")) == 0)
	{
		aMetadata->PaddingTop = TSizeValue::FromString(val, eMetadataUnitType_px);
	}
	else if (CSldCompare::StrCmp(key, U16("padding_right")) == 0)
	{
		aMetadata->PaddingRight = TSizeValue::FromString(val, eMetadataUnitType_px);
	}
	else if (CSldCompare::StrCmp(key, U16("padding_bottom")) == 0)
	{
		aMetadata->PaddingBottom = TSizeValue::FromString(val, eMetadataUnitType_px);
	}
	else if (CSldCompare::StrCmp(key, U16("padding_left")) == 0)
	{
		aMetadata->PaddingLeft = TSizeValue::FromString(val, eMetadataUnitType_px);
	}
	else if (CSldCompare::StrCmp(key, U16("shadow")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->Shadow);
	}
	else if (CSldCompare::StrCmp(key, U16("overlay_src")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->OverlaySrc);
	}
	else if (CSldCompare::StrCmp(key, U16("overlay_pos")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->OverlayPos);
	}
	else if (CSldCompare::StrCmp(key, U16("baseline")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->Baseline);
	}
	else if (CSldCompare::StrCmp(key, U16("border_style")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->BorderStyle);
	}
	else if (CSldCompare::StrCmp(key, U16("border_size")) == 0)
	{
		aMetadata->BorderSize = TSizeValue::FromString(val, eMetadataUnitType_px);
	}
	else if (CSldCompare::StrCmp(key, U16("border_color")) == 0)
	{
		error = ParseUIntParam(val, 16, &aMetadata->BorderColor);
	}
	return error;
}

static ESldError Paragraph(ParseContext *ctx, const UInt16 *key, const UInt16 *val, TMetadataParagraph *aMetadata)
{
	ESldError error = eOK;
	if (CSldCompare::StrCmp(key, U16("depth")) == 0)
	{
		aMetadata->Depth = TSizeValue::FromString(val, eMetadataUnitType_em);
	}
	else if (CSldCompare::StrCmp(key, U16("indent")) == 0)
	{
		aMetadata->Indent = TSizeValue::FromString(val, eMetadataUnitType_em);
	}
	else if (CSldCompare::StrCmp(key, U16("align")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->Align);
	}
	else if (CSldCompare::StrCmp(key, U16("dir")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->TextDirection);
	}
	else if (CSldCompare::StrCmp(key, U16("margin-top")) == 0)
	{
		aMetadata->MarginTop = TSizeValue::FromString(val, eMetadataUnitType_em);
	}
	else if (CSldCompare::StrCmp(key, U16("margin-bottom")) == 0)
	{
		aMetadata->MarginBottom = TSizeValue::FromString(val, eMetadataUnitType_em);
	}
	return error;
}

static ESldError PopupArticle(ParseContext *ctx, const UInt16 *key, const UInt16 *val, TMetadataPopupArticle *aMetadata)
{
	ESldError error = eOK;
	if (CSldCompare::StrCmp(key, U16("list_idx")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->ListIndex);
	}
	else if (CSldCompare::StrCmp(key, U16("entry_idx")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->EntryIndex);
	}
	else if (CSldCompare::StrCmp(key, U16("title")) == 0)
	{
		error = ctx->PushDecodedString(val, &aMetadata->Title);
	}
	else if (CSldCompare::StrCmp(key, U16("label")) == 0)
	{
		error = ctx->PushString(val, &aMetadata->Label);
	}
	else
	{
		error = ParseExtKey(ctx, key, val, &aMetadata->ExtKey);
	}
	return error;
}

static ESldError PopupImage(ParseContext *ctx, const UInt16 *key, const UInt16 *val, TMetadataPopupImage *aMetadata)
{
	ESldError error = eOK;
	if (CSldCompare::StrCmp(key, U16("popup_img_idx")) == 0)
	{
		error = CSldCompare::StrToInt32(val, 16, &aMetadata->PictureIndex);
	}
	else if (CSldCompare::StrCmp(key, U16("popup_show_width")) == 0)
	{
		aMetadata->ShowWidth = TSizeValue::FromString(val, eMetadataUnitType_px);
	}
	else if (CSldCompare::StrCmp(key, U16("popup_show_height")) == 0)
	{
		aMetadata->ShowHeight = TSizeValue::FromString(val, eMetadataUnitType_px);
	}
	else
	{
		error = ParseExtKey(ctx, key, val, &aMetadata->ExtKey, U16("popup_dictid"), U16("popup_listidx"), U16("popup_key"));
	}
	return error;
}

static ESldError Scene3D(ParseContext *ctx, const UInt16 *key, const UInt16 *val, TMetadataScene3D *aMetadata)
{
	ESldError error = eOK;
	if (CSldCompare::StrCmp(key, U16("scene_idx")) == 0)
	{
		error = CSldCompare::StrToInt32(val, 16, &aMetadata->Id);
	}
	else
	{
		error = ParseExtKey(ctx, key, val, &aMetadata->ExtKey);
	}
	return error;
}

static ESldError SlideShow(ParseContext *ctx, const UInt16 *key, const UInt16 *val, TMetadataSlideShow *aMetadata)
{
	ESldError error = eOK;
	if (CSldCompare::StrCmp(key, U16("slide_show_flow")) == 0)
	{
		error = ParseUIntParam(val, 16, &aMetadata->Flow);
	}
	else if (CSldCompare::StrCmp(key, U16("slide_show_move_effect")) == 0)
	{
		error = ParseUIntParam(val, 16, &aMetadata->MoveEffect);
	}
	else if (CSldCompare::StrCmp(key, U16("slide_show_place")) == 0)
	{
		error = ParseUIntParam(val, 16, &aMetadata->Place);
	}
	else if (CSldCompare::StrCmp(key, U16("slide_show_show_effect")) == 0)
	{
		error = ParseUIntParam(val, 16, &aMetadata->ShowEffect);
	}
	else if (CSldCompare::StrCmp(key, U16("slide_show_sound")) == 0)
	{
		error = ParseUIntParam(val, 16, &aMetadata->Sound);
	}
	else if (CSldCompare::StrCmp(key, U16("slide_show_time")) == 0)
	{
		error = ParseUIntParam(val, 16, &aMetadata->Time);
	}
	else if (CSldCompare::StrCmp(key, U16("slide_show_time_step")) == 0)
	{
		error = ctx->PushString(val, &aMetadata->TimeStep);
	}
	else if (CSldCompare::StrCmp(key, U16("slide_show_list_idx")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->ListIndex);
	}
	else if (CSldCompare::StrCmp(key, U16("slide_show_entry_idx")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->WordIndex);
	}
	else if (CSldCompare::StrCmp(key, U16("width")) == 0)
	{
		aMetadata->Width = TSizeValue::FromString(val);
	}
	else if (CSldCompare::StrCmp(key, U16("height")) == 0)
	{
		aMetadata->Height = TSizeValue::FromString(val);
	}
	return error;
}

static ESldError Sound(ParseContext *ctx, const UInt16 *key, const UInt16 *val, TMetadataSound *aMetadata)
{
	ESldError error = eOK;
	if (CSldCompare::StrCmp(key, U16("snd_idx")) == 0)
	{
		error = CSldCompare::StrToInt32(val, 16, &aMetadata->Index);
	}
	else if (CSldCompare::StrCmp(key, U16("snd_lang")) == 0)
	{
		error = ctx->PushString(val, &aMetadata->Lang);
	}
	else
	{
		error = ParseExtKey(ctx, key, val, &aMetadata->ExtKey);
	}
	return error;
}

static ESldError Switch(ParseContext *ctx, const UInt16 *key, const UInt16 *val, TMetadataSwitch *aMetadata)
{
	ESldError error = eOK;
	if (CSldCompare::StrCmp(key, U16("inline")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->Inline);
	}
	else if (CSldCompare::StrCmp(key, U16("manage")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->Manage);
	}
	else if (CSldCompare::StrCmp(key, U16("label")) == 0)
	{
		error = ctx->PushString(val, &aMetadata->Label);
	}
	else if (CSldCompare::StrCmp(key, U16("thematic")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->Thematic);
	}
	else if (CSldCompare::StrCmp(key, U16("states_number")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->NumStates);
	}
	return error;
}

static ESldError SwitchControl(ParseContext *ctx, const UInt16 *key, const UInt16 *val, TMetadataSwitchControl *aMetadata)
{
	ESldError error = eOK;
	if (CSldCompare::StrCmp(key, U16("states_number")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->NumStates);
	}
	else if (CSldCompare::StrCmp(key, U16("thematic")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->Thematic);
	}
	return error;
}

static ESldError Table(ParseContext *ctx, const UInt16 *key, const UInt16 *val, TMetadataTable *aMetadata)
{
	ESldError error = eOK;
	if (CSldCompare::StrCmp(key, U16("width")) == 0)
	{
		if (CSldCompare::StrCmp(val, U16("FULL")) == 0)
		{
			aMetadata->Width.Set(100 * SLD_SIZE_VALUE_SCALE, eMetadataUnitType_percent); // 100%
		}
		else if (CSldCompare::StrCmp(val, U16("AUTO")) != 0)
		{
			aMetadata->Width = TSizeValue::FromString(val);
		}
	}
	else if (CSldCompare::StrCmp(key, U16("cellpadding")) == 0)
	{
		aMetadata->CellPadding = TSizeValue::FromString(val);
	}
	else if (CSldCompare::StrCmp(key, U16("cellspacing")) == 0)
	{
		aMetadata->CellSpacing = TSizeValue::FromString(val);
	}
	return error;
}

static ESldError TableCell(ParseContext *ctx, const UInt16 *key, const UInt16 *val, TMetadataTableCell *aMetadata)
{
	ESldError error = eOK;
	if (CSldCompare::StrCmp(key, U16("rowspan")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->RowSpan);
	}
	else if (CSldCompare::StrCmp(key, U16("colspan")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->ColSpan);
	}
	else if (CSldCompare::StrCmp(key, U16("bgcolor")) == 0)
	{
		if (CSldCompare::StrCmp(val, U16("transparent")) == 0)
			aMetadata->BgColor = 0xFFFFFF00; // transparent white
		else
			error = ParseHtmlColorParam(val, CSldCompare::StrLen(val), &aMetadata->BgColor);
	}
	else if (CSldCompare::StrCmp(key, U16("border-style")) == 0)
	{
		if (CSldCompare::StrCmp(val, U16("NONE")) == 0 || CSldCompare::StrCmp(val, U16("none")) == 0)
			aMetadata->BorderStyle = eBorderStyleNone;
		else if (CSldCompare::StrCmp(val, U16("SOLID")) == 0)
			aMetadata->BorderStyle = eBorderStyleSolid;
	}
	else if (CSldCompare::StrCmp(key, U16("border-size")) == 0)
	{
		aMetadata->BorderSize = TSizeValue::FromString(val, eMetadataUnitType_px);
	}
	else if (CSldCompare::StrCmp(key, U16("border-color")) == 0)
	{
		error = ParseHtmlColorParam(val, CSldCompare::StrLen(val), &aMetadata->BorderColor);
	}
	else if (CSldCompare::StrCmp(key, U16("width")) == 0)
	{
		aMetadata->Width = TSizeValue::FromString(val);
	}
	else if (CSldCompare::StrCmp(key, U16("text-align")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->TextAlign);
	}
	else if (CSldCompare::StrCmp(key, U16("text-valign")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->TextVertAlign);
	}
	else if (CSldCompare::StrCmp(key, U16("border-top-width")) == 0)
	{
		aMetadata->BorderTopWidth = TSizeValue::FromString(val, eMetadataUnitType_px);
	}
	else if (CSldCompare::StrCmp(key, U16("border-bottom-width")) == 0)
	{
		aMetadata->BorderBottomWidth = TSizeValue::FromString(val, eMetadataUnitType_px);
	}
	else if (CSldCompare::StrCmp(key, U16("border-left-width")) == 0)
	{
		aMetadata->BorderLeftWidth = TSizeValue::FromString(val, eMetadataUnitType_px);
	}
	else if (CSldCompare::StrCmp(key, U16("border-right-width")) == 0)
	{
		aMetadata->BorderRightWidth = TSizeValue::FromString(val, eMetadataUnitType_px);
	}
	return error;
}

static ESldError TaskBlockEntry(ParseContext *ctx, const UInt16 *key, const UInt16 *val, TMetadataTaskBlockEntry *aMetadata)
{
	ESldError error = eOK;
	if (CSldCompare::StrCmp(key, U16("type")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->Type);
	}
	return error;
}

static ESldError Test(ParseContext *ctx, const UInt16 *key, const UInt16 *val, TMetadataTest *aMetadata)
{
	ESldError error = eOK;
	if (CSldCompare::StrCmp(key, U16("type")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->Type);
	}
	else if (CSldCompare::StrCmp(key, U16("order")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->Order);
	}
	else if (CSldCompare::StrCmp(key, U16("difficulty")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->Difficulty);
	}
	else if (CSldCompare::StrCmp(key, U16("max_points")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->MaxPoints);
	}
	else if (CSldCompare::StrCmp(key, U16("draft")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->HasDraft);
	}
	return error;
}

static ESldError TestContainer(ParseContext *ctx, const UInt16 *key, const UInt16 *val, TMetadataTestContainer *aMetadata)
{
	ESldError error = eOK;
	if (CSldCompare::StrCmp(key, U16("mode")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->Mode);
	}
	else if (CSldCompare::StrCmp(key, U16("difficulty")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->Difficulty);
	}
	else if (CSldCompare::StrCmp(key, U16("max_points")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->MaxPoints);
	}
	else if (CSldCompare::StrCmp(key, U16("show_in_demo")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->Demo);
	}
	return error;
}

static ESldError TestControl(ParseContext *ctx, const UInt16 *key, const UInt16 *val, TMetadataTestControl *aMetadata)
{
	ESldError error = eOK;
	if (!CSldCompare::StrCmp(key, U16("checklist")))
	{
		error = ctx->PushString(val, &aMetadata->Checklist);
	}
	else if (!CSldCompare::StrCmp(key, U16("unique")))
	{
		error = ParseUIntParam(val, 10, &aMetadata->Unique);
	}
	else if (!CSldCompare::StrCmp(key, U16("type")))
	{
		error = ParseUIntParam(val, 10, &aMetadata->Type);
	}
	return error;
}

static ESldError TestInput(ParseContext *ctx, const UInt16 *key, const UInt16 *val, TMetadataTestInput *aMetadata)
{
	ESldError error = eOK;
	if (CSldCompare::StrCmp(key, U16("type")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->Type);
	}
	else if (CSldCompare::StrCmp(key, U16("group")) == 0)
	{
		error = ctx->PushDecodedString(val, &aMetadata->Group);
	}
	else if (CSldCompare::StrCmp(key, U16("answer")) == 0)
	{
		error = ctx->PushDecodedString(val, &aMetadata->Answers);
	}
	else if (CSldCompare::StrCmp(key, U16("init")) == 0)
	{
		error = ctx->PushDecodedString(val, &aMetadata->Initial);
	}
	else if (CSldCompare::StrCmp(key, U16("cols")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->Cols);
	}
	else if (CSldCompare::StrCmp(key, U16("rows")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->Rows);
	}
	else if (CSldCompare::StrCmp(key, U16("level")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->Level);
	}
	else if (CSldCompare::StrCmp(key, U16("id")) == 0)
	{
		error = ctx->PushDecodedString(val, &aMetadata->Id);
	}
	return error;
}

static ESldError TestResultElement(ParseContext *ctx, const UInt16 *key, const UInt16 *val, TMetadataTestResultElement *aMetadata)
{
	ESldError error = eOK;
	if (CSldCompare::StrCmp(key, U16("colour")) == 0)
	{
		error = ParseUIntParam(val, 16, &aMetadata->Color);
	}
	return error;
}

static ESldError TestSpear(ParseContext *ctx, const UInt16 *key, const UInt16 *val, TMetadataTestSpear *aMetadata)
{
	ESldError error = eOK;
	if (!CSldCompare::StrCmp(key, U16("id")))
	{
		error = ctx->PushString(val, &aMetadata->Id);
	}
	return error;
}

static ESldError TestTarget(ParseContext *ctx, const UInt16 *key, const UInt16 *val, TMetadataTestTarget *aMetadata)
{
	ESldError error = eOK;
	if (!CSldCompare::StrCmp(key, U16("id")))
	{
		error = ctx->PushString(val, &aMetadata->Id);
	}
	else if (!CSldCompare::StrCmp(key, U16("link")))
	{
		error = ctx->PushString(val, &aMetadata->Links);
	}
	else if (!CSldCompare::StrCmp(key, U16("static")))
	{
		error = ParseUIntParam(val, 10, &aMetadata->Static);
	}
	return error;
}

static ESldError TestToken(ParseContext *ctx, const UInt16 *key, const UInt16 *val, TMetadataTestToken *aMetadata)
{
	ESldError error = eOK;
	if (CSldCompare::StrCmp(key, U16("order")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->Order);
	}
	else if (CSldCompare::StrCmp(key, U16("group")) == 0)
	{
		error = ctx->PushDecodedString(val, &aMetadata->Group);
	}
	else if (CSldCompare::StrCmp(key, U16("text")) == 0)
	{
		error = ctx->PushDecodedString(val, &aMetadata->Text);
	}
	return error;
}

static ESldError TextControl(ParseContext *ctx, const UInt16 *key, const UInt16 *val, TMetadataTextControl *aMetadata)
{
	ESldError error = eOK;
	if (CSldCompare::StrCmp(key, U16("scope")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->Scope);
	}
	return error;
}

static ESldError TimeLine(ParseContext *ctx, const UInt16 *key, const UInt16 *val, TMetadataTimeLine *aMetadata)
{
	ESldError error = eOK;
	if (CSldCompare::StrCmp(key, U16("width")) == 0)
	{
		aMetadata->Width = TSizeValue::FromString(val, eMetadataUnitType_px);
	}
	else  if (CSldCompare::StrCmp(key, U16("height")) == 0)
	{
		aMetadata->Height = TSizeValue::FromString(val, eMetadataUnitType_px);
	}
	else if (CSldCompare::StrCmp(key, U16("width_line")) == 0)
	{
		aMetadata->LineWidth = TSizeValue::FromString(val, eMetadataUnitType_px);
	}
	else  if (CSldCompare::StrCmp(key, U16("height_line")) == 0)
	{
		aMetadata->LineHeight = TSizeValue::FromString(val, eMetadataUnitType_px);
	}
	else  if (CSldCompare::StrCmp(key, U16("position_line")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->Position);
	}
	else  if (CSldCompare::StrCmp(key, U16("default_time")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->DefaultTime);
	}
	return error;
}

static ESldError TimeLineItem(ParseContext *ctx, const UInt16 *key, const UInt16 *val, TMetadataTimeLineItem *aMetadata)
{
	ESldError error = eOK;
	if (CSldCompare::StrCmp(key, U16("internal_list_index")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->ListIndex);
	}
	else if (CSldCompare::StrCmp(key, U16("internal_word_index")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->EntryIndex);
	}
	else if (CSldCompare::StrCmp(key, U16("id")) == 0)
	{
		error = ctx->PushDecodedString(val, &aMetadata->Id);
	}
	else if (CSldCompare::StrCmp(key, U16("start_time")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->StartTime);
	}
	else if (CSldCompare::StrCmp(key, U16("end_time")) == 0)
	{
		error = ParseUIntParam(val, 10, &aMetadata->EndTime);
	}
	else
	{
		error = ParseExtKey(ctx, key, val, &aMetadata->ExtKey, U16("external_dict_id"), U16("external_list_index"), U16("external_key"));
	}
	return error;
}

static ESldError UiElement(ParseContext *ctx, const UInt16 *key, const UInt16 *val, TMetadataUiElement *aMetadata)
{
	ESldError error = eOK;
	if (CSldCompare::StrCmp(key, U16("type")) == 0)
	{
		error = ctx->PushString(val, &aMetadata->Type);
	}
	return error;
}

static ESldError Url(ParseContext *ctx, const UInt16 *key, const UInt16 *val, TMetadataUrl *aMetadata)
{
	ESldError error = eOK;
	if (CSldCompare::StrCmp(key, U16("src")) == 0)
	{
		error = ctx->PushDecodedString(val, &aMetadata->Src);
	}
	return error;
}

static ESldError VideoSource(ParseContext *ctx, const UInt16 *key, const UInt16 *val, TMetadataVideoSource* aMetadata)
{
	ESldError error = eOK;
	if (CSldCompare::StrCmp(key, U16("video_idx")) == 0)
	{
		error = CSldCompare::StrToInt32(val, 16, &aMetadata->Id);
	}
	else if (CSldCompare::StrCmp(key, U16("video_codec")) == 0)
	{
		UInt16 codecType = eVideoType_Unknown;
		if ((error = ParseUIntParam(val, 10, &codecType)) == eOK)
			aMetadata->Codec = codecType;
	}
	else if (CSldCompare::StrCmp(key, U16("video_resolution")) == 0)
	{
		error = CSldCompare::StrToInt32(val, 10, &aMetadata->Resolution);
	}
	else
	{
		error = ParseExtKey(ctx, key, val, &aMetadata->ExtKey);
	}
	return error;
}

} // namespace handlers
} // anon namspace

/** ***
* Получает настройки стилей для всех вариантов написания текущего слова
******/
ESldError CSldMetadataParser::GetVariantStylePreferences(const UInt16* aStr, Int32** aVariantStyles, Int32 aNumberOfVariants)
{
	if (!aVariantStyles)
		return eMemoryNullPointer;

	ESldError error = eOK;

	// заполняем массив настройками по умолчанию
	for (UInt16 i=0;i<aNumberOfVariants;i++)
	{
		(*aVariantStyles)[i] = SLD_DEFAULT_STYLE_VARIANT_INDEX;
	}

	SldU16StringRef str = aStr;
	UInt16 key[MetaParamMaxNameSize] = { 0 };
	UInt16 val[SLD_META_PARAM_MAX_VAL_SIZE] = { 0 };
	while (GetNextParam(str, key, val))
	{
		Int32 variantIndex;
		error = CSldCompare::StrToInt32(key, 10, &variantIndex);
		if (error != eOK)
			return error;

		if (variantIndex >= aNumberOfVariants)
			return eMetadataErrorStylePreferencesParsing;

		Int32 styleIndex;
		error = CSldCompare::StrToInt32(val, 10, &styleIndex);
		if (error != eOK)
			return error;

		(*aVariantStyles)[variantIndex] = styleIndex;
	}

	return eOK;
}

/** *********************************************************************
* Получает параметры картинки
*
* @param[in]	aMetadata			- ссылка на параметры метаданных
* @param[out]	aImageItem			- сюда будет сохранена информация о картинке предпросмотра
* @param[out]	aFullImageItem		- сюда будет сохранена информация о полноразмерной картинке

* @return код ошибки
************************************************************************/
ESldError CSldMetadataParser::GetImageMetadata(const CSldMetadataProxy<eMetaImage> &aMetadata, CSldImageItem* aImageItem, CSldImageItem* aFullImageItem)
{
	if (!aImageItem || !aFullImageItem)
		return eMemoryNullPointer;

	if (!aMetadata || aMetadata.isClosing())
		return aMetadata.error();

	aImageItem->SetPictureIndex(aMetadata->PictureIndex);
	aImageItem->SetExtDict(aMetadata.c_str(aMetadata->ExtKey.DictId));
	aImageItem->SetExtListIdx(aMetadata->ExtKey.ListIdx);
	aImageItem->SetShowWidth(aMetadata->ShowWidth);
	aImageItem->SetShowHeight(aMetadata->ShowHeight);
	aImageItem->SetScalability(aMetadata->Scalable);
	aImageItem->SetSystemFlag(aMetadata->System);

	ESldError error = aImageItem->SetExtKey(aMetadata.c_str(aMetadata->ExtKey.Key));
	if (error != eOK)
		return error;

	aFullImageItem->SetPictureIndex(aMetadata->FullPictureIndex);
	aFullImageItem->SetExtDict(aMetadata.c_str(aMetadata->FullExtKey.DictId));
	aFullImageItem->SetExtListIdx(aMetadata->FullExtKey.ListIdx);
	aFullImageItem->SetShowWidth(aMetadata->FullShowWidth);
	aFullImageItem->SetShowHeight(aMetadata->FullShowHeight);
	error = aFullImageItem->SetExtKey(aMetadata.c_str(aMetadata->FullExtKey.Key));

	return error;
}

/** *********************************************************************
* Получает параметры картинки
*
* @param[in]	aStr				- входная строка параметров
* @param[out]	aMetadata			- сюда сохраняются параметры картинки
*
* @return код ошибки
************************************************************************/
ESldError CSldMetadataParser::GetMetadata(SldU16StringRef aStr, TMetadataImage *aMetadata)
{
	return parse(this, aStr, aMetadata, handlers::Image);
}

/** *********************************************************************
* Получает параметры зоны выделения
*
* @param[in]	aStr				- входная строка параметров
* @param[out]	aImageAreaItem		- сюда будет сохранена информация о зоне выделения

* @return код ошибки
************************************************************************/
ESldError CSldMetadataParser::GetMetadata(SldU16StringRef aStr, TMetadataImageArea *aMetadata)
{
	return parse(this, aStr, aMetadata, handlers::ImageArea);
}

/** *********************************************************************
* Получает параметры зоны выделения
*
* @param[in]	aMetadata			- ссылка на параметры метаданных
* @param[out]	aImageAreaItem		- сюда будет сохранена информация о зоне выделения

* @return код ошибки
************************************************************************/
ESldError CSldMetadataParser::GetImageAreaMetadata(const CSldMetadataProxy<eMetaImageArea> &aMetadata, CSldImageAreaItem* aImageAreaItem)
{
	if (!aImageAreaItem)
		return eMemoryNullPointer;

	if (!aMetadata || aMetadata.isClosing())
		return aMetadata.error();

	aImageAreaItem->SetLeft(aMetadata->Left);
	aImageAreaItem->SetTop(aMetadata->Top);
	aImageAreaItem->SetWidth(aMetadata->Width);
	aImageAreaItem->SetHeight(aMetadata->Height);
	aImageAreaItem->SetType((ESldImageAreaType)aMetadata->Type);
	aImageAreaItem->SetPercent(aMetadata->Percent);
	aImageAreaItem->SetMask(aMetadata->Mask);

	ESldError error = aImageAreaItem->SetId(aMetadata.c_str(aMetadata->Id));
	if (error == eOK) error = aImageAreaItem->SetActionString(aMetadata.c_str(aMetadata->ActionScript));
	if (error == eOK) error = aImageAreaItem->SetCoordsString(aMetadata.c_str(aMetadata->Coords));

	return error;
}

/** *********************************************************************
* Получает параметры всплывающей картинки
*
* @param[in]	aStr				- строка параметров
* @param[out]	aMetadata			- сюда сохраняются параметры всплывающей картинки
*
* @return код ошибки
************************************************************************/
ESldError CSldMetadataParser::GetMetadata(SldU16StringRef aStr, TMetadataPopupImage *aMetadata)
{
	return parse(this, aStr, aMetadata, handlers::PopupImage);
}

/** *********************************************************************
* Получает параметры ресурса видео
*
* @param[in]	aStr				- входная строка параметров
* @param[out]	aMetadata			- сюда записываются параметры ресурса видео
*
* @return код ошибки
************************************************************************/
ESldError CSldMetadataParser::GetMetadata(SldU16StringRef aStr,TMetadataVideoSource *aMetadata)
{
	return parse(this, aStr, aMetadata, handlers::VideoSource);
}

/** *********************************************************************
* Получает параметры ресурса видео
*
* @param[in]	aMetadata			- ссылка на параметры метаданных
* @param[out]	aVideoType			- сюда сохраняется информация о типе видео
*
* @return код ошибки
************************************************************************/
ESldError CSldMetadataParser::GetVideoSourceItemMetadata(const CSldMetadataProxy<eMetaVideoSource> &aMetadata, CSldVideoItem* aVideoItem)
{
	if (!aVideoItem)
		return eMemoryNullPointer;

	if (!aMetadata || aMetadata.isClosing())
		return aMetadata.error();

	// Очищаем информацию о ресурсе
	aVideoItem->Clear();

	aVideoItem->SetVideoId(aMetadata->Id);
	aVideoItem->SetVideoCodec((ESldVideoType)aMetadata->Codec);
	aVideoItem->SetVideoResolution(aMetadata->Resolution);
	ESldError error = aVideoItem->SetExtDict(aMetadata.c_str(aMetadata->ExtKey.DictId));
	aVideoItem->SetExtListIdx(aMetadata->ExtKey.ListIdx);
	if (error == eOK) aVideoItem->SetExtKey(aMetadata.c_str(aMetadata->ExtKey.Key));

	return error;
}

/** *********************************************************************
* Получает параметры озвучки
*
* @param[in]	aStr		- строка параметров
* @param[out]	aMetadata	- сюда сохраняются параметры озвучки
*
* @return код ошибки
************************************************************************/
ESldError CSldMetadataParser::GetMetadata(SldU16StringRef aStr, TMetadataSound *aMetadata)
{
	return parse(this, aStr, aMetadata, handlers::Sound);
}

/************************************************************************
* Получает параметры 3d сцены
*
* @param[in]	aStr					- строка параметров
* @param[out]	aMetadata				- сюда сохраняется параметры 3d сцены
*
* @return код ошибки
************************************************************************/
ESldError CSldMetadataParser::GetMetadata(SldU16StringRef aStr, TMetadataScene3D *aMetadata)
{
	return parse(this, aStr, aMetadata, handlers::Scene3D);
}

/************************************************************************
* Получает параметры слайдшоу
*
* @param[in]	aStr			- строка параметров
* @param[out]	aMetadata		- сюда сохраняются все необходимые данные по слайдшоу
*
* @return код ошибки
************************************************************************/
ESldError CSldMetadataParser::GetMetadata(SldU16StringRef aStr, TMetadataSlideShow *aMetadata)
{
	return parse(this, aStr, aMetadata, handlers::SlideShow);
}

/************************************************************************
* Получает параметры слайдшоу
*
* @param[in]	aMetadata			- ссылка на параметры метаданных
* @param[out]	aSlideShowItem	- сюда сохраняются все необходимые данные по слайдшоу
* @param[out]	aTimeStep		- сюда время отображения слайдов(ShowTime для автопоказа)
*
* @return код ошибки
************************************************************************/
ESldError CSldMetadataParser::GetSlideShowMetadata(const CSldMetadataProxy<eMetaSlideShow> &aMetadata, TSldSlideShowStruct* aSlideShowItem, UInt16* aTimeStep)
{
	if (!aSlideShowItem || !aTimeStep)
		return eMemoryNullPointer;

	aSlideShowItem->Clear();

	if (!aMetadata || aMetadata.isClosing())
		return aMetadata.error();

	aSlideShowItem->SlideShowFlow = aMetadata->Flow;
	aSlideShowItem->SlideShowMoveEffect = aMetadata->MoveEffect;
	aSlideShowItem->SlideShowPlace = aMetadata->Place;
	aSlideShowItem->SlideShowShowEffect = aMetadata->ShowEffect;
	aSlideShowItem->SlideShowSound = aMetadata->Sound;
	aSlideShowItem->SlideShowTime = aMetadata->Time;
	aSlideShowItem->SlideShowListIndex = aMetadata->ListIndex;
	aSlideShowItem->SlideShowWordIndex = aMetadata->WordIndex;
	aSlideShowItem->SlideShowWidth = aMetadata->Width;
	aSlideShowItem->SlideShowHeight = aMetadata->Height;
	CSldCompare::StrCopy(aTimeStep, aMetadata.c_str(aMetadata->TimeStep));

	return eOK;
}

/************************************************************************
* Получает параметры медиаконтейнера
*
* @param[in]	aStr			- строка параметров
* @param[out]	aMetadata		- объект медиа контейнера
*
* @return код ошибки
************************************************************************/
ESldError CSldMetadataParser::GetMetadata(SldU16StringRef aStr, TMetadataMediaContainer *aMetadata)
{
	return parse(this, aStr, aMetadata, handlers::MediaContainer);
}

/************************************************************************
* Получает параметры о элементе блока с результатами тестов "Шарик"
*
* @param[in]	aStr			- строка параметров
* @param[out]	aMetadata		- сюда сохраняются параметры о элементе блока с результатами тестов "Шарик"
*
* @return код ошибки
************************************************************************/
ESldError CSldMetadataParser::GetMetadata(SldU16StringRef aStr, TMetadataTestResultElement *aMetadata)
{
	return parse(this, aStr, aMetadata, handlers::TestResultElement);
}

/************************************************************************
* Получает параметры об области текста для тестов с выделением
*
* @param[in]	aStr			- строка параметров
* @param[out]	aMetadata		- сюда сохраняются параметры об области текста для тестов с выделением
*
* @return код ошибки
************************************************************************/
ESldError CSldMetadataParser::GetMetadata(SldU16StringRef aStr, TMetadataTextControl *aMetadata)
{
	return parse(this, aStr, aMetadata, handlers::TextControl);
}

/************************************************************************
* Получает параметры об элементе блока "Дано/Найти/Решение"
*
* @param[in]	aStr			- строка параметров
* @param[out]	aMetadata		- сюда сохраняются параметры об элементе блока "Дано/Найти/Решение"
*
* @return код ошибки
************************************************************************/
ESldError CSldMetadataParser::GetMetadata(SldU16StringRef aStr, TMetadataTaskBlockEntry *aMetadata)
{
	return parse(this, aStr, aMetadata, handlers::TaskBlockEntry);
}

/************************************************************************
* Получает параметры элементарного объекта
*
* @param[in]	aStr			- строка параметров
* @param[out]	aMetadata		- сюда сохраняются параметры элементарного объекта
*
* @return код ошибки
************************************************************************/
ESldError CSldMetadataParser::GetMetadata(SldU16StringRef aStr, TMetadataAtomicObject *aMetadata)
{
	return parse(this, aStr, aMetadata, handlers::AtomicObject);
}

/************************************************************************
* Получает параметры подписи картинки
*
* @param[in]	aStr			- строка параметров
* @param[out]	aMetadata		- сюда сохраняются параметры подписи картинки
*
* @return код ошибки
************************************************************************/
ESldError CSldMetadataParser::GetMetadata(SldU16StringRef aStr, TMetadataCaption *aMetadata)
{
	return parse(this, aStr, aMetadata, handlers::Caption);
}

/// Получает параметры элемента географической карты
ESldError CSldMetadataParser::GetMetadata(SldU16StringRef aStr, TMetadataMapElement *aMetadata)
{
	return parse(this, aStr, aMetadata, handlers::MapElement);
}

/// Получает параметры географической карты
ESldError CSldMetadataParser::GetMetadata(SldU16StringRef aStr, TMetadataMap *aMetadata)
{
	return parse(this, aStr, aMetadata, handlers::Map);
}

/** *********************************************************************
* Получает параметры таблицы
*
* @param[in]	aStr		- строка параметров
* @param[out]	aMetadata	- сюда сохраняются параметры таблицы
*
* @return код ошибки
************************************************************************/
ESldError CSldMetadataParser::GetMetadata(SldU16StringRef aStr,TMetadataTable *aMetadata)
{
	return parse(this, aStr, aMetadata, handlers::Table);
}

/** *********************************************************************
* Получает параметры столбца таблицы
*
* @param[in]	aStr				- строка параметров
* @param[out]	aMetadata			- сюда сохраняются параметры столбца таблицы
*
* @return код ошибки
************************************************************************/
ESldError CSldMetadataParser::GetMetadata(SldU16StringRef aStr,TMetadataTableCell *aMetadata)
{
	return parse(this, aStr, aMetadata, handlers::TableCell);
}

/** *********************************************************************
* Получает параметры параграфа
*
* @param[in]	aStr			- строка параметров
* @param[out]	aMetadata		- сюда сохраняются параметры параграфа
*
* @return код ошибки
************************************************************************/
ESldError CSldMetadataParser::GetMetadata(SldU16StringRef aStr, TMetadataParagraph *aMetadata)
{
	// resest the struct by hand as we need to add special handling for Align
	aMetadata->Align = ~0;

	ESldError error = parse(this, aStr, aMetadata, handlers::Paragraph);
	// Если выравнивание явно не задано, то для направления текста справа налево
	// по умолчанию должно быть установлено выравнивание по правой границе,
	// для направления текста слева направо (и для некорректно установленного направления) - по левой
	if (error == eOK && aMetadata->Align == (UInt16)(~0))
	{
		if (aMetadata->TextDirection == eTextDirection_RTL)
			aMetadata->Align = eTextAlign_Right;
		else
			aMetadata->Align = eTextAlign_Left;
	}
	return error;
}

/** *********************************************************************
* Получает параметры ссылки
*
* @param[in]	aStr		- строка параметров
* @param[out]	aMetadata	- сюда сохраняются параметры ссылки
*
* @return код ошибки
************************************************************************/
ESldError CSldMetadataParser::GetMetadata(SldU16StringRef aStr, TMetadataLink *aMetadata)
{
	return parse(this, aStr, aMetadata, handlers::Link);
}

/** *********************************************************************
* Получает параметры формулы
*
* @param[in]	aStr		      - строка параметров
* @param[out]	aMetadata         - параметры формулы
*
* @return код ошибки
************************************************************************/
ESldError CSldMetadataParser::GetMetadata(SldU16StringRef aStr, TMetadataFormula *aMetadata)
{
	return parse(this, aStr, aMetadata, handlers::Formula);
}

/** *********************************************************************
* Получает параметры контейнера абстрактных ресурсов
*
* @param[in]	aStr		      - строка параметров
* @param[out]	aMetadata	      - сюда сохранются параметры контейнера абстрактных ресурсов
*
* @return код ошибки
************************************************************************/
ESldError CSldMetadataParser::GetMetadata(SldU16StringRef aStr, TMetadataAbstractResource *aMetadata)
{
	return parse(this, aStr, aMetadata, handlers::AbstractResource);
}


/** *********************************************************************
* Получает параметры ссылки для флеш карточки
*
* @param[in]	aStr		- строка параметров
* @param[out]	aMetadata	- сюда сохраняются параметры ссылки для флеш карточки
*
* @return код ошибки
************************************************************************/
ESldError CSldMetadataParser::GetMetadata(SldU16StringRef aStr, TMetadataFlashCardsLink *aMetadata)
{
	return parse(this, aStr, aMetadata, handlers::FlashCardsLink);
}

/** *********************************************************************
* Получает параметры внешней ссылки
*
* @param[in]	aStr		- строка параметров
* @param[out]	aMetadata	- сюда копируются параметры внешней ссылки
*
* @return код ошибки
************************************************************************/
ESldError CSldMetadataParser::GetMetadata(SldU16StringRef aStr, TMetadataUrl *aMetadata)
{
	return parse(this, aStr, aMetadata, handlers::Url);
}

/** *********************************************************************
* Получает параметры метки
*
* @param[in]	aStr			- строка параметров
* @param[out]	TMetadataLabel	- сюда сохраняются параметры метки
*
* @return код ошибки
************************************************************************/
ESldError CSldMetadataParser::GetMetadata(SldU16StringRef aStr, TMetadataLabel *aMetadata)
{
	return parse(this, aStr, aMetadata, handlers::Label);
}

/** *********************************************************************
* Получает параметры блока Hide
*
* @param[in]	aStr		- строка параметров
* @param[out]	aMetadata	- сюда сохраняются параметры блока Hide
*
* @return код ошибки
************************************************************************/
ESldError CSldMetadataParser::GetMetadata(SldU16StringRef aStr, TMetadataHide *aMetadata)
{
	return parse(this, aStr, aMetadata, handlers::Hide);
}

/** *********************************************************************
* Получает параметры блока switch
*
* @param[in]	aStr			- строка параметров
* @param[out]	aMetadata		- сюда сохраняются параметры блока switch
*
* @return код ошибки
************************************************************************/
ESldError CSldMetadataParser::GetMetadata(SldU16StringRef aStr, TMetadataSwitch *aMetadata)
{
	return parse(this, aStr, aMetadata, handlers::Switch);
}

/** *********************************************************************
* Получает параметры блока switch-control
*
* @param[in]	aStr			- строка параметров
* @param[out]	aMetadata		- сюда сохраняются параметры блока switch-control
*
* @return код ошибки
************************************************************************/
ESldError CSldMetadataParser::GetMetadata( SldU16StringRef aStr, TMetadataSwitchControl *aMetadata)
{
	return parse(this, aStr, aMetadata, handlers::SwitchControl);
}

/** *********************************************************************
* Получает параметры блока managed-switch
*
* @param[in]	aStr		- строка параметров
* @param[out]	aMetadata		- сюда сохраняем параметры блока managed-switch
*
* @return код ошибки
************************************************************************/
ESldError CSldMetadataParser::GetMetadata(SldU16StringRef aStr, TMetadataManagedSwitch *aMetadata)
{
	return parse(this, aStr, aMetadata, handlers::ManagedSwitch);
}

/** *********************************************************************
* Получает параметры блока TestContainer
*
* @param[in]	aStr		- строка параметров
* @param[out]	aMetadata	- сюда сохраняются параметры блока TestContainer
*
* @return код ошибки
************************************************************************/
ESldError CSldMetadataParser::GetMetadata(SldU16StringRef aStr, TMetadataTestContainer *aMetadata)
{
	return parse(this, aStr, aMetadata, handlers::TestContainer);
}

/** *********************************************************************
* Получает параметры блока test
*
* @param[in]	aStr		- строка параметров
* @param[out]	aMetadata	- сюда сохраняются параметры блока test
*
* @return код ошибки
************************************************************************/
ESldError CSldMetadataParser::GetMetadata(SldU16StringRef aStr, TMetadataTest *aMetadata)
{
	return parse(this, aStr, aMetadata, handlers::Test);
}

/** *********************************************************************
* Получает параметры блока test_input
*
* XXX: Распарсивание строки на вопросов на отдельные вопросы - на стороне оболочки (???)
*
* @param[in]	aStr			- строка параметров
* @param[out]	aMetadata		- сюда сохраняются параметры блока test_input
*
* @return код ошибки
************************************************************************/
ESldError CSldMetadataParser::GetMetadata(SldU16StringRef aStr, TMetadataTestInput *aMetadata)
{
	return parse(this, aStr, aMetadata, handlers::TestInput);
}

/** *********************************************************************
* Получает параметры блока test_token
*
* @param[in]	aStr		- строка параметров
* @param[out]	aMetadata	- сюда сохраняются параметры блока test_token
*
* @return код ошибки
************************************************************************/
ESldError CSldMetadataParser::GetMetadata(SldU16StringRef aStr, TMetadataTestToken *aMetadata)
{
	return parse(this, aStr, aMetadata, handlers::TestToken);
}

/** *********************************************************************
* Получает параметры ответного блока теста на сопоставление
*
* @param[in]	aStr		- строка параметров
* @param[out]	aMetadata	- сюда сохраняется параметры ответного блока теста на сопоставление
*
* @return код ошибки
************************************************************************/
ESldError CSldMetadataParser::GetMetadata(SldU16StringRef aStr, TMetadataTestSpear *aMetadata)
{
	return parse(this, aStr, aMetadata, handlers::TestSpear);
}

/** *********************************************************************
* Получает параметры блока ожидающего ответ элемента теста на сопоставление
*
* @param[in]	aStr		- строка параметров
* @param[out]	aMetadata	- сюда сохраняются параметры блока ожидающего ответ элемента теста на сопоставление
*
* @return код ошибки
************************************************************************/
ESldError CSldMetadataParser::GetMetadata(SldU16StringRef aStr, TMetadataTestTarget *aMetadata)
{
	return parse(this, aStr, aMetadata, handlers::TestTarget);
}

/** *********************************************************************
* Получает параметры блока ожидающего ответ элемента теста на сопоставление
*
* @param[in]	aStr		- строка параметров
* @param[out]	aMetadata	- сюда сохраняются параметры блока ожидающего ответ элемента теста на сопоставление
*
* @return код ошибки
************************************************************************/
ESldError CSldMetadataParser::GetMetadata(SldU16StringRef aStr, TMetadataTestControl *aMetadata)
{
	return parse(this, aStr, aMetadata, handlers::TestControl);
}

/** *********************************************************************
* Получает параметры блока UI Element
*
* @param[in]	aStr		- строка параметров
* @param[out]	aMetadata	- сюда записываются параметры блока UI Element
*
* @return код ошибки
************************************************************************/
ESldError CSldMetadataParser::GetMetadata(SldU16StringRef aStr, TMetadataUiElement *aMetadata)
{
	return parse(this, aStr, aMetadata, handlers::UiElement);
}

/***********************************************************************
* Получает параметры блока info_block
*
* @param[in]	aStr		- строка параметров
* @param[out]	aMetadata	- сюда сохраняются параметры блока info_block
*
* @return код ошибки
************************************************************************/
ESldError CSldMetadataParser::GetMetadata(SldU16StringRef aStr, TMetadataInfoBlock *aMetadata)
{
	return parse(this, aStr, aMetadata, handlers::InfoBlock);
}

/***********************************************************************
* Получает параметры всплывающей статьи
*
* @param[in]	aStr		- строка параметров
* @param[out]	aMetadata	- сюда сохраняются параметры всплывающей статьи
*
* @return код ошибки
************************************************************************/
ESldError CSldMetadataParser::GetMetadata(SldU16StringRef aStr, TMetadataPopupArticle *aMetadata)
{
	return parse(this, aStr, aMetadata, handlers::PopupArticle);
}

/** *********************************************************************
* Получает параметры фоновой картинки
*
* @param[in]	aStr				- входная строка параметров
* @param[out]	aMetadata			- сюда сохраняются параметры фоновой картинки
*
* @return код ошибки
************************************************************************/
ESldError CSldMetadataParser::GetMetadata(SldU16StringRef aStr, TMetadataBackgroundImage *aMetadata)
{
	return parse(this, aStr, aMetadata, handlers::BackgroundImage);
}

static inline bool isSvgSpace(UInt16 chr)
{
	return chr == ' ' || chr == '\t' || chr == '\r' || chr == '\n' || chr == '\f';
}

/** *********************************************************************
* Получает параметры svg картинки
*
* @param[in]	aData		- входная строка параметров
* @param[in]	aFormatType	- указатель, по которому будет сохранен формат картинки
* @param[out]	aWidth		- сюда сохраняется ширина картинки
* @param[out]	aHeight		- сюда сохраняется высота картинки
*
* @return код ошибки
************************************************************************/
ESldError CSldMetadataParser::GetSvgInfo(TImageElement* aImageElement, UInt32* aFormatType, TSizeValue *const aWidth, TSizeValue *const aHeight)
{
	if (!aImageElement || !aFormatType)
		return eMemoryNullPointer;

	sld2::DynArray<UInt8> terminatedData(sld2::default_init, aImageElement->Size + 1);
	if (terminatedData.empty())
		return eMemoryNotEnoughMemory;
	sldMemMove(terminatedData.data(), aImageElement->Data, aImageElement->Size);
	terminatedData[aImageElement->Size] = 0;

	// полная svg картинка может содержать не только тег <svg> - там может быть и xml сигнатура, и комментарии - удалим их
	const UInt8* startPosition = CSldCompare::StrStrA(terminatedData.data(), (const UInt8 *const)("<svg"));
	if (!startPosition)
		return eOK;

	startPosition += 4; // strlen("<svg") - skip past "<svg"
	while (isSvgSpace(*startPosition))
		startPosition++; // skip all of the whitespace at the beginning

	const UInt8* endPosition = sld2::StrChr(startPosition, (UInt8)'>');
	if (!endPosition)
		return eMetadataErrorImageParsing;

	// нашлась и открывающая, и закрывающая скобка - это тег svg картинки
	*aFormatType = ePictureFormatType_SVG;

	// all our parsing "helpers" are implemented using utf16, convert the string to it
	sld2::DynArray<UInt16> svgTag(sld2::default_init, endPosition - startPosition);
	if (svgTag.empty())
		return eMemoryNotEnoughMemory;

	const UInt32 len = sld2::utf::convert(startPosition, svgTag.size(), svgTag.data());
	if (len == 0)
		return eMetadataErrorImageParsing;

	SldU16StringRef str(svgTag.data(), len);
	UInt16 key[MetaParamMaxNameSize] = { 0 };
	UInt16 val[SLD_META_PARAM_MAX_VAL_SIZE] = { 0 };
	while (GetNextParam(str, key, val, isSvgSpace))
	{
		if (CSldCompare::StrCmp(key, U16("width")) == 0 && aWidth)
		{
			*aWidth = TSizeValue::FromString(val, eMetadataUnitType_px);
		}
		else if (CSldCompare::StrCmp(key, U16("height")) == 0 && aHeight)
		{
			*aHeight = TSizeValue::FromString(val, eMetadataUnitType_px);
		}
	}

	return eOK;
}

/** *********************************************************************
* Получает параметры элемена легенды
*
* @param[in]	aStr				- входная строка параметров
* @param[out]	aMetadata			- сюда сохраняются параметры элемена легенды
*
* @return код ошибки
************************************************************************/
ESldError CSldMetadataParser::GetMetadata(SldU16StringRef aStr, TMetadataLegendItem *aMetadata)
{
	return parse(this, aStr, aMetadata, handlers::LegendItem);
}

/** *********************************************************************
* Получает параметры элемента кроссворда
*
* @param[in]	aStr				- входная строка параметров
* @param[out]	aMetadata			- сюда сохраняются параметры элемента кроссворда
*
* @return код ошибки
************************************************************************/
ESldError CSldMetadataParser::GetMetadata(SldU16StringRef aStr, TMetadataCrosswordItem *aMetadata)
{
	return parse(this, aStr, aMetadata, handlers::CrosswordItem);
}

/** *********************************************************************
* Получает параметры внешней вставки
*
* @param[in]	aStr			- строка параметров
* @param[out]	aMetadata		- сюда сохраняются параметры внешней вставки
*
* @return код ошибки
************************************************************************/
ESldError CSldMetadataParser::GetMetadata(SldU16StringRef aStr, TMetadataExternArticle *aMetadata)
{
	return parse(this, aStr, aMetadata, handlers::ExternArticle);
}

/** *********************************************************************
* Получает параметры списка
*
* @param[in]	aStr			- строка параметров
* @param[out]	aType			- сюда сохраняем параметры списка
*
* @return код ошибки
************************************************************************/
ESldError CSldMetadataParser::GetMetadata(SldU16StringRef aStr, TMetadataList *aMetadata)
{
	return parse(this, aStr, aMetadata, handlers::List);
}

/** *********************************************************************
* Получает параметры списка
*
* @param[in]	aStr			- строка параметров
* @param[out]	aMetadata		- сюда сохраняем парметры "интерактивного объекта"
*
* @return код ошибки
************************************************************************/
ESldError CSldMetadataParser::GetMetadata(SldU16StringRef aStr, TMetadataInteractiveObject *aMetadata)
{
	return parse(this, aStr, aMetadata, handlers::InteractiveObject);
}

/** *********************************************************************
* Получает параметры ленты времени
*
* @param[in]	aStr			- строка параметров
* @param[out]	aMetadata       - сюда сохраняем параметры ленты времени
*
* @return код ошибки
************************************************************************/
ESldError CSldMetadataParser::GetMetadata(SldU16StringRef aStr, TMetadataTimeLine *aMetadata)
{
	return parse(this, aStr, aMetadata, handlers::TimeLine);
}

/** *********************************************************************
* Получает параметры элемента ленты времени
*
* @param[in]	aStr			- строка параметров
* @param[out]	aMetadata		- сюда сохраняем параметры элемента ленты времени
*
* @return код ошибки
************************************************************************/
ESldError CSldMetadataParser::GetMetadata(SldU16StringRef aStr, TMetadataTimeLineItem *aMetadata)
{
	return parse(this, aStr, aMetadata, handlers::TimeLineItem);
}

/**
 * Выдает строку со стилем (блока css параметров)
 *
 * @param [in]  aIndex     - id стиля css
 * @param [in]  aString    - строка куда записывается строка со стилем
 * @param [in]  aResolver  - указатель на объект "резолвящий" ссылки на внутренние ресурсы в url'ы
 *                           может быть NULL если парсер был создан с резолвером
 *
 * @return код ошибки
 * ВАЖНО! необходимо проверять длину полученой строки
 *        функция может вернуть eOK и при этом выдать пустую строку, это "нормально"
 */
ESldError CSldMetadataParser::GetCSSStyleString(UInt32 aIndex, SldU16String &aString, CSldCSSUrlResolver *aResolver) const
{
	if (!m_dictionary)
		return eMetadataErrorNoCSSData;

	if (!aResolver)
		aResolver = m_resolver;

	if (!aResolver)
		return eMemoryNullPointer;

	if (aIndex == InvalidDataIndex)
	{
		aString.clear();
		return eOK;
	}

	CSldCSSDataManager *manager = m_dictionary->GetCSSDataManager();
	if (!manager)
		return eMetadataErrorNoCSSData;

	// mask off the dictionary id
	aIndex &= sld2::bit_mask32(MergedCSSBlockIndexBits);
	return manager->GetCSSStyleString(aIndex, &aString, aResolver);
}
