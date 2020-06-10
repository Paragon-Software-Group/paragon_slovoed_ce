#include "Styles.h"

#include "Log.h"
#include "Tools.h"
#include "XmlTagContent.h"


/** 
	�����������, ���������� ������������� ��-���������
*/
TRGBColor::TRGBColor(void)
{
	m_Red	= 0;
	m_Green	= 0;
	m_Blue	= 0;
	m_Alpha = 255;
}

/** 
	�����������, ���������� �������������

	@param vRed		- ������� ������������
	@param vGreen	- ������� ������������
	@param vBlue	- ����� ������������
	@param vAlpha	- �����-�����
*/
TRGBColor::TRGBColor(UInt8 vRed, UInt8 vGreen, UInt8 vBlue, UInt8 vAlpha)
{
	SetColor(vRed, vGreen, vBlue, vAlpha);
}

/** 
	���������� ������� ������������ �����
	
	@return ����� �������� �� 0 �� 255.
*/
UInt8 TRGBColor::R(void) const
{
	return m_Red;
}

/** 
	���������� ������� ������������ �����

	@return ����� �������� �� 0 �� 255.
*/
UInt8 TRGBColor::G(void) const
{
	return m_Green;
}

/** 
	���������� ����� ������������ �����

	@return ����� �������� �� 0 �� 255.
*/
UInt8 TRGBColor::B(void) const
{
	return m_Blue;
}

/** 
	���������� �������� �����-������

	@return ����� �������� �� 0 (������ ������������) �� 255 (������ ��������������).
*/
UInt8 TRGBColor::Alpha(void) const
{
	return m_Alpha;
}

/** 
	������������� ����

	@param vRed		- ������� ������������
	@param vGreen	- ������� ������������
	@param vBlue	- ����� ������������
	@param vAlpha	- �����-�����
*/
void TRGBColor::SetColor(UInt8 vRed, UInt8 vGreen, UInt8 vBlue, UInt8 vAlpha)
{
	m_Red = vRed;
	m_Green = vGreen;
	m_Blue = vBlue;
	m_Alpha = vAlpha;
}

/*********************************************************************************************/

/// �����������
TSlovoedStyleVariant::TSlovoedStyleVariant(void)
{
	m_VariantType = eStyleVariant_Default;
	m_Visible = true;
	m_FontLevel = eLevelNormal;
	m_BackgroundColor = TRGBColor(255, 255, 255, 0);
	m_Bold = eBoldValueNormal;
	m_Italic = false;
	SetUnderlineStyle(eUnderlineNone, false, TRGBColor(0, 0, 0, 0));
	m_Strikethrough = false;
	m_Overline = false;
	m_FontSize = eSizeNormal;
	m_LineHeight = eSizeNormal;
	m_FontFamily = eFontFamily_SansSerif;
	m_FontName = eFontName_DejaVu_Sans;
	m_Unclickable = 0;
	m_BackgroundImageTop = SLD_INDEX_PICTURE_NO;
	m_BackgroundImageBottom = SLD_INDEX_PICTURE_NO;
}

/// ���������� ��� ��������
UInt32 TSlovoedStyleVariant::VariantType(void) const
{
	return m_VariantType;
}
	
/// ������������� ��� ��������
void TSlovoedStyleVariant::SetVariantType(UInt32 aType)
{
	m_VariantType = aType;
}


/// ���������
/**
	@return ���� ���������
*/
bool TSlovoedStyleVariant::Visible(void) const
{
	return m_Visible;
}


///	������������� ���� ���������
void TSlovoedStyleVariant::SetVisible(bool aState)
{
	m_Visible = aState;
}

/// ���������� ������������ ��������� ������
/**
	@return ������������ ��������� ������
	
	@see TFontLevel
*/
ESldStyleLevelEnum TSlovoedStyleVariant::FontLevel(void) const
{
	return m_FontLevel;
}

/// ������������� ������������ ��������� ������
/**
	@param vFontLevel	- ������������ ��������� ������
	
	@see TFontLevel
*/
void TSlovoedStyleVariant::SetFontLevel(ESldStyleLevelEnum vFontLevel)
{
	m_FontLevel = vFontLevel;
}

/// ���������� ���� ������
/**	
	@return ���� ������
*/
TRGBColor TSlovoedStyleVariant::Color(void) const
{
	return m_Color;
}

/// ������������� ���� ������
/**
	@param aColor	- ����
	
	@see TRGBColor
*/
void TSlovoedStyleVariant::SetColor(TRGBColor aColor)
{
	m_Color = aColor;
}

/// ���������� ���� ����
/**
	@return ���� ����
*/
TRGBColor TSlovoedStyleVariant::BackgroundColor(void) const
{
	return m_BackgroundColor;
}

/// ������������� ���� ����
/**
	@param aColor	- ����

	@see TRGBColor
*/
void TSlovoedStyleVariant::SetBackgroundColor(TRGBColor aColor)
{
	m_BackgroundColor = aColor;
}

/// ������������� ����� �������������
/**
	@param aUnderlineStyle		- ������� ������������
	@param aUseColor			- ������������ ������������ ����
	@param aColor				- ����

	@see TRGBColor
*/
void TSlovoedStyleVariant::SetUnderlineStyle(UInt32 aUnderlineStyle, UInt32 aUseColor, TRGBColor aColor)
{
	m_Underline = aUnderlineStyle;
	m_UnderlineUseCustomColor = (aUseColor != 0);
	m_UnderlineColor = aColor;
}

/// ������������� �������� ������������ (bold value, weight)
void TSlovoedStyleVariant::SetBoldValue(UInt32 aValue)
{
	m_Bold = aValue;
}

/// ���������� �������� ������������ (bold value, weight)
UInt32 TSlovoedStyleVariant::GetBoldValue() const
{
	return m_Bold;
}

/// ��������� �����
/**
* @return ���� �����������
*/
bool TSlovoedStyleVariant::Italic(void) const
{
	return m_Italic;
}


/// ������������� ��������� �����
void TSlovoedStyleVariant::SetItalic(bool aState)
{
	m_Italic = aState;
}


/// ������������ �����
/**
	@return ����� ��������������
*/
UInt32 TSlovoedStyleVariant::Underline(void) const
{
	return m_Underline;
}

/// ����� �� ���� ���� ��� �������������
UInt32 TSlovoedStyleVariant::UnderlineUseColor(void) const
{
	return m_UnderlineUseCustomColor;
}

/// ���� �������������
TRGBColor TSlovoedStyleVariant::GetUnderlineColor(void) const
{
	return m_UnderlineColor;
}


/// ������������� ������������ �����
void TSlovoedStyleVariant::SetUnderline(bool aState)
{
	m_Underline = aState;
}


/// ������������� �����
/**	
	@return ���� ���������������
*/
bool TSlovoedStyleVariant::Strikethrough(void) const
{
	return m_Strikethrough;
}


/// ������������� ������������� �����
void TSlovoedStyleVariant::SetStrikethrough(bool aState)
{
	m_Strikethrough = aState;
}


/// ������������ ����� (����� ������ ������)

bool TSlovoedStyleVariant::Overline(void) const
{
	return m_Overline;
}


/// ������������� ������������ �����
void TSlovoedStyleVariant::SetOverline(bool aState)
{
	m_Overline = aState;
}


/// ���������� ������ ������
/**	
	@return ������ ������
	
	@see TFontSize
*/
UInt32 TSlovoedStyleVariant::FontSize(void) const
{
	return m_FontSize;
}

/// ������������� ������ ������
/**
	@param vFontSize	- ������ ������
	
	@see TFontSize
*/
void TSlovoedStyleVariant::SetFontSize(UInt32 vFontSize)
{
	m_FontSize = vFontSize;
}

/// ������������� ������ ������
void TSlovoedStyleVariant::SetFontSize(const TSizeValue &aFontSize)
{
	m_FontSizeValue = aFontSize;
}

/// ���������� ������ ������
TSizeValue TSlovoedStyleVariant::FontSizeValue() const
{
	return m_FontSizeValue;
}

/// ���������� ������ ������
/**
	@return ������ ������
	
	@see TFontSize
*/
UInt32 TSlovoedStyleVariant::LineHeight(void) const
{
	return m_LineHeight;
}


/// ������������� ������ ������������ ���������
/**
	@param vLineHeight	- ������ ���������
	
	@see TFontSize
*/
void TSlovoedStyleVariant::SetLineHeight(UInt32 vLineHeight)
{
	m_LineHeight = vLineHeight;
}

/// ������������� ������ ������������ ���������
void TSlovoedStyleVariant::SetLineHeight(const TSizeValue &aLineHeight)
{
	m_LineHeightValue = aLineHeight;
}

/// ���������� ������ ������������ ���������
TSizeValue TSlovoedStyleVariant::LineHeightValue() const
{
	return m_LineHeightValue;
}

/// ���������� ���������, ������� ��������� ��������� ������
/**
	@return ���������
*/
ESldStyleFontFamilyEnum TSlovoedStyleVariant::FontFamily(void) const
{
	return m_FontFamily;
}

/// ������������� ���������, ������� ��������� ��������� ������
/**
	@param aFamily	- ���������
*/
void TSlovoedStyleVariant::SetFontFamily(ESldStyleFontFamilyEnum aFamily)
{
	m_FontFamily = aFamily;
}

/// ���������� ���������, ������� ��������� �������� ������
/**
	@return ���������
*/
ESldStyleFontNameEnum TSlovoedStyleVariant::FontName(void) const
{
	return m_FontName;
}

/// ������������� ���������, ������� ��������� �������� ������
/**
	@param aFontName	- ���������
*/
void TSlovoedStyleVariant::SetFontName(ESldStyleFontNameEnum aFontName)
{
	m_FontName = aFontName;
}

/// ���������� �������
wstring TSlovoedStyleVariant::Prefix(void) const
{
	return m_Prefix;
}

/// ������������� �������
void TSlovoedStyleVariant::SetPrefix(wstring vText)
{
	m_Prefix = vText;
}
	
/// ���������� ��������
wstring TSlovoedStyleVariant::Postfix(void) const
{
	return m_Postfix;
}

/// ������������� ��������
void TSlovoedStyleVariant::SetPostfix(wstring vText)
{
	m_Postfix = vText;
}

/// ���������� ������ ������� ������� ��������
UInt32 TSlovoedStyleVariant::BackgroundImageTop(void) const
{
	return m_BackgroundImageTop;
}

/// ������������� ������������ ������ ��������
/**
* @param vIndex	- ������ ��������, ������������ ��� �������
*/
void TSlovoedStyleVariant::SetBackgroundImageTop(UInt32 vIndex)
{
	m_BackgroundImageTop = vIndex;
}

/// ���������� ������ ������ ������� ��������
UInt32 TSlovoedStyleVariant::BackgroundImageBottom(void) const
{
	return m_BackgroundImageBottom;
}

/// ������������� ������������ ����� ��������
/**
* @param vIndex	- ������ ��������, ������������ ��� �������
*/
void TSlovoedStyleVariant::SetBackgroundImageBottom(UInt32 vIndex)
{
	m_BackgroundImageBottom = vIndex;
}

/// ���������� �������������� ������ ��� ���������
UInt32 TSlovoedStyleVariant::Unclickable(void) const
{
	return m_Unclickable;
}

/// ������������� �������������� ������ ��� ���������
/**
* @param aUnclickable	- �������������� ������ ��� ���������
*/
void TSlovoedStyleVariant::SetUnclickable(UInt32 aUnclickable)
{
	m_Unclickable = aUnclickable;
}

/*********************************************************************************************/


///	�����������, ���������� ������������� ��-���������

TSlovoedStyle::TSlovoedStyle(void)
{
	m_Number = 0;
	m_TextType = eMetaText;
	m_Language = 0;
	m_DefaultVariantIndex = 0;
}

/// ���������� ����� �����
/**
	 @return ����� �����
*/
UInt16 TSlovoedStyle::Number(void) const
{
	return m_Number;
}

/// ������������� ����� �����
/**
	@param vNumber	- ����� �����
*/
void TSlovoedStyle::SetNumber(UInt16 vNumber)
{
	m_Number = vNumber;
}

/// ���������� ���
/**
	@return ������ � �����
*/
const wstring& TSlovoedStyle::Tag(void) const
{
	return m_Tag;
}

/// ������������� ���
/**
	@param vTag	- ������ � �����
*/
void TSlovoedStyle::SetTag(wstring vTag)
{
	m_Tag = vTag;
}

/// ���������� ���� �����
/**
	@return ���� �����
*/
UInt32 TSlovoedStyle::Language(void) const
{
	return m_Language;
}

/// ������������� ���� �����
/**
	@param vLanguage	- ���� �����
*/
void TSlovoedStyle::SetLanguage(UInt32 vLanguage)
{
	m_Language = vLanguage;
}

/// ���������� ���������� ��������� ������������� �����
UInt32 TSlovoedStyle::UsageCount(void) const
{
	return (UInt32)m_Usages.size();
}

/// ���������� �� ������ �������� ���������, ������� ��������� ������� ������������� �����
/**
	@return ���������
*/
ESldStyleUsageEnum TSlovoedStyle::GetUsage(UInt32 aIndex) const
{
	if (aIndex >= UsageCount())
		return eStyleUsage_Unknown;
		
	return m_Usages[aIndex];
}

///	��������� � ������ ���������, ������� ��������� ������� ������������� �����
/**
	@param vUsage	- ���������
*/
void TSlovoedStyle::AddUsage(ESldStyleUsageEnum vUsage)
{
	m_Usages.push_back(vUsage);
}

/// ���������� ���������� ��������� �����
UInt32 TSlovoedStyle::VariantsCount(void) const
{
	return (UInt32)m_StyleVariants.size();
}

/// ���������� ������� ����� �� ������
/**
	@param aIndex	- ����� �������������� ��������
	
	@return ��������� �� �������, ���� NULL, ���� ����� ������� �� ������
*/
const TSlovoedStyleVariant* TSlovoedStyle::GetStyleVariant(UInt32 aIndex) const
{
	if (aIndex >= VariantsCount())
		return NULL;
	
	return &(m_StyleVariants[aIndex]);
}

/// ��������� ������� �����
/**
	@param aStyleVariant	- ��������� �� ������� �����, ������� ����� ��������
	
	@return ��� ������
*/
void TSlovoedStyle::AddStyleVariant(const TSlovoedStyleVariant &aStyleVariant)
{
	m_StyleVariants.push_back(aStyleVariant);
}

/// ���������� ����� �������� ��� ����������� ��-���������
UInt32 TSlovoedStyle::DefaultVariantIndex(void) const
{
	return m_DefaultVariantIndex;
}
	
/// ������������� ����� �������� ��� ����������� ��-���������
void TSlovoedStyle::SetDefaultVariantIndex(UInt32 aIndex)
{
	m_DefaultVariantIndex = aIndex;
}

/*********************************************************************************************/


/// �����������, ���������� ������������� ��-���������

TSlovoedStyleManager::TSlovoedStyleManager(void)
{
	// ��������� ����� ��-���������
	TSlovoedStyle Style;
	TSlovoedStyleVariant StyleVariant;
	Style.AddStyleVariant(StyleVariant);
	Style.AddUsage(eStyleUsage_Unknown);

	AddStyle(Style);
}

/// ���������� ���������� ������
UInt16 TSlovoedStyleManager::StylesCount(void) const
{
	return (UInt16)m_Styles.size();
}

UInt16 TSlovoedStyleManager::addStyle(TSlovoedStyle &aStyle)
{
	// another one of those funny indexing spots...
	// not sure why so much hate to 0 based indexes...
	const UInt16 styleNumber = static_cast<UInt16>(m_Styles.size()) + 1;
	aStyle.SetNumber(styleNumber);
	m_Styles.push_back(aStyle);
	m_UsedStyles.resize(styleNumber + 1);
	return styleNumber;
}

/// ��������� ���� �����
/**
	@param vStyle	- ����������� �����
*/
UInt16 TSlovoedStyleManager::AddStyle(TSlovoedStyle& vStyle)
{
	if (m_StylesMap.find(vStyle.Tag()) != m_StylesMap.end())
	{
		STString<512> wbuf(L"Duplicate style definition: '%s'. Please resolve.", vStyle.Tag());
		sldXLog("NOTE: %s\n", wbuf);
		sld::printf(eLogStatus_Warning, "\n%s", wbuf);
	}

	const UInt16 styleNumber = addStyle(vStyle);
	m_StylesMap.emplace(vStyle.Tag(), styleNumber);
	return styleNumber;
}

/**
 * ���������� ����� ����� �� ���� ����������
 * �������� *������* ��� ����������!
 */
UInt16 TSlovoedStyleManager::GetMetadataStyleNumber(ESldStyleMetaTypeEnum aType)
{
	if (!isMetadataType(aType))
		return GetDefaultStylesNumber();

	auto it = m_MetadataStyles.find(aType);
	if (it != m_MetadataStyles.cend())
		return it->second;

	TSlovoedStyle Style;
	TSlovoedStyleVariant UnTypedStyleVariant;

	Style.SetTextType(aType);
	Style.AddStyleVariant(UnTypedStyleVariant);
	Style.AddUsage(eStyleUsage_Unknown);

	const UInt16 styleNumber = addStyle(Style);
	m_MetadataStyles.emplace(aType, styleNumber);
	m_UsedStyles[styleNumber] = true;
	return styleNumber;
}

/// ���������� ����� ����� �� �����
UInt16 TSlovoedStyleManager::GetStyleNumberByName(sld::wstring_ref aTagName)
{
	static std::wstring tagName;
	tagName.assign(aTagName.data(), aTagName.size());
	const auto it = m_StylesMap.find(tagName);
	if (it != m_StylesMap.cend())
	{
		m_UsedStyles[it->second] = true;
		return it->second;
	}

	m_UnknownStyles.insert(tagName);
	return GetDefaultStylesNumber();
}

/// ���������� ����� ����� ��-���������
UInt16 TSlovoedStyleManager::GetDefaultStylesNumber(void) const
{
	return m_Styles[0].Number();
}

/// �� ����� ���� �� ������ ���������� ������ � �������� ����� ���� � ������� ������
wstring TSlovoedStyleManager::GetVariantStyleStr(const wstring &aStyleName) const
{
	if (aStyleName.empty())
		return wstring();

	const auto it = m_StylesMap.find(aStyleName);
	if (it == m_StylesMap.end())
		return wstring();

	return STString<128>(L"%u", it->second).str();
}

std::vector<std::wstring> TSlovoedStyleManager::unusedStyles() const
{
	std::vector<std::wstring> styles;
	for (auto&& style : m_Styles)
	{
		if (!m_UsedStyles[style.Number()] && !style.Tag().empty())
			styles.push_back(style.Tag());
	}
	return styles;
}
