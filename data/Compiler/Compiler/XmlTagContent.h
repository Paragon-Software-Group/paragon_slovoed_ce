#pragma once

#include "sld_Types.h"
#include "sld2_Error.h"

#include <string>
#include <unordered_map>

#include "PugiXML/pugixml.hpp"
#include "string_ref.h"

namespace sld {
namespace enums {

template <typename T>
struct Descriptor {
	sld::wstring_ref string;
	T value;

	operator std::pair<const sld::wstring_ref, T>() const { return{ string, value }; }
};

template <typename T, size_t DataCount>
inline T findValue(const Descriptor<T>(&aData)[DataCount], sld::wstring_ref aStr, T aDefault)
{
	for (const auto &descriptor : aData)
	{
		if (descriptor.string == aStr)
			return descriptor.value;
	}
	return aDefault;
}

template <typename T, size_t DataCount>
static inline T findValueHashed(const Descriptor<T>(&aData)[DataCount], sld::wstring_ref aStr, T aDefault)
{
	static const std::unordered_map<sld::wstring_ref, T> map(std::begin(aData), std::end(aData));
	const auto &it = map.find(aStr);
	return it != map.end() ? it->second : aDefault;
}

} // namespace xml
} // namespace sld

using namespace std;

ESldStyleUsageEnum GetStyleUsageByXmlTagContent(wstring aStr);

ESldStyleFontFamilyEnum GetStyleFontFamilyByXmlTagContent(wstring aStr);

ESldStyleFontNameEnum GetStyleFontNameByXmlTagContent(wstring aStr);

ESlovoEdContainerDatabaseTypeEnum GetDatabaseTypeByXmlTagContent(wstring aStr);

EMediaSourceTypeEnum GetMediaSourceTypeByXmlTagContent(wstring aStr);

EFullTextSearchLinkType GetFullTextSearchLinkTypeByXmlTagContent(wstring aStr);

EFullTextSearchShiftType GetFullTextSearchShiftTypeByXmlTagContent(wstring aStr);

EDictionaryBrandName GetDictionaryBrandNameByXmlTagContent(wstring aStr);
wstring GetDictionaryXmlBrandNameByBrandId(UInt32 aBrand);

UInt8 GetClassLevelMajor(const wstring aLevel);
UInt8 GetClassLevelMinor(const wstring aLevel);
UInt8 GetUInt8(const wstring aValue);
UInt8 GetRevision(const wstring aRevision);
UInt8 GetBookPart(const wstring aBookPart);
UInt16 GetPublishYear(const wstring aPublishYear);
ESldContentType GetContentType(const wstring aContentType);
EEducationalLevel GetEducationalLevel(const wstring& aEducationalLevel);
UInt32 GetListTypeFromString(const std::wstring &aStr);
EAlphabetType GetAlphabetTypeFromString(const std::wstring &aStr);

ESldStyleMetaTypeEnum GetTextTypeByTagName(sld::wstring_ref aName);
sld::wstring_ref GetTagNameByTextType(ESldStyleMetaTypeEnum aType);
static inline bool isMetadataType(ESldStyleMetaTypeEnum aType) {
	return aType > eMetaPhonetics && aType < eMeta_Last;
}
static inline bool isMetadataTag(sld::wstring_ref aName) {
	return isMetadataType(GetTextTypeByTagName(aName));
}

int ParseLanguageCodeAttrib(pugi::xml_node aNode, UInt32 *aLangCode);
int ParseLanguageCodeNode(pugi::xml_node aNode, UInt32 *aLangCode);

enum class BoolType { YesNo = 0, OnOff, COUNT };
int ParseBoolParamNode(pugi::xml_node aNode, bool &aValue, BoolType aType);

int ParseCompressionConfig(const sld::wstring_ref &aString, CompressionConfig &aConfig);
