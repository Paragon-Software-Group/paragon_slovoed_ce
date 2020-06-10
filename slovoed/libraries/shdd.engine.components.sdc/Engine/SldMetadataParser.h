#ifndef _C_SLD_METADATA_PARSER_
#define _C_SLD_METADATA_PARSER_

#include "SldError.h"
#include "SldTypes.h"
#include "SldImageItem.h"
#include "SldImageAreaItem.h"
#include "SldVideoItem.h"
#include "SldImageManager.h"
#include "SldMetadataManager.h"
#include "SldMetadataTypes.h"
#include "SldString.h"

// forward declarataions
class CSldCSSUrlResolver;
class CSldDictionary;
class CSldMetadataParser;

/*
	Строка метаданных состоит из подряд идущих записей следующего вида:

	param_name="param_value"param_delimiter

	param_name		- имя параметра
	param_value		- значение параметра
	param_delimiter	- строка, разделяющая параметры

	либо равна "close"
*/

/// Максимальная длина строки с именем параметра
static const UInt32 MetaParamMaxNameSize = 255;

namespace sld2 {
namespace metadata {
namespace detail {

// базовая структура для метаданных без структур
class ProxyEmptyBase : protected TMetadataEmptyStruct
{
protected:
	TMetadataEmptyStruct &_data() { return *this; }
};

// базовый класс для метаданных со структурами
template <typename T>
class ProxyStructBase
{
public:
	/// Предоставляет доступ к структуре метаданных
	const T* operator->() const { return &m_metaData; }

protected:
	T &_data() { return m_metaData; }

private:
	// metadata struct
	T m_metaData;
};

template <ESldStyleMetaTypeEnum Enum>
using struct_type = typename SldMetadataTraits<Enum>::struct_type;

template <ESldStyleMetaTypeEnum Enum>
using ProxyBase = sld2::conditional<sld2::is_same<TMetadataEmptyStruct, struct_type<Enum>>::value,
									ProxyEmptyBase, ProxyStructBase<struct_type<Enum>>>;

} // namespace detail

// функции переписывания индексов в мерженых словарях

template <typename T> ESldError fixupIndices(const TMergedMetaInfo*, UInt32, T&) { return eOK; }

inline ESldError fixupIndices(const TMergedMetaInfo *aInfo, UInt32 aDictIdx, TMetadataImage &aMetadata)
{
	aMetadata.PictureIndex = aInfo->ToMergedPictureIndex(aDictIdx, aMetadata.PictureIndex);
	aMetadata.FullPictureIndex = aInfo->ToMergedPictureIndex(aDictIdx, aMetadata.FullPictureIndex);
	return eOK;
}

inline ESldError fixupIndices(const TMergedMetaInfo *aInfo, UInt32 aDictIdx, TMetadataBackgroundImage &aMetadata)
{
	aMetadata.PictureIndex = aInfo->ToMergedPictureIndex(aDictIdx, aMetadata.PictureIndex);
	return eOK;
}

inline ESldError fixupIndices(const TMergedMetaInfo *aInfo, UInt32 aDictIdx, TMetadataPopupImage &aMetadata)
{
	aMetadata.PictureIndex = aInfo->ToMergedPictureIndex(aDictIdx, aMetadata.PictureIndex);
	return eOK;
}

inline ESldError fixupIndices(const TMergedMetaInfo *aInfo, UInt32 aDictIdx, TMetadataSound &aMetadata)
{
	aMetadata.Index = aInfo->ToMergedSoundIndex(aDictIdx, aMetadata.Index);
	return eOK;
}

inline ESldError fixupIndices(const TMergedMetaInfo *aInfo, UInt32 aDictIdx, TMetadataLink &aMetadata)
{
	return aInfo->ToMergedLinkIndexes(aDictIdx, aMetadata.ListIndex, aMetadata.EntryIndex);
}

inline ESldError fixupIndices(const TMergedMetaInfo *aInfo, UInt32 aDictIdx, TMetadataMediaContainer &aMetadata)
{
	aMetadata.Background = aInfo->ToMergedSoundIndex(aDictIdx, aMetadata.Background);
	return eOK;
}

inline ESldError fixupIndices(const TMergedMetaInfo *aInfo, UInt32 aDictIdx, TMetadataFlashCardsLink &aMetadata)
{
	ESldError error = aInfo->ToMergedLinkIndexes(aDictIdx, aMetadata.FrontListId, aMetadata.FrontListEntryId);
	return error == eOK ? aInfo->ToMergedLinkIndexes(aDictIdx, aMetadata.BackListId, aMetadata.BackListEntryId) : error;
}

inline ESldError fixupIndices(const TMergedMetaInfo *aInfo, UInt32 aDictIdx, TMetadataPopupArticle &aMetadata)
{
	return aInfo->ToMergedLinkIndexes(aDictIdx, aMetadata.ListIndex, aMetadata.EntryIndex);
}

inline ESldError fixupIndices(const TMergedMetaInfo *aInfo, UInt32 aDictIdx, TMetadataList &aMetadata)
{
	aMetadata.Style = aInfo->ToMergedStyleIndex(aDictIdx, aMetadata.Style);
	return eOK;
}

// функции исправления данных

template <typename T> void fixup(T&) {}

inline void fixup(TMetadataSwitch &aMetadata) {
	// there was a pretty long time when switch thematics were initialzed to ~0
	// fix em up to the current default of eSwitchThematicDefault
	if (aMetadata.Thematic == static_cast<UInt16>(~0))
		aMetadata.Thematic = eSwitchThematicDefault;
}

} // namespace metadata
} // namespace sld2

class CSldMetadataProxyBase
{
public:
	/// Возвращает строку по ссылке на нее
	SldU16StringRef string_ref(const TMetadataString &aStr) const;

	/// Возвращает строку со стилем css
	ESldError cssStyle(SldU16String &aString) const;

	/// Возвращает id стиля css
	UInt32 cssStyleId() const { return m_cssBlockIndex; }

protected:
	CSldMetadataProxyBase(const CSldMetadataParser &aParser, UInt32 aCSSBlockIndex)
		: m_parser(aParser), m_cssBlockIndex(aCSSBlockIndex) {}

	CSldMetadataProxyBase& operator=(const CSldMetadataProxyBase&) = delete;

private:
	// reference to the metadata parser
	const CSldMetadataParser &m_parser;
	// css block index
	UInt32 m_cssBlockIndex;
};

/**
 * Класс - обертка для парсинга метаданных
 *
 * Является возвращаемым значением вызова функции (ий)
 *	CSldMetadataParser::GetMetadata(const UInt16* aStr)
 *
 * Результат распарсивания метаданных можно проверить через `operator bool()` - приведение
 * объекта к булевому значению.
 * Конкретный код ошибки (если интересует) можно получить по
 *  ::error()
 *
 * Флаг является ли тэг закрывающим проверяется по
 *  ::isClosing()
 *
 * Доступ к структуре метаданных предоставляется по `operator->()`. Т.е. класс можно считать
 * аналогом "указателя" на метаданные.
 *
 * Доступ к строкам метаданных предоставляется через 2 функции:
 *  ::c_str(const TMetadataString &aStr)
 *  ::string(const TMetadataString &aStr)
 * ВАЖНО:
 *  из-за особенностей реализации доступа к строкам в ядре, строка возвращенная из ::c_str()
 *  актуальна *только* до следующего вызова одной из этих функций
 *
 * Доступ к css стилю блока (в строковом представлении) предоставляется через 2 функции:
 *  ::cssStyle(SldU16String *aString)
 *  ::cssStyle()
 * ВАЖНО:
 *  соответствия между "пустотой" выданой строки и возможными ошибками как такового нету.
 *  т.е. при ошибке функции всегда возвращают пустую строку, но пустая возвращеная строка
 *  не является признаком ошибки
 * Также возможно получить id css стиля по
 *  ::cssStyleId()
 * возвращенный id можно использовать в качестве "ключа" *уникальных* css стилей
 * [т.е. у блоков с одинаковым cssStyleId гарантированно одинаковый css стиль и их [стили]
 *  можно спокойно кэшировать]
 */
template <ESldStyleMetaTypeEnum Enum>
class CSldMetadataProxy : public sld2::metadata::detail::ProxyBase<Enum>, public CSldMetadataProxyBase
{
	friend class CSldMetadataParser;

	CSldMetadataProxy(const CSldMetadataParser &aParser, bool aIsClosing, UInt32 aCSSBlockIndex)
		: CSldMetadataProxyBase(aParser, aCSSBlockIndex), m_error(eOK), m_closing(aIsClosing) {}

	CSldMetadataProxy& operator=(const CSldMetadataProxy&) = delete;

public:
	using struct_type = sld2::metadata::detail::struct_type<Enum>;

	/// Возвращает строку по ссылке на нее
	/// ВАЖНО: строка актуальна *только* до следующего вызова данной функции (или ::string())
	/// ВАЖНО: может вернуть NULL (но в общем случае только при ошибке при сборке базы)
	const UInt16* c_str(const TMetadataString &aStr) const { return string_ref(aStr).data(); }

	/// Возвращает строку по ссылке на нее
	SldU16String string(const TMetadataString &aStr) const { return to_string(string_ref(aStr)); }

	/// Оператор приведения к bool - дает возможность проверять результат парсинга
	operator bool() const { return m_error == eOK; }

	/// Возвращает ошибку-результат парсинга строки параметров метаданных
	ESldError error() const { return m_error; }

	/// Возвращает флаг является ли данный тэг метаданных закрывающим
	bool isClosing() const { return m_closing != 0; }

	using CSldMetadataProxyBase::cssStyle;
	/// Возвращает строку со стилем css
	ESldError cssStyle(SldU16String *aString) const { return aString ? cssStyle(*aString) : eMemoryNullPointer; }
	/// Возвращает строку со стилем css
	SldU16String cssStyle() const;

private:
	// error - result of metadata parsing
	ESldError m_error;
	// closing flag
	UInt32 m_closing;
};

/// Класс, занимающийся разбором параметров блоков метаданных и метаинформации
class CSldMetadataParser
{
public:
	enum : UInt32 { InvalidDataIndex = ~0U };

	/// Конструктор
	CSldMetadataParser(CSldDictionary *aDict, CSldCSSUrlResolver *aResolver)
		: CSldMetadataParser(aDict, aResolver, nullptr, 0) {}

	template <UInt16 Size>
	CSldMetadataParser(CSldDictionary *aDict, CSldCSSUrlResolver *aResolver, UInt16(&aBuffer)[Size])
		: CSldMetadataParser(aDict, aResolver, aBuffer, Size) {}

	CSldMetadataParser() : CSldMetadataParser(nullptr, nullptr, nullptr, 0) {}

	template <UInt16 Size>
	CSldMetadataParser(UInt16(&aBuffer)[Size]) : CSldMetadataParser(nullptr, nullptr, aBuffer, Size) {}

	/// Деструктор
	~CSldMetadataParser();

	/// Возвращает указатель на строку по ссылке на нее
	const UInt16* GetString(const TMetadataString *aString) const { return aString ? GetStringRef(*aString).data() : nullptr; }
	const UInt16* GetString(const TMetadataString &aString) const { return GetStringRef(aString).data(); }
	SldU16StringRef GetStringRef(const TMetadataString &aString) const;

public:

	/// Получает настройки стилей для всех вариантов написания текущего слова
	static ESldError GetVariantStylePreferences(const UInt16* aStr, Int32** aVariantStyles, Int32 aNumberOfVariants);

private:
	// @private - данные функциии используются *только* для "старых" строчных метаданных

	/// Получает параметры контейнера абстрактных ресурса
	ESldError GetMetadata(SldU16StringRef aStr, TMetadataAbstractResource *aMetadata);

	/// Получает параметры элементарного объекте
	ESldError GetMetadata(SldU16StringRef aStr, TMetadataAtomicObject *aMetadata);

	/// Получает параметры фоновой картинки
	ESldError GetMetadata(SldU16StringRef aStr, TMetadataBackgroundImage *aMetadata);

	/// Получает параметры подписи картинки
	ESldError GetMetadata(SldU16StringRef aStr, TMetadataCaption *aMetadata);

	/// Получает параметры элемента кроссворда
	ESldError GetMetadata(SldU16StringRef aStr, TMetadataCrosswordItem *aMetadata);

	/// Получает параметры внешней вставки
	ESldError GetMetadata(SldU16StringRef aStr, TMetadataExternArticle *aMetadata);

	/// Получает параметры блока flash_cards_link
	ESldError GetMetadata(SldU16StringRef aStr, TMetadataFlashCardsLink *aMetadata);

	/// Получает параметры формулы
	ESldError GetMetadata(SldU16StringRef aStr, TMetadataFormula *aMetadata);

	/// Получает параметры блока Hide
	ESldError GetMetadata(SldU16StringRef aStr, TMetadataHide *aMetadata);

	/// Получает параметры картинки
	ESldError GetMetadata(SldU16StringRef aStr, TMetadataImage *aMetadata);

	/// Получает параметры выделенной области на изображении
	ESldError GetMetadata(SldU16StringRef aStr, TMetadataImageArea *aMetadata);

	/// Получает параметры блока info_block
	ESldError GetMetadata(SldU16StringRef aStr, TMetadataInfoBlock *aMetadata);

	/// Получает параметры интерактивного объекта
	ESldError GetMetadata(SldU16StringRef aStr, TMetadataInteractiveObject *aMetadata);

	/// Получает параметры метки
	ESldError GetMetadata(SldU16StringRef aStr, TMetadataLabel *aMetadata);

	/// Получает параметры элемента легенды
	ESldError GetMetadata(SldU16StringRef aStr, TMetadataLegendItem *aMetadata);

	/// Получает параметры ссылки
	ESldError GetMetadata(SldU16StringRef aStr, TMetadataLink *aMetadata);

	/// Получаем параметры списка
	ESldError GetMetadata(SldU16StringRef aStr, TMetadataList *aMetadata);

	/// Получает параметры блока managed-switch
	ESldError GetMetadata(SldU16StringRef aStr, TMetadataManagedSwitch *aMetadata);

	/// Получает параметры географической карты
	ESldError GetMetadata(SldU16StringRef aStr, TMetadataMap *aMetadata);

	/// Получает параметры элемента географической карты
	ESldError GetMetadata(SldU16StringRef aStr, TMetadataMapElement *aMetadata);

	/// Получает параметры медиаконтейнера
	ESldError GetMetadata(SldU16StringRef aStr, TMetadataMediaContainer *aMetadata);

	/// Получает параметры параграфа
	ESldError GetMetadata(SldU16StringRef aStr, TMetadataParagraph *aMetadata);

	/// Получает параметры всплывающей статьи
	ESldError GetMetadata(SldU16StringRef aStr, TMetadataPopupArticle *aMetadata);

	/// Получает параметры всплывающей картинки
	ESldError GetMetadata(SldU16StringRef aStr, TMetadataPopupImage *aMetadata);

	/// Получает параметры 3d сцены
	ESldError GetMetadata(SldU16StringRef aStr, TMetadataScene3D *aMetadata);

	/// Получает параметры слайдшоу
	ESldError GetMetadata(SldU16StringRef aStr, TMetadataSlideShow *aMetaData);

	/// Получает параметры озвучки
	ESldError GetMetadata(SldU16StringRef aStr, TMetadataSound *aMetadata);

	/// Получает параметры блока switch
	ESldError GetMetadata(SldU16StringRef aStr, TMetadataSwitch *aMetadata);

	/// Получает параметры блока switch-control
	ESldError GetMetadata(SldU16StringRef aStr, TMetadataSwitchControl *aMetadata);

	/// Получает параметры таблицы
	ESldError GetMetadata(SldU16StringRef aStr, TMetadataTable *aMetadata);

	/// Получает параметры столбца таблицы
	ESldError GetMetadata(SldU16StringRef aStr, TMetadataTableCell *aMetadata);

	/// Получает параметры об элементе блока "Дано/Найти/Решение"
	ESldError GetMetadata(SldU16StringRef aStr, TMetadataTaskBlockEntry *aMetadata);

	/// Получает параметры блока test
	ESldError GetMetadata(SldU16StringRef aStr, TMetadataTest *aMetadata);

	/// Получает параметры блока test
	ESldError GetMetadata(SldU16StringRef aStr, TMetadataTestContainer *aMetadata);

	/// Получает параметры блока с ответами для проверки теста на сопоставление
	ESldError GetMetadata(SldU16StringRef aStr, TMetadataTestControl *aMetadata);

	/// Получает параметры блока test_input
	ESldError GetMetadata(SldU16StringRef aStr, TMetadataTestInput *aMetadata);

	/// Получает параметры о элементе блока с результатами тестов "Шарик"
	ESldError GetMetadata(SldU16StringRef aStr, TMetadataTestResultElement *aMetadata);

	/// Получает параметры блока прицельного элемента теста на сопоставление
	ESldError GetMetadata(SldU16StringRef aStr, TMetadataTestSpear *aMetadata);

	/// Получает параметры блока целевого элемента теста на сопоставление
	ESldError GetMetadata(SldU16StringRef aStr, TMetadataTestTarget *aMetadata);

	/// Получает параметры блока test_token
	ESldError GetMetadata(SldU16StringRef aStr, TMetadataTestToken *aMetadata);

	/// Получает параметры об области текста для тестов с выделением
	ESldError GetMetadata(SldU16StringRef aStr, TMetadataTextControl *aMetadata);

	/// Получает параметры ленты времени
	ESldError GetMetadata(SldU16StringRef aStr, TMetadataTimeLine *aMetadata);

	/// Получает параметры элемента ленты времени
	ESldError GetMetadata(SldU16StringRef aStr, TMetadataTimeLineItem *aMetadata);

	/// Получает параметры блока ui element
	ESldError GetMetadata(SldU16StringRef aStr, TMetadataUiElement *aMetadata);

	/// Получает параметры внешней ссылки
	ESldError GetMetadata(SldU16StringRef aStr, TMetadataUrl *aMetadata);

	/// Получает параметры ресурса видео
	ESldError GetMetadata(SldU16StringRef aStr, TMetadataVideoSource *aMetadata);

	// generic getter for structured-only metadata
	template <typename T> ESldError GetMetadata(SldU16StringRef, T*) { return eMetadataErrorParsingTextMetadata; }

public:
	/// Получает параметры картинки (свойства выставляются по членам соответствующего класса)
	static ESldError GetImageMetadata(const CSldMetadataProxy<eMetaImage> &aMetadata, CSldImageItem* aImageItem, CSldImageItem* aFullImageItem);

	/// Получает параметры выделенной области на изображении
	static ESldError GetImageAreaMetadata(const CSldMetadataProxy<eMetaImageArea> &aMetadata, CSldImageAreaItem* aImageAreaItem);

	/// Получает параметры ресурса видео
	static ESldError GetVideoSourceItemMetadata(const CSldMetadataProxy<eMetaVideoSource> &aMetadata, CSldVideoItem* aVideoType);

	/// Получает параметры слайдшоу
	static ESldError GetSlideShowMetadata(const CSldMetadataProxy<eMetaSlideShow> &aMetadata, TSldSlideShowStruct* aSlideShowItem, UInt16* aTimeStep);

	/// Получает параметры svg картинки
	static ESldError GetSvgInfo(TImageElement* aImageElement, UInt32* aFormatType, TSizeValue* const aWidth, TSizeValue* const aHeight);

	/// Получает строку с параметрами css
	ESldError GetCSSStyleString(UInt32 aIndex, SldU16String &aString, CSldCSSUrlResolver *aResolver = NULL) const;

	/**
	 * Шаблонный метод получения структурированных метаданных
	 *
	 *  Предполагаемый шаблон использования:
	 *   @code
	 *   // парсим строку параметров метаданных в структуру
	 *   const auto label = parser.GetMetadata<eMetaLabel>(aText);
	 *   // проверка результата парсинга
	 *   if (!label)
	 *       // доступ к конкретному коду ошибки
	 *       return label.error();
	 *
	 *   // проверка является ли тэг закрывающим
	 *   if (label.isClosing())
	 *      ; // обработка закрывающего тэга метаданных
	 *   else
	 *   {
	 *       // обработка открывающего тэга метаданных
	 *
	 *       // доступ к "обычным" полям структуры предоставляется через `operator->()`
	 *       UInt32 type = label->Type;
	 *
	 *       // доступ к строковым полям структуры предоставляется через 2 функции
	 *       SldU16String sldu16data = label.string(label->Data); // в виде SldU16String
	 *       const UInt16 *data = label.c_str(label->Data); // в виде C-"строки"
	 *
	 *       // доступ к css стилю метаданных предоставляется через 2 функции
	 *       SldU16String cssStyle = label.cssStyle(); // без доступа к ошибкам
	 *       ESldError error = label.cssStyle(&cssStyle); // с возвращением ошибки
	 *
	 *       // также можно получить доступ к id css стиля по
	 *       UInt32 cssStyleId = label.cssStyleId();
	 *       // полученный id (если он != ::InvalidDataIndex) можно использовать в
	 *       // в качестве ключа *уникальных* css стилей
	 *   }
	 *   @endcode
	 *
	 * @param[in] aStr  - строка с параметрами метаданных
	 *
	 * @return прокси структура для доступа к метаданным
	 */
	template <ESldStyleMetaTypeEnum Enum>
	CSldMetadataProxy<Enum> GetMetadata(SldU16StringRef aStr)
	{
		const CommonInfo info = ParseCommonInfo(aStr);
		CSldMetadataProxy<Enum> metaData(*this, info.isClosing, info.cssBlockIdx);
		if (!info.isClosing)
			metaData.m_error = LoadMetadata(info.metadataIdx, aStr, metaData);
		return metaData;
	}

	/**
	 * Шаблонный метод получения структурированных метаданных
	 *
	 * @param[in] aStr - строка с параметрами метаданных
	 *
	 * @return прокси структура для доступа к метаданным
	 */
	template <typename MetaDataType>
	CSldMetadataProxy<MetaDataType::metaType> GetMetadata(SldU16StringRef aStr)
	{
		return GetMetadata<MetaDataType::metaType>(aStr);
	}

	/**
	 * Шаблонный метод распарсивания метаданных
	 *
	 * @param[in] aStr      - строка с параметрами метаданных
	 * @param[in] aType     - тип метаданных
	 * @param[in] aCallback - объект который будет вызываться при успешном распарсивании
	 *
	 * @return код ошибки
	 */
	template <typename Callback>
	ESldError ParseBlockString(SldU16StringRef aStr, ESldStyleMetaTypeEnum aType, Callback&& aCallback)
	{
		const CommonInfo info = ParseCommonInfo(aStr);
		switch (aType)
		{
#define DISPATCH(_enum) case _enum : return ParseBlockString< _enum >(aStr, info, sld2::forward<Callback>(aCallback));
			SLD_FOREACH_METADATA_TYPE(DISPATCH)
#undef DISPATCH
		default: break;
		}
		return eCommonUnknownSldStyleType;
	}

public:
	// the context used for parsing metadata
	struct ParseContext;
	// we need to make the struct public so it's accessible in the .cpp file
	struct TStrings {
		UInt16 *data;
		UInt16 current;
		UInt16 capacity;
		bool owned;
	};
private:
	// make non copyable
	CSldMetadataParser(const CSldMetadataParser&) = delete;
	CSldMetadataParser& operator=(const CSldMetadataParser&) = delete;

	/// Инициализация
	CSldMetadataParser(CSldDictionary *aDictionary, CSldCSSUrlResolver *aResolver, UInt16 *aMem, UInt16 aMemSize);

	// metadata loading helper
	template <ESldStyleMetaTypeEnum Enum>
	ESldError LoadMetadata(UInt32 aIndex, SldU16StringRef aStr, CSldMetadataProxy<Enum> &aMetadata)
	{
		ESldError error = LoadMetadata(aIndex, aStr, aMetadata._data());
		if (error == eOK && m_mergeInfo)
			error = sld2::metadata::fixupIndices(m_mergeInfo, m_mergedDictIdx, aMetadata._data());
		sld2::metadata::fixup(aMetadata._data());
		return error;
	}

	// helper for loading metadata
	template <typename T>
	ESldError LoadMetadata(UInt32 aIndex, SldU16StringRef aStr, T &aMetadata)
	{
		if (m_metadataManager && aIndex != InvalidDataIndex)
		{
			ESldError error = m_metadataManager->LoadMetadata(aIndex, &aMetadata);
			if (error == eOK && aStr.size())
				error = LoadEmbeddedString(aStr);
			return error;
		}

		return GetMetadata(aStr, &aMetadata);
	}

	// overload for structless metadata
	ESldError LoadMetadata(UInt32, SldU16StringRef, TMetadataEmptyStruct&) { return eOK; }

	// binary metadata indices + close flag struct
	struct CommonInfo {
		bool isClosing;
		UInt32 metadataIdx;
		UInt32 cssBlockIdx;
	};
	// parses the block string into the above struct
	CommonInfo ParseCommonInfo(SldU16StringRef &aStr) const;

	// copies & fixes up embedded metadata strings into the internal buffer
	ESldError LoadEmbeddedString(SldU16StringRef aStr);

	// helper for the main function, does the actual per metadata parsing & callback invoke
	template <ESldStyleMetaTypeEnum Enum, typename Callback>
	ESldError ParseBlockString(SldU16StringRef aStr, const CommonInfo &aInfo, Callback&& cb)
	{
		ESldError error = eOK;
		CSldMetadataProxy<Enum> metaData(*this, aInfo.isClosing, aInfo.cssBlockIdx);
		if (!aInfo.isClosing)
			error = LoadMetadata(aInfo.metadataIdx, aStr, metaData);
		if (error == eOK)
			cb(metaData);
		return error;
	}

	// the strings parsed from "textual" metadata
	TStrings m_strings;
	// ref to the dictionary
	CSldDictionary *m_dictionary;
	// ref to the metadata manager
	CSldMetadataManager *m_metadataManager;
	// css resolver
	CSldCSSUrlResolver *m_resolver;
	// ref to merge info
	const TMergedMetaInfo *m_mergeInfo;
	// dictionary id (for merged dictionaries)
	UInt32 m_mergedDictIdx;
};

inline SldU16StringRef CSldMetadataProxyBase::string_ref(const TMetadataString &aStr) const {
	return m_parser.GetStringRef(aStr);
}

inline ESldError CSldMetadataProxyBase::cssStyle(SldU16String &aString) const
{
	return m_parser.GetCSSStyleString(m_cssBlockIndex, aString, nullptr);
}

template<ESldStyleMetaTypeEnum Enum>
inline SldU16String CSldMetadataProxy<Enum>::cssStyle() const
{
	SldU16String style;
	ESldError error = cssStyle(style);
	return error == eOK ? style : SldU16String();
}

#endif
