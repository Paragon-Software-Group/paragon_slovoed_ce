#pragma once

#if _MSC_VER && !__INTEL_COMPILER
  #pragma warning(disable : 4503)
#endif

/// @file sld_Types.h

#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <string.h>
#include <wchar.h>

#include <climits>
#include <string>

#ifdef _MSC_VER
#  include <winerror.h>
#elif defined(_LINUX)
#  include <unistd.h>
#endif

#include "Engine/SldPlatform.h"
#include "Engine/SldDefines.h"
#include "Engine/SldTypeDefs.h"

#ifndef ERROR_NOT_ENOUGH_MEMORY
#  define ERROR_NOT_ENOUGH_MEMORY (0x03)
#endif

#ifdef _LINUX
int _wcsicmp(const wchar_t* str1, const wchar_t* str2);
int _wcsnicmp(const wchar_t* str1, const wchar_t* str2);
wchar_t* _wcslwr(wchar_t* aStr);
#endif

/// Тип ресурса с элемантами дерева бинарного поиска для списка слов
/**
Для первого списка слов будет ATST
Для второго списка слов будет BTST
*/
#define RESOURCE_TYPE_SEARCH_TREE_POINT	'TSTA'

/// Тип ресурса для хранения количества переводов статей на которые ссылаются слова из списка слов
/** 
*	Для первого списка слов будет AIND
*	Для второго списка слов будет BIND
*/
#define RESOURCE_TYPE_INDEXES_COUNT		'CNIA'

/// Тип ресурса для хранения номеров статей на которые ссылаются слова из списка слов
/** 
*	Для первого списка слов будет AIND
*	Для второго списка слов будет BIND
*/
#define RESOURCE_TYPE_INDEXES_DATA		'DNIA'

/// Тип ресурса для хранения заголовка индексов
/** 
*	Для первого списка слов будет AIQA
*	Для второго списка слов будет BIQA
*/
#define RESOURCE_TYPE_INDEXES_HEADER	'HNIA'

/// Тип ресурса для хранения таблицы быстрого доступа для индексов (RESOURCE_TYPE_INDEXES_COUNT и RESOURCE_TYPE_INDEXES_DATA)
/** 
*	Для первого списка слов будет AIQA
*	Для второго списка слов будет BIQA
*/
#define RESOURCE_TYPE_INDEXES_QA		'AQIA'

/// Тип ресурса, хранящего информацию об альтернативных заголовках
/**
*	Для первого списка слов будет AAHI
*	Для второго списка слов будет BAHI
*/
#define RESOURCE_TYPE_ALTERNATIVE_HEADWORD_INFO	'IHAA'

/// Тип ресурса для сжатых данных статей
#define RESOURCE_TYPE_ARTICLE_DATA		'DTRA'
/// Тип ресурса для описания стиля текста
#define RESOURCE_TYPE_ARTICLE_STYLE		'STRA'
/// Тип ресурса для дерева сжатия статей
#define RESOURCE_TYPE_ARTICLE_TREE		'TTRA'
/// Тип ресурса для таблицы быстрого доступа статей
#define RESOURCE_TYPE_ARTICLE_QA		'QTRA'


/// Способы сжатия данных
enum ESldCompressionType
{
	/// Не сжатые данные, записаны напрямую в виде текста, никакого декодирования не нужно.
	eCompressionTypeNoCompression = 0,

};

/// Варианты сжатия, по соотношению время сжатия/качество сжатия
enum ECompressionMode
{
	/// Быстрое сжатие
	eCompressionFast = 0,
	/// Среднее сжатие
	eCompressionMedium,
	/// Хорошее сжатие
	eCompressionGood,
	/// Оптимальное сжатие
	eCompressionOptimal,
	/// ??? сжатие
	eCompression4,
	/// Полное сжатие
	eCompressionFull,
	/// Очень долгое сжатие. Не применять
	eCompressionExtraFull
};

struct CompressionConfig
{
	ESldCompressionType type;
	ECompressionMode mode;

	CompressionConfig() : type(eCompressionTypeNoCompression), mode(eCompressionFast) {}
	CompressionConfig(ESldCompressionType aType, ECompressionMode aMode) : type(aType), mode(aMode) {}
};


/// Номер дерева для (де)кодирования списка блоков.
#define SLD_TREE_BLOCKTYPE		(0)

/// Номер дерева для (де)кодирования количества символов которые равны у текущего и предыдущего слов.
#define SLD_TREE_LIST_SHIFT	(0)
/// Номер дерева для (де)кодирования текстовых данных - остаток слова
#define SLD_TREE_TEXT	(1)


/// Язык с которого производится перевод
#define SLD_LANGUAGE_FROM	(0)

/// Язык на который производится перевод
#define SLD_LANGUAGE_TO		(1)


/// Версия индексов
#define SLD_INDEXES_VERSION		(1)

/// Код языка для общей для всех языков таблицы символов-разделителей
#define SLD_SYMBOLS_DELIMITERS_LANGUAGE_CODE	(0)

/// Путь кодирования не найден.
#define CHAR_CHAIN_NO_PATH_FOUND			(0xffffffff)

/// Путь кодирования не найден.
#define CHAR_CHAIN_MAXIMUM_SIZE				(0xffffffff)

/// Количество итераций для попыток преодолеть битовую границу.
#define CHAR_CHAIN_FORCE_BIT_BORDER_COUNT	(32)

/// Максимальный размер префикса для стиля в символах (UInt16)
#define SLD_MAX_STYLE_PREFIX_SIZE			(16)
/// Максимальный размер постфикса для стиля в символах (UInt16)
#define SLD_MAX_STYLE_POSTFIX_SIZE			(16)

/// Максимальный размер дерева
#define SLD_MAX_HEADER_SIZE					(64*1024-16)

#define SLD_TABLE_SPLIT_VERSION				(4)

/// Количество слов между точками быстрого доступа по умолчанию
#define SLD_DEFAULT_QA_INTERLEAVE			(128)

/// Количество бит которые закодированы в индексе по умолчанию
#define SLD_DEFAULT_QA_INTERLEAVE_SIZE		(1024)

/// Минимальное расстояние между точками быстрого доступа
#define SLD_DEFAULT_MIN_QA_INTERLEAVE_SIZE	(16)

/// Минимальное количество слов, необходимое для добавления дерева быстрого поиска
#define SLD_MIN_WORDS_COUNT_FOR_QUICK_SEARCH	(512)


/// Структура, описывающая свойства одного варианта написания списка слов
typedef struct TListVariantProperty
{
	/// Номер варианта написания
	UInt32 Number;
	/// Тип варианта написания (см. #EListVariantTypeEnum)
	UInt32 Type;
	/// Код языка
	UInt32 LangCode;
	/// Зарезервировано
	UInt32 Reserved[5];

}TListVariantProperty;

#define SLD_SIZE_VALUE_SCALE (100)

/// Структурка отвечающая за размеры
typedef struct TSizeValue
{
	/// Число
	/// ВАЖНО! Число хранится в умноженном на SLD_SIZE_VALUE_SCALE (100) виде.
	/// Т.е. 1 хранится как 100, 1.01 - 101, .01 - 1 и т.п.
	Int32 Value;
	/// Единица измерения
	UInt32 Units;

	/// Создает TSizeValue с дефолтными значениями (невалидными)
	TSizeValue() : Value(-1), Units(eMetadataUnitType_UNKNOWN) {}

	/// Создает TSizeValue c переданными значениями
	/// ВАЖНО! aValue должен быть "скалированным" - т.е. 320 вместо 3.2, 100 вместо 1 и т.п.
	TSizeValue(Int32 aValue, EMetadataUnitType aUnits) : Value(aValue), Units(aUnits) {}

	/// Присваивает заданные значения
	void Set(Int32 aValue, EMetadataUnitType aUnits) { Value = aValue; Units = aUnits; }

	/// Возвращает является ли размер "валидным"
	bool IsValid() const { return Units < eMetadataUnitType_end; }

	/// Возврщает размер в виде "скалированного" Float32
	Float32 AsFloat32() const { return (Float32)Value / (Float32)SLD_SIZE_VALUE_SCALE; }

	/// Возвращает целую часть размера
	Int32 Integer() const { return Value / SLD_SIZE_VALUE_SCALE; }

	/// Возвращает дробную часть размера - от 0 до 100
	UInt32 Fractional() const { return (Value >= 0 ? Value : -Value) % SLD_SIZE_VALUE_SCALE; }

} TSizeValue;

inline bool operator==(const TSizeValue &lhs, const TSizeValue &rhs)
{
	return lhs.Units == rhs.Units && lhs.Value == rhs.Value;
}
inline bool operator!=(const TSizeValue &lhs, const TSizeValue &rhs)
{
	return !(lhs == rhs);
}

/// Признак того что в стиль зашиты размеры с единицами измерения
#define SIZE_VALUE_UNITS	(0xFFFFFFFFUL)

/// Заголовок варианта стиля
typedef struct TStyleVariantHeader
{
	/// Размер структуры в байтах
	UInt32 structSize;
	
	/// Тип варианта (см. #EStyleVariantType)
	UInt32 VariantType;
	
	/// Флаг, если установлен, значит текст видимый
	UInt32 Visible;
	
	/// Информация, как должен восприниматься текст (см. #TTextType)
	/// Если тип - метаданные, то в тексте содержатся только 
	/// конкретные данные для обработки - например ссылки или еще что-то
	UInt32 TextType;
	
	/// Выравнивание текста по высоте (см. #TFontLevel)
	UInt32 Level;
	
	/// Величина красной компоненты цвета шрифта
	UInt32 ColorRed;
	/// Величина зеленой компоненты цвета шрифта
	UInt32 ColorGreen;
	/// Величина синей компоненты цвета шрифта
	UInt32 ColorBlue;
	/// Значение альфа-канала
	UInt32 ColorAlpha;

	/// Величина красной компоненты цвета фона
	UInt32 BackgroundColorRed;
	/// Величина зеленой компоненты цвета фона
	UInt32 BackgroundColorGreen;
	/// Величина синей компоненты цвета фона
	UInt32 BackgroundColorBlue;
	/// Значение альфа-канала
	UInt32 BackgroundColorAlpha;
	
	/// Стиль насыщенности
	UInt32 Bold;

	/// Флаг, если установлен, значит текст написан наклонным
	UInt32 Italic;

	/// Стиль подчеркивания
	UInt32 Underline;
	
	/// Флаг, если установлен, значит текст перечеркнут
	UInt32 Strikethrough;

	/// Размер текста (см. #TFontSize)
	/// Число больше 5 - конкретный размер текста
	/// Если == SIZE_VALUE_UNITS => смотри TextSizeValue
	UInt32 TextSize;

	/// Высота строки (см. #TFontSize)
	/// Число больше 5 - абсолютное значение
	/// Если == SIZE_VALUE_UNITS => смотри LineHeightValue
	UInt32 LineHeight;

	/// Семейство шрифта (см. #ESldStyleFontFamilyEnum)
	UInt32 FontFamily;

	/// Название шрифта (см. #ESldStyleFontNameEnum)
	UInt32 FontName;
	
	/// Префикс
	UInt16 Prefix[SLD_MAX_STYLE_PREFIX_SIZE + 1];
	
	/// Постфикс
	UInt16 Postfix[SLD_MAX_STYLE_POSTFIX_SIZE + 1];

	/// Флаг, если установлен, значит текст надчеркнут
	UInt32 Overline;

	/// Индекс верхней фоновой картинки
	Int32 BackgroundTopImageIndex;

	/// Индекс нижней фоновой картинки
	Int32 BackgroundBottomImageIndex;

	/// Если установлен данный флаг - текст размеченным данным стилем будет недоступен для кроссрефа
	UInt32 Unclickable;

	/// Флаг того, что для подчеркивания нужно использовать свой, отдельно определенный цвет
	UInt32 UnderlineUseCustomColor;

	/// Красная составляющая цвета для подчеркивания 
	UInt32 UnderlineColorRed;

	/// Зеленая составляющая цвета для подчеркивания 
	UInt32 UnderlineColorGreen;

	/// Синяя составляющая цвета для подчеркивания 
	UInt32 UnderlineColorBlue;

	/// Альфа составляющая цвета для подчеркивания 
	UInt32 UnderlineColorAlpha;

	/// Размер текста с явно заданными единицами измерения
	TSizeValue TextSizeValue;

	/// Высота строки текста с явно заданными единицами измерения
	TSizeValue LineHeightValue;

	/// Зарезервировано
	UInt32 Reserved[14];

} TStyleVariantHeader;


/// Маска, которая отвечает за признак наличия в таблице сравнения таблицы пар соответствий символов верхнего и нижнего регистров
#define SLD_CMP_TABLE_FEATURE_SYMBOL_PAIR_TABLE_MASK	(0x1)

typedef struct WrongCMPType
{
	unsigned short	ch; 
	unsigned short	reserved1;
	int				Line;
}WrongCMPType;

/// Таблица сравнения, разобранная на основные части
typedef struct TCompareTableSplit
{
	/// Указатель на заголовок таблицы сравнения
	CMPHeaderType				*Header;

	/// Указатель на массив весов простых символов(одна масса на один символ)
	CMPSimpleType				*Simple;

	/// Указатель на массив сложных символов
	CMPComplexType				*Complex;

	/// Указатель на массив символов разделителей
	CMPDelimiterType			*Delimiter;

	/// Указатель на массив родных символов языка
	CMPNativeType				*Native;

	/// Указатель на массив частичных разделителей
	CMPHalfDelimiterType		*HalfDelimiter;

	/// Указатель на заголовок таблицы соответствия символов в верхнем и нижнем регистрах
	CMPSymbolPairTableHeader	*HeaderPairSymbols;

	/// Указатель на массив пар соответствий символов в верхнем и нижнем регистрах для данного языка
	CMPSymbolPair				*NativePair;

	/// Указатель на массив пар соответствий символов в верхнем и нижнем регистрах для всех символов, которые мы знаем
	CMPSymbolPair				*CommonPair;

	/// Указатель на массив пар соответствий символов в верхнем и нижнем регистрах для символов, встречающихся в списках слов данного словаря
	CMPSymbolPair				*DictionaryPair;

	/// Таблица масс символов
	UInt16						*SimpleMassTable;

	/// Таблица свойств наиболее часто используемых символов
	UInt8						*MostUsageCharTable;

	/// Размер всей таблицы
	UInt32						TableSize;

}TCompareTableSplit;

/// Перечисление, описывающее, каким образом происходят ссылки в списке слов полнотекстового поиска
enum EFullTextSearchLinkType
{
	/// Ссылки указывают на id статей
	eLinkType_ArticleId = 0,
	/// Ссылки указывают на id записей в списке слов
	eLinkType_ListEntryId,
	/// Неизвестный тип
	eLinkType_Unknown = 0xFFFF
};

/// У слова в словарном списке есть точное соответсвие
#define		SLD_SIMPLE_SORTED_NORMAL_WORD		(1)
/// У слова в словарном списке нет точного соответсвия
#define		SLD_SIMPLE_SORTED_VIRTUAL_WORD		(-1)

/// Информация о ресурсе
struct TResourceInfo
{
	/// Индекс имени (обычно путь к файлу), привязанного к ресурсу.
	UInt32 NameIndex;
};

#define ATOMIC_ID_PREFIX		(L"AtomicArticle_")
