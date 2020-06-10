#ifndef _SLD_COMPARE_H_
#define _SLD_COMPARE_H_

#include "SldError.h"
#include "SldDefines.h"
#include "SldPlatform.h"
#include "SldTypes.h"
#include "SldSDCReadMy.h"
#include "SldSymbolsTable.h"
// TODO: избавиться от разного уровня вложений в include
//#include "../DataReader.h"

/// Максимальная длина строки со значением параметра
static const UInt32 MetaParamMaxValSize = 1024;

/// Таблица сравнения разобранная на основные части
typedef struct TCompareTableSplit
{
	/// Указатель на заголовок таблицы сравнения
	CMPHeaderType									*Header;

	/// Указатель на массив весов простых символов(одна масса на один символ)
	CMPSimpleType									*Simple;

	/// Указатель на массив сложных символов
	CMPComplexType									*Complex;

	/// Указатель на массив символов разделителей
	CMPDelimiterType								*Delimiter;

	/// Указатель на массив родных символов языка
	CMPNativeType									*Native;

	/// Указатель на массив частичных разделителей
	CMPHalfDelimiterType							*HalfDelimiter;

	/// Указатель на заголовок таблицы соответствия символов в верхнем и нижнем регистрах
	CMPSymbolPairTableHeader						*HeaderPairSymbols;

	/// Указатель на массив пар соответствий символов в верхнем и нижнем регистрах для данного языка
	CMPSymbolPair									*NativePair;

	/// Указатель на массив пар соответствий символов в верхнем и нижнем регистрах для всех символов, которые мы знаем
	CMPSymbolPair									*CommonPair;

	/// Указатель на массив пар соответствий символов в верхнем и нижнем регистрах для символов, встречающихся в списках слов данного словаря
	CMPSymbolPair									*DictionaryPair;

	/// Таблица масс символов
	sld2::Array<UInt16, MAX_UINT16_VALUE>			SimpleMassTable;

	/// Таблица свойств наиболее часто используемых символов
	sld2::Array<UInt8, CMP_MOST_USAGE_SYMBOL_RANGE>	MostUsageCharTable;

	/// Массив символов, сортированных по массе
	sld2::DynArray<Int16>							SortedMass;

	/// Размер всей таблицы
	UInt32											TableSize;
}TCompareTableSplit;

/// Маска для флага - определяет что это - вес символа или индекс таблицы сложных символов.
#define CMP_MASK_OF_INDEX_FLAG		(0x8000)
/// Маска для индекса - в случае, если это индекс, убирает флаг, оставляя чистый индекс.
#define CMP_MASK_OF_INDEX			(0x7fff)
/// Идентификатор символа игнорирования.
#define CMP_IGNORE_SYMBOL			(0)
/// Идентификатор символа который не найден.
#define CMP_NOT_FOUND_SYMBOL		(0xffff)

/// Код символа-разделителя в списке слов(используется в китайском).
#define CMP_DELIMITER				(9)

/// Символ по умолчанию для замены тех символов которые не найдены.
#define CMP_DEFAULT_CHAR			(0x98)

/// Масса символа для нулевых символов(в GetStrOfMass())
#define CMP_MASS_ZERO				(0x7A00)
/// Масса символа для разделителей(в GetStrOfMass())
#define CMP_MASS_DELIMITER			(0x7A01)
/// Виртуальная масса нуля
#define CMP_MASS_ZERO_DIGIT			(0x7A10)

/// Символ для экранирования спец. символов в поисковом запросе
#define CMP_QUERY_SPECIAL_SYMBOL_ESCAPE_CHAR	'%'
/// Спец. символ в поисковом запросе - операция И
#define CMP_QUERY_SPECIAL_SYMBOL_AND			'&'
/// Спец. символ в поисковом запросе - операция ИЛИ
#define CMP_QUERY_SPECIAL_SYMBOL_OR				'|'
/// Спец. символ в поисковом запросе - операция НЕ
#define CMP_QUERY_SPECIAL_SYMBOL_NOT			'!'
/// Спец. символ в поисковом запросе - открывающая скобка
#define CMP_QUERY_SPECIAL_SYMBOL_OPEN_BR		'('
/// Спец. символ в поисковом запросе - закрывающая скобка
#define CMP_QUERY_SPECIAL_SYMBOL_CLOSE_BR		')'
/// Спец. символ в поисковом запросе - последовательность любых символов
#define CMP_QUERY_SPECIAL_SYMBOL_ANY_CHARS		'*'
/// Спец. символ в строке масс - последовательность любых символов
#define CMP_MASS_SPECIAL_SYMBOL_ANY_CHARS		(0x7B01)
/// Спец. символ в поисковом запросе - любой символ
#define CMP_QUERY_SPECIAL_SYMBOL_ONE_CHAR		'?'
/// Спец. символ в строке масс - любой символ
#define CMP_MASS_SPECIAL_SYMBOL_ONE_CHAR		(0x7B02)

/// Версия таблицы сравнения занимающая небольшое место.
#define CMP_VERSION_1		1
/// Версия таблицы сравнения имеющая прямое отображение символов в их веса.
#define CMP_VERSION_2		2

/// Макрос определения достигнут ли конец строки
#define CMP_IS_EOL(header, str)		(*(str)==0 || ((header)->EOL==*(str)))

/// Расчет номера элемента
#define TIO(X, Y)	X][Y

/// Определение наименьшего из 3 символов
#define sldMin3(i1, i2, i3)			( sldMin2((i1), sldMin2((i2), (i3)))  )
/// Определение наименьшего из 2 символов
#define sldMin2(i1, i2)				( (i1)<(i2) ? (i1) : (i2))

/// Проверяет, является ли символ числом
#define CMP_IS_DIGIT(ch)			( ((ch) > 0x2F) && ((ch) < 0x3A) ? 1 : 0)
/// Возвращает виртуальную массу символа
#define CMP_VIRTUAL_DIGIT_MASS(ch)	( CMP_MASS_ZERO_DIGIT + (ch) - 0x30 )

// шаблонные функции по работе со строками
namespace sld2 {

/**
 * Определяет длину строки
 *
 * @param[in]  aStr - указатель на строку
 *
 * @return длина строки или 0, если передан нулевой указатель на строку
 */
template <typename Char>
UInt32 StrLen(const Char *aStr)
{
	return aStr ? char_traits<Char>::length(aStr) : 0;
}

/**
 * Бинарное сравнение 2 строк
 *
 * @param[in]  aStr1 - указатель на первую из сравниваемых строк
 * @param[in]  aStr2 - указатель на вторую из сравниваемых строк
 *
 * @return   0 - строки равны
 *          >0 - первая строка больше второй
 *          <0 - первая строка меньше второй
 */
template <typename Char>
Int32 StrCmp(const Char *aStr1, const Char *aStr2)
{
	if (!aStr1 || !aStr2)
		return 0;

	for (; *aStr1 == *aStr2; aStr1++, aStr2++)
	{
		if (*aStr1 == 0)
			return 0;
	}
	return char_traits<Char>::cmp(*aStr1, *aStr2);
}

/**
 * Сравнение 2 строк с учетом регистра с максимальным количеством сравниваемых символов
 *
 * @param[in]  aStr1  - указатель на первую из сравниваемых строк
 * @param[in]  aStr2  - указатель на вторую из сравниваемых строк
 * @param[in]  aCount - максимальное количество символов для сравнения
 *
 * @return   0 - строки равны
 *          >0 - первая строка больше второй
 *          <0 - первая строка меньше второй
 */
template <typename Char>
Int32 StrNCmp(const Char *aStr1, const Char *aStr2, UInt32 aCount)
{
	if (!aStr1 || !aStr2)
		return 0;

	for (; aCount; aStr1++, aStr2++, --aCount)
	{
		Int32 r = sld2::char_traits<Char>::cmp(*aStr1, *aStr2);
		if (r != 0)
			return r;
		if (*aStr1 == 0)
			break;
	}
	return 0;
}

/**
 * Возвращает равны ли 2 строки
 *
 * @param[in]  aStr1 - указатель на первую из сравниваемых строк
 * @param[in]  aStr2 - указатель на вторую из сравниваемых строк
 */
template <typename Char>
bool StrEqual(const Char *aStr1, const Char *aStr2)
{
	return StrCmp(aStr1, aStr2) == 0;
}

/**
 * Возвращает равны ли 2 строки
 *
 * @param[in]  aStr1  - указатель на первую из сравниваемых строк
 * @param[in]  aStr2  - указатель на вторую из сравниваемых строк
 * @param[in]  aCount - максимальное количество символов для сравнения
 */
template <typename Char>
bool StrEqual(const Char *aStr1, const Char *aStr2, UInt32 aCount)
{
	return StrNCmp(aStr1, aStr2, aCount) == 0;
}

/**
 * Копирует строку
 *
 * @param[out]  aDest   - указатель на буфер, куда будем копировать
 * @param[in]   aSource - указатель на строку, которую будем копировать
 *
 * @return количество скопированных символов (исключая nul-terminator)
 */
template <typename Char>
UInt32 StrCopy(Char* aDest, const Char* aSource)
{
	if (!aDest || !aSource)
		return 0;

	UInt32 count = 0;
	for (; *aSource; aSource++, aDest++, count++)
		*aDest = *aSource;
	*aDest = 0;

	return count;
}

/**
 * Копирует строку длиной с максимальным количеством копируемых символов
 *
 * @param[out]  aDest   - указатель на буфер, куда будем копировать
 * @param[in]   aSource - указатель на строку, которую будем копировать
 * @param[in]   aCount  - максимальное количество символов для копирования
 *
 * @return количество скопированных символов (исключая nul-terminator)
 */
template <typename Char>
UInt32 StrNCopy(Char* aDest, const Char* aSource, UInt32 aCount)
{
	if (!aDest || !aSource)
		return 0;

	UInt32 count = 0;
	for (; *aSource && count < aCount; aSource++, aDest++, count++)
		*aDest = *aSource;
	if (count < aCount)
		*aDest = 0;

	return count;
}

/**
 * Ищет указанный символ в строке
 *
 * @param[in]  aStr - указатель на строку
 * @param[in]  aChr - символ
 *
 * @return если символ найден, тогда указатель на его положение в строке, иначе nullptr
 */
template <typename Char>
const Char* StrChr(const Char *aStr, Char aChr)
{
	if (!aStr)
		return nullptr;

	for (; *aStr != aChr; aStr++)
	{
		if (*aStr == 0)
			return nullptr;
	}
	return aStr;
}
template <typename Char>
Char* StrChr(Char *aStr, Char aChr)
{
	return (Char*)StrChr((const Char*)aStr, aChr);
}

/**
 * Ищет указанную подстроку в строке
 *
 * @param[in]  aSource - строка, в которой происходит поиск
 * @param[in]  aStr    - подстрока, которую ищем
 *
 * @return  указатель на первое вхождение искомой подстроки в строку, или nullptr,
 *          если подстрока не найдена. Если aStr == nullptr, возвращает aSource
 */
template <typename Char>
const Char* StrStr(const Char *aSource, const Char *const aStr)
{
	if (!aSource || !aStr)
		return aSource;

	if (*aStr == 0)
		return aSource;

	// the outer loop finds the first character of aStr in aSource
	// the inner one compares the rest of the substring on match
	for (; *aSource; aSource++)
	{
		if (*aSource != *aStr)
			continue;

		const Char *substring = aStr;
		for (const Char *string = aSource; ; string++, substring++)
		{
			if (*substring == 0)
				return aSource;

			if (*string != *substring)
				break;
		}
	}
	return nullptr;
}
template <typename Char>
Char* StrStr(Char *aSource, const Char *const aStr)
{
	return (Char*)((const Char*)aSource, aStr);
}

/**
 * Переворачивает кусок строки на месте (основано на Kernighan & Ritchie's "Ansi C")
 *
 * @param  aBegin - начало переворачиваемого участка
 * @param  aEnd   - конец переворачиваемого участка
 */
template <typename Char>
void StrReverse(Char *aBegin, Char *aEnd)
{
	if (!aBegin || !aEnd)
		return;

	for (; aBegin < aEnd; aBegin++, aEnd--)
	{
		Char chr = *aEnd;
		*aEnd = *aBegin;
		*aBegin = chr;
	}
}

} // namespace sld2

/// Класс работы со строками.
class CSldCompare
{
public:

	/// Конструктор
	CSldCompare(void);
	
	/// Конструктор копирования
	CSldCompare(const CSldCompare& aRef);
	
	/// Оператор присваивания
	CSldCompare& operator=(const CSldCompare& aRef);
	
	/// Деструктор
	~CSldCompare(void);
	
public:
	
	/// Инициализация
	ESldError Open(CSDCReadMy &aData, UInt32 aLanguageSymbolsTableCount, UInt32 aLanguageDelimiterSymbolsTableCount);
	
	
	/// Возвращает количество записей в таблице соответствия символов верхнего и нижнего регистров для текущей таблицы сравнения
	Int32 GetSymbolPairTableElementsCount(ESymbolPairTableTypeEnum aSymbolTable) const;
	
	/// Возвращает код символа в верхнем регистре по номеру записи и типу таблицы в таблице соответствия символов верхнего и нижнего регистров
	/// для текущей таблицы сравнения
	UInt16 GetUpperSymbolFromSymbolPairTable(Int32 aIndex, ESymbolPairTableTypeEnum aSymbolTable) const;
	
	/// Возвращает код символа в нижнем регистре по номеру записи и типу таблицы в таблице соответствия символов верхнего и нижнего регистров
	/// для текущей таблицы сравнения
	UInt16 GetLowerSymbolFromSymbolPairTable(Int32 aIndex, ESymbolPairTableTypeEnum aSymbolTable) const;
	
	
	/// Преобразует символ к верхнему регистру
	UInt16 ToUpperChr(UInt16 aChr) const;
	
	/// Преобразует символ к нижнему регистру
	UInt16 ToLowerChr(UInt16 aChr) const;
	
	/// Преобразует строку к верхнему регистру
	ESldError ToUpperStr(const UInt16* aStr, UInt16* aOutStr) const;
	
	/// Преобразует строку к нижнему регистру
	ESldError ToLowerStr(const UInt16* aStr, UInt16* aOutStr) const;
	
	/// Возвращает строку символов, имеющих во всех таблицах сортировки тот же вес, что и переданный символ
	UInt16* GetSimilarMassSymbols(UInt16 aCh) const;

	/// Возвращает строку символов, имеющих в таблице сортировки с заданным индексом тот же вес, что и переданный символ
	UInt16* GetSimilarMassSymbols(UInt16 aCh, UInt32 aTableIndex) const;

	/// Сравнение 2 строк по таблице сравнения (сравниваются веса символов), с указанием конкретной таблицы.
	Int32 StrICmp(const UInt16 *str1, const UInt16 *str2, UInt32 aTableIndex) const;

	/// Сравнение 2 строк по таблице сравнения (сравниваются веса символов)
	Int32 StrICmp(const UInt16 *str1, const UInt16 *str2) const;

	/// Сравнение 2 строк по таблице сравнения (сравниваются веса символов), с указанием конкретной таблицы.
	Int32 StrICmp(SldU16StringRef str1, SldU16StringRef str2, UInt32 aTableIndex) const;

	/// Сравнение 2 строк по таблице сравнения (сравниваются веса символов)
	Int32 StrICmp(SldU16StringRef str1, SldU16StringRef str2) const;

	
	Int32 StrICmpByLanguage(const UInt16 *str1, const UInt16 *str2, ESldLanguage aLanguageCode) const;

	/// Бинарное сравнение 2 строк
	static Int32 StrCmp(const UInt16 *str1, const UInt16 *str2);

	/// Определяет длину строки
	static UInt32 StrLen(const UInt16 *aStr);
	
	/// Копирует строку
	static UInt32 StrCopy(UInt16* aStrDest, const UInt16* aStrSource);

	/// Возвращает количество символов в строке, имеющих массу
	UInt32 StrEffectiveLen(const SldU16StringRef aStrSource, Int8 aEraseNotFoundSymbols = 1) const;

	/// Копирует строку игнорируя символы с нулевой массой
	UInt32 StrEffectiveCopy(UInt16* aStrOut, const UInt16* aStrSource, Int8 aEraseNotFoundSymbols = 1) const;

	/// Копирует строку игнорируя символы с нулевой массой
	void GetEffectiveString(const UInt16* aStrSource, SldU16String & aEffectiveString, Int8 aEraseNotFoundSymbols = 1) const;

	/// Копирует строку игнорируя символы с нулевой массой
	SldU16String GetEffectiveString(const SldU16StringRef aStrSource, Int8 aEraseNotFoundSymbols = 1) const;

	/// Копирует строку, игнорируя разделители в начале и конце строки
	SldU16String TrimDelimiters(const SldU16StringRef aStrSource) const;
	/// Возвращает SldU16StringRef без разделители в начале и конце строки
	SldU16StringRef TrimDelimitersRef(const SldU16StringRef aStrSource) const;

	/// Копирует строку, игнорируя нулевые символы в начале и конце строки
	SldU16String TrimIngnores(const SldU16StringRef aStrSource) const;
	/// Возвращает SldU16StringRef без нулевых символов в начале и конце строки
	SldU16StringRef TrimIngnoresRef(const SldU16StringRef aStrSource) const;

	/// Обмен двух элементов в сортируемом массиве
	static void Swap(UInt16 *aStr, Int32 aFirstIndex, Int32 aSecondIndex);
	
	/// Быстрая сортировка
	static void DoQuickSort(UInt16 *aStr, Int32 aFirstIndex, Int32 aLastIndex);
	
	/// Сравнение 2 строк с учетом регистра
	static Int32 StrCmpA(const UInt8 *str1, const UInt8 *str2);

	/// Определяет длину строки
	static Int32 StrLenA(const UInt8 *str);

	/// Копирует строку
	static UInt32 StrCopyA(UInt8* aStrDest, const UInt8* aStrSource);

	/// Копирует строку длиной не более N
	static UInt32 StrNCopyA(UInt8* aStrDest, const UInt8* aStrSource, UInt32 aSize);

	/// Копирует строку длиной не более N
	static UInt32 StrNCopy(UInt16* aStrDest, const UInt16* aStrSource, UInt32 aSize);

	/// Ищет указанный символ в строке
	static UInt8* StrChrA(UInt8* aStr, UInt8 aChr);
	
	/// Ищет указанную подстроку в строке
	static const UInt16* StrStr(const UInt16* aSourceStr, const UInt16* aSearchStr);

	/// Ищет указанную подстроку в строке
	static const UInt8* StrStrA(const UInt8* aSourceStr, const UInt8* aSearchStr);

	/// Конвертирует строку из UTF8 в UTF16
	static UInt16 StrUTF8_2_UTF16(UInt16* unicode, const UInt8* utf8);
	
	/// Конвертирует строку из UTF8 в UTF32
	static UInt16 StrUTF8_2_UTF32(UInt32* unicode, const UInt8* utf8);
	
	/// Конвертирует строку из UTF16 в UTF8
	static UInt16 StrUTF16_2_UTF8(UInt8* utf8, const UInt16* unicode);
	
	/// Конвертирует строку из UTF16 в UTF32
	static UInt16 StrUTF16_2_UTF32(UInt32* unicode32, const UInt16* unicode16);
	
	/// Конвертирует строку из UTF32 в UTF8
	static UInt16 StrUTF32_2_UTF8(UInt8* utf8, const UInt32* unicode);
	
	/// Конвертирует строку из UTF32 в UTF16
	static UInt16 StrUTF32_2_UTF16(UInt16* unicode16, const UInt32* unicode32);

	/// Переворачивает кусок строки на месте
	static ESldError StrReverse(UInt16* aBegin, UInt16* aEnd);

	/// Функция преобразования из юникода в однобайтовую кодировку с учетом языка на котором предполагается фраза.
	static ESldError Unicode2ASCIIByLanguage(const UInt16* aUnicode, UInt8* aAscii, ESldLanguage aLanguageCode);
	/// Функция преобразования из однобайтовой кодировки в юникод с учетом языка на котором предполагается фраза.
	static ESldError ASCII2UnicodeByLanguage(const UInt8* aAscii, UInt16* aUnicode, ESldLanguage aLanguageCode);

	/// Получает число из его текстового представления
	static ESldError StrToInt32(const UInt16* aStr, UInt32 aRadix, Int32* aNumber);

	/// Получает дестичное число с плавающей точкой из строки
	static ESldError StrToFloat32(const UInt16 *aStr, const UInt16 **aEnd, Float32 *aNumber);

	/// Получает число из его текстового представления
	static ESldError StrToUInt32(const UInt16* aStr, UInt32 aRadix, UInt32* aNumber);

	/// Получает строку из числа
	static ESldError UInt32ToStr(UInt32 aNumber, UInt16* aStr, UInt32 aRadix = 10);

	/// Получает число с которого начинается строка
	static ESldError StrToBeginInt32(const UInt16* aStr, UInt32 aRadix, Int32* aNumber);

	/// Закрытие объекта
	ESldError Close(void);
	
	/// Возвращает количество таблиц сравнения
	ESldError GetTablesCount(UInt32* aCount) const;
	
	/// Возвращает код языка таблицы сравнения по номеру таблицы
	ESldLanguage GetTableLanguage(UInt32 aTableIndex) const;
	
	/// Возвращает флаг того, есть ли в таблице сравнения таблица пар соответствий символов верхнего и нижнего регистров определенного типа
	ESldError IsTableHasSymbolPairTable(UInt32 aTableIndex, ESymbolPairTableTypeEnum aTableType, UInt32* aFlag) const;
	
	/// Проверяет, принадлежит ли символ определеленному языку,
	/// либо к общим символам-разделителям для всех языков словарной базы (aLang == SldLanguage::Delimiters)
	ESldError IsSymbolBelongToLanguage(UInt16 aSymbolCode, ESldLanguage aLangCode, UInt32* aFlag, UInt32* aResultFlag) const;
	
	/// Проверяет, является ли символ разделителем в конкретном языке
	ESldError IsSymbolBelongToLanguageDelimiters(UInt16 aSymbolCode, ESldLanguage aLang, UInt32* aFlag, UInt32* aResultFlag) const;
		

	/// Устанавливает таблицу сравнения для указанного языка
	ESldError SetDefaultLanguage(ESldLanguage aLanguageCode);

	/// Возвращает код языка по умолчанию
	ESldLanguage GetDefaultLanguage() const;
	
	/// Имеется ли дополнительная таблица сравнения для текущего языка
	bool IsAddTableDefine() const;

	/// Возвращает индекс дополнительной таблицы сравнения для текущего языка
	UInt32 GetAddTableIndex() const { return m_DefaultAddTable; };

	/// Метод подготавливает строку для сравнения по шаблону
	UInt32 PrepareTextForAnagramSearch(UInt16* aDestStr, const UInt16* aSourceStr);
	/// Кодирует спец. символы в строке запроса ('&', '|', '(', ')', '!', '*', '?')
	static ESldError EncodeSearchQuery(UInt16* aDestStr, const UInt16* aSourceStr);
	/// Кодирует спец. символы в слове из словаря ('&', '|', '(', ')', '!', '*', '?')
	static ESldError EncodeSearchWord(UInt16* aDestStr, const UInt16* aSourceStr);
	
	/// Метод сравнения шаблона и строчки на соответствие. 
	UInt32 WildCompare(const UInt16* aWildCard, const UInt16* aText) const;

	UInt8 GetCompareLen(const UInt16* aWildCard, const UInt16* aText) const;
	/// Метод сравнения анаграмм
	UInt8 AnagramCompare(UInt16* aSearchStr, const UInt16* aCurrentWord, UInt8* aFlagArray, UInt32 aSearchStrLen) const;

	/// Метод проверяет, есть ли в строке разделители
	UInt32 QueryIsExistDelim(const UInt16* aStr);
	
	/// Метод проверяет, является ли поисковый запрос "умным"
	static UInt32 IsSmartWildCardSearchQuery(const UInt16* aStr);
	/// Метод проверяет наличие в строке символов '*' и '?'
	static UInt32 IsWordHasWildCardSymbols(const UInt16* aStr);
	/// Метод корректирует "умный" поисковый запрос
	static ESldError CorrectSmartWildCardSearchQuery(const UInt16* aQuery, UInt16** aOut);
	/// Метод корректирует "неумный" поисковый запрос
	static ESldError CorrectNonSmartWildCardSearchQuery(const UInt16* aQuery, UInt16** aOut);
	
	/// Метод проверяет, является ли поисковый запрос "умным"
	static UInt32 IsSmartFullTextSearchQuery(const UInt16* aStr);
	/// Метод корректирует "умный" поисковый запрос
	static ESldError CorrectSmartFullTextSearchQuery(const UInt16* aQuery, UInt16** aOut);
	/// Метод корректирует "неумный" поисковый запрос
	static ESldError CorrectNonSmartFullTextSearchQuery(const UInt16* aQuery, UInt16** aOut);

	/// Метод проверяет наличие в строке aStr символов '*' и '?'
	UInt32 QueryIsExistWildSym(const UInt16* aStr);

	/// Функция вычисляет расстояние редактирования
	Int32 FuzzyCompare(const UInt16 *aStr1, const UInt16 *aStr2, Int32 aStr1len, Int32 aStr2len, Int32 (*aFuzzyBuffer)[ARRAY_DIM]);

	/// Преобразует строку символов в строку их масс 
	ESldError GetStrOfMass(const UInt16* aSourceStr, SldU16String & aMassStr, Int8 aEraseZeroSymbols = 1, Int8 aUseMassForDigit = 0) const;
	/// Преобразует строку символов в строку их масс с учетом разделителей
	ESldError GetStrOfMassWithDelimiters(const UInt16* aSourceStr, SldU16String & aMassStr, Int8 aEraseZeroSymbols = 1, Int8 aUseMassForDigit = 0) const;
	/// Преобразует поисковый паттерн в паттерн масс с учетом спец символов
	ESldError GetSearchPatternOfMass(const UInt16* aSourceStr, SldU16String & aMassStr, Int8 aUseMassForDigit = 0) const;
	
	/// Разделяет запрос на слова.
	ESldError DivideQueryByParts(const UInt16 *aText, SldU16WordsArray& aTextWords) const;
	/// Разделяет запрос на слова, используя переданные разделители
	ESldError DivideQueryByParts(const UInt16 *aText, const UInt16 *aDelimitersStr, SldU16WordsArray& aTextWords) const;

	/// Разделяет запрос на слова.
	void DivideQueryByParts(SldU16StringRef aText, CSldVector<SldU16StringRef> &aTextWords) const;
	/// Разделяет запрос на слова, используя переданные разделители
	void DivideQueryByParts(SldU16StringRef aText, const UInt16 *aDelimitersStr, CSldVector<SldU16StringRef> &aTextWords) const;

	/// Разделяет запрос на слова, возвращая альтернативные варианты для частичных разделителей
	ESldError DivideQuery(const UInt16 *aText, SldU16WordsArray& aTextWords, SldU16WordsArray& aAltrnativeWords) const;
	
	/// Функция возвращает тип алфавита для введенного текста в контексте текущего языка
	UInt32 GetAlphabetTypeByText(const UInt16 *text) const;

	/// Проверяет, является ли данный символ игнорируемым в заданной таблице сортировки
	Int8 IsZeroSymbol(UInt16 aChar, UInt32 aTableIndex) const;
	/// Проверяет, является ли данный символ игнорируемым в текущей дефолтной таблице сортировки
	Int8 IsZeroSymbol(UInt16 aChar) const;

	/// Проверяет, является ли данный символ разделителем в заданной таблице сортировки
	Int8 IsDelimiter(UInt16 aChar, UInt32 aTableIndex) const;
	/// Проверяет, является ли данный символ разделителем в текущей дефолтной таблице сортировки
	Int8 IsDelimiter(UInt16 aChar) const;

	/// Проверяет, является ли данный символ частичным разделителем в заданной таблице сортировки
	Int8 IsHalfDelimiter(UInt16 aChar, UInt32 aTableIndex) const;
	/// Проверяет, является ли данный символ частичным разделителем в текущей дефолтной таблице сортировки
	Int8 IsHalfDelimiter(UInt16 aChar) const;

	/// Проверяет, является ли данный символ Emoji-символом
	static Int8 IsEmoji(const UInt16 aChar, const EEmojiTypes aType = eSlovoedEmoji);

	/// Проверяет, является ли данный символ пробельным
	static Int8 IsWhitespace(const UInt16 aChar);

	/// Проверяет, является ли данный символ значимым хотя бы в одной из таблиц
	Int8 IsMarginalSymbol(const UInt16 aChar) const;

	/// Проверяет целостность скобок
	Int8 CheckBracket(const UInt16 *aText) const;

	UInt32 GetTableVersion() const { return m_CMPTable[0].Header->Version; }

	/// "Возвращает" строку разделителей в таблице сортировки заданного языка
	ESldError GetDelimiters(ESldLanguage aLangCode, const UInt16 **aDelimitersStr, UInt32 *aDelimitersCount) const;
	/// "Возвращает" строку разделителей в текущей дефолтной таблице сортировки
	ESldError GetDelimiters(const UInt16 **aDelimitersStr, UInt32 *aDelimitersCount) const;

	/// Добавляет выбранный селектор после всех Emoji-символов в строке
	static ESldError AddEmojiSelector(SldU16String & aString, const EEmojiTypes aType = eSlovoedEmoji, const UInt16 aSelector = SLD_DEFAULT_EMOJI_SELECTOR);

	/// Удаляет селекторы после всех Emoji-символов в строке
	static ESldError ClearEmojiSelector(SldU16String & aString, const EEmojiTypes aType = eSlovoedEmoji);

	/// Конвертирует 4-х символьную строчку в 4-х байтное число (например код языка, ID базы)
	static UInt32 UInt16StrToUInt32Code(const SldU16StringRef aString);

	/// Возвращает следующий по массе простой символ
	UInt16 GetNextMassSymbol(const UInt16 aSymbol) const;

	/// Возвращает слово по позиции во фразе
	SldU16StringRef GetWordByPosition(const SldU16StringRef aPhrase, const UInt32 aPos) const;

	/// Заменяет выбранное слово во фразе
	void ReplaceWordInPhraseByIndex(SldU16String& aPhrase, const SldU16StringRef aNewWord, const UInt32 aWordIndex) const;

  SldU16WordsArray ExpandBrackets(SldU16StringRef aText) const;

private:

	/// Очистка объекта
	void Clear(void);

	/// Возвращает массу символа
	UInt16 GetMass(UInt16 aChr, const UInt16 *aSimpleTable, UInt16 aNotFound) const;

	/// Возвращает массу для сложного символа
	UInt32 GetComplex(const UInt16 *str, UInt16 index, UInt16 *mass, const CMPComplexType *complex) const;
	
	/// Метод сравнения шаблона и слова на соответствие
	Int8 DoWildCompare(const UInt16* aTemplate, const UInt16* aText) const;

	/// Получает число из его текстового представления, основная функция
	static ESldError StrToInt32Base(const UInt16* aStr, UInt32 aRadix, Int32* aNumber);

private:
	
	/// Таблицы сравнения
	sld2::DynArray<TCompareTableSplit> m_CMPTable;

	/// Информация о таблицах сравнения
	sld2::DynArray<TCMPTableElement> m_CMPTableInfo;

	/// Таблица сравнения по умолчанию.
	UInt32				m_DefaultTable;
	
	/// Дополнительная таблица сравнения по умолчанию (если есть).
	UInt32				m_DefaultAddTable;
	
	/// Массив указателей на таблицы символов языков (таблицы для каждого языка + общая таблица символов-разделителей для всех языков)
	sld2::DynArray<CSldSymbolsTable>	m_LanguageSymbolsTable;

	/// Массив указателей на таблицы символов-разделителей для конкретных языков
	sld2::DynArray<CSldSymbolsTable>	m_LanguageDelimiterSymbolsTable;
};

#endif //_SLD_COMPARE_H_
