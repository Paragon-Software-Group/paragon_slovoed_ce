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


/// Класс, описывающий цвет в формате RGB
class TRGBColor
{
public:

	/// Конструктор
	TRGBColor(void);
	/// Конструктор
	TRGBColor(UInt8 vRed, UInt8 vGreen, UInt8 vBlue, UInt8 vAlpha);

	/// Возвращает красную составляющую цвета
	UInt8 R(void) const;
	/// Возвращает зеленую составляющую цвета
	UInt8 G(void) const;
	/// Возвращает синюю составляющую цвета
	UInt8 B(void) const;
	/// Возвращает значение альфа-канала
	UInt8 Alpha(void) const;
	
	/// Устанавливает цвет
	void SetColor(UInt8 vRed, UInt8 vGreen, UInt8 vBlue, UInt8 vAlpha);

private:
	
	/// Красная составляющая цвета
	UInt8 m_Red;
	
	/// Синяя составляющая цвета
	UInt8 m_Green;
	
	/// Зеленая составляющая цвета
	UInt8 m_Blue;
	
	/// Альфа-канал
	UInt8 m_Alpha;
	
};


/// Класс, представляющий вариант стиля
class TSlovoedStyleVariant
{
public:

	/// Конструктор
	TSlovoedStyleVariant(void);

public:

	/// Возвращает тип варианта
	UInt32 VariantType(void) const;
	/// Устанавливает тип варианта
	void SetVariantType(UInt32 aType);

	/// Видимость
	bool Visible(void) const;
	/// Устанавливает флаг видимости
	void SetVisible(bool aState);

	/// Возвращает вертикальное положение шрифта
	ESldStyleLevelEnum FontLevel(void) const;
	/// Устанавливает вертикальное положение шрифта
	void SetFontLevel(ESldStyleLevelEnum vFontLevel);
	
	/// Возвращает цвет шрифта
	TRGBColor Color(void) const;
	/// Устанавливает цвет шрифта
	void SetColor(TRGBColor aColor);
	
	/// Возвращает цвет фона
	TRGBColor BackgroundColor(void) const;
	/// Устанавливает цвет фона
	void SetBackgroundColor(TRGBColor aColor);

	/// Устанавливает величину насыщенности (bold value, weight)
	void SetBoldValue(UInt32 aValue);
	/// Возвращает величину насыщенности (bold value, weight)
	UInt32 GetBoldValue() const;
	
	/// Наклонный шрифт
	bool Italic(void) const;
	/// Устанавливает наклонный шрифт
	void SetItalic(bool aState);
	
	/// Устанавливает стиль подчеркивания
	void SetUnderlineStyle(UInt32 aUnderlineStyle, UInt32 aUseColor, TRGBColor aColor);
	/// Подчеркнутый шрифт
	UInt32 Underline(void) const;
	/// Устанавливает подчеркнутый шрифт
	void SetUnderline(bool aState);
	/// Задан ли свой цвет для подчеркивания
	UInt32 UnderlineUseColor(void) const;
	/// цвет подчеркивания
	TRGBColor GetUnderlineColor(void) const;

	/// Перечеркнутый шрифт
	bool Strikethrough(void) const;
	/// Устанавливает перечеркнутый шрифт
	void SetStrikethrough(bool aState);
	
	/// Надчеркнутый шрифт (черта сверху текста)
	bool Overline(void) const;
	/// Устанавливает надчеркнутый шрифт
	void SetOverline(bool aState);
	
	/// Возвращает размер шрифта
	UInt32 FontSize(void) const;
	/// Устанавливает размер шрифта
	void SetFontSize(UInt32 vFontSize);
	/// Устанавливает размер шрифта
	void SetFontSize(const TSizeValue &aFontSize);
	/// Возвращает размер шрифта
	TSizeValue FontSizeValue() const;

	/// Возвращает константу, которая описывает семейство шрифта
	ESldStyleFontFamilyEnum FontFamily(void) const;
	/// Устанавливает константу, которая описывает семейство шрифта
	void SetFontFamily(ESldStyleFontFamilyEnum aFamily);

	/// Возвращает константу, которая описывает название шрифта
	ESldStyleFontNameEnum FontName(void) const;
	/// Устанавливает константу, которая описывает название шрифта
	void SetFontName(ESldStyleFontNameEnum aFontName);

	/// Возвращает высоту строки
	UInt32 LineHeight(void) const;
	/// Устанавливает высоту строки
	void SetLineHeight(UInt32 vLineHeight);
	/// Устанавливает высоту строки
	void SetLineHeight(const TSizeValue &aLineHeight);
	/// Возвращает высоту строки
	TSizeValue LineHeightValue() const;
	
	/// Возвращает префикс
	wstring Prefix(void) const;
	/// Устанавливает префикс
	void SetPrefix(wstring vText);
	
	/// Возвращает постфикс
	wstring Postfix(void) const;
	/// Устанавливает постфикс
	void SetPostfix(wstring vText);

	/// Возвращает индекс верхней фоновой картинки
	UInt32 BackgroundImageTop(void) const;
	/// Возвращает индекс нижней фоновой картинки
	UInt32 BackgroundImageBottom(void) const;
	/// Устанавливает отображаемую сверху картинку
	void SetBackgroundImageTop(UInt32 vText);
	/// Устанавливает отображаемую снизу картинку
	void SetBackgroundImageBottom(UInt32 vText);

	/// Возвращает кликабельность текста для кроссрефа
	UInt32 Unclickable(void) const;
	/// Устанавливает кликабельность текста для кроссрефа
	void SetUnclickable(UInt32 vIndex);

private:

	/// Тип варианта стиля (см. #EStyleVariantType)
	UInt32 m_VariantType;

	/// Видимость
	bool m_Visible;

	/// Вертикальное положение шрифта
	ESldStyleLevelEnum m_FontLevel;
	
	/// Цвет шрифта
	TRGBColor m_Color;
	
	/// Цвет фона
	TRGBColor m_BackgroundColor;
	
	/// Жирность
	/**
		true	- жирный шрифт
		false	- нежирный шрифт
	*/
	UInt32 m_Bold;
	
	/// Наклонность
	/**
		true	- наклонный шрифт
		false	- ненаклонный шрифт
	*/
	bool m_Italic;
	
	/// Подчеркнутость
	/// @see ESldUnderlineStyle
	UInt32 m_Underline;

	/// флаг, говорит нам о том, что для подчеркивания цвет задан отдельно 
	bool m_UnderlineUseCustomColor;

	/// Цвет для подчеркивания 
	TRGBColor m_UnderlineColor;


	/// Перечеркнутость
	/**
		true	- перечеркнутый шрифт
		false	- неперечеркнутый шрифт
	*/
	bool m_Strikethrough;
	
	/// Надчеркнутость (черта сверху текста)
	/**
		true	- надчеркнутый шрифт
		false	- ненадчеркнутый шрифт
	*/
	bool m_Overline;
	
	/// Размер шрифта (@see ESldStyleSizeEnum)
	UInt32 m_FontSize;

	/// Размер шрифта
	TSizeValue m_FontSizeValue;

	/// Размер шрифта (@see ESldStyleSizeEnum)
	UInt32 m_LineHeight;

	/// Высота строки
	TSizeValue m_LineHeightValue;

	/// Семейство шрифта
	ESldStyleFontFamilyEnum m_FontFamily;
	
	/// Название шрифта
	ESldStyleFontNameEnum m_FontName;
	
	/// Префикс
	wstring m_Prefix;
	
	/// Постфикс
	wstring m_Postfix;

	/// Если установлен данный флаг - текст размеченным данным стилем будет недоступен для кроссрефа
	UInt32 m_Unclickable;

	/// Глобальный индекс картинки, отображаемой над текстом
	UInt32 m_BackgroundImageTop;

	/// Глобальный индекс картинки, отображаемой под текстом
	UInt32 m_BackgroundImageBottom;
};


/// Класс, представляющий стиль
class TSlovoedStyle
{
public:

	/// Конструктор
	TSlovoedStyle(void);

public:
	
	/// Возвращает номер стиля
	UInt16 Number(void) const;
	/// Устанавливает номер стиля
	void SetNumber(UInt16 vNumber);

	/// Возвращает тип текста
	ESldStyleMetaTypeEnum GetTextType() const { return m_TextType; }
	/// Устанавливает тип текста
	void SetTextType(ESldStyleMetaTypeEnum aTextType) { m_TextType = aTextType; }

	/// Возвращает тег
	const wstring& Tag(void) const;
	/// Устанавливает тег
	void SetTag(wstring vTag);
	
	/// Возвращает язык стиля
	UInt32 Language(void) const;
	/// Устанавливает язык стиля
	void SetLanguage(UInt32 vLanguage);
	
	/// Возвращает количество вариантов использования стиля
	UInt32 UsageCount(void) const;
	/// Возвращает по номеру варианта константу, которая описывает вариант использование стиля
	ESldStyleUsageEnum GetUsage(UInt32 aIndex) const;
	/// Добавляет в список константу, которая описывает вариант использования стиля
	void AddUsage(ESldStyleUsageEnum vUsage);
	
	/// Возвращает количество вариантов стиля
	UInt32 VariantsCount(void) const;
	/// Возвращает вариант стиля по номеру
	const TSlovoedStyleVariant* GetStyleVariant(UInt32 aIndex) const;
	/// Добавляет вариант стиля
	void AddStyleVariant(const TSlovoedStyleVariant &aStyleVariant);
	
	/// Возвращает номер варианта для отображения по-умолчанию
	UInt32 DefaultVariantIndex(void) const;
	/// Устанавливает номер варианта для отображения по-умолчанию
	void SetDefaultVariantIndex(UInt32 aIndex);

private:

	/// Номер стиля
	UInt16 m_Number;

	/// Тип текста
	ESldStyleMetaTypeEnum m_TextType;

	/// Тег, содержит имя тега, для которого используется данный стиль
	wstring m_Tag;

	/// Язык стиля
	UInt32 m_Language;

	/// Номер варианта для отображения по-умолчанию
	UInt32 m_DefaultVariantIndex;

	/// Массив Usage - для чего используется данный стиль
	vector<ESldStyleUsageEnum> m_Usages;

	/// Варианты отображения стиля
	vector<TSlovoedStyleVariant> m_StyleVariants;
};


/// Класс, управляющий стилями словаря
class TSlovoedStyleManager
{
public:
	
	/// Конструктор
	TSlovoedStyleManager(void);
	
	/// Возвращает количество стилей
	UInt16 StylesCount(void) const;
	
	/// Добавляет один стиль
	UInt16 AddStyle(TSlovoedStyle& vStyle);

	/// Возвращает номер стиля по типу метаданных
	UInt16 GetMetadataStyleNumber(ESldStyleMetaTypeEnum aType);

	/// Возвращает номер стиля по имени
	UInt16 GetStyleNumberByName(sld::wstring_ref aTagName);
	
	/// Возвращает номер стиля по-умолчанию
	UInt16 GetDefaultStylesNumber(void) const;

	/// По имени тэга со стилем возвращает строку с индексом этого тега в таблице стилей
	wstring GetVariantStyleStr(const wstring &aStyleName) const;

private:

	UInt16 addStyle(TSlovoedStyle& vStyle);

	/// Массив стилей
	vector<TSlovoedStyle> m_Styles;

	/// Map стилей имя -> номер
	unordered_map<wstring, UInt16> m_StylesMap;

	/// Map стилей имя -> номер
	std::unordered_map<ESldStyleMetaTypeEnum, UInt16> m_MetadataStyles;

	/// "Мапка" стилей для которых был вызван ::GetStyleNumberByName()
	std::vector<bool> m_UsedStyles;

	/// Стили которые не были добавлены, но для которых был вызван ::GetStyleNumberByName()
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
