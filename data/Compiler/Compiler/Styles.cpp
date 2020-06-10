#include "Styles.h"

#include "Log.h"
#include "Tools.h"
#include "XmlTagContent.h"


/** 
	Конструктор, производит инициализацию по-умолчанию
*/
TRGBColor::TRGBColor(void)
{
	m_Red	= 0;
	m_Green	= 0;
	m_Blue	= 0;
	m_Alpha = 255;
}

/** 
	Конструктор, производит инициализацию

	@param vRed		- красная составляющая
	@param vGreen	- зеленая составляющая
	@param vBlue	- синяя составляющая
	@param vAlpha	- альфа-канал
*/
TRGBColor::TRGBColor(UInt8 vRed, UInt8 vGreen, UInt8 vBlue, UInt8 vAlpha)
{
	SetColor(vRed, vGreen, vBlue, vAlpha);
}

/** 
	Возвращает красную составляющую цвета
	
	@return целое значение от 0 до 255.
*/
UInt8 TRGBColor::R(void) const
{
	return m_Red;
}

/** 
	Возвращает зеленую составляющую цвета

	@return целое значение от 0 до 255.
*/
UInt8 TRGBColor::G(void) const
{
	return m_Green;
}

/** 
	Возвращает синюю составляющую цвета

	@return целое значение от 0 до 255.
*/
UInt8 TRGBColor::B(void) const
{
	return m_Blue;
}

/** 
	Возвращает значение альфа-канала

	@return целое значение от 0 (полная прозрачность) до 255 (полная непрозрачность).
*/
UInt8 TRGBColor::Alpha(void) const
{
	return m_Alpha;
}

/** 
	Устанавливает цвет

	@param vRed		- красная составляющая
	@param vGreen	- зеленая составляющая
	@param vBlue	- синяя составляющая
	@param vAlpha	- альфа-канал
*/
void TRGBColor::SetColor(UInt8 vRed, UInt8 vGreen, UInt8 vBlue, UInt8 vAlpha)
{
	m_Red = vRed;
	m_Green = vGreen;
	m_Blue = vBlue;
	m_Alpha = vAlpha;
}

/*********************************************************************************************/

/// Конструктор
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

/// Возвращает тип варианта
UInt32 TSlovoedStyleVariant::VariantType(void) const
{
	return m_VariantType;
}
	
/// Устанавливает тип варианта
void TSlovoedStyleVariant::SetVariantType(UInt32 aType)
{
	m_VariantType = aType;
}


/// Видимость
/**
	@return флаг видимости
*/
bool TSlovoedStyleVariant::Visible(void) const
{
	return m_Visible;
}


///	Устанавливает флаг видимости
void TSlovoedStyleVariant::SetVisible(bool aState)
{
	m_Visible = aState;
}

/// Возвращает вертикальное положение шрифта
/**
	@return вертикальное положение шрифта
	
	@see TFontLevel
*/
ESldStyleLevelEnum TSlovoedStyleVariant::FontLevel(void) const
{
	return m_FontLevel;
}

/// Устанавливает вертикальное положение шрифта
/**
	@param vFontLevel	- вертикальное положение шрифта
	
	@see TFontLevel
*/
void TSlovoedStyleVariant::SetFontLevel(ESldStyleLevelEnum vFontLevel)
{
	m_FontLevel = vFontLevel;
}

/// Возвращает цвет шрифта
/**	
	@return цвет шрифта
*/
TRGBColor TSlovoedStyleVariant::Color(void) const
{
	return m_Color;
}

/// Устанавливает цвет шрифта
/**
	@param aColor	- цвет
	
	@see TRGBColor
*/
void TSlovoedStyleVariant::SetColor(TRGBColor aColor)
{
	m_Color = aColor;
}

/// Возвращает цвет фона
/**
	@return цвет фона
*/
TRGBColor TSlovoedStyleVariant::BackgroundColor(void) const
{
	return m_BackgroundColor;
}

/// Устанавливает цвет фона
/**
	@param aColor	- цвет

	@see TRGBColor
*/
void TSlovoedStyleVariant::SetBackgroundColor(TRGBColor aColor)
{
	m_BackgroundColor = aColor;
}

/// Устанавливает стиль подчеркивания
/**
	@param aUnderlineStyle		- красная составляющая
	@param aUseColor			- использовать определенный цвет
	@param aColor				- цвет

	@see TRGBColor
*/
void TSlovoedStyleVariant::SetUnderlineStyle(UInt32 aUnderlineStyle, UInt32 aUseColor, TRGBColor aColor)
{
	m_Underline = aUnderlineStyle;
	m_UnderlineUseCustomColor = (aUseColor != 0);
	m_UnderlineColor = aColor;
}

/// Устанавливает величину насыщенности (bold value, weight)
void TSlovoedStyleVariant::SetBoldValue(UInt32 aValue)
{
	m_Bold = aValue;
}

/// Возвращает величину насыщенности (bold value, weight)
UInt32 TSlovoedStyleVariant::GetBoldValue() const
{
	return m_Bold;
}

/// Наклонный шрифт
/**
* @return флаг наклонности
*/
bool TSlovoedStyleVariant::Italic(void) const
{
	return m_Italic;
}


/// Устанавливает наклонный шрифт
void TSlovoedStyleVariant::SetItalic(bool aState)
{
	m_Italic = aState;
}


/// Подчеркнутый шрифт
/**
	@return стиль подчеркнутости
*/
UInt32 TSlovoedStyleVariant::Underline(void) const
{
	return m_Underline;
}

/// Задан ли свой цвет для подчеркивания
UInt32 TSlovoedStyleVariant::UnderlineUseColor(void) const
{
	return m_UnderlineUseCustomColor;
}

/// цвет подчеркивания
TRGBColor TSlovoedStyleVariant::GetUnderlineColor(void) const
{
	return m_UnderlineColor;
}


/// Устанавливает подчеркнутый шрифт
void TSlovoedStyleVariant::SetUnderline(bool aState)
{
	m_Underline = aState;
}


/// Перечеркнутый шрифт
/**	
	@return флаг перечеркнутости
*/
bool TSlovoedStyleVariant::Strikethrough(void) const
{
	return m_Strikethrough;
}


/// Устанавливает перечеркнутый шрифт
void TSlovoedStyleVariant::SetStrikethrough(bool aState)
{
	m_Strikethrough = aState;
}


/// Надчеркнутый шрифт (черта сверху текста)

bool TSlovoedStyleVariant::Overline(void) const
{
	return m_Overline;
}


/// Устанавливает надчеркнутый шрифт
void TSlovoedStyleVariant::SetOverline(bool aState)
{
	m_Overline = aState;
}


/// Возвращает размер шрифта
/**	
	@return размер шрифта
	
	@see TFontSize
*/
UInt32 TSlovoedStyleVariant::FontSize(void) const
{
	return m_FontSize;
}

/// Устанавливает размер шрифта
/**
	@param vFontSize	- размер шрифта
	
	@see TFontSize
*/
void TSlovoedStyleVariant::SetFontSize(UInt32 vFontSize)
{
	m_FontSize = vFontSize;
}

/// Устанавливает размер шрифта
void TSlovoedStyleVariant::SetFontSize(const TSizeValue &aFontSize)
{
	m_FontSizeValue = aFontSize;
}

/// Возвращает размер шрифта
TSizeValue TSlovoedStyleVariant::FontSizeValue() const
{
	return m_FontSizeValue;
}

/// Возвращает высоту строки
/**
	@return высота строки
	
	@see TFontSize
*/
UInt32 TSlovoedStyleVariant::LineHeight(void) const
{
	return m_LineHeight;
}


/// Устанавливает размер межстрочного интервала
/**
	@param vLineHeight	- размер интервала
	
	@see TFontSize
*/
void TSlovoedStyleVariant::SetLineHeight(UInt32 vLineHeight)
{
	m_LineHeight = vLineHeight;
}

/// Устанавливает размер межстрочного интервала
void TSlovoedStyleVariant::SetLineHeight(const TSizeValue &aLineHeight)
{
	m_LineHeightValue = aLineHeight;
}

/// Возвращает размер межстрочного интервала
TSizeValue TSlovoedStyleVariant::LineHeightValue() const
{
	return m_LineHeightValue;
}

/// Возвращает константу, которая описывает семейство шрифта
/**
	@return константа
*/
ESldStyleFontFamilyEnum TSlovoedStyleVariant::FontFamily(void) const
{
	return m_FontFamily;
}

/// Устанавливает константу, которая описывает семейство шрифта
/**
	@param aFamily	- константа
*/
void TSlovoedStyleVariant::SetFontFamily(ESldStyleFontFamilyEnum aFamily)
{
	m_FontFamily = aFamily;
}

/// Возвращает константу, которая описывает название шрифта
/**
	@return константа
*/
ESldStyleFontNameEnum TSlovoedStyleVariant::FontName(void) const
{
	return m_FontName;
}

/// Устанавливает константу, которая описывает название шрифта
/**
	@param aFontName	- константа
*/
void TSlovoedStyleVariant::SetFontName(ESldStyleFontNameEnum aFontName)
{
	m_FontName = aFontName;
}

/// Возвращает префикс
wstring TSlovoedStyleVariant::Prefix(void) const
{
	return m_Prefix;
}

/// Устанавливает префикс
void TSlovoedStyleVariant::SetPrefix(wstring vText)
{
	m_Prefix = vText;
}
	
/// Возвращает постфикс
wstring TSlovoedStyleVariant::Postfix(void) const
{
	return m_Postfix;
}

/// Устанавливает постфикс
void TSlovoedStyleVariant::SetPostfix(wstring vText)
{
	m_Postfix = vText;
}

/// Возвращает индекс верхней фоновой картинки
UInt32 TSlovoedStyleVariant::BackgroundImageTop(void) const
{
	return m_BackgroundImageTop;
}

/// Устанавливает отображаемую сверху картинку
/**
* @param vIndex	- индекс картинки, отображаемой над текстом
*/
void TSlovoedStyleVariant::SetBackgroundImageTop(UInt32 vIndex)
{
	m_BackgroundImageTop = vIndex;
}

/// Возвращает индекс нижней фоновой картинки
UInt32 TSlovoedStyleVariant::BackgroundImageBottom(void) const
{
	return m_BackgroundImageBottom;
}

/// Устанавливает отображаемую снизу картинку
/**
* @param vIndex	- индекс картинки, отображаемой под текстом
*/
void TSlovoedStyleVariant::SetBackgroundImageBottom(UInt32 vIndex)
{
	m_BackgroundImageBottom = vIndex;
}

/// Возвращает кликабельность текста для кроссрефа
UInt32 TSlovoedStyleVariant::Unclickable(void) const
{
	return m_Unclickable;
}

/// Устанавливает кликабельность текста для кроссрефа
/**
* @param aUnclickable	- кликабельность текста для кроссрефа
*/
void TSlovoedStyleVariant::SetUnclickable(UInt32 aUnclickable)
{
	m_Unclickable = aUnclickable;
}

/*********************************************************************************************/


///	Конструктор, производит инициализацию по-умолчанию

TSlovoedStyle::TSlovoedStyle(void)
{
	m_Number = 0;
	m_TextType = eMetaText;
	m_Language = 0;
	m_DefaultVariantIndex = 0;
}

/// Возвращает номер стиля
/**
	 @return номер стиля
*/
UInt16 TSlovoedStyle::Number(void) const
{
	return m_Number;
}

/// Устанавливает номер стиля
/**
	@param vNumber	- номер стиля
*/
void TSlovoedStyle::SetNumber(UInt16 vNumber)
{
	m_Number = vNumber;
}

/// Возвращает тег
/**
	@return строка с тегом
*/
const wstring& TSlovoedStyle::Tag(void) const
{
	return m_Tag;
}

/// Устанавливает тег
/**
	@param vTag	- строка с тегом
*/
void TSlovoedStyle::SetTag(wstring vTag)
{
	m_Tag = vTag;
}

/// Возвращает язык стиля
/**
	@return язык стиля
*/
UInt32 TSlovoedStyle::Language(void) const
{
	return m_Language;
}

/// Устанавливает язык стиля
/**
	@param vLanguage	- язык стиля
*/
void TSlovoedStyle::SetLanguage(UInt32 vLanguage)
{
	m_Language = vLanguage;
}

/// Возвращает количество вариантов использования стиля
UInt32 TSlovoedStyle::UsageCount(void) const
{
	return (UInt32)m_Usages.size();
}

/// Возвращает по номеру варианта константу, которая описывает вариант использование стиля
/**
	@return константа
*/
ESldStyleUsageEnum TSlovoedStyle::GetUsage(UInt32 aIndex) const
{
	if (aIndex >= UsageCount())
		return eStyleUsage_Unknown;
		
	return m_Usages[aIndex];
}

///	Добавляет в список константу, которая описывает вариант использования стиля
/**
	@param vUsage	- константа
*/
void TSlovoedStyle::AddUsage(ESldStyleUsageEnum vUsage)
{
	m_Usages.push_back(vUsage);
}

/// Возвращает количество вариантов стиля
UInt32 TSlovoedStyle::VariantsCount(void) const
{
	return (UInt32)m_StyleVariants.size();
}

/// Возвращает вариант стиля по номеру
/**
	@param aIndex	- номер запрашиваемого варианта
	
	@return указатель на вариант, либо NULL, если такой вариант не найден
*/
const TSlovoedStyleVariant* TSlovoedStyle::GetStyleVariant(UInt32 aIndex) const
{
	if (aIndex >= VariantsCount())
		return NULL;
	
	return &(m_StyleVariants[aIndex]);
}

/// Добавляет вариант стиля
/**
	@param aStyleVariant	- указатель на вариант стиля, который нужно добавить
	
	@return код ошибки
*/
void TSlovoedStyle::AddStyleVariant(const TSlovoedStyleVariant &aStyleVariant)
{
	m_StyleVariants.push_back(aStyleVariant);
}

/// Возвращает номер варианта для отображения по-умолчанию
UInt32 TSlovoedStyle::DefaultVariantIndex(void) const
{
	return m_DefaultVariantIndex;
}
	
/// Устанавливает номер варианта для отображения по-умолчанию
void TSlovoedStyle::SetDefaultVariantIndex(UInt32 aIndex)
{
	m_DefaultVariantIndex = aIndex;
}

/*********************************************************************************************/


/// Конструктор, производит инициализацию по-умолчанию

TSlovoedStyleManager::TSlovoedStyleManager(void)
{
	// добавляем стиль по-умолчанию
	TSlovoedStyle Style;
	TSlovoedStyleVariant StyleVariant;
	Style.AddStyleVariant(StyleVariant);
	Style.AddUsage(eStyleUsage_Unknown);

	AddStyle(Style);
}

/// Возвращает количество стилей
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

/// Добавляет один стиль
/**
	@param vStyle	- добавляемый стиль
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
 * Возвращает номер стиля по типу метаданных
 * Работает *только* для метаданных!
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

/// Возвращает номер стиля по имени
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

/// Возвращает номер стиля по-умолчанию
UInt16 TSlovoedStyleManager::GetDefaultStylesNumber(void) const
{
	return m_Styles[0].Number();
}

/// По имени тэга со стилем возвращает строку с индексом этого тега в таблице стилей
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
