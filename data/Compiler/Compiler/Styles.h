#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "sld_Types.h"
#include "sld2_Error.h"
#include "sld_Platform.h"

#include "string_ref.h"
#include "util.h" // for RangeImpl

using namespace std;


/// �����, ����������� ���� � ������� RGB
class TRGBColor
{
public:

	/// �����������
	TRGBColor(void);
	/// �����������
	TRGBColor(UInt8 vRed, UInt8 vGreen, UInt8 vBlue, UInt8 vAlpha);

	/// ���������� ������� ������������ �����
	UInt8 R(void) const;
	/// ���������� ������� ������������ �����
	UInt8 G(void) const;
	/// ���������� ����� ������������ �����
	UInt8 B(void) const;
	/// ���������� �������� �����-������
	UInt8 Alpha(void) const;
	
	/// ������������� ����
	void SetColor(UInt8 vRed, UInt8 vGreen, UInt8 vBlue, UInt8 vAlpha);

private:
	
	/// ������� ������������ �����
	UInt8 m_Red;
	
	/// ����� ������������ �����
	UInt8 m_Green;
	
	/// ������� ������������ �����
	UInt8 m_Blue;
	
	/// �����-�����
	UInt8 m_Alpha;
	
};


/// �����, �������������� ������� �����
class TSlovoedStyleVariant
{
public:

	/// �����������
	TSlovoedStyleVariant(void);

public:

	/// ���������� ��� ��������
	UInt32 VariantType(void) const;
	/// ������������� ��� ��������
	void SetVariantType(UInt32 aType);

	/// ���������
	bool Visible(void) const;
	/// ������������� ���� ���������
	void SetVisible(bool aState);

	/// ���������� ������������ ��������� ������
	ESldStyleLevelEnum FontLevel(void) const;
	/// ������������� ������������ ��������� ������
	void SetFontLevel(ESldStyleLevelEnum vFontLevel);
	
	/// ���������� ���� ������
	TRGBColor Color(void) const;
	/// ������������� ���� ������
	void SetColor(TRGBColor aColor);
	
	/// ���������� ���� ����
	TRGBColor BackgroundColor(void) const;
	/// ������������� ���� ����
	void SetBackgroundColor(TRGBColor aColor);

	/// ������������� �������� ������������ (bold value, weight)
	void SetBoldValue(UInt32 aValue);
	/// ���������� �������� ������������ (bold value, weight)
	UInt32 GetBoldValue() const;
	
	/// ��������� �����
	bool Italic(void) const;
	/// ������������� ��������� �����
	void SetItalic(bool aState);
	
	/// ������������� ����� �������������
	void SetUnderlineStyle(UInt32 aUnderlineStyle, UInt32 aUseColor, TRGBColor aColor);
	/// ������������ �����
	UInt32 Underline(void) const;
	/// ������������� ������������ �����
	void SetUnderline(bool aState);
	/// ����� �� ���� ���� ��� �������������
	UInt32 UnderlineUseColor(void) const;
	/// ���� �������������
	TRGBColor GetUnderlineColor(void) const;

	/// ������������� �����
	bool Strikethrough(void) const;
	/// ������������� ������������� �����
	void SetStrikethrough(bool aState);
	
	/// ������������ ����� (����� ������ ������)
	bool Overline(void) const;
	/// ������������� ������������ �����
	void SetOverline(bool aState);
	
	/// ���������� ������ ������
	UInt32 FontSize(void) const;
	/// ������������� ������ ������
	void SetFontSize(UInt32 vFontSize);
	/// ������������� ������ ������
	void SetFontSize(const TSizeValue &aFontSize);
	/// ���������� ������ ������
	TSizeValue FontSizeValue() const;

	/// ���������� ���������, ������� ��������� ��������� ������
	ESldStyleFontFamilyEnum FontFamily(void) const;
	/// ������������� ���������, ������� ��������� ��������� ������
	void SetFontFamily(ESldStyleFontFamilyEnum aFamily);

	/// ���������� ���������, ������� ��������� �������� ������
	ESldStyleFontNameEnum FontName(void) const;
	/// ������������� ���������, ������� ��������� �������� ������
	void SetFontName(ESldStyleFontNameEnum aFontName);

	/// ���������� ������ ������
	UInt32 LineHeight(void) const;
	/// ������������� ������ ������
	void SetLineHeight(UInt32 vLineHeight);
	/// ������������� ������ ������
	void SetLineHeight(const TSizeValue &aLineHeight);
	/// ���������� ������ ������
	TSizeValue LineHeightValue() const;
	
	/// ���������� �������
	wstring Prefix(void) const;
	/// ������������� �������
	void SetPrefix(wstring vText);
	
	/// ���������� ��������
	wstring Postfix(void) const;
	/// ������������� ��������
	void SetPostfix(wstring vText);

	/// ���������� ������ ������� ������� ��������
	UInt32 BackgroundImageTop(void) const;
	/// ���������� ������ ������ ������� ��������
	UInt32 BackgroundImageBottom(void) const;
	/// ������������� ������������ ������ ��������
	void SetBackgroundImageTop(UInt32 vText);
	/// ������������� ������������ ����� ��������
	void SetBackgroundImageBottom(UInt32 vText);

	/// ���������� �������������� ������ ��� ���������
	UInt32 Unclickable(void) const;
	/// ������������� �������������� ������ ��� ���������
	void SetUnclickable(UInt32 vIndex);

private:

	/// ��� �������� ����� (��. #EStyleVariantType)
	UInt32 m_VariantType;

	/// ���������
	bool m_Visible;

	/// ������������ ��������� ������
	ESldStyleLevelEnum m_FontLevel;
	
	/// ���� ������
	TRGBColor m_Color;
	
	/// ���� ����
	TRGBColor m_BackgroundColor;
	
	/// ��������
	/**
		true	- ������ �����
		false	- �������� �����
	*/
	UInt32 m_Bold;
	
	/// �����������
	/**
		true	- ��������� �����
		false	- ����������� �����
	*/
	bool m_Italic;
	
	/// ��������������
	/// @see ESldUnderlineStyle
	UInt32 m_Underline;

	/// ����, ������� ��� � ���, ��� ��� ������������� ���� ����� �������� 
	bool m_UnderlineUseCustomColor;

	/// ���� ��� ������������� 
	TRGBColor m_UnderlineColor;


	/// ���������������
	/**
		true	- ������������� �����
		false	- ��������������� �����
	*/
	bool m_Strikethrough;
	
	/// �������������� (����� ������ ������)
	/**
		true	- ������������ �����
		false	- �������������� �����
	*/
	bool m_Overline;
	
	/// ������ ������ (@see ESldStyleSizeEnum)
	UInt32 m_FontSize;

	/// ������ ������
	TSizeValue m_FontSizeValue;

	/// ������ ������ (@see ESldStyleSizeEnum)
	UInt32 m_LineHeight;

	/// ������ ������
	TSizeValue m_LineHeightValue;

	/// ��������� ������
	ESldStyleFontFamilyEnum m_FontFamily;
	
	/// �������� ������
	ESldStyleFontNameEnum m_FontName;
	
	/// �������
	wstring m_Prefix;
	
	/// ��������
	wstring m_Postfix;

	/// ���� ���������� ������ ���� - ����� ����������� ������ ������ ����� ���������� ��� ���������
	UInt32 m_Unclickable;

	/// ���������� ������ ��������, ������������ ��� �������
	UInt32 m_BackgroundImageTop;

	/// ���������� ������ ��������, ������������ ��� �������
	UInt32 m_BackgroundImageBottom;
};


/// �����, �������������� �����
class TSlovoedStyle
{
public:

	/// �����������
	TSlovoedStyle(void);

public:
	
	/// ���������� ����� �����
	UInt16 Number(void) const;
	/// ������������� ����� �����
	void SetNumber(UInt16 vNumber);

	/// ���������� ��� ������
	ESldStyleMetaTypeEnum GetTextType() const { return m_TextType; }
	/// ������������� ��� ������
	void SetTextType(ESldStyleMetaTypeEnum aTextType) { m_TextType = aTextType; }

	/// ���������� ���
	const wstring& Tag(void) const;
	/// ������������� ���
	void SetTag(wstring vTag);
	
	/// ���������� ���� �����
	UInt32 Language(void) const;
	/// ������������� ���� �����
	void SetLanguage(UInt32 vLanguage);
	
	/// ���������� ���������� ��������� ������������� �����
	UInt32 UsageCount(void) const;
	/// ���������� �� ������ �������� ���������, ������� ��������� ������� ������������� �����
	ESldStyleUsageEnum GetUsage(UInt32 aIndex) const;
	/// ��������� � ������ ���������, ������� ��������� ������� ������������� �����
	void AddUsage(ESldStyleUsageEnum vUsage);
	
	/// ���������� ���������� ��������� �����
	UInt32 VariantsCount(void) const;
	/// ���������� ������� ����� �� ������
	const TSlovoedStyleVariant* GetStyleVariant(UInt32 aIndex) const;
	/// ��������� ������� �����
	void AddStyleVariant(const TSlovoedStyleVariant &aStyleVariant);
	
	/// ���������� ����� �������� ��� ����������� ��-���������
	UInt32 DefaultVariantIndex(void) const;
	/// ������������� ����� �������� ��� ����������� ��-���������
	void SetDefaultVariantIndex(UInt32 aIndex);

private:

	/// ����� �����
	UInt16 m_Number;

	/// ��� ������
	ESldStyleMetaTypeEnum m_TextType;

	/// ���, �������� ��� ����, ��� �������� ������������ ������ �����
	wstring m_Tag;

	/// ���� �����
	UInt32 m_Language;

	/// ����� �������� ��� ����������� ��-���������
	UInt32 m_DefaultVariantIndex;

	/// ������ Usage - ��� ���� ������������ ������ �����
	vector<ESldStyleUsageEnum> m_Usages;

	/// �������� ����������� �����
	vector<TSlovoedStyleVariant> m_StyleVariants;
};


/// �����, ����������� ������� �������
class TSlovoedStyleManager
{
public:
	
	/// �����������
	TSlovoedStyleManager(void);
	
	/// ���������� ���������� ������
	UInt16 StylesCount(void) const;
	
	/// ��������� ���� �����
	UInt16 AddStyle(TSlovoedStyle& vStyle);

	/// ���������� ����� ����� �� ���� ����������
	UInt16 GetMetadataStyleNumber(ESldStyleMetaTypeEnum aType);

	/// ���������� ����� ����� �� �����
	UInt16 GetStyleNumberByName(sld::wstring_ref aTagName);
	
	/// ���������� ����� ����� ��-���������
	UInt16 GetDefaultStylesNumber(void) const;

	/// �� ����� ���� �� ������ ���������� ������ � �������� ����� ���� � ������� ������
	wstring GetVariantStyleStr(const wstring &aStyleName) const;

private:

	UInt16 addStyle(TSlovoedStyle& vStyle);

	/// ������ ������
	vector<TSlovoedStyle> m_Styles;

	/// Map ������ ��� -> �����
	unordered_map<wstring, UInt16> m_StylesMap;

	/// Map ������ ��� -> �����
	std::unordered_map<ESldStyleMetaTypeEnum, UInt16> m_MetadataStyles;

	/// "�����" ������ ��� ������� ��� ������ ::GetStyleNumberByName()
	std::vector<bool> m_UsedStyles;

	/// ����� ������� �� ���� ���������, �� ��� ������� ��� ������ ::GetStyleNumberByName()
	std::unordered_set<std::wstring> m_UnknownStyles;

public:

	auto styles() const -> RangeImpl<decltype(m_Styles.begin())> {
		return{ m_Styles.begin(), m_Styles.end() };
	}

	auto unknownStyles() const -> RangeImpl<decltype(m_UnknownStyles.begin())> {
		return{ m_UnknownStyles.begin(), m_UnknownStyles.end() };
	}

	std::vector<std::wstring> unusedStyles() const;
};
