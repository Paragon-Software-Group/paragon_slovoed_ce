#include "SldCSSDataManager.h"

#include "SldMacros.h"
#include "SldCompare.h"
#include "SldCSSPropertyType.h"
#include "SldDictionary.h"
#include "SldIntFormatter.h"
#include "SldMetadataTypes.h"
#include "SldStringStore.h"
#include "SldTools.h"
#include "SldUtil.h"

CSldCSSDataManager::CSldCSSDataManager(CSldDictionary &aDict, CSDCReadMy &aReader,
									   const CSDCReadMy::Resource &aHeaderResource, ESldError &aError)
	: m_dictionary(aDict), m_reader(aReader), m_resourceHeaderSize(0), m_propsIndexBits(0)
{
#define error_out(_err) do { aError = _err; return; } while(0)

	const UInt8 *data = aHeaderResource.ptr();

	// get info from the main header
	auto storedHeader = (const TSldCSSDataHeader*)data;
	// check for incompatibilities
	if (storedHeader->_size > sizeof(*storedHeader) ||
		storedHeader->_pad0 != 0)
	{
		error_out(eCommonTooHighDictionaryVersion);
	}

	TSldCSSDataHeader header = {};
	sldMemCopy(&header, storedHeader, storedHeader->_size);

	data += header._size;

	// copy property resource descriptors
	ESldError error = sld2::arrayCopy(data, header.propsResourceCount, header.resourceDescriptorSize, &m_propsDescriptors);
	if (error != eOK)
		error_out(error == eCommonWrongSizeOfData ? eCommonTooHighDictionaryVersion : error);
	data += header.propsResourceCount * header.resourceDescriptorSize;

	// copy block resource descriptors
	error = sld2::arrayCopy(data, header.blocksResourceCount, header.resourceDescriptorSize, &m_blocksDescriptors);
	if (error != eOK)
		error_out(error == eCommonWrongSizeOfData ? eCommonTooHighDictionaryVersion : error);
	data += header.blocksResourceCount * header.resourceDescriptorSize;

	if (header.stringsResourceType)
	{
		auto strings = sld2::make_unique<CSldStringStore>(m_reader, header.stringsResourceType, error);
		if (!strings || error != eOK)
			error_out(strings ? error : eMemoryNotEnoughMemory);

		m_strings = sld2::move(strings);
	}

	m_resourceHeaderSize = header.resourceHeaderSize;
	m_propsIndexBits = header.propsIndexBits;

	aError = eOK;
#undef error_out
}

// to string methods
namespace {

struct Context
{
	CSldCSSUrlResolver &resolver;
	CSldDictionary &dict;
	SldU16String scratchString;

	CSldStringStore *strings;

	Context(CSldCSSUrlResolver& aResolver, CSldDictionary &aDict, CSldStringStore *aStrings)
		: resolver(aResolver), dict(aDict), strings(aStrings) {}

	SldU16StringRef getString(UInt32 aIndex, UInt32 aResIndex)
	{
		if (strings)
		{
			// for strings from a string store aIndex is actually an offset in the store
			SldU16StringRef string;
			ESldError error = strings->GetStringDirect(aResIndex, aIndex, string);
			return error == eOK ? string : nullptr;
		}
		else
		{
			const Int32 listIndex = dict.GetCSSDataStringsListIndex();
			if (listIndex == SLD_DEFAULT_LIST_INDEX)
				return nullptr;

			UInt16 *word;
			ESldError error = dict.GetWordByGlobalIndex(listIndex, aIndex, 0, &word);
			return error == eOK ? word : nullptr;
		}
	}
};

struct PropertyDescriptor
{
	// the "name" of the property
	const UInt16 *name;
	// count of supported enum values
	UInt16 enumCount;
	// supported enum values
	const UInt16 *const *enums;
};

typedef bool(*PropValueHandler)(Context &ctx, const PropertyDescriptor &aProp, const TSldCSSValue &aValue, SldU16String &aString);

namespace values {

struct Formatter : public sld2::fmt::FormatterBase<32>
{
	UInt16* formatFractional(UInt16 *ptr, UInt32 aValue, UInt32 aStep, UInt32 aMax)
	{
		aValue *= aStep;
		for (UInt32 pow10 = 10; pow10 <= aStep * aMax; pow10 *= 10, aValue /= 10)
			*--ptr = '0' + aValue % 10;
		*--ptr = '.';
		return ptr;
	}

	SldU16StringRef formatNumber(bool aNegative, UInt32 aValue, UInt32 aFractional)
	{
		UInt16 *ptr = start();

		if (aFractional > 0)
			ptr = formatFractional(ptr, aFractional, TSldCSSValue::Number_FracStep, TSldCSSValue::Number_FracMax);

		ptr = format(ptr, aValue, 10);

		if (aNegative)
			*--ptr = '-';

		return finalize(ptr);
	}

	SldU16StringRef formatColor(UInt32 aColor)
	{
		if (aColor == 0)
			return SldU16StringRefLiteral("transparent");

		if ((aColor & 0xff) == TSldCSSValue::Color_AlphaMax)
		{
			UInt16 *ptr = start();

			ptr = format(ptr, aColor, 16);
			for (UInt32 len = 8 - length(ptr); len; len--)
				*--ptr = '0';
			*--ptr = '#';

			return finalize(ptr).substr(0, 7);
		}

		// the longest string possible is rgba(255,255,255,0.005) - 23 chars
		// which perfectly fits into 31 char formatter buffer

		union {
			struct { UInt8 a, b, g, r; };
			UInt32 u32;
		} color;
		color.u32 = aColor;

		UInt16 *ptr = start();
		*--ptr = ')';

		if (color.a > 0)
			ptr = formatFractional(ptr, color.a, TSldCSSValue::Color_AlphaStep, TSldCSSValue::Color_AlphaMax);
		*--ptr = '0';

		*--ptr = ',';
		ptr = format(ptr, color.b, 10);
		*--ptr = ',';
		ptr = format(ptr, color.g, 10);
		*--ptr = ',';
		ptr = format(ptr, color.r, 10);

		ptr = pappend(ptr, SldU16StringRefLiteral("rgba("));

		return finalize(ptr);
	}
};

static bool none(Context&, const PropertyDescriptor&, const TSldCSSValue&, SldU16String &aString)
{
	aString.append(SldU16StringRefLiteral("none"));
	return true;
}

static bool auto_(Context&, const PropertyDescriptor&, const TSldCSSValue&, SldU16String &aString)
{
	aString.append(SldU16StringRefLiteral("auto"));
	return true;
}

static bool initial(Context&, const PropertyDescriptor&, const TSldCSSValue&, SldU16String &aString)
{
	aString.append(SldU16StringRefLiteral("initial"));
	return true;
}

static bool inherit(Context&, const PropertyDescriptor&, const TSldCSSValue&, SldU16String &aString)
{
	aString.append(SldU16StringRefLiteral("inherit"));
	return true;
}

static bool number(Context&, const PropertyDescriptor&, const TSldCSSValue &aValue, SldU16String &aString)
{
	auto&& number = aValue.number;
	if (number.fractional >= TSldCSSValue::Number_FracMax)
		return false;

	Formatter fmt;
	aString.append(fmt.formatNumber(number.sign, number.integer, number.fractional));
	return true;
}

static bool length(Context&, const PropertyDescriptor&, const TSldCSSValue &aValue, SldU16String &aString)
{
	const TSizeValue size(aValue.length.value, (EMetadataUnitType)aValue.length.units);
	if (!size.IsValid())
		return false;

	Formatter fmt;
	aString.append(fmt.format(size));
	return true;
}

static bool percentage(Context&, const PropertyDescriptor&, const TSldCSSValue &aValue, SldU16String &aString)
{
	Formatter fmt;
	aString.append(fmt.format(TSizeValue(aValue.percentage.value, eMetadataUnitType_percent)));
	return true;
}

static bool color(Context&, const PropertyDescriptor&, const TSldCSSValue &aValue, SldU16String &aString)
{
	const UInt32 color = aValue.color.u32;
	if ((color & 0xff) > TSldCSSValue::Color_AlphaMax)
		return false;

	Formatter fmt;
	aString.append(fmt.formatColor(color));
	return true;
}

static bool url(Context &ctx, const PropertyDescriptor&, const TSldCSSValue &aValue, SldU16String &aString)
{
	const SldU16StringRef string = ctx.getString(aValue.url.index, aValue.url.resource);
	if (string.empty())
		return false;

	aString.append(SldU16StringRefLiteral("url("));
	aString.append(string);
	aString.push_back(')');
	return true;
}

static bool string(Context &ctx, const PropertyDescriptor&, const TSldCSSValue &aValue, SldU16String &aString)
{
	const SldU16StringRef string = ctx.getString(aValue.string.index, aValue.string.resource);
	if (string.empty())
		return false;

	aString.append(string);
	return true;
}

static bool imageUrl(Context &ctx, const PropertyDescriptor&, const TSldCSSValue &aValue, SldU16String &aString)
{
	const SldU16String url = ctx.resolver.ResolveImageUrl(aValue.imageUrl.index);
	if (url.empty())
		return false;

	aString.append(SldU16StringRefLiteral("url('"));
	aString.append(url);
	aString.append(SldU16StringRefLiteral("')"));
	return true;
}

static bool stringLiteral(Context &ctx, const PropertyDescriptor&, const TSldCSSValue &aValue, SldU16String &aString)
{
	const SldU16StringRef string = ctx.getString(aValue.string.index, aValue.string.resource);
	if (string.empty())
		return false;

	aString.append(string);
	return true;
}

static bool enumeration(Context&, const PropertyDescriptor &aProp, const TSldCSSValue &aValue, SldU16String &aString)
{
	const UInt32 value = aValue.enumeration.u32;
	if (value >= aProp.enumCount)
		return false;

	aString.append(aProp.enums[value]);
	return true;
}

static const PropValueHandler handlers[] = {
	/* [Initial] = */          initial,
	/* [Inherit] = */          inherit,
	/* [Auto] = */             auto_,
	/* [None] = */             none,
	/* [Number] = */           number,
	/* [Enum] = */             enumeration,
	/* [Length] = */           length,
	/* [Percentage] = */       percentage,
	/* [Color] = */            color,
	/* [Url] = */              url,
	/* [String] = */           string,

	/* [SldImageUrl] = */      imageUrl,
	/* [SldStringLiteral] = */ stringLiteral,
};
static_assert(sizeof(handlers) / sizeof(handlers[0]) == TSldCSSValue::Type_Count,
			  "CSS value handlers table is out of sync.");

} // namespace values

namespace props {

#include "SldCSSProperties.incl"

static_assert(sizeof(descriptors) / sizeof(descriptors[0]) == eSldCSSPropertyType_Count,
			  "CSS property descriptor table is out of sync.");

} // namespace props

// css property stringifier
static ESldError toString(Context &ctx, const TSldCSSProperty &aProp, SldU16String &aString)
{
	const PropertyDescriptor *descriptor = props::descriptors[aProp.type];
	if (!descriptor)
		return eOK;

	ctx.scratchString.clear();
	for (UInt32 i = 0; i < aProp.count; i++)
	{
		const TSldCSSValue &value = aProp.values[i];
		// fully skip a property if it has unsupported values
		if (value.type > TSldCSSValue::Type_Last)
			return eCommonTooHighDictionaryVersion;

		// fully skip a property if it has invalid values
		const PropValueHandler handler = values::handlers[value.type];
		if (!handler(ctx, *descriptor, value, ctx.scratchString))
			return eCommonTooHighDictionaryVersion; // not really true but oh well...
		ctx.scratchString.push_back(' ');
	}

	if (!ctx.scratchString.empty())
	{
		// remove trailing separator
		ctx.scratchString.pop_back();

		aString.append(descriptor->name);
		aString.push_back(':');
		aString.append(ctx.scratchString);
		if (aProp.important)
			aString.append(SldU16StringRefLiteral(" !important"));
		aString.push_back(';');
	}
	return eOK;
}

} // anon namespace

/// Выдает строку css параметров по глобальному индексу блока
ESldError CSldCSSDataManager::GetCSSStyleString(UInt32 aIndex, SldU16String *aString, CSldCSSUrlResolver *aResolver)
{
	if (!aString || !aResolver)
		return eMemoryNullPointer;

	aString->clear();

	const TSldCSSPropertyBlock *block;
	ESldError error = LoadCSSPropertyBlock(aIndex, &block);
	if (error != eOK)
		return error;

	Context ctx(*aResolver, m_dictionary, m_strings.get());

	for (UInt32 i = 0; i < block->count; i++)
	{
		const UInt32 propIndex = sld2::readBits(block->props, m_propsIndexBits, i);
		const TSldCSSProperty *prop;
		error = LoadCSSProperty(propIndex, &prop);
		if (error != eOK)
			return error;

		if (prop->type > eSldCSSPropertyType_Last || prop->_pad0 != 0)
			return eCommonTooHighDictionaryVersion;

		error = toString(ctx, *prop, *aString);
		if (error != eOK)
			return error;
	}

	return eOK;
}

// looks up the resource and local indexes by the struct global index
static ESldError getResourceIndex(sld2::Span<const TSldCSSResourceDescriptor> aDescriptors,
                                  UInt32 aGlobalIndex, UInt32 *aResourceIndex, UInt32 *aLocalIndex)
{
	// the descriptors (and their respective resources) are guaranteed to be stored with a
	// monotonically increasing `endIndex` where the previous resource's `startIndex` is the last
	// ones `endIndex` (with a `0` implicit `startIndex` for the first resource.
	// so we can simply iterate over the descriptors returning the first one where the global
	// index is less than `endIndex`
	// XXX: we can acutally even binary search over this due to the sorted nature of it
	UInt32 startIndex = 0;
	for (UInt32 i = 0; i < aDescriptors.size(); i++)
	{
		const TSldCSSResourceDescriptor *descriptor = &aDescriptors[i];
		if (aGlobalIndex < descriptor->endIndex)
		{
			*aResourceIndex = i;
			*aLocalIndex = aGlobalIndex - startIndex;
			return eOK;
		}
		startIndex = descriptor->endIndex;
	}
	return eMetadataErrorInvalidCSSIndex;
}

// generic struct loader
template <typename StructType>
static ESldError loadStruct(sld2::Span<const TSldCSSResourceDescriptor> aDescriptors,
                            CSDCReadMy::Resource &aResource, CSDCReadMy &aReader, UInt32 aResType,
                            UInt32 aHeaderSize, UInt32 aIndex, const StructType **aStruct)
{
	// look up the resource and local indexes by the global index
	UInt32 resourceIndex, localIndex;
	ESldError error = getResourceIndex(aDescriptors, aIndex, &resourceIndex, &localIndex);
	if (error != eOK)
		return error;

	if (aResource.type() != aResType || aResource.index() != resourceIndex)
	{
		auto res = aReader.GetResource(aResType, resourceIndex);
		if (res != eOK)
			return res.error();
		aResource = res.resource();
	}

	const TSldCSSResourceHeader *header = (const TSldCSSResourceHeader*)aResource.ptr();
	if (header->_pad0 != 0)
		return eCommonTooHighDictionaryVersion;

	// get offset
	const UInt32 *offsets = (const UInt32*)(aResource.ptr() + aHeaderSize);
	const UInt32 offset = sld2::readBits(offsets, header->offsetBits, localIndex);

	*aStruct = (const StructType*)(aResource.ptr() + offset);
	return eOK;
}

// loads a css property block
ESldError CSldCSSDataManager::LoadCSSPropertyBlock(UInt32 aIndex, const TSldCSSPropertyBlock **aBlock)
{
	return loadStruct(m_blocksDescriptors,
	                  m_blocksResource, m_reader, SLD_RESOURCE_CSS_DATA_BLOCKS,
	                  m_resourceHeaderSize, aIndex, aBlock);
}

// loads a single css property
ESldError CSldCSSDataManager::LoadCSSProperty(UInt32 aIndex, const TSldCSSProperty **aProp)
{
	return loadStruct(m_propsDescriptors,
	                  m_propsResource, m_reader, SLD_RESOURCE_CSS_DATA_PROPERTIES,
	                  m_resourceHeaderSize, aIndex, aProp);
}

