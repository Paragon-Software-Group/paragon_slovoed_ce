#include "SldCompare.h"

#include "SldUnicode.h"
#include "SldStack.h"

// Unicode support based on SldUnicode.h
namespace {
template <typename CharTo, typename CharFrom>
inline UInt16 convert(const CharFrom *source, CharTo *dest)
{
	if (!source)
		return 0;

	const UInt32 sourceLength = sld2::char_traits<CharFrom>::length(source);
	// handle the 0 source length case explicitly to be able to catch conversion errors
	if (sourceLength == 0)
	{
		if (dest)
			*dest = '\0';
		return 1;
	}

	const UInt32 length = sld2::utf::convert(source, sourceLength, dest);
	if (length == 0)
		return 0;

	if (dest)
		dest[length] = '\0';

	return (UInt16)(length + 1);
}
} // anon namespace

  /** ********************************************************************
  * Добавляем таблицу сравнения из бинарных данных
  *
  * @param[in]	aTable		- ссылка на таблицу сравнения которую необходимо инициализировать
  * @param[in]	aData		- данные таблицы сравнения
  *
  * @return код ошибки
  ************************************************************************/
static ESldError initTable(TCompareTableSplit &aTable, sld2::Span<const UInt8> aData)
{
	const CMPHeaderType *header = (const CMPHeaderType*)aData.data();
	const CMPSimpleType *simple = (const CMPSimpleType*)(aData.data() + header->HeaderSize);
	const UInt32 tableSize = aData.size() + sizeof(CMPHeaderType) - header->HeaderSize;
	aTable.Header = (CMPHeaderType*)sldMemNew(tableSize);
	if (!aTable.Header)
		return eMemoryNotEnoughMemory;

	sldMemZero(aTable.Header, sizeof(CMPHeaderType));
	sldMemMove(aTable.Header, header, header->HeaderSize);

	aTable.Simple = (CMPSimpleType*)(aTable.Header + 1);
	sldMemMove(aTable.Simple, simple, aData.size() - header->HeaderSize);

	auto & massTable = aTable.SimpleMassTable;
	sldMemSet(massTable.data(), CMP_NOT_FOUND_SYMBOL, massTable.size() * sizeof(massTable[0]));

	UInt16 maxMass = 0;
	for (UInt32 i = 0; i < aTable.Header->SimpleCount; i++)
	{
		const UInt16 mass = aTable.Simple[i].mass;
		if (mass < CMP_MASK_OF_INDEX_FLAG && mass > maxMass)
			maxMass = mass;
	}

	auto & sortedMass = aTable.SortedMass;
	sortedMass.resize(maxMass + 1);
	sld2::memzero_n(sortedMass.data(), sortedMass.size());

	for (UInt32 currentChar = 0; currentChar < aTable.Header->SimpleCount; currentChar++)
	{
		const auto & symbol = aTable.Simple[currentChar];
		if (symbol.mass != 0 && symbol.mass < CMP_MASK_OF_INDEX_FLAG && sortedMass[symbol.mass] == 0)
			sortedMass[symbol.mass] = symbol.ch;
		massTable[symbol.ch] = symbol.mass != 0 ? symbol.mass : CMP_IGNORE_SYMBOL;
	}

	aTable.Complex = (CMPComplexType*)(aTable.Simple + aTable.Header->SimpleCount);
	UInt8 *ptr = (UInt8*)(aTable.Complex + aTable.Header->ComplexCount);

	auto & mostUsageChar = aTable.MostUsageCharTable;
	sldMemZero(mostUsageChar.data(), mostUsageChar.size());
	if (aTable.Header->DelimiterCount)
	{
		aTable.Delimiter = (CMPDelimiterType*)ptr;
		ptr += aTable.Header->DelimiterCount * sizeof(aTable.Delimiter[0]);

		for (UInt32 chInex = 0; chInex < aTable.Header->DelimiterCount; chInex++)
		{
			if (aTable.Delimiter[chInex] < CMP_MOST_USAGE_SYMBOL_RANGE)
				mostUsageChar[aTable.Delimiter[chInex]] = eCmpDelimiterCharType;
		}
	}

	if (aTable.Header->NativeCount)
	{
		aTable.Native = (CMPNativeType*)ptr;
		ptr += aTable.Header->NativeCount * sizeof(aTable.Native[0]);

		for (UInt32 chInex = 0; chInex < aTable.Header->NativeCount; chInex++)
		{
			if (aTable.Native[chInex] < CMP_MOST_USAGE_SYMBOL_RANGE)
				mostUsageChar[aTable.Native[chInex]] = eCmpNativeCharType;
		}
	}

	if (aTable.Header->HalfDelimiterCount)
	{
		aTable.HalfDelimiter = (CMPHalfDelimiterType*)ptr;
		ptr += aTable.Header->HalfDelimiterCount * sizeof(aTable.HalfDelimiter[0]);
	}

	if (aTable.Header->FeatureFlag & SLD_CMP_TABLE_FEATURE_SYMBOL_PAIR_TABLE)
	{
		aTable.HeaderPairSymbols = (CMPSymbolPairTableHeader*)ptr;
		ptr += aTable.HeaderPairSymbols->structSize;

		if (aTable.HeaderPairSymbols->NativePairCount)
		{
			aTable.NativePair = (CMPSymbolPair*)ptr;
			ptr += aTable.HeaderPairSymbols->NativePairCount * sizeof(aTable.NativePair[0]);
		}

		if (aTable.HeaderPairSymbols->CommonPairCount)
		{
			aTable.CommonPair = (CMPSymbolPair*)ptr;
			ptr += aTable.HeaderPairSymbols->CommonPairCount * sizeof(aTable.CommonPair[0]);
		}

		if (aTable.HeaderPairSymbols->DictionaryPairCount)
			aTable.DictionaryPair = (CMPSymbolPair*)ptr;
	}

	aTable.TableSize = tableSize;

	return eOK;
}

// освобождает память используемую таблицой сравнения
static void freeTable(TCompareTableSplit &aTable)
{
	if (aTable.Header)
		sldMemFree(aTable.Header);
	aTable.Header = nullptr;
}

/// Конструктор
CSldCompare::CSldCompare(void)
{
	Clear();
}

/// Конструктор копирования
CSldCompare::CSldCompare(const CSldCompare& aRef)
{
	*this = aRef;
}

/// Оператор присваивания
CSldCompare& CSldCompare::operator=(const CSldCompare& aRef)
{
	if (this == &aRef)
		return *this;
	Close();

	decltype(m_CMPTable) cmpTable;
	if (!cmpTable.resize(aRef.m_CMPTable.size()))
		return *this;

	for (UInt32 i = 0; i < aRef.m_CMPTable.size(); i++)
	{
		auto data = sld2::make_span((const UInt8*)aRef.m_CMPTable[i].Header, aRef.m_CMPTable[i].TableSize);
		if (initTable(m_CMPTable[i], data) != eOK)
		{
			for (auto&& table : cmpTable)
				freeTable(table);
			return *this;
		}
	}

	m_CMPTable = sld2::move(cmpTable);
	m_CMPTableInfo = aRef.m_CMPTableInfo;
	m_LanguageSymbolsTable = aRef.m_LanguageSymbolsTable;
	m_LanguageDelimiterSymbolsTable = aRef.m_LanguageDelimiterSymbolsTable;

	m_DefaultTable = aRef.m_DefaultTable;
	m_DefaultAddTable = aRef.m_DefaultAddTable;

	return *this;
}

/// Деструктор
CSldCompare::~CSldCompare(void)
{
	for (auto&& table : m_CMPTable)
		freeTable(table);
}

/// Очистка объекта
void CSldCompare::Clear(void)
{
	m_DefaultTable = 0;
	m_DefaultAddTable = 0xFFFF;
}

/** *********************************************************************
* Считываем и подготавливаем таблицы сравнения к работе
*
* @param[in]	aData								- класс ввода данных из контейнера
* @param[in]	aLanguageSymbolsTableCount			- количество таблиц символов, которые нужно загрузить
* @param[in]	aLanguageDelimiterSymbolsTableCount	- количество таблиц символов-разделителей, которые нужно загрузить
*
* @return код ошибки
************************************************************************/
ESldError CSldCompare::Open(CSDCReadMy &aData, UInt32 aLanguageSymbolsTableCount, UInt32 aLanguageDelimiterSymbolsTableCount)
{
	auto res = aData.GetResource(SLD_RESOURCE_COMPARE_TABLE_HEADER, 0);
	if (res != eOK)
		return res.error();

	auto tmpHeader = (const TCMPTablesHeader*)res.ptr();

	decltype(m_CMPTableInfo) cmpTableInfo;
	if (!cmpTableInfo.resize(sld2::default_init, tmpHeader->NumberOfCMPtables))
		return eMemoryNotEnoughMemory;

	sldMemCopy(cmpTableInfo.data(), res.ptr() + tmpHeader->structSize, sizeof(cmpTableInfo[0]) * cmpTableInfo.size());

	decltype(m_CMPTable) cmpTable;
	if (!cmpTable.resize(tmpHeader->NumberOfCMPtables))
		return eMemoryNotEnoughMemory;

	// cleans up the table in case of an error
	struct _stackGuard_t {
		decltype(cmpTable) &d;
		~_stackGuard_t() { for (auto&& table : d) freeTable(table); }
	} _stackGuard{ cmpTable };

	UInt32 count = 0;
	ESldError error = eOK;
	for (; error == eOK; count++)
	{
		res = aData.GetResource(SLD_RESOURCE_COMPARE_TABLE, count);
		if (res != eOK)
			break;

		if (count >= cmpTable.size())
			error = eCommonWrongCMPTableCount;
		else
			error = initTable(cmpTable[count], res.data());
	}
	if (error != eOK)
		return error;

	if (count != cmpTable.size())
		return eCommonWrongCMPTableCount;

	/// Загружаем таблицы символов
	decltype(m_LanguageSymbolsTable) symbolsTable;
	if (aLanguageSymbolsTableCount)
	{
		// Выделяем память для таблиц
		if (!symbolsTable.resize(aLanguageSymbolsTableCount))
			return eMemoryNotEnoughMemory;

		// Загружаем таблицы
		for (UInt32 i = 0; i < symbolsTable.size(); i++)
		{
			error = symbolsTable[i].Init(aData, SLD_RESOURCE_SYMBOLS_TABLE, i);
			if (error != eOK)
				return error;
		}
	}

	/// Загружаем таблицы символов-разделителей
	decltype(m_LanguageDelimiterSymbolsTable) delimitersTable;
	if (aLanguageDelimiterSymbolsTableCount)
	{
		// Выделяем память для таблиц
		if (!delimitersTable.resize(aLanguageDelimiterSymbolsTableCount))
			return eMemoryNotEnoughMemory;

		// Загружаем таблицы
		for (UInt32 i = 0; i < delimitersTable.size(); i++)
		{
			error = delimitersTable[i].Init(aData, RESOURCE_TYPE_SYMBOLS_DELIMITERS_TABLE, i);
			if (error != eOK)
				return error;
		}
	}

	m_CMPTable = sld2::move(cmpTable);
	m_CMPTableInfo = sld2::move(cmpTableInfo);
	m_LanguageSymbolsTable = sld2::move(symbolsTable);
	m_LanguageDelimiterSymbolsTable = sld2::move(delimitersTable);

	return eOK;
}

/** ********************************************************************
* Освобождаем память выделенную для таблиц 
*
* @return код ошибки
************************************************************************/
ESldError CSldCompare::Close()
{
	for (auto&& table : m_CMPTable)
		freeTable(table);

	m_CMPTable.resize(0);
	m_CMPTableInfo.resize(0);
	m_LanguageSymbolsTable.resize(0);
	m_LanguageDelimiterSymbolsTable.resize(0);

	Clear();

	return eOK;
}

/** ********************************************************************
* Бинарное сравнение 2 строк
*
* @param[in]	str1	- указатель на первую из сравниваемых строк
* @param[in]	str2	- указатель на вторую из сравниваемых строк
*
* @return	0 - строки равны, 
*			1 первая строка больше второй, 
*			-1 первая строка меньше второй
************************************************************************/
Int32 CSldCompare::StrCmp(const UInt16 *str1, const UInt16 *str2)
{
	Int32 r = sld2::StrCmp(str1, str2);
	return r < 0 ? -1 : r != 0;
}

/** *********************************************************************
* Определяет длину строки
*
* @param[in]	aStr	- указатель на строку
*
* @return длина строки или 0, если передан нулевой указатель на строку
************************************************************************/
UInt32 CSldCompare::StrLen(const UInt16 *aStr)
{
	return sld2::StrLen(aStr);
}

/** *********************************************************************
* Копирует строку
*
* @param[out]	aStrDest	- указатель на буфер, куда будем копировать
* @param[in]	aStrSource	- указатель на строку, которую будем копировать
*
* @return количество скопированных символов
************************************************************************/
UInt32 CSldCompare::StrCopy(UInt16* aStrDest, const UInt16* aStrSource)
{
	return sld2::StrCopy(aStrDest, aStrSource);
}

/***********************************************************************
* Копирует строку игнорируя разделители(символы с нулевой массой)
*
* @param[out]	aStrOut					- указатель на строку, по которому будет записан результат
*										  размер выделенной памяти проверяется снуружи и должен
*										  быть не меньше StrLen(aStrSource) + 1
* @param[in]	aStrSource				- указатель на строку, которую будем копировать
* @param[in]	aEraseNotFoundSymbols	- флаг того, будут ли учтены отстутвсующие в таблице
*										  сортировки символы:
*										  0 - отсутсвующие символы останутся в строке
*										  1 - отсутсвующие символы будут вырезаны
*
* @return количество скопированных символов
************************************************************************/
UInt32 CSldCompare::StrEffectiveCopy(UInt16* aStrOut, const UInt16* aStrSource, Int8 aEraseNotFoundSymbols) const
{
	if (!aStrSource || !aStrOut)
		return 0;

	UInt32 count = 0;
	UInt16 flag = aEraseNotFoundSymbols == 0 ? CMP_NOT_FOUND_SYMBOL : CMP_IGNORE_SYMBOL;
	while (*aStrSource)
	{
		if (GetMass(*aStrSource, m_CMPTable[m_DefaultTable].SimpleMassTable.data(), flag))
			aStrOut[count++] = *aStrSource;
		aStrSource++;
	}
	aStrOut[count] = 0;
	return count;
}

/***********************************************************************
* Копирует строку игнорируя разделители(символы с нулевой массой)
*
* @param[in]	aStrSource				- указатель на строку, которую будем копировать
* @param[out]	aEffectiveString		- строка без нулевых символов
* @param[in]	aEraseNotFoundSymbols	- флаг того, будут ли учтены отстутвсующие в таблице сортировки символы:
*										  0 - отсутсвующие символы останутся в строке
*										  1 - отсутсвующие символы будут вырезаны
*
* @return количество скопированных символов
************************************************************************/
void CSldCompare::GetEffectiveString(const UInt16* aStrSource, SldU16String & aEffectiveString, Int8 aEraseNotFoundSymbols) const
{
	aEffectiveString.clear();

	if (!aStrSource)
		return;

	while (*aStrSource)
	{
		if (GetMass(*aStrSource, m_CMPTable[m_DefaultTable].SimpleMassTable.data(), aEraseNotFoundSymbols == 0 ? CMP_NOT_FOUND_SYMBOL : CMP_IGNORE_SYMBOL))
			aEffectiveString.push_back(*aStrSource);
		aStrSource++;
	}
}

/***********************************************************************
* Копирует строку игнорируя разделители(символы с нулевой массой)
*
* @param[in]	aStrSource				- строка, которую будем копировать
* @param[in]	aEraseNotFoundSymbols	- флаг того, будут ли учтены отстутвсующие в таблице сортировки символы:
*										  0 - отсутсвующие символы останутся в строке
*										  1 - отсутсвующие символы будут вырезаны
*
* @return строка без нулевых символов
************************************************************************/
SldU16String CSldCompare::GetEffectiveString(const SldU16StringRef aStrSource, Int8 aEraseNotFoundSymbols) const
{
	SldU16String result;
	result.reserve(StrEffectiveLen(aStrSource, aEraseNotFoundSymbols));
	const UInt16 notFoundCode = aEraseNotFoundSymbols == 0 ? CMP_NOT_FOUND_SYMBOL : CMP_IGNORE_SYMBOL;
	for (const auto ch : aStrSource)
	{
		if (GetMass(ch, m_CMPTable[m_DefaultTable].SimpleMassTable.data(), notFoundCode) != CMP_IGNORE_SYMBOL)
			result.push_back(ch);
	}

	return result;
}

SldU16StringRef TrimSymbolsRef(const SldU16StringRef aStrSource, const CSldCompare* aCmp, Int8(CSldCompare::*aFunc)(UInt16 aChar)const)
{
	UInt32 startIndex = 0;
	for (; startIndex < aStrSource.length(); startIndex++)
	{
		if (!(aCmp->*aFunc)(aStrSource[startIndex]))
			break;
	}

	UInt32 endIndex = aStrSource.length() - 1;
	for (; endIndex > startIndex; endIndex--)
	{
		if (!(aCmp->*aFunc)(aStrSource[endIndex]))
			break;
	}

	return SldU16StringRef(&aStrSource[startIndex], endIndex - startIndex + 1);
}

/***********************************************************************
* Возвращает SldU16StringRef без разделители в начале и конце строки
*
* @param[in]	aStrSource				- строка, которую будем копировать
*
* @return SldU16StringRef без разделителей
*		  Время жизни результата зависит от времени жизни aStrSource
************************************************************************/
SldU16StringRef CSldCompare::TrimDelimitersRef(const SldU16StringRef aStrSource) const
{
	return TrimSymbolsRef(aStrSource, this, &CSldCompare::IsDelimiter);
}

/***********************************************************************
* Копирует строку, игнорируя разделители в начале и конце строки
*
* @param[in]	aStrSource				- строка, которую будем копировать
*
* @return строка без разделителей
************************************************************************/
SldU16String CSldCompare::TrimDelimiters(const SldU16StringRef aStrSource) const
{
	return SldU16String(TrimDelimitersRef(aStrSource));
}

/***********************************************************************
* Возвращает SldU16StringRef без нулевых символов в начале и конце строки
*
* @param[in]	aStrSource				- строка, которую будем копировать
*
* @return SldU16StringRef нулевых символов
*		  Время жизни результата зависит от времени жизни aStrSource
************************************************************************/
SldU16StringRef CSldCompare::TrimIngnoresRef(const SldU16StringRef aStrSource) const
{
	return TrimSymbolsRef(aStrSource, this, &CSldCompare::IsZeroSymbol);
}

/***********************************************************************
* Копирует строку, игнорируя нулевые символы в начале и конце строки
*
* @param[in]	aStrSource				- строка, которую будем копировать
*
* @return строка без нулевых символов
************************************************************************/
SldU16String CSldCompare::TrimIngnores(const SldU16StringRef aStrSource) const
{
	return SldU16String(TrimIngnoresRef(aStrSource));
}

/***********************************************************************
* Возвращает количество символов в строке, имеющих массу
*
* @param[in]	aStrSource				- строка, которую нужно проверить
* @param[in]	aEraseNotFoundSymbols	- флаг того, будут ли учтены отстутвсующие в таблице
*										  сортировки символы:
*										  0 - отсутсвующие символы будут учетны
*										  1 - отсутсвующие символы не будут учтены
* @return количество символов, имеющих массу
************************************************************************/
UInt32 CSldCompare::StrEffectiveLen(const SldU16StringRef aStrSource, Int8 aEraseNotFoundSymbols) const
{
	UInt32 result = 0;
	const UInt16 notFoundCode = aEraseNotFoundSymbols == 0 ? CMP_NOT_FOUND_SYMBOL : CMP_IGNORE_SYMBOL;
	for (const auto ch : aStrSource)
	{
		if (GetMass(ch, m_CMPTable[m_DefaultTable].SimpleMassTable.data(), notFoundCode) != CMP_IGNORE_SYMBOL)
			result++;
	}

	return result;
}

/** ********************************************************************
* Сравнение 2 строк с учетом регистра
*
* @param[in]	str1	- указатель на первую из сравниваемых строк
* @param[in]	str2	- указатель на вторую из сравниваемых строк
*
* @return	0 - строки равны, 
*			1 первая строка больше второй, 
*			-1 первая строка меньше второй
************************************************************************/
Int32 CSldCompare::StrCmpA(const UInt8 *str1, const UInt8 *str2)
{
	Int32 r = sld2::StrCmp(str1, str2);
	return r < 0 ? -1 : r != 0;
}

/** *********************************************************************
* Определяет длину строки
*
* @param[in]	aStr	- указатель на строку
*
* @return длина строки или 0, если передан нулевой указатель на строку
************************************************************************/
Int32 CSldCompare::StrLenA(const UInt8 *aStr)
{
	return sld2::StrLen(aStr);
}

/** *********************************************************************
* Копирует строку
*
* @param[out]	StrDest		- указатель на буфер, куда будем копировать
* @param[in]	StrSource	- указатель на строку, которую будем копировать
*
* @return количество скопированных символов
************************************************************************/
UInt32 CSldCompare::StrCopyA(UInt8* aStrDest, const UInt8* aStrSource)
{
	return sld2::StrCopy(aStrDest, aStrSource);
}

/** *********************************************************************
* Копирует строку длиной не более aSize
*
* @param[out]	aStrDest	- указатель на буфер, куда будем копировать
* @param[in]	aStrSource	- указатель на строку, которую будем копировать
* @param[in]	aSize		- максимальное количество символов для копирования
*
* @return количество скопированных символов
************************************************************************/
UInt32 CSldCompare::StrNCopyA(UInt8* aStrDest, const UInt8* aStrSource, UInt32 aSize)
{
	return sld2::StrNCopy(aStrDest, aStrSource, aSize);
}

/** *********************************************************************
* Копирует строку длиной не более aSize
*
* @param[out]	aStrDest	- указатель на буфер, куда будем копировать
* @param[in]	aStrSource	- указатель на строку, которую будем копировать
* @param[in]	aSize		- максимальное количество символов для копирования
*
* @return количество скопированных символов
************************************************************************/
UInt32 CSldCompare::StrNCopy(UInt16* aStrDest, const UInt16* aStrSource, UInt32 aSize)
{
	UInt32 count = sld2::StrNCopy(aStrDest, aStrSource, aSize);
	if (aStrDest)
		aStrDest[count] = '\0';
	return count;
}

/** *********************************************************************
* Ищет указанный символ в строке
*
* @param[in]	aStr	- указатель на строку в которой будем искать символ
* @param[in]	aChr	- код символа
*
* @return если символ найден, тогда указатель на его положение в строке, иначе NULL
************************************************************************/
UInt8* CSldCompare::StrChrA(UInt8* aStr, UInt8 aChr)
{
	return sld2::StrChr(aStr, aChr);
}

/** *********************************************************************
* Ищет указанную подстроку в строке
*
* @param[in]	aSourceStr	- строка, в которой происходит поиск
* @param[in]	aSearchStr	- подстрока, которую ищем
*
* @return	указатель на первое вхождение искомой подстроки в строку, или NULL,
*			если подстрока не найдена. Если aSearchStr == NULL, возвращает aSourceStr
************************************************************************/
const UInt16* CSldCompare::StrStr(const UInt16* aSourceStr, const UInt16* aSearchStr)
{
	return sld2::StrStr(aSourceStr, aSearchStr);
}

/** *********************************************************************
* Ищет указанную подстроку в строке
*
* @param[in]	aSourceStr	- строка, в которой происходит поиск
* @param[in]	aSearchStr	- подстрока, которую ищем
*
* @return	указатель на первое вхождение искомой подстроки в строку, или NULL,
*			если подстрока не найдена. Если aSearchStr == NULL, возвращает aSourceStr
************************************************************************/
const UInt8* CSldCompare::StrStrA(const UInt8* aSourceStr, const UInt8* aSearchStr)
{
	return sld2::StrStr(aSourceStr, aSearchStr);
}

/** *********************************************************************
* Конвертирует строку из UTF8 в UTF16
*
* Так как длина полученой строки не однозначна, данная функция
* позволяет узнать необходимый размер буффера (включая конечный '\0')
*
* @param[out]	unicode	- указатель на буффер для сохранения строчки в требуемой кодировке
* @param[in]	utf8	- указатель на буффер исходной строчки
*
* @return	размер строчки в элементах (включая конечный '\0')
*			(т.е. чтобы узнать количество памяти - нужно умножить на размер одного элемента)
*			Если произошла ошибка формата - 0
************************************************************************/
UInt16 CSldCompare::StrUTF8_2_UTF16(UInt16* unicode, const UInt8* utf8)
{
	return convert(utf8, unicode);
}

/** *********************************************************************
* Конвертирует строку из UTF8 в UTF32
*
* Так как длина полученой строки не однозначна, данная функция
* позволяет узнать необходимый размер буффера (включая конечный '\0')
*
* @param[out]	unicode	- указатель на буффер для сохранения строчки в требуемой кодировке
* @param[in]	utf8	- указатель на буффер исходной строчки
*
* @return	размер строчки в элементах (включая конечный '\0')
*			(т.е. чтобы узнать количество памяти - нужно умножить на размер одного элемента)
*			Если произошла ошибка формата - 0
************************************************************************/
UInt16 CSldCompare::StrUTF8_2_UTF32(UInt32* unicode, const UInt8* utf8)
{
	return convert(utf8, unicode);
}

/** *********************************************************************
* Конвертирует строку из UTF16 в UTF8
*
* Так как длина полученой строки не однозначна, данная функция
* позволяет узнать необходимый размер буффера (включая конечный '\0')
*
* @param[out]	utf8	- указатель на буффер для UTF8-текста. Если NULL, тогда
*						  только расчитывается необходимый размер буффера
* @param[in]	unicode	- указатель на исходную юникодную строчку
*
* @return	размер строчки в элементах (включая конечный '\0')
*			(т.е. чтобы узнать количество памяти - нужно умножить на размер одного элемента)
*			Если произошла ошибка формата - 0
************************************************************************/
UInt16 CSldCompare::StrUTF16_2_UTF8(UInt8* utf8, const UInt16* unicode)
{
	return convert(unicode, utf8);
}

/** *********************************************************************
* Конвертирует строку из UTF16 в UTF32
*
* Так как длина полученой строки не однозначна, данная функция
* позволяет узнать необходимый размер буффера (включая конечный '\0')
*
* @param[in]	unicode16	- указатель на буффер исходной юникодной строчки
* @param[out]	unicode32	- указатель на буффер для сохранения новой юникодной строчки
*
* @return	размер строчки в элементах (включая конечный '\0')
*			(т.е. чтобы узнать количество памяти - нужно умножить на размер одного элемента)
*			Если произошла ошибка формата - 0
************************************************************************/
UInt16 CSldCompare::StrUTF16_2_UTF32(UInt32* unicode32, const UInt16* unicode16)
{
	return convert(unicode16, unicode32);
}

/** *********************************************************************
* Конвертирует строку из UTF32 в UTF8
*
* Так как длина полученой строки не однозначна, данная функция
* позволяет узнать необходимый размер буффера (включая конечный '\0')
*
* @param[out]	utf8	- указатель на буффер для UTF8-текста. Если NULL, тогда
*						  только расчитывается необходимый размер буффера
* @param[in]	unicode	- указатель на исходную юникодную строчку
*
* @return	размер строчки в элементах (включая конечный '\0')
*			(т.е. чтобы узнать количество памяти - нужно умножить на размер одного элемента)
************************************************************************/
UInt16 CSldCompare::StrUTF32_2_UTF8(UInt8* utf8, const UInt32* unicode)
{
	return convert(unicode, utf8);
}

/** *********************************************************************
* Конвертирует строку из UTF32 в UTF16
*
* Так как длина полученой строки не однозначна, данная функция
* позволяет узнать необходимый размер буффера (включая конечный '\0')
*
* @param[in]	unicode32	- указатель на буффер исходной юникодной строчки
* @param[out]	unicode16	- указатель на буффер для сохранения новой юникодной строчки
*
* @return	размер строчки в элементах (включая конечный '\0')
*			(т.е. чтобы узнать количество памяти - нужно умножить на размер одного элемента)
************************************************************************/
UInt16 CSldCompare::StrUTF32_2_UTF16(UInt16* unicode16, const UInt32* unicode32)
{
	return convert(unicode32, unicode16);
}

/** ********************************************************************
* Сравнение 2 строк без учета регистра, для указанного языка
*
* @param[in]	str1			- указатель на первую из сравниваемых строк
* @param[in]	str2			- указатель на вторую из сравниваемых строк
* @param[in]	aLanguageCode	- код языка, таблицу которого нужно использовать
*
* @return	0 - строки равны, 
*			1 первая строка больше второй, 
*			-1 первая строка меньше второй
************************************************************************/
Int32 CSldCompare::StrICmpByLanguage(const UInt16 *str1, const UInt16 *str2, ESldLanguage aLanguageCode) const
{
	for (UInt32 i = 0; i < m_CMPTable.size(); i++)
	{
		if (m_CMPTableInfo[i].LanguageCode == aLanguageCode)
		{
			return StrICmp(str1, str2, i);
		}
	}

	return StrICmp(str1, str2, m_DefaultTable);
}

/** *********************************************************************
* Сравнение 2 строк по таблице сравнения (сравниваются веса символов)
*
* @param[in]	str1	- указатель на первую из сравниваемых строк
* @param[in]	str2	- указатель на вторую из сравниваемых строк
*
* @return	0 - строки равны, 
*			1 первая строка больше второй, 
*			-1 первая строка меньше второй
************************************************************************/
Int32 CSldCompare::StrICmp(const UInt16 *str1, const UInt16 *str2) const
{
	return StrICmp(str1, str2, m_DefaultTable);
}

/** *********************************************************************
* Возвращает количество записей в таблице соответствия символов верхнего и нижнего регистров
* для текущей таблицы сравнения
*
* @param[in]	aSymbolTable	- тип таблицы, количество записей в которой хотим узнать
*
* @return количество записей в таблице, 0 - если указанной таблицы не существует
************************************************************************/
Int32 CSldCompare::GetSymbolPairTableElementsCount(ESymbolPairTableTypeEnum aSymbolTable) const
{
	// Нет таблиц преобразования
	if (!(m_CMPTable[m_DefaultTable].Header->FeatureFlag & SLD_CMP_TABLE_FEATURE_SYMBOL_PAIR_TABLE))
		return 0;
		
	switch (aSymbolTable)
	{
		case eSymbolPairTableType_Native:
			return m_CMPTable[m_DefaultTable].HeaderPairSymbols->NativePairCount;
		case eSymbolPairTableType_Common:
			return m_CMPTable[m_DefaultTable].HeaderPairSymbols->CommonPairCount;
		case eSymbolPairTableType_Dictionary:
			return m_CMPTable[m_DefaultTable].HeaderPairSymbols->DictionaryPairCount;
		default:
			return 0;
	}
}

/** *********************************************************************
* Из таблицы соответствия символов верхнего и нижнего регистров
* возвращает код символа в верхнем регистре по номеру записи и типу таблицы
* для текущей таблицы сравнения
*
* Метод используется для перебора всех записей
*
* @param[in]	aIndex			- номер записи в таблице соответствия
* @param[in]	aSymbolTable	- тип используемой таблицы соответствия
*
* @return код символа или 0, если не найдена указанная таблица или запись в ней
************************************************************************/
UInt16 CSldCompare::GetUpperSymbolFromSymbolPairTable(Int32 aIndex, ESymbolPairTableTypeEnum aSymbolTable) const
{
	// Нет таблиц соответствия
	if (!(m_CMPTable[m_DefaultTable].Header->FeatureFlag & SLD_CMP_TABLE_FEATURE_SYMBOL_PAIR_TABLE))
		return 0;
		
	switch (aSymbolTable)
	{
		case eSymbolPairTableType_Native:
		{
			if ((UInt32)aIndex >= m_CMPTable[m_DefaultTable].HeaderPairSymbols->NativePairCount)
				return 0;
			return m_CMPTable[m_DefaultTable].NativePair[aIndex].up;
		}
		case eSymbolPairTableType_Common:
		{
			if ((UInt32)aIndex >= m_CMPTable[m_DefaultTable].HeaderPairSymbols->CommonPairCount)
				return 0;
			return m_CMPTable[m_DefaultTable].CommonPair[aIndex].up;
		}
		case eSymbolPairTableType_Dictionary:
		{
			if ((UInt32)aIndex >= m_CMPTable[m_DefaultTable].HeaderPairSymbols->DictionaryPairCount)
				return 0;
			return m_CMPTable[m_DefaultTable].DictionaryPair[aIndex].up;
		}
		default:
		{
			return 0;
		}
	}
}

/** *********************************************************************
* Из таблицы соответствия символов верхнего и нижнего регистров
* возвращает код символа в нижнем регистре по номеру записи и типу таблицы
* для текущей таблицы сравнения
*
* Метод используется для перебора всех записей
*
* @param[in]	aIndex			- номер записи в таблице соответствия
* @param[in]	aSymbolTable	- тип используемой таблицы соответствия
*
* @return код символа или 0, если не найдена указанная таблица или запись в ней
************************************************************************/
UInt16 CSldCompare::GetLowerSymbolFromSymbolPairTable(Int32 aIndex, ESymbolPairTableTypeEnum aSymbolTable) const
{
	// Нет таблиц соответствия
	if (!(m_CMPTable[m_DefaultTable].Header->FeatureFlag & SLD_CMP_TABLE_FEATURE_SYMBOL_PAIR_TABLE))
		return 0;
		
	switch (aSymbolTable)
	{
		case eSymbolPairTableType_Native:
		{
			if ((UInt32)aIndex >= m_CMPTable[m_DefaultTable].HeaderPairSymbols->NativePairCount)
				return 0;
			return m_CMPTable[m_DefaultTable].NativePair[aIndex].low;
		}
		case eSymbolPairTableType_Common:
		{
			if ((UInt32)aIndex >= m_CMPTable[m_DefaultTable].HeaderPairSymbols->CommonPairCount)
				return 0;
			return m_CMPTable[m_DefaultTable].CommonPair[aIndex].low;
		}
		case eSymbolPairTableType_Dictionary:
		{
			if ((UInt32)aIndex >= m_CMPTable[m_DefaultTable].HeaderPairSymbols->DictionaryPairCount)
				return 0;
			return m_CMPTable[m_DefaultTable].DictionaryPair[aIndex].low;
		}
		default:
		{
			return 0;
		}
	}
}

/** *********************************************************************
* Преобразует символ к верхнему регистру
*
* @param[in]	aChr	- символ, который нужно преобразовать
*
* @return	код преобразованного символа,
*			либо код исходного символа, если преобразование произвести невозможно
************************************************************************/
UInt16 CSldCompare::ToUpperChr(UInt16 aChr) const
{
	// Ищем во всех таблицах
	for (auto&& cmpTable : m_CMPTable)
	{
		// Нет таблиц преобразования вообще
		if (!(cmpTable.Header->FeatureFlag & SLD_CMP_TABLE_FEATURE_SYMBOL_PAIR_TABLE))
			continue;

		// Сначала ищем в таблице символов конкретного языка
		// Основную часть символов скорее всего можно будет найти здесь, потом ищем
		// в общей таблице символов
		const sld2::Span<const CMPSymbolPair> symbolTables[] = {
			sld2::make_span(cmpTable.NativePair, cmpTable.HeaderPairSymbols->NativePairCount),
			sld2::make_span(cmpTable.CommonPair, cmpTable.HeaderPairSymbols->CommonPairCount),
		};

		for (auto&& symbolTable : symbolTables)
		{
			// Ищем линейным поиском, так как записи в таблице не отсортированы по символам в нижнем регистре
			for (auto&& symbolPair : symbolTable)
			{
				if (symbolPair.low == aChr)
					return symbolPair.up;
			}
		}
	}

	return aChr;
}

/** *********************************************************************
* Преобразует символ к нижнему регистру
*
* @param[in]	aChr	- символ, который нужно преобразовать
*
* @return	код преобразованного символа,
*			либо код исходного символа, если преобразование произвести невозможно
************************************************************************/
UInt16 CSldCompare::ToLowerChr(UInt16 aChr) const
{
	// Ищем во всех таблицах
	for (auto&& cmpTable : m_CMPTable)
	{
		// Нет таблиц преобразования вообще
		if (!(cmpTable.Header->FeatureFlag & SLD_CMP_TABLE_FEATURE_SYMBOL_PAIR_TABLE))
			continue;

		// Сначала ищем в таблице символов конкретного языка
		// Основную часть символов скорее всего можно будет найти здесь, потом ищем
		// в в общей таблице символов
		const sld2::Span<const CMPSymbolPair> symbolTables[] = {
			sld2::make_span(cmpTable.NativePair, cmpTable.HeaderPairSymbols->NativePairCount),
			sld2::make_span(cmpTable.CommonPair, cmpTable.HeaderPairSymbols->CommonPairCount),
		};

		for (auto&& symbolTable : symbolTables)
		{
			// Ищем символ в таблице двоичным поиском, поскольку записи в таблице отсортированы по символам в верхнем регистре
			if (!symbolTable.size() || symbolTable.front().up > aChr || symbolTable.back().up < aChr)
				continue;

			struct pred {
				UInt32 chr;
				bool operator()(const CMPSymbolPair &symbolPair) const { return symbolPair.up < chr; }
			};
			UInt32 index = sld2::lower_bound(symbolTable.data(), symbolTable.size(), pred{ aChr });
			if (index != symbolTable.size() && symbolTable[index].up == aChr)
				return symbolTable[index].low;
		}
	}

	return aChr;
}

/** *********************************************************************
* Преобразует строку к верхнему регистру
*
* @param[in]	aStr	- указатель на исходную строку, которую нужно преобразовать
* @param[out]	aOutStr	- указатель на строку, куда будет сохранена преобразованная строка,
*						  должно быть выделено достаточное количество памяти
* 
* @return код ошибки
************************************************************************/
ESldError CSldCompare::ToUpperStr(const UInt16* aStr, UInt16* aOutStr) const
{
	if (!aStr || !aOutStr)
		return eMemoryNullPointer;
	
	while (*aStr)
	{
		*aOutStr = ToUpperChr(*aStr);
		aStr++;
		aOutStr++;
	}
	*aOutStr = 0;
	
	return eOK;
}

/** *********************************************************************
* Преобразует строку к нижнему регистру
*
* @param[in]	aStr	- указатель на исходную строку, которую нужно преобразовать
* @param[out]	aOutStr	- указатель на строку, куда будет сохранена преобразованная строка
*						  должно быть выделено достаточное количество памяти
* 
* @return код ошибки
************************************************************************/
ESldError CSldCompare::ToLowerStr(const UInt16* aStr, UInt16* aOutStr) const
{
	if (!aStr || !aOutStr)
		return eMemoryNullPointer;
	
	while (*aStr)
	{
		*aOutStr = ToLowerChr(*aStr);
		aStr++;
		aOutStr++;
	}
	*aOutStr = 0;
	
	return eOK;
}

/** ********************************************************************
* Сравнение 2 строк по таблице сравнения (сравниваются веса символов)
*
* @param[in]	str1		- указатель на первую из сравниваемых строк
* @param[in]	str2		- указатель на вторую из сравниваемых строк
* @param[in]	aTableIndex	- номер таблицы сравнения который нужно использовать
*
* @return	0 - строки равны, 
*			1 первая строка больше второй, 
*			-1 первая строка меньше второй
************************************************************************/
Int32 CSldCompare::StrICmp(const UInt16 *str1, const UInt16 *str2, UInt32 aTableIndex) const
{
	UInt16 mass1[CMP_CHAIN_LEN + 1] = {0};
	UInt16 mass2[CMP_CHAIN_LEN + 1] = {0};
	UInt16 *cmp1=mass1, *cmp2=mass2;
	UInt16 last_char_mass1, last_char_mass2;
	const CMPHeaderType *header;
	const CMPComplexType *complex;

	if (aTableIndex >= m_CMPTable.size())
		return StrCmp(str1, str2);

	// Вычисляем положение частей таблицы сравнения.
	header = m_CMPTable[aTableIndex].Header;
	complex = m_CMPTable[aTableIndex].Complex;

	// Compare strings while both strings not finished

	// Compare strings while both strings not finished
	while ((!CMP_IS_EOL(header, str1) || *(cmp1+1)) && (!CMP_IS_EOL(header, str2) || *(cmp2+1)))
	{
		// Пропускаем все игнорируемые символы.
		while (*str1 && (last_char_mass1 = GetMass(*str1, m_CMPTable[aTableIndex].SimpleMassTable.data(), CMP_IGNORE_SYMBOL)) == CMP_IGNORE_SYMBOL)
			str1++;
		while (*str2 && (last_char_mass2 = GetMass(*str2, m_CMPTable[aTableIndex].SimpleMassTable.data(), CMP_IGNORE_SYMBOL)) == CMP_IGNORE_SYMBOL)
			str2++;

		// Проверяем, не закончились ли строчки
		if (!(!CMP_IS_EOL(header, str1) || *(cmp1+1)))
			break;
		if (!(!CMP_IS_EOL(header, str2) || *(cmp2+1)))
			break;

		if (!*(cmp1+1))
		{
			if (last_char_mass1&CMP_MASK_OF_INDEX_FLAG)
			{
				// Otherwise calculate conversion value separately and
				// shift pointer by properly number of bytes
				// (not only 1, may be bigger).
				str1 += GetComplex(str1, last_char_mass1&CMP_MASK_OF_INDEX, mass1, complex);
				cmp1 = mass1;
				last_char_mass1 = 0;
			}else
			{
				*cmp1 = last_char_mass1;
				str1++;
			}
		}else
		{
			cmp1++;
		}

		if (!*(cmp2+1))
		{
			if (last_char_mass2&CMP_MASK_OF_INDEX_FLAG)
			{
				// Otherwise calculate conversion value separately and
				// shift pointer by properly number of bytes
				// (not only 2, may be bigger).
				str2 += GetComplex(str2, last_char_mass2&CMP_MASK_OF_INDEX, mass2, complex);
				cmp2 = mass2;
				last_char_mass2 = 0;
			}else
			{
				*cmp2 = last_char_mass2;
				str2++;
			}
		}else
		{
			cmp2++;
		}

		// Compare. If first symbol smaller then other, then first 
		// string smaller then second. return -1.
		if (*cmp1 < *cmp2)
		{
			return -1;
		}
		else if (*cmp2 < *cmp1)
		{
			return 1;
		}
	}

	// Сравниваем остаточные веса.
	if (*(cmp1+1) > *(cmp2+1))
		return 1;
	if (*(cmp1+1) < *(cmp2+1))
		return -1;

	// skip unused symbols
	while (!CMP_IS_EOL(header, str1) && (last_char_mass1 = GetMass(*str1, m_CMPTable[aTableIndex].SimpleMassTable.data(), CMP_IGNORE_SYMBOL)) == CMP_IGNORE_SYMBOL)
		str1++;
	while (!CMP_IS_EOL(header, str2) && (last_char_mass2 = GetMass(*str2, m_CMPTable[aTableIndex].SimpleMassTable.data(), CMP_IGNORE_SYMBOL)) == CMP_IGNORE_SYMBOL)
		str2++;

	// if strings are equal (both string are empty now)
	if (CMP_IS_EOL(header, str1) && CMP_IS_EOL(header, str2))
	{
		return 0;
	}
	else if (CMP_IS_EOL(header, str1))
	{
		return -1;
	}
	else
	{
		return 1;
	}
}

// "local" version for StringRef based routines
static UInt16 GetComplex(const UInt16 *str, const UInt32 len, UInt16 index, UInt16 *mass, const CMPComplexType *complex)
{
	UInt32 best_len = 0;
	UInt16 best_index = 0;

	// Ищем наиболее подходящий сложный символ(символ наиболее полно совпадающий с строчкой)
	while (complex[index].chain[0] == str[0])
	{
		const auto &chain = complex[index].chain;
		// считаем количество совпадений.
		UInt32 count = 1; // одно совпадение уже есть.
		while (count < CMP_CHAIN_LEN && chain[count] == str[count] && count < len)
			count++;

		if (best_len < count && (count >= CMP_CHAIN_LEN || chain[count] == 0))
		{
			best_len = count;
			best_index = index;
		}
		index++;
	}

	// если хоть один символ был найден
	if (best_len)
	{
		// тогда возвращаем данные по нему.
		sld2::memcopy_n(mass, complex[best_index].mass, 4);
		return best_len;
	}

	// ничего не найдено - возвращаем символ игнорировать, и сообщаем, что
	// текущий символ нужно пропустить.
	mass[0] = CMP_NOT_FOUND_SYMBOL;
	return 1;
}

/**
 * Сравнение 2 строк по таблице сравнения (сравниваются веса символов)
 *
 * @param[in] aStr1        - первая из сравниваемых строк
 * @param[in] aStr2        - вторая из сравниваемых строк
 * @param[in] aTableIndex  - номер таблицы сравнения который нужно использовать
 *
 * @return  0 - строки равны
 *          1 - первая строка больше второй
 *         -1 - первая строка меньше второй
 */
Int32 CSldCompare::StrICmp(SldU16StringRef aStr1, SldU16StringRef aStr2, UInt32 aTableIndex) const
{
	if (aTableIndex >= m_CMPTable.size())
		return aStr1.compare(aStr2);

	if (aStr1.empty() && aStr2.empty())
		return 0;

	UInt16 mass1[CMP_CHAIN_LEN + 1] = { 0 };
	UInt16 mass2[CMP_CHAIN_LEN + 1] = { 0 };
	UInt16 *cmp1 = mass1, *cmp2 = mass2;

	// Вычисляем положение частей таблицы сравнения.
	const CMPHeaderType *header = m_CMPTable[aTableIndex].Header;
	const UInt16 *simple = m_CMPTable[aTableIndex].SimpleMassTable.data();
	const CMPComplexType *complex = m_CMPTable[aTableIndex].Complex;
	const UInt16 EOL = header->EOL;

	const UInt16 *str1 = aStr1.begin();
	const UInt16 *str1end = aStr1.end();
	const UInt16 *str2 = aStr2.begin();
	const UInt16 *str2end = aStr2.end();

#define cmp__not_eol(_s) (_s < _s ## end && *_s != EOL)

	// Compare strings while both strings not finished

	while ((cmp__not_eol(str1) || *(cmp1 + 1)) && (cmp__not_eol(str2) || *(cmp2 + 1)))
	{
		UInt16 last_char_mass1, last_char_mass2;

		// Пропускаем все игнорируемые символы.
		while (str1 < str1end && (last_char_mass1 = GetMass(*str1, simple, CMP_IGNORE_SYMBOL)) == CMP_IGNORE_SYMBOL)
			str1++;

		while (str2 < str2end && (last_char_mass2 = GetMass(*str2, simple, CMP_IGNORE_SYMBOL)) == CMP_IGNORE_SYMBOL)
			str2++;

		// Проверяем, не закончились ли строчки
		if (!(cmp__not_eol(str1) || *(cmp1 + 1)))
			break;
		if (!(cmp__not_eol(str2) || *(cmp2 + 1)))
			break;

		if (!*(cmp1 + 1))
		{
			if (last_char_mass1 & CMP_MASK_OF_INDEX_FLAG)
			{
				// Otherwise calculate conversion value separately and
				// shift pointer by properly number of bytes
				// (not only 1, may be bigger).
				str1 += ::GetComplex(str1, str1end - str1, last_char_mass1 & CMP_MASK_OF_INDEX, mass1, complex);
				cmp1 = mass1;
				last_char_mass1 = 0;
			}
			else
			{
				*cmp1 = last_char_mass1;
				str1++;
			}
		}
		else
		{
			cmp1++;
		}

		if (!*(cmp2 + 1))
		{
			if (last_char_mass2 & CMP_MASK_OF_INDEX_FLAG)
			{
				// Otherwise calculate conversion value separately and
				// shift pointer by properly number of bytes
				// (not only 2, may be bigger).
				str2 += ::GetComplex(str2, str2end - str2, last_char_mass2 & CMP_MASK_OF_INDEX, mass2, complex);
				cmp2 = mass2;
				last_char_mass2 = 0;
			}
			else
			{
				*cmp2 = last_char_mass2;
				str2++;
			}
		}
		else
		{
			cmp2++;
		}

		// Compare. If first symbol smaller then other, then first
		// string smaller then second. return -1.
		if (*cmp1 < *cmp2)
			return -1;

		if (*cmp2 < *cmp1)
			return 1;
	}

	// Сравниваем остаточные веса.
	if (*(cmp1 + 1) > *(cmp2 + 1))
		return 1;
	if (*(cmp1 + 1) < *(cmp2 + 1))
		return -1;

	// skip unused symbols
	while (str1 < str1end && GetMass(*str1, simple, CMP_IGNORE_SYMBOL) == CMP_IGNORE_SYMBOL)
		str1++;

	while (str2 < str2end && GetMass(*str2, simple, CMP_IGNORE_SYMBOL) == CMP_IGNORE_SYMBOL)
		str2++;

#undef cmp__not_eol

	return (str1 == str1end || *str1 == EOL) ? (str2 == str2end || *str2 == EOL) ? 0 : -1 : 1;
}

/**
 * Сравнение 2 строк по таблице сравнения (сравниваются веса символов)
 *
 * @param[in] aStr1  - первая из сравниваемых строк
 * @param[in] aStr2  - вторая из сравниваемых строк
 *
 * @return  0 - строки равны
 *          1 - первая строка больше второй
 *         -1 - первая строка меньше второй
 */
Int32 CSldCompare::StrICmp(SldU16StringRef aStr1, SldU16StringRef aStr2) const
{
	return StrICmp(aStr1, aStr2, m_DefaultTable);
}

/** ********************************************************************
* Возвращает массу переданного символа
* @param	aChr			- символ, массу которого нужно определить
* @param	aSimpleTable	- указатель на таблицу с описанием простых символов
* @param	aNotFound		- тип символа, возвращаемый в случае, если массу не удалось определить
*
* @return если символ есть в таблице, то его масса, иначе CMP_IGNORE_SYMBOL
************************************************************************/
UInt16 CSldCompare::GetMass(UInt16 aChr, const UInt16 *aSimpleTable, UInt16 aNotFound) const
{
	return aSimpleTable[aChr] == CMP_NOT_FOUND_SYMBOL ? aNotFound : aSimpleTable[aChr];
}

/** *********************************************************************
* Возвращает строку символов, имеющих в таблице сортировки с заданным индексом
* тот же вес, что и переданный символ
*
* ВНИМАНИЕ! Память для результирующей строки выделяется в этом методе
* и должна быть освобождена в вызывающем методе вызовом функции sldMemFree()
*
* @param[in]	aChr		- символ, для которого ищем все символы с такой же массой
* @param[in]	aTableIndex	- индекс таблицы сортировки	
*
* @return указатель на строку символов заданной массы
************************************************************************/
UInt16* CSldCompare::GetSimilarMassSymbols(UInt16 aChr, UInt32 aTableIndex) const
{
	if (aTableIndex > m_CMPTable.size())
		return NULL;

	const CMPHeaderType* header = m_CMPTable[aTableIndex].Header;
	const CMPSimpleType* simple = m_CMPTable[aTableIndex].Simple;

	UInt16 simpleCount = header->SimpleCount;
	UInt32 Version = header->Version;

	// Если это таблица второй версии, то символов с такой же массой просто нет - поэтому возвращаем NULL
	if (Version == CMP_VERSION_2)
		return NULL;

	// Определяем массу символа, если такой символ не найден возвращаем NULL
	UInt16 mass = GetMass(aChr, m_CMPTable[aTableIndex].SimpleMassTable.data(), CMP_NOT_FOUND_SYMBOL);
	if (mass == CMP_NOT_FOUND_SYMBOL)
		return NULL;
		
	UInt32 count = 0;
	for (int i=0; i < simpleCount; i++)
	{
		if (simple[i].ch == aChr)
			continue;

		if (simple[i].mass == mass)
			count++;
	}
	// Символ с такой массой единственный, поэтому возвращаем NULL
	if (count == 0)
		return NULL;

	// Выделяем память
	UInt16* symbols = (UInt16*)sldMemNew((count + 1)*sizeof(UInt16));
	if (!symbols)
		return NULL;

	// Заполняем наш массив
	count = 0;
	for (int i = 0; i < simpleCount; i++)
	{
		if (simple[i].ch == aChr)
			continue;

		if (simple[i].mass == mass)
			symbols[count++] = simple[i].ch;
	}
	symbols[count] = 0;

	return symbols;	
}

/** *********************************************************************
* Возвращает строку символов, имеющих во всех таблицах сортировки
* тот же вес, что и переданный символ; символы строки отсортированы
*
* ВНИМАНИЕ! Память для результирующей строки выделяется в этом методе
* и должна быть освобождена в вызывающем методе вызовом функции sldMemFree()
*
* @param[in]	aCh	- символ, для которого ищем все символы с такой же массой	
*
* @return указатель на строку символов заданной массы
************************************************************************/
UInt16* CSldCompare::GetSimilarMassSymbols(UInt16 aCh) const
{
	UInt16 *defaultTableSyms = GetSimilarMassSymbols(aCh, m_DefaultTable);
	if (!defaultTableSyms)
		return NULL;

	UInt16 *otherTableSyms;

	UInt32 len = StrLen(defaultTableSyms);
	UInt32 count = 0;
	
	for (UInt32 i = 0; i < m_CMPTable.size(); i++)
	{
		if (i == m_DefaultTable)
			continue;

		otherTableSyms = GetSimilarMassSymbols(aCh, i);
		if (!otherTableSyms)
			continue;
		
		UInt32 j = 0; 
		while(otherTableSyms[j]) 
		{
			if (!sld2::binary_search(defaultTableSyms, len, otherTableSyms[j]))
				count++;

			j++;
		}
		sldMemFree(otherTableSyms);
	}

	UInt16 *result = (UInt16 *) sldMemNew((len + count + 1)*sizeof(UInt16));
	if (!result)
		return NULL;
	
	count = StrCopy(result, defaultTableSyms);

	for (UInt32 i = 0; i < m_CMPTable.size(); i++)
	{
		if (i == m_DefaultTable)
			continue;

		otherTableSyms = GetSimilarMassSymbols(aCh, i);
		if (!otherTableSyms)
			continue;
		
		UInt32 j = 0; 
		while (otherTableSyms[j])
		{
			if (!sld2::binary_search(defaultTableSyms, len, otherTableSyms[j]))
				result[count++] = otherTableSyms[j];

			j++;
		}
		sldMemFree(otherTableSyms);
	}
	sldMemFree(defaultTableSyms);

	result[count] = 0;

	if (count > len)
		DoQuickSort(result, 0, count - 1);

	return result;
}

/***********************************************************************
* Быстрая сортировка
*
* @param aStr			- указатель на массив
* @param aFirstIndex	- индекс первого элемента в массиве
* @param aLastIndex		- индекс последнего элемента в массиве
*
* @return
************************************************************************/
void CSldCompare::DoQuickSort(UInt16 *aStr, Int32 aFirstIndex, Int32 aLastIndex)
{
	Int32 i = aFirstIndex;
	Int32 j = aLastIndex;
	UInt16 p = aStr[(i + j) >> 1];
 
    do
    {
        while (aStr[i] < p)
			i++;
		
		while (aStr[j] > p)
			j--;
        
        if (i <= j)
        {
            if (i < j)
				Swap(aStr, i, j);

			i++;
            j--;
        }
    }
    while (i <= j);
    
	if (i < aLastIndex)
		DoQuickSort(aStr, i, aLastIndex);
		
	if (aFirstIndex < j)
		DoQuickSort(aStr, aFirstIndex, j);
}

/***********************************************************************
* Обмен двух элементов в сортируемом массиве
*
* @param aStr			- указатель на массив элементов
* @param aFirstIndex	- номер первого элемента массива для обмена
* @param aSecondIndex	- номер второго элемента массива для обмена
*
* @return
************************************************************************/
void CSldCompare::Swap(UInt16 *aStr, Int32 aFirstIndex, Int32 aSecondIndex)
{
	UInt16 temp = aStr[aFirstIndex];
	aStr[aFirstIndex] = aStr[aSecondIndex];
	aStr[aSecondIndex] = temp;
}

/**
*********************************************************************
+ DESCRIPTION:	This function finds description for the complex symbols,
+				and substitute one with masses.
+
+ PARAMETERS:	UInt16 *str - pointer to string for matching with complex 
+								symbols
+				UInt16 index - index of start complex table item
+				UInt16 *mass - pointer to the array for masses of the 
+								complex symbol.
+				CMPComplexType *complex - pointer to the table with 
+								description of the complex symbols.
+
+ RETURNED:		number of chars we need skip.
+
+ REVISION HISTORY:
+
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/** ********************************************************************
* 
* @param	str		- 
* @param	index	- 
* @param	mass	- 
* @param	complex	- 
*
* @return 
************************************************************************/
UInt32 CSldCompare::GetComplex(const UInt16 *str, UInt16 index, UInt16 *mass, const CMPComplexType *complex) const
{
	UInt32 best_len = 0;
	UInt16 best_index = 0;
	UInt32 count;

	// Ищем наиболее подходящий сложный символ(символ наиболее полно совпадающий с строчкой)
	while (complex[index].chain[0] == *str)
	{
		// считаем количество совпадений.
		count = 1;// одно совпадение уже есть.
		while (count < CMP_CHAIN_LEN && complex[index].chain[count] == str[count] && str[count])
		{
			count++;
		}

		if (best_len < count && (count >= CMP_CHAIN_LEN || complex[index].chain[count] == 0))
		{
			best_len = count;
			best_index = index;
		}
		index++;
	}

	// если хоть один символ был найден
	if (best_len)
	{
		// тогда возвращаем данные по нему.
		mass[0] = complex[best_index].mass[0];
		mass[1] = complex[best_index].mass[1];
		mass[2] = complex[best_index].mass[2];
		mass[3] = complex[best_index].mass[3];
		return best_len;
	}

	// ничего не найдено - возвращаем символ игнорировать, и сообщаем, что 
	// текущий символ нужно пропустить.
	mass[0] = CMP_NOT_FOUND_SYMBOL;
	return 1;
}


/** ********************************************************************
*  Установлена ли дополнительная таблица сравнения для текущего языка?
*  
* @return bool (true - имеется, false - нет)
************************************************************************/
bool CSldCompare::IsAddTableDefine() const
{
	return (m_DefaultAddTable != 0xFFFF);
}


/** ********************************************************************
*  Устанавливает таблицу сравнения для указанного языка
*  
* @param[in]	aLanguageCode	- код языка для которого нужно установить 
*								  таблицу сравнения
*
* @return код ошибки
************************************************************************/
ESldError CSldCompare::SetDefaultLanguage(ESldLanguage aLanguageCode)
{
	bool tableIsSet = false;	
	for (UInt32 i = 0; i < m_CMPTable.size(); i++)
	{
		if (m_CMPTableInfo[i].LanguageCode == aLanguageCode)
		{
			if (m_CMPTableInfo[i].Priority == 0) 
			{
				m_DefaultTable = i;
				tableIsSet = true;
			}
			else if (m_CMPTableInfo[i].Priority == 1)
				m_DefaultAddTable = i;
		}
	}
	if (tableIsSet)
		return  eOK;

	return eCommonWrongLanguage;
}


/** *********************************************************************
* Возвращает количество таблиц сравнения
*  
* @param[out]	aCount	- сюда сохраняется количество таблиц сравнения
*
* @return код ошибки
************************************************************************/
ESldError CSldCompare::GetTablesCount(UInt32* aCount) const
{
	if (!aCount)
		return eMemoryNullPointer;

	*aCount = m_CMPTable.size();
	
	return eOK;
}

/** *********************************************************************
* Возвращает код языка таблицы сравнения по номеру таблицы
*  
* @param[in]	aTableIndex	- номер таблицы сравнения
* @param[out]	aLangCode	- указатель на переменную, куда сохраняется код языка
*
* @return код ошибки
************************************************************************/
ESldLanguage CSldCompare::GetTableLanguage(UInt32 aTableIndex) const
{
	if (aTableIndex >= m_CMPTable.size())
		return SldLanguage::Unknown;
	
	return SldLanguage::fromCode(m_CMPTableInfo[aTableIndex].LanguageCode);
}

/** *********************************************************************
* Возвращает флаг того, есть ли в таблице сравнения
* таблица пар соответствий символов верхнего и нижнего регистров определенного типа
*  
* @param[in]	aTableIndex	- номер таблицы сравнения
* @param[in]	aTableType	- тип запрашиваемой таблицы пар соответствий
* @param[out]	aFlag		- сюда сохраняется флаг:
*							  0 - нет,
*							  1 - есть
*
* @return код ошибки
************************************************************************/
ESldError CSldCompare::IsTableHasSymbolPairTable(UInt32 aTableIndex, ESymbolPairTableTypeEnum aTableType, UInt32* aFlag) const
{
	if (!aFlag)
		return eMemoryNullPointer;
	*aFlag = 0;
	
	if (aTableIndex >= m_CMPTable.size())
		return eCommonWrongIndex;
	
	// Нет таблиц преобразования вообще
	if (!(m_CMPTable[aTableIndex].Header->FeatureFlag & SLD_CMP_TABLE_FEATURE_SYMBOL_PAIR_TABLE))
		return eOK;
	
	// Есть ли в нужной таблице элементы
	UInt32 ElementsCount = 0;
	switch (aTableType)
	{
		case eSymbolPairTableType_Native:
		{
			ElementsCount = m_CMPTable[aTableIndex].HeaderPairSymbols->NativePairCount;
			break;
		}
		case eSymbolPairTableType_Common:
		{
			ElementsCount = m_CMPTable[aTableIndex].HeaderPairSymbols->CommonPairCount;
			break;
		}
		case eSymbolPairTableType_Dictionary:
		{
			ElementsCount = m_CMPTable[aTableIndex].HeaderPairSymbols->DictionaryPairCount;
			break;
		}
		default:
		{
			break;
		}
	}
	
	if (!ElementsCount)
		return eOK;
	
	*aFlag = 1;
	
	return eOK;
}

/** *********************************************************************
* Проверяет, принадлежит ли символ определеленному языку,
* либо к общим символам-разделителям для всех языков словарной базы (aLang == SldLanguage::Delimiters)
*
* @param[in]	aSymbolCode	- код проверяемого символа
* @param[in]	aLang		- код языка или константа SldLanguage::Delimiters - код языка для общих символов-разделителей
* @param[out]	aFlag		- указатель на переменную, куда сохраняется результат:
*							  0 - символ не принадлежит языку,
*							  1 - символ принадлежит языку
*
* @param[out]	aResultFlag	- указатель на переменную, куда будет сохранен флаг результата:
*							  1 - результат однозначно определен,
*							  0 - результат не определен (соответствующая таблица символов либо сам символ отсутствует)
*
* @return код ошибки
************************************************************************/
ESldError CSldCompare::IsSymbolBelongToLanguage(UInt16 aSymbolCode, ESldLanguage aLang, UInt32* aFlag, UInt32* aResultFlag) const
{
	if (!aFlag || !aResultFlag)
		return eMemoryNullPointer;

	*aFlag = 0;
	*aResultFlag = 0;

	ESldError error;
	UInt32 Flag = 0;
	UInt32 Count = 0;

	// Если хотят узнать, принадлежит ли символ к общим символам-разделителям для всех языков словарной базы,
	// смотрим только в соответствующей таблице символов и выходим
	if (aLang == SldLanguage::Delimiters)
	{
		// если это таблицы четвертой версии и более, то смотрим символы разделители в них
		if (m_CMPTable[0].Header->DelimiterCount)
		{
			*aResultFlag = 1;
			for (UInt32 tableIdx = 0; tableIdx < m_CMPTable.size(); tableIdx++)
			{
				if (m_CMPTableInfo[tableIdx].Priority != 0)
					continue;

				if(aSymbolCode < CMP_MOST_USAGE_SYMBOL_RANGE)
				{
					if(m_CMPTable[tableIdx].MostUsageCharTable[aSymbolCode] == eCmpDelimiterCharType)
					{
						*aFlag = 1;
						return eOK;
					}
					else
					{
						continue;
					}
				}
				else if (sld2::binary_search(m_CMPTable[tableIdx].Delimiter, m_CMPTable[tableIdx].Header->DelimiterCount, aSymbolCode))
				{
					*aFlag = 1;
					return eOK;
				}
			}
			return eOK;
		}

		for (auto&& table : m_LanguageSymbolsTable)
		{
			if (table.GetLanguageCode() == aLang)
			{
				*aFlag = table.IsSymbolBelongToLanguage(aSymbolCode);
				*aResultFlag = 1;
				break;
			}
		}
		
		return eOK;
	}

	// Найдем таблицу сортировки для нужного языка и посмотрим,
	// есть ли там таблицы пар соответствий символов типа eSymbolPairTableType_Native
	for (UInt32 tableIdx = 0; tableIdx < m_CMPTable.size(); tableIdx++)
	{
		if (m_CMPTableInfo[tableIdx].LanguageCode == aLang)
		{
			if (m_CMPTable[tableIdx].Header->NativeCount)
			{
				*aResultFlag = 1;

				if(aSymbolCode < CMP_MOST_USAGE_SYMBOL_RANGE)
				{
					*aFlag = m_CMPTable[tableIdx].MostUsageCharTable[aSymbolCode] == eCmpNativeCharType;
				}
				else
				{
					*aFlag = sld2::binary_search(m_CMPTable[tableIdx].Native, m_CMPTable[tableIdx].Header->NativeCount, aSymbolCode);
				}
				return eOK;
			}
			
			error = IsTableHasSymbolPairTable(tableIdx, eSymbolPairTableType_Native, &Flag);
			if (error != eOK)
				return error;
			
			// Если есть нужная таблица - ищем по ней линейным поиском по всем элементам up и low, и выходим
			// Эта таблица имеет больший приоритет для распознавания языка символа, чем таблицы символов
			if (Flag)
			{
				// Найдем символ или нет - результат однозначный
				*aResultFlag = 1;
				
				CMPSymbolPair* pTable = m_CMPTable[tableIdx].NativePair;
				Count = m_CMPTable[tableIdx].HeaderPairSymbols->NativePairCount;
				
				for (UInt32 charIdx = 0; charIdx < Count; charIdx++)
				{
					if (pTable[charIdx].up == aSymbolCode || pTable[charIdx].low == aSymbolCode)
					{
						*aFlag = 1;
						break;
					}
				}
				
				return eOK;
			}
		}
	}

	// Если нет таблицы сортировки для нужного языка, или в ней нет нужной таблицы пар соответствий символов,
	// или в ней нет символов - ищем по таблице символов, и выходим
	for (auto&& table : m_LanguageSymbolsTable)
	{
		if (table.GetLanguageCode() == aLang)
		{
			*aFlag = table.IsSymbolBelongToLanguage(aSymbolCode);
			*aResultFlag = 1;
			break;
		}
	}

	return eOK;
}

/** *********************************************************************
* Проверяет, является ли символ разделителем в конкретном языке
*
* @param[in]	aSymbolCode	- код проверяемого символа
* @param[in]	aLangCode	- код проверяемого языка
* @param[out]	aFlag		- указатель на переменную, куда сохраняется результат:
*							  0 - не является,
*							  1 - является
*
* @param[out]	aResultFlag	- указатель на переменную, куда будет сохранен флаг результата:
*							  0 - результат не может быть определен,
*							  1 - результат однозначно определен
*
* @return код ошибки
************************************************************************/
ESldError CSldCompare::IsSymbolBelongToLanguageDelimiters(UInt16 aSymbolCode, ESldLanguage aLang, UInt32* aFlag, UInt32* aResultFlag) const
{
	if (!aFlag || !aResultFlag)
		return eMemoryNullPointer;
	
	*aFlag = 0;
	*aResultFlag = 0;

	// Если в таблице сортировки есть символы-разделители, то смотрим только в них и выходим
	for (UInt32 tableIdx = 0; tableIdx < m_CMPTable.size(); tableIdx++)
	{
		if (m_CMPTableInfo[tableIdx].LanguageCode == aLang && m_CMPTable[tableIdx].Header->DelimiterCount)
		{
			*aResultFlag = 1;

			if (aSymbolCode < CMP_MOST_USAGE_SYMBOL_RANGE)
			{
				*aFlag = m_CMPTable[tableIdx].MostUsageCharTable[aSymbolCode] == eCmpDelimiterCharType;
			}
			else
			{
				*aFlag = sld2::binary_search(m_CMPTable[tableIdx].Delimiter, m_CMPTable[tableIdx].Header->DelimiterCount, aSymbolCode);
			}

			return eOK;
		}
	}
	
	ESldError error;

	// Если есть таблицы символов-разделителей для конкретных языков - смотрим только в них и выходим
	if (m_LanguageDelimiterSymbolsTable.size() && aLang != SldLanguage::Delimiters)
	{
		for (auto&& table : m_LanguageDelimiterSymbolsTable)
		{
			if (table.GetLanguageCode() == aLang)
			{
				*aFlag = table.IsSymbolBelongToLanguage(aSymbolCode);
				*aResultFlag = 1;
				break;
			}
		}
	}
	// Если нет таблиц, ищем в общей таблице символов-разделителей для всех языков
	else
	{
		error = IsSymbolBelongToLanguage(aSymbolCode, SldLanguage::Delimiters, aFlag, aResultFlag);
		if (error != eOK)
			return error;
	}
	
	return eOK;
}

/** *********************************************************************
* Возвращает язык таблицы сравнения, используемой по умолчанию
*  
* @param[out]	aLanguageCode - указатель на переменную, куда сохраняется код языка
*
* @return код ошибки
************************************************************************/
ESldLanguage CSldCompare::GetDefaultLanguage() const
{
	return GetTableLanguage(m_DefaultTable);
}

/** *********************************************************************
* Метод подготавливает строку для сравнения по шаблону
*  
* @param[out]	aDestStr	- сюда записывается результирующая строка, память выделяется и освобождается снаружи
* @param[in]	aSourceStr	- исходная строка
*
* @return код ошибки
************************************************************************/
UInt32 CSldCompare::PrepareTextForAnagramSearch(UInt16* aDestStr, const UInt16* aSourceStr)
{
	if (!aDestStr || !aSourceStr)
		return 0;
	
	const UInt16* src = aSourceStr;
	UInt32 charCount = 0;

	while (*src)
	{
		// Delete combining marks
		if (*src >= 0x0300 && *src <= 0x036F)
		{
			src++;
			continue;
		}
		aDestStr[charCount++] = *src++;
	}

	aDestStr[charCount] = 0;

	//while(charCount && IsDelimiter(aDestStr[charCount], m_DefaultTable))
	//{
	//	aDestStr[charCount] = 0;
	//}
	
	return charCount;
}

/** *********************************************************************
* Кодирует спец. символы в строке запроса ('&', '|', '(', ')', '!', '*', '?')
*  
* @param[out]	aDestStr	- сюда записывается результирующая строка, память выделяется и освобождается снаружи
* @param[in]	aSourceStr	- исходная строка
*
* @return код ошибки
************************************************************************/
ESldError CSldCompare::EncodeSearchQuery(UInt16* aDestStr, const UInt16* aSourceStr)
{
	if (!aDestStr || !aSourceStr)
		return eMemoryNullPointer;
	
	const UInt16* src = aSourceStr;
	UInt16* dst = aDestStr;
	UInt16 ch = 0;
	while (*src)
	{
		if (*src == CMP_QUERY_SPECIAL_SYMBOL_ESCAPE_CHAR)
		{
			ch = *(src+1);
			if (ch)
			{
				if (ch == CMP_QUERY_SPECIAL_SYMBOL_AND) // ZФ
				{
					*dst++ = 0x005A;
					*dst++ = 0x0424;
					src += 2;
				}
				else if (ch == CMP_QUERY_SPECIAL_SYMBOL_OR) // ZЭ
				{
					*dst++ = 0x005A;
					*dst++ = 0x042D;
					src += 2;
				}
				else if (ch == CMP_QUERY_SPECIAL_SYMBOL_NOT) // ZЩ
				{
					*dst++ = 0x005A;
					*dst++ = 0x0429;
					src += 2;
				}
				else if (ch == CMP_QUERY_SPECIAL_SYMBOL_OPEN_BR) // ZЦ
				{
					*dst++ = 0x005A;
					*dst++ = 0x0426;
					src += 2;
				}
				else if (ch == CMP_QUERY_SPECIAL_SYMBOL_CLOSE_BR) // QФ
				{
					*dst++ = 0x0051;
					*dst++ = 0x0424;
					src += 2;
				}
				else if (ch == CMP_QUERY_SPECIAL_SYMBOL_ANY_CHARS) // QЭ
				{
					*dst++ = 0x0051;
					*dst++ = 0x042D;
					src += 2;
				}
				else if (ch == CMP_QUERY_SPECIAL_SYMBOL_ONE_CHAR) // QЩ
				{
					*dst++ = 0x0051;
					*dst++ = 0x0429;
					src += 2;
				}
				else
				{
					*dst++ = *src++;
				}
			}
			else
				src++;
		}
		else
			*dst++ = *src++;
	}
	*dst = 0;
	
	return eOK;
}

/** *********************************************************************
* Кодирует спец. символы в слове из словаря ('&', '|', '(', ')', '!', '*', '?')
*  
* @param[out]	aDestStr	- сюда записывается результирующая строка, память выделяется и освобождается снаружи
* @param[in]	aSourceStr	- исходная строка
*
* @return код ошибки
************************************************************************/
ESldError CSldCompare::EncodeSearchWord(UInt16* aDestStr, const UInt16* aSourceStr)
{
	if (!aDestStr || !aSourceStr)
		return eMemoryNullPointer;
	
	const UInt16* src = aSourceStr;
	UInt16* dst = aDestStr;
	UInt16 ch = 0;
	while (*src)
	{
		ch = *src;
		if (ch == CMP_QUERY_SPECIAL_SYMBOL_AND) // ZФ
		{
			*dst++ = 0x005A;
			*dst++ = 0x0424;
		}
		else if (ch == CMP_QUERY_SPECIAL_SYMBOL_OR) // ZЭ
		{
			*dst++ = 0x005A;
			*dst++ = 0x042D;
		}
		else if (ch == CMP_QUERY_SPECIAL_SYMBOL_NOT) // ZЩ
		{
			*dst++ = 0x005A;
			*dst++ = 0x0429;
		}
		else if (ch == CMP_QUERY_SPECIAL_SYMBOL_OPEN_BR) // ZЦ
		{
			*dst++ = 0x005A;
			*dst++ = 0x0426;
		}
		else if (ch == CMP_QUERY_SPECIAL_SYMBOL_CLOSE_BR) // QФ
		{
			*dst++ = 0x0051;
			*dst++ = 0x0424;
		}
		else if (ch == CMP_QUERY_SPECIAL_SYMBOL_ANY_CHARS) // QЭ
		{
			*dst++ = 0x0051;
			*dst++ = 0x042D;
		}
		else if (ch == CMP_QUERY_SPECIAL_SYMBOL_ONE_CHAR) // QЩ
		{
			*dst++ = 0x0051;
			*dst++ = 0x0429;
		}
		else
		{
			*dst++ = ch;
		}
		src++;
	}
	*dst = 0;
	
	return eOK;
}

/** *********************************************************************
* Метод сравнения шаблона и слова на соответствие
*
* @param[in]	aWildCard	- строка масс шаблона
* @param[in]	aText		- масса слова (без пробелов) для сравнения на соответствие шаблону
*
* @return	0 - слово не соответствует шаблону
*			1 - слово соответствует шаблону
************************************************************************/
Int8 CSldCompare::DoWildCompare(const UInt16* aTemplate, const UInt16* aText) const
{
	Int8 anyCharActive = 0;

	while (*aText)
	{
		if (*aText == *aTemplate)
		{
			if (anyCharActive != 0)
			{
				if (DoWildCompare(aTemplate, aText))
					return 1;
				else
					aText++;
			}
			else if (anyCharActive != 0)
			{
				aText++;
			}
			else
			{
				aTemplate++;
				aText++;
			}
			continue;
		}

		if (*aText == CMP_MASS_ZERO || anyCharActive != 0)
		{
			aText++;
			continue;
		}

		if (*aTemplate == CMP_MASS_SPECIAL_SYMBOL_ANY_CHARS)
		{
			aTemplate++;
			anyCharActive = 1;
			continue;
		}

		if(*aTemplate == CMP_MASS_SPECIAL_SYMBOL_ONE_CHAR)
		{
			aTemplate++;
			aText++;
			continue;
		}

		break;
	}

	while (*aTemplate == CMP_MASS_SPECIAL_SYMBOL_ANY_CHARS || (*aTemplate == CMP_MASS_SPECIAL_SYMBOL_ONE_CHAR && anyCharActive != 0))
	{
		aTemplate++;
	}
	
	if ((*aText == 0 || *aText == CMP_MASS_DELIMITER) && (*aTemplate == 0))
		return 1;
	else
		return 0;
}

/** *********************************************************************
* Метод сравнения шаблона и строчки на соответствие
*
* @param[in]	aWildCard	- строка масс шаблона
* @param[in]	aText		- строчка масс для сравнения на соответствие шаблону
*
* @return	0 - строчка не соответствует шаблону
*			1 - строчка соответствует шаблону
************************************************************************/
UInt32 CSldCompare::WildCompare(const UInt16* aWildCard, const UInt16* aText) const
{
	UInt8 Result = 0;
	Result = DoWildCompare(aWildCard, aText);
	if (Result)
		return 1;
	
	// Продолжаем искать с ближайшего разделителя
	while (*aText)
	{		
		if(*aText == CMP_MASS_DELIMITER)
		{
			while(*aText == CMP_MASS_DELIMITER)
			{
				aText++;
			}
			if(DoWildCompare(aWildCard, aText))
			{
				return 1;
			}
		}
		else
		{
			aText++;
		}
	}

	return 0;
}

UInt8 CSldCompare::GetCompareLen(const UInt16* aWildCard, const UInt16* aText) const
{
	UInt8 Result = (UInt8)(~0);
	if (DoWildCompare(aWildCard, aText))
		return Result;

	// Продолжаем искать с ближайшего разделителя
	while (*aText)
	{
		if (*aText == CMP_MASS_DELIMITER)
		{
			Result--;
			while (*aText == CMP_MASS_DELIMITER)
			{
				aText++;
			}
			if (DoWildCompare(aWildCard, aText))
			{
				return Result;
			}
		}
		else
		{
			aText++;
		}
	}

	return 0;
}

/** *********************************************************************
* Метод сравнения анаграмм
*
* @param[in]	aSearchStr		- шаблон
* @param[in]	aCurrentWord	- строчка для сравнения на соответствие шаблону
* @param[in]	aFlagArray		- строчка в которой сохраняются флаги, что символ
*								  с определенным индексом уже учтен при сравнении
*								  для ускорения работы память для строчки выделяется снаружи
* @param[in]	aSearchStrLen	- длинна сравниваемых строк (они должны быть равны)
*
* @return	0 - данные строки не анаграммы
*			1 - данные строки анаграммы
************************************************************************/
UInt8 CSldCompare::AnagramCompare(UInt16* aSearchStr, const UInt16* aCurrentWord, UInt8* aFlagArray, UInt32 aSearchStrLen) const
{
	if(!aSearchStr || !aCurrentWord || !aFlagArray || !aSearchStrLen)
		return 0;

	sldMemZero(aFlagArray, sizeof(UInt8) * aSearchStrLen);

	for (Int32 k = 0; k < aSearchStrLen; k++)
	{
		UInt8 found = 0;
		for (Int32 j = 0; j < aSearchStrLen; j++)
		{
			if (m_CMPTable[m_DefaultTable].SimpleMassTable[aSearchStr[k]] == m_CMPTable[m_DefaultTable].SimpleMassTable[aCurrentWord[j]] && !aFlagArray[j])
			{
				found = 1;
				aFlagArray[j]++;
				break;
			}
		}
		if (!found)
		{
			return 0;
		}
	}

	return 1;
}

/** *********************************************************************
* Метод корректирует "неумный" запрос для поиска по шаблону:
* - добавляет логическое "И" между словами запроса
* - добавляет "*" в начало и конец каждого слова
*
* ВНИМАНИЕ! Память для строки aOut выделяется в этом методе
* и должна быть освобождена в вызывающем методе вызовом функции sldMemFree()
*
* @param[in]	aQuery	- строка запроса
* @param[out]	aOut	- указатель, по которому записывается указатель на строку с результатом
*
* @return код ошибки
************************************************************************/
ESldError CSldCompare::CorrectNonSmartWildCardSearchQuery(const UInt16* aQuery, UInt16** aOut)
{
	// "text1 text2" -> "(*text1*)&(*text2*)"
	
	if (!aQuery || !aOut)
		return eMemoryNullPointer;
	
	*aOut = NULL;
	
	Int32 Len = StrLen(aQuery);
	Int32 DestLen = (Len+1)*4;

	UInt16* Dest = (UInt16*)sldMemNewZero(DestLen*sizeof(Dest[0]));
	if (!Dest)
		return eMemoryNotEnoughMemory;

	*aOut = Dest;
	
	const UInt16* SourcePtr = aQuery;
	UInt16* DestPtr = Dest;
	
	Int32 WordsCount = 0;
	while (*SourcePtr)
	{
		while (*SourcePtr && *SourcePtr==' ')
			SourcePtr++;
		
		if (!(*SourcePtr))
		{
			if (!WordsCount)
				*DestPtr++ = '*';
			break;
		}
		
		if (WordsCount)
			*DestPtr++ = '&';
		*DestPtr++ = '(';
		*DestPtr++ = '*';
		
		while (*SourcePtr && *SourcePtr!=' ')
			*DestPtr++ = *SourcePtr++;
		
		*DestPtr++ = '*';
		*DestPtr++ = ')';
		
		WordsCount++;
	}
	
	*DestPtr = 0;
	
	return eOK;
}

/** *********************************************************************
* Метод корректирует "умный" запрос для поиска по шаблону
*
* ВНИМАНИЕ! Память для строки aOut выделяется в этом методе
* и должна быть освобождена в вызывающем методе вызовом функции sldMemFree()
*
* @param[in]	aQuery	- строка запроса
* @param[out]	aOut	- указатель, по которому записывается указатель на строку с результатом
*
* @return код ошибки
************************************************************************/
ESldError CSldCompare::CorrectSmartWildCardSearchQuery(const UInt16* aQuery, UInt16** aOut)
{
	if (!aQuery || !aOut)
		return eMemoryNullPointer;
	
	*aOut = NULL;
	
	Int32 Len = StrLen(aQuery);
	Int32 DestLen = (Len+1)*4;

	UInt16* Dest = (UInt16*)sldMemNewZero(DestLen*sizeof(Dest[0]));
	if (!Dest)
		return eMemoryNotEnoughMemory;

	*aOut = Dest;
	
	StrCopy(Dest, aQuery);
	Len = StrLen(Dest);
	
	// если запрос пустой, то заменяем его на '*'
	if (!Len)
	{
		Dest[0] = '*';
		Dest[1] = 0;
		return eOK;
	}
	
	// заменям символы табуляции на пробелы
	Int32 i = 0;
	while (Dest[i])
	{
		if (Dest[i] == '\t')
			Dest[i] = ' ';
		i++;
	}
	
	return eOK;
}

/** *********************************************************************
* Метод корректирует "неумный" запрос для полнотекстового поиска
* (добавляет логическое "И" между словами запроса)
*
* ВНИМАНИЕ! Память для строки aOut выделяется в этом методе
* и должна быть освобождена в вызывающем методе вызовом функции sldMemFree()
*
* @param[in]	aQuery	- строка запроса
* @param[out]	aOut	- указатель, по которому записывается указатель на строку с результатом
*
* @return код ошибки
************************************************************************/
ESldError CSldCompare::CorrectNonSmartFullTextSearchQuery(const UInt16* aQuery, UInt16** aOut)
{
	// "text1 text2" -> "(text1)&(text2)"
	
	if (!aQuery || !aOut)
		return eMemoryNullPointer;
	
	*aOut = NULL;
	
	Int32 Len = StrLen(aQuery);
	Int32 DestLen = (Len+1)*4;

	UInt16* Dest = (UInt16*)sldMemNewZero(DestLen*sizeof(Dest[0]));
	if (!Dest)
		return eMemoryNotEnoughMemory;

	*aOut = Dest;
	
	const UInt16* SourcePtr = aQuery;
	UInt16* DestPtr = Dest;
	
	Int32 WordsCount = 0;
	while (*SourcePtr)
	{
		while (*SourcePtr && *SourcePtr==' ')
			SourcePtr++;
		
		if (!(*SourcePtr))
			break;
		
		if (WordsCount)
			*DestPtr++ = '&';
		*DestPtr++ = '(';
		
		while (*SourcePtr && *SourcePtr!=' ')
			*DestPtr++ = *SourcePtr++;
		
		*DestPtr++ = ')';
		
		WordsCount++;
	}
	
	*DestPtr = 0;
	
	return eOK;
}

/** *********************************************************************
* Метод корректирует "умный" запрос для полнотекстового поиска
*
* ВНИМАНИЕ! Память для строки aOut выделяется в этом методе
* и должна быть освобождена в вызывающем методе вызовом функции sldMemFree()
*
* @param[in]	aQuery		- строка запроса
* @param[out]	aOut		- указатель, по которому записывается указатель на строку с результатом
*
* @return	код ошибки
************************************************************************/
ESldError CSldCompare::CorrectSmartFullTextSearchQuery(const UInt16* aQuery, UInt16** aOut)
{
	if (!aQuery || !aOut)
		return eMemoryNullPointer;
	
	*aOut = NULL;
	
	Int32 Len = StrLen(aQuery);
	Int32 DestLen = (Len+1)*4;

	UInt16* Dest = (UInt16*)sldMemNewZero(DestLen*sizeof(Dest[0]));
	if (!Dest)
		return eMemoryNotEnoughMemory;

	*aOut = Dest;
	
	StrCopy(Dest, aQuery);
	Len = StrLen(Dest);
	
	// если запрос пустой
	if (!Len)
		return eOK;
	
	// заменям символы табуляции на пробелы
	Int32 i = 0;
	while (Dest[i])
	{
		if (Dest[i] == '\t')
			Dest[i] = ' ';
		i++;
	}
	
	return eOK;
}

/** *********************************************************************
* Метод проверяет наличие в поисковом запросе спец. символов поиска по шаблону ('*' и '?')
*
* @param[in]	aStr	- исходная строка 
*
* @return	0 - строка не содержит символы
*			1 - строка содержит символы
************************************************************************/
UInt32 CSldCompare::QueryIsExistWildSym(const UInt16* aStr)
{
	if (!aStr)
		return 0;
	
	UInt32 i = 0;
	while (aStr[i])
	{
		if (aStr[i] == '*' || aStr[i] == '?')
			return 1;
			
		i++;
	}
	
	return 0;
}

/** *********************************************************************
* Метод проверяет наличие в строке (предположительно слове из списка слов)
* спец. символов поиска по шаблону ('*' и '?')
*
* @param[in]	aStr	- исходная строка 
*
* @return	0 - строка не содержит символы
*			1 - строка содержит символы
************************************************************************/
UInt32 CSldCompare::IsWordHasWildCardSymbols(const UInt16* aStr)
{
	if (!aStr)
		return 0;
	
	UInt32 i = 0;
	while (aStr[i])
	{
		if (aStr[i] == '*' || aStr[i] == '?')
			return 1;
		i++;
	}
	
	return 0;
}

/** *********************************************************************
* Метод проверяет, есть ли в строке разделители
*
* @param[in]	aStr	- строка поиска
*
* @return	1 - есть
*			0 - нет
************************************************************************/
UInt32 CSldCompare::QueryIsExistDelim(const UInt16* aStr)
{
	if (!aStr)
		return 0;
		
	UInt32 i = 0;
	while (aStr[i])
	{ 
		if (aStr[i] == '(' || 
			aStr[i] == ')' || 
			aStr[i] == '|' ||
			aStr[i] == '&' || 
			aStr[i] == '!' ||
			aStr[i] == ' ')
			return 1;
			
		i++;
	}
	
	return 0;
}

/** *********************************************************************
* Метод проверяет, является ли запрос для поиска по шаблону "умным"
*
* @param[in]	aStr	- строка запроса
*
* @return	1 - да
*			0 - нет
************************************************************************/
UInt32 CSldCompare::IsSmartWildCardSearchQuery(const UInt16* aStr)
{
	if (!aStr)
		return 0;
		
	UInt32 i = 0;
	while (aStr[i])
	{ 
		if (aStr[i] == '(' || 
			aStr[i] == ')' || 
			aStr[i] == '|' ||
			aStr[i] == '&' || 
			aStr[i] == '!')
			return 1;
			
		i++;
	}
	
	return 0;
}

/***********************************************************************
* Метод проверяет, является ли запрос для полнотекстового поиска "умным"
*
* @param aStr - строка запроса
*
* @return	1 - да
*			0 - нет
************************************************************************/
UInt32 CSldCompare::IsSmartFullTextSearchQuery(const UInt16* aStr)
{
	if (!aStr)
		return 0;
		
	UInt32 i = 0;
	while (aStr[i])
	{ 
		if (aStr[i] == '(' || 
			aStr[i] == ')' || 
			aStr[i] == '|' ||
			aStr[i] == '&' || 
			aStr[i] == '!')
			return 1;
			
		i++;
	}
	
	return 0;
}

/** *********************************************************************
* Функция получает расстояние редактирования, т.е. количество операций
* с помощью которых можно из строки st1 получить строку st2. Допустимы
* следующие виды операций:
* - вставка
* - замена
* - удаление
* - обмен символов стоящих на соседних позициях
*
* @param[in]	aStr1			- первая строка для изучения
* @param[in]	aStr2			- вторая строка для изучения
* @param[in]	aStr1len		- длина первой строки для изучения
* @param[in]	aStr2len		- длина второй строки для изучения
* @param[in]	aFuzzyBuffer	- указатель на квадратную матрицу для обработки;
*								  Матрица выделяется и инициализируется один раз снаружи
*
* @return расстояние редактирования
***********************************************************************/
Int32 CSldCompare::FuzzyCompare(const UInt16 *aStr1, const UInt16 *aStr2, Int32 aStr1len, Int32 aStr2len, Int32 (*aFuzzyBuffer)[ARRAY_DIM])
{
	Int32	x1, x2;
	UInt16	ch1;
	Int32	diff;

	//////////////////////////////////////////////////////////////////////////
	// Рабочий цикл.
	for (x1=1;x1<aStr1len;x1++)
	{
		ch1=aStr1[x1-1];
		for (x2=1;x2<aStr2len;x2++)
		{
			// Если символы равны, тогда в данном месте ничего делать не нужно.
			if (ch1 == aStr2[x2-1])
				diff = 0; 
			else
				diff = 1;

			// шаг по x1 означает удаление символа из первой строки,
			// шаг по x2 - вставку в первую строку
			// шаг по x1 и x2 - замену символа или отсутствие необходимых изменений

			// т.о. здесь выполняются проверки на:
			// - удаление символа из первой строки (x1-1, x2)
			// - вставка символа в первую строку (x1, x2-1)
			// - замена символа в строке
			aFuzzyBuffer[TIO(x1, x2)] = sldMin3(aFuzzyBuffer[TIO(x1-1, x2)]+1,
												aFuzzyBuffer[TIO(x1, x2-1)]+1,
												aFuzzyBuffer[TIO(x1-1, x2-1)]+diff);

			// Перестановка соседних символов.
			if (x1 > 1 && x2 > 1 && ch1 == aStr2[x2-2] && aStr1[x1-2] == aStr2[x2-1])
			{
				aFuzzyBuffer[TIO(x1, x2)] = sldMin2(aFuzzyBuffer[TIO(x1, x2)],
													aFuzzyBuffer[TIO(x1-2, x2-2)] + diff);
			}
		}
	}

	// Получаем длину оптимального преобразования.
	return aFuzzyBuffer[TIO(aStr1len-1, aStr2len-1)];
}

/** *********************************************************************
* Функция преобразования из юникода в однобайтовую кодировку с учетом
* языка на котором предполагается фраза
*
* ВНИМАНИЕ!!! Данная функция не требует дополнительных данных кроме кода языка,
* но она и не может преобразовывать языки, данные о которых в ней не прописаны.
*
* Данная функция должна поддерживать только те языки для которых у нас есть
* морфология или озвучка. Символы которые не могут попасть в данную кодовую страницу 
* игнорируются
*
* @param[in]	aUnicode		- указатель на юникодную строчку
* @param[out]	aAscii			- указатель на буффер для итоговой однобайтовой строчки
* @param[in]	aLanguageCode	- код языка который требуется использовать при перекодировании
*
* @return код ошибки
***********************************************************************/
ESldError CSldCompare::Unicode2ASCIIByLanguage(const UInt16* aUnicode, UInt8* aAscii, ESldLanguage aLanguageCode)
{
	if (!aUnicode || !aAscii)
		return eMemoryNullPointer;

	
	if (aLanguageCode != SldLanguage::Russian &&
		aLanguageCode != SldLanguage::English &&
		aLanguageCode != SldLanguage::French &&
		aLanguageCode != SldLanguage::Italian &&
		aLanguageCode != SldLanguage::German &&
		aLanguageCode != SldLanguage::Spanish &&
		aLanguageCode != SldLanguage::Dutch &&
		aLanguageCode != SldLanguage::Catalan)
	{
		return eCompareUnknownLanguageCode;
	}
	
	if (aLanguageCode == SldLanguage::Russian)
	{
		while (*aUnicode)
		{
			if (*aUnicode >= 0x0410 && *aUnicode <= 0x44f)
			{
				*aAscii = (*aUnicode - 0x0410) + 0xc0; // номер символа среди русских + код символа русское большое А
			}
			else if (*aUnicode == 0x0401)	// Большая буква Ё
			{
				*aAscii = 0xa8;
			}
			else if (*aUnicode == 0x0451) // маленькая буква ё
			{
				*aAscii = 0xb8;
			}
			else if (*aUnicode < 0x0080)
			{
				*aAscii = (UInt8)*aUnicode;
			}
			else
			{
				*aAscii = CMP_DEFAULT_CHAR;
			}

			aUnicode++;
			aAscii++;
		}
	}
	else
	{
		while (*aUnicode)
		{
			// стандартная часть и почти вся расширенная латиница
			if (*aUnicode < 0x0080 || (*aUnicode >= 0x00A0 && *aUnicode < 0x0100))
			{
				*aAscii = (UInt8)*aUnicode;
			}
			else
			if (*aUnicode == 0x0152)	// лигатура OE
			{
				*aAscii = 0x8c;
			}
			else if (*aUnicode == 0x0153)	// лигатура oe
			{
				*aAscii = 0x9c;
			}
			else if (*aUnicode == 0x0160)	// лигатура S с галочкой v
			{
				*aAscii = 0x8A;
			}
			else if (*aUnicode == 0x0161)	// лигатура s с галочкой v
			{
				*aAscii = 0x9A;
			}
			else if (*aUnicode == 0x017D)	// лигатура Z с галочкой v
			{
				*aAscii = 0x8E;
			}
			else if (*aUnicode == 0x017E)	// лигатура z с галочкой v
			{
				*aAscii = 0x9E;
			}
			else
			{
				*aAscii = CMP_DEFAULT_CHAR;
			}

			aUnicode++;
			aAscii++;
		}
	}

	*aAscii = 0;
	return eOK;
}


/** *********************************************************************
* Функция преобразования из однобайтовой кодировки в юникод с учетом
* языка на котором предполагается фраза
*
* ВНИМАНИЕ!!! Данная функция не требует дополнительных данных кроме кода языка,
* но она и не может преобразовывать языки данные о которых в ней не прописаны
*
* Данная функция должна поддерживать только те языки для которых у нас есть
* морфология или озвучка. Символы которые не могут попасть в данную кодовую страницу 
* игнорируются
*
* @param[in]	ascii			- указатель на однобайтовую строчку
* @param[out]	aUnicode		- указатель на буффер для юникодной строчки
* @param[in]	aLanguageCode	- код языка который требуется использовать при перекодировании
*
* @return код ошибки
***********************************************************************/
ESldError CSldCompare::ASCII2UnicodeByLanguage(const UInt8* aAscii, UInt16* aUnicode, ESldLanguage aLanguageCode)
{
	if (!aUnicode || !aAscii)
		return eMemoryNullPointer;

	if (aLanguageCode != SldLanguage::Russian &&
		aLanguageCode != SldLanguage::English &&
		aLanguageCode != SldLanguage::French &&
		aLanguageCode != SldLanguage::Italian &&
		aLanguageCode != SldLanguage::German &&
		aLanguageCode != SldLanguage::Spanish &&
		aLanguageCode != SldLanguage::Dutch &&
		aLanguageCode != SldLanguage::Catalan)
	{
		return eCompareUnknownLanguageCode;
	}

	if (aLanguageCode == SldLanguage::Russian)
	{
		while (*aAscii)
		{
			if (*aAscii >= 0xc0)
			{
				*aUnicode = *aAscii - 0xc0 + 0x0410; // номер символа среди русских + код символа русское большое А
			}
			else if (*aAscii == 0xa8)	// Большая буква Ё
			{
				*aUnicode = 0x0401;
			}
			else if (*aAscii == 0xb8) // маленькая буква ё
			{
				*aUnicode = 0x0451;
			}
			else if (*aAscii <0x0080)
			{
				*aUnicode = *aAscii;
			}
			else
			{
				*aUnicode = CMP_DEFAULT_CHAR;
			}

			aUnicode++;
			aAscii++;
		}
	}
	else
	{
		while (*aAscii)
		{
			// стандартная часть и почти вся расширенная латиница.
			if (*aAscii < 0x0080 || *aAscii >= 0x00A0)
			{
				*aUnicode = *aAscii;
			}
			else if (*aAscii == 0x8c)	// лигатура OE
			{
				*aUnicode = 0x0152;
			}
			else if (*aAscii == 0x9c)	// лигатура oe
			{
				*aUnicode = 0x0153;
			}
			else if (*aAscii == 0x8A)	// лигатура S с галочкой v
			{
				*aUnicode = 0x0160;
			}
			else if (*aAscii == 0x9A)	// лигатура s с галочкой v
			{
				*aUnicode = 0x0161;
			}
			else if (*aAscii == 0x8E)	// лигатура Z с галочкой v
			{
				*aUnicode = 0x017D;
			}
			else if (*aAscii == 0x9E)	// лигатура z с галочкой v
			{
				*aUnicode = 0x017E;
			}
			else
			{
				*aUnicode = CMP_DEFAULT_CHAR;
			}

			aUnicode++;
			aAscii++;
		}
	}

	*aUnicode = 0;
	return eOK;
}

/** *********************************************************************
* Преобразует строку символов в строку их масс
*
* @param[in]	aSourceStr			- исходная юникодная строка для преобразования
* @param[out]	aMassStr			- строка масс
*									  память для выходной строки выделяется и освобождается снаружи
* @param[in]	aEraseZeroSymbols	- флаг, определяющий останутся ли нулевые символы в строке
*									  0 - вместо нулевых сиволов будет возвращен символ CMP_MASS_ZERO
*									  1 - разделители будут вырезаны из строчки
* @param[in]	aUseMassForDigit	- Числа получают виртуальную массу CMP_MASS_ZERO_DIGIT, если не имеют собственной
*									  0 - числа не учитываются в сравнении
*									  1 - числа учитываются в сравнении
*
* @return код ошибки
************************************************************************/
ESldError CSldCompare::GetStrOfMass(const UInt16* aSourceStr, SldU16String & aMassStr, Int8 aEraseZeroSymbols, Int8 aUseMassForDigit) const
{
	if (!aSourceStr)
		return eMemoryNullPointer;

	aMassStr.clear();

	if (!*aSourceStr)
		return eOK;

	aMassStr.reserve(StrLen(aSourceStr));
	
	const UInt16* str = aSourceStr;
	UInt16 last_char_mass = 0;

	while (!CMP_IS_EOL(m_CMPTable[m_DefaultTable].Header, str))
	{
		if(aUseMassForDigit && CMP_IS_DIGIT(*str))
		{
			last_char_mass = GetMass(*str, m_CMPTable[m_DefaultTable].SimpleMassTable.data(), CMP_IGNORE_SYMBOL);
			aMassStr.push_back(last_char_mass == CMP_IGNORE_SYMBOL ? CMP_VIRTUAL_DIGIT_MASS(*str) : last_char_mass);
			str++;
			continue;
		}

		last_char_mass = GetMass(*str, m_CMPTable[m_DefaultTable].SimpleMassTable.data(), CMP_IGNORE_SYMBOL);

		// Проверяем, входит ли символ в комплексную часть таблицы (например лигатура)
		if (last_char_mass != CMP_NOT_FOUND_SYMBOL && last_char_mass & CMP_MASK_OF_INDEX_FLAG)
		{
			UInt16 mass[CMP_CHAIN_LEN+1] = {0};
			UInt32 skip_count = GetComplex(str, last_char_mass & CMP_MASK_OF_INDEX, mass, m_CMPTable[m_DefaultTable].Complex);
			for (UInt32 i = 0; i < CMP_CHAIN_LEN; i++)
			{
				if (mass[i] && mass[i] != CMP_IGNORE_SYMBOL && mass[i] != CMP_NOT_FOUND_SYMBOL)
				{
					aMassStr.push_back(mass[i]);
				}
			}
			str += skip_count;
		}
		else
		{
			if (last_char_mass != CMP_IGNORE_SYMBOL)
			{
				aMassStr.push_back(last_char_mass);
			}
			else if(aEraseZeroSymbols == 0)
			{
				aMassStr.push_back(CMP_MASS_ZERO);
			}
			
			str++;
		}
	}

	return eOK;
}

/** *********************************************************************
* Преобразует строку символов в строку их масс с учетом разделителей
*
* @param[in]	aSourceStr			- исходная юникодная строка для преобразования
* @param[out]	aMassStr			- строка масс
* @param[in]	aEraseZeroSymbols	- флаг, определяющий останутся ли нулевые символы в строке
*									  0 - вместо нулевых сиволов будет возвращен символ CMP_MASS_ZERO
*									  1 - разделители будут вырезаны из строчки
* @param[in]	aUseMassForDigit	- Числа получают виртуальную массу CMP_MASS_ZERO_DIGIT, если не имеют собственной
*									  0 - числа не учитываются в сравнении
*									  1 - числа учитываются в сравнении
*
* @return код ошибки
************************************************************************/
ESldError CSldCompare::GetStrOfMassWithDelimiters(const UInt16* aSourceStr, SldU16String & aMassStr, Int8 aEraseZeroSymbols, Int8 aUseMassForDigit) const
{
	if (!aSourceStr)
		return eMemoryNullPointer;

	aMassStr.clear();

	if (!*aSourceStr)
		return eOK;
	
	aMassStr.reserve(StrLen(aSourceStr));

	const UInt16* str = aSourceStr;
	UInt16 last_char_mass = 0;
	UInt32 delimiterFlag;

	while (!CMP_IS_EOL(m_CMPTable[m_DefaultTable].Header, str))
	{
		delimiterFlag = 0;

		if(aUseMassForDigit && CMP_IS_DIGIT(*str))
		{
			last_char_mass = GetMass(*str, m_CMPTable[m_DefaultTable].SimpleMassTable.data(), CMP_IGNORE_SYMBOL);
			aMassStr.push_back(last_char_mass == CMP_IGNORE_SYMBOL ? CMP_VIRTUAL_DIGIT_MASS(*str) : last_char_mass);
			str++;
			continue;
		}

		last_char_mass = GetMass(*str, m_CMPTable[m_DefaultTable].SimpleMassTable.data(), CMP_IGNORE_SYMBOL);

		if (m_CMPTable[m_DefaultTable].Header->DelimiterCount && *str < CMP_MOST_USAGE_SYMBOL_RANGE)
		{
			// Подобная операция проводится и в IsSymbolBelongToLanguage(), но в таком виде проверка выполняется значительно быстрее
			if (m_CMPTable[m_DefaultTable].MostUsageCharTable[*str] == eCmpDelimiterCharType)
			{
				delimiterFlag = 1;
			}
		}
		else
		{
			UInt32 resultFlag = 0;
			ESldError error = IsSymbolBelongToLanguage(*str, SldLanguage::Delimiters, &delimiterFlag, &resultFlag);
			if (error != eOK)
				return error;
		}

		// Проверяем, входит ли символ в комплексную часть таблицы (например лигатура)
		if (last_char_mass & CMP_MASK_OF_INDEX_FLAG)
		{
			UInt16 mass[CMP_CHAIN_LEN+1] = {0};
			UInt32 skip_count = GetComplex(str, last_char_mass & CMP_MASK_OF_INDEX, mass, m_CMPTable[m_DefaultTable].Complex);
			for (UInt32 i = 0; i < CMP_CHAIN_LEN; i++)
			{
				if (mass[i] && mass[i] != CMP_IGNORE_SYMBOL && mass[i] != CMP_NOT_FOUND_SYMBOL)
				{
					aMassStr.push_back(mass[i]);
				}
			}
			str += skip_count;
		}
		else
		{
			if (last_char_mass != CMP_IGNORE_SYMBOL)
			{
				aMassStr.push_back(last_char_mass);
			}
			else if(delimiterFlag)
			{
				aMassStr.push_back(CMP_MASS_DELIMITER);
			}
			else if(aEraseZeroSymbols == 0)
			{
				aMassStr.push_back(CMP_MASS_ZERO);
			}
			str++;
		}
	}

	return eOK;
}

/** *********************************************************************
* Преобразует поисковый паттерн в паттерн масс с учетом спец символов
* Cимволы-джокеры заменяются на CMP_MASS_SPECIAL_SYMBOL_ANY_CHARS и CMP_MASS_SPECIAL_SYMBOL_ONE_CHAR
* Числа получают виртуальную массу CMP_MASS_ZERO_DIGIT, если не имеют собственной
*
* @param[in]	aSourceStr		- исходная юникодная строка для преобразования
* @param[out]	aMassStr		- строка масс
* @param[in]	aUseMassForDigit	- Числа получают виртуальную массу CMP_MASS_ZERO_DIGIT, если не имеют собственной
*									  0 - числа не учитываются в сравнении
*									  1 - числа учитываются в сравнении
*
* @return код ошибки
************************************************************************/
ESldError CSldCompare::GetSearchPatternOfMass(const UInt16* aSourceStr, SldU16String & aMassStr, Int8 aUseMassForDigit) const
{
	if (!aSourceStr)
		return eMemoryNullPointer;
	
	aMassStr.clear();

	if (!*aSourceStr)
		return eOK;
	
	aMassStr.reserve(StrLen(aSourceStr));

	const UInt16* str = aSourceStr;
	UInt16 last_char_mass = 0;

	// Вычисляем положение частей таблицы сравнения
	const CMPHeaderType* header = m_CMPTable[m_DefaultTable].Header;
	const CMPComplexType* complex = m_CMPTable[m_DefaultTable].Complex;

	while (!CMP_IS_EOL(header, str))
	{
		if(*str == CMP_QUERY_SPECIAL_SYMBOL_ANY_CHARS)
		{
			if (*(str + 1) != CMP_QUERY_SPECIAL_SYMBOL_ANY_CHARS)
				aMassStr.push_back(CMP_MASS_SPECIAL_SYMBOL_ANY_CHARS);
			str++;
			continue;
		}
		else if(*str == CMP_QUERY_SPECIAL_SYMBOL_ONE_CHAR)
		{
			aMassStr.push_back(CMP_MASS_SPECIAL_SYMBOL_ONE_CHAR);
			str++;
			continue;
		}
		else if(aUseMassForDigit && CMP_IS_DIGIT(*str))
		{
			last_char_mass = GetMass(*str, m_CMPTable[m_DefaultTable].SimpleMassTable.data(), CMP_IGNORE_SYMBOL);
			aMassStr.push_back(last_char_mass == CMP_IGNORE_SYMBOL ? CMP_VIRTUAL_DIGIT_MASS(*str) : last_char_mass);
			str++;
			continue;
		}

		last_char_mass = GetMass(*str, m_CMPTable[m_DefaultTable].SimpleMassTable.data(), CMP_IGNORE_SYMBOL);

		// Проверяем, входит ли символ в комплексную часть таблицы (например лигатура)
		if (last_char_mass & CMP_MASK_OF_INDEX_FLAG)
		{
			UInt16 mass[CMP_CHAIN_LEN+1] = {0};
			UInt32 skip_count = GetComplex(str, last_char_mass & CMP_MASK_OF_INDEX, mass, complex);
			for (UInt32 i = 0; i < CMP_CHAIN_LEN; i++)
			{
				if (mass[i] && mass[i] != CMP_IGNORE_SYMBOL && mass[i] != CMP_NOT_FOUND_SYMBOL)
				{
					aMassStr.push_back(mass[i]);
				}
			}
			str += skip_count;
		}
		else if (last_char_mass != CMP_IGNORE_SYMBOL)
		{
			aMassStr.push_back(last_char_mass);
			str++;
		}
		else
		{
			str++;
		}
	}

	return eOK;
}

/** *********************************************************************
* Разделяет строчку на слова
*
* @param[in]	aText			- исходная юникодная строка для разделения
* @param[out]	aTextWords		- указатель на массив строк в который будут записаны слова из строчки. 
*								  Память освобождается в FreeParts
* @param[out]	aTextCount		- количество слов в строчке
*
* @return код ошибки
************************************************************************/
ESldError CSldCompare::DivideQueryByParts(const UInt16 *aText, SldU16WordsArray& aTextWords) const
{
	return DivideQueryByParts(aText, NULL, aTextWords);
}

/** *********************************************************************
* Разделяет строчку на слова на основе переданного списка разделителей
*
* @param[in]	aText			- исходная юникодная строка для разделения
* @param[in]	aDelimitersStr	- строка с разделителями
* @param[out]	aTextWords		- указатель на массив строк в который будут записаны слова из строчки. 
*								  Память освобождается в FreeParts
* @param[out]	aTextCount		- количество слов в строчке
*
* @return код ошибки
************************************************************************/
ESldError CSldCompare::DivideQueryByParts(const UInt16 *aText, const UInt16 *aDelimitersStr, SldU16WordsArray& aTextWords) const
{
	aTextWords.clear();

	if (!aText)
		return eMemoryNullPointer;

	UInt32 TextLen = CSldCompare::StrLen(aText);
	if (!TextLen)
		return eOK;

	// Разбиваем исходный запрос на слова
	UInt32 isDelimiterSymbol = 0;
	UInt32 wordLen = 0;
	const UInt16* wordBegin = aText;

	for (UInt32 charIndex = 0; charIndex < TextLen; charIndex++)
	{
		isDelimiterSymbol = 0;
		if (aDelimitersStr)
		{
			const UInt16* DelimPtr = aDelimitersStr;
			// при использовании переданной строки разделителей результат всегда можно точно определить
			while (*DelimPtr)
			{
				if (*DelimPtr == aText[charIndex])
				{
					isDelimiterSymbol = 1;
					break;
				}

				DelimPtr++;
			}
		}
		else
		{
			isDelimiterSymbol = IsDelimiter(aText[charIndex]);
		}

		if (isDelimiterSymbol)
		{
			if (wordLen)
			{
				aTextWords.push_back(SldU16String(wordBegin, wordLen));
				wordBegin = &aText[charIndex];
				wordLen = 0;
			}
			wordBegin++;
		}
		else
		{
			wordLen++;
		}
	}

	if (wordLen)
	{
		aTextWords.push_back(SldU16String(wordBegin, wordLen));
	}

	return eOK;
}

/**
 * Разделяет строчку на слова
 *
 * @param[in]   aText       - исходная юникодная строка для разделения
 * @param[out]  aTextWords  - ссылка на массив подстрок в который будут записаны слова из строчки
 *
 * ВАЖНО!
 * В массив записываются *подстроки* из исходной строки! Из это следует что:
 *  - они *не nul-терминированы*
 *  - время их "жизни" *привязано к времени "жизни" входной строки*
 */
void CSldCompare::DivideQueryByParts(SldU16StringRef aText, CSldVector<SldU16StringRef> &aTextWords) const
{
	DivideQueryByParts(aText, nullptr, aTextWords);
}

/**
 * Разделяет строчку на слова на основе переданного списка разделителей
 *
 * @param[in]   aText           - исходная юникодная строка для разделения
 * @param[in]   aDelimitersStr  - строка с разделителями
 * @param[out]  aTextWords      - ссылка на массив подстрок в который будут записаны слова из строчки
 *
 * ВАЖНО!
 * В массив записываются *подстроки* из исходной строки! Из это следует что:
 *  - они *не nul-терминированы*
 *  - время их "жизни" *привязано к времени "жизни" входной строки*
 */
void CSldCompare::DivideQueryByParts(SldU16StringRef aText, const UInt16 *aDelimitersStr, CSldVector<SldU16StringRef> &aTextWords) const
{
	aTextWords.clear();

	if (aText.empty())
		return;

	// Разбиваем исходный запрос на слова
	for (UInt32 len = 0; len < aText.size(); )
	{
		bool isDelimiterSymbol = false;
		if (aDelimitersStr)
			isDelimiterSymbol = sld2::StrChr(aDelimitersStr, aText[len]) != nullptr;
		else
			isDelimiterSymbol = !!IsDelimiter(aText[len]);

		if (isDelimiterSymbol)
		{
			if (len)
			{
				aTextWords.emplace_back(aText.substr(0, len));
				aText = aText.substr(len);
				len = 0;
			}
			aText = aText.substr(1);
		}
		else
		{
			len++;
		}
	}

	if (aText.size())
		aTextWords.emplace_back(aText);
}

/** *********************************************************************
* Разделяет запрос на слова, возвращая альтернативные варианты для частичных разделителей
*
* @param[in]	aText			- исходная юникодная строка для разделения
* @param[in]	aDelimitersStr	- строка с разделителями
* @param[out]	aTextWords		- указатель на массив строк в который будут записаны слова из строчки. 
*								  Память освобождается в FreeParts
* @param[out]	aAltrnativeWords- указатель на массив альтернативных строк (например для запроса didn't
*								  сюда попадет didnt, в то время, как в основном массиве окажуться didn и t)
* @param[out]	aTextCount		- количество слов в строчке
*
* @return код ошибки
************************************************************************/
ESldError CSldCompare::DivideQuery(const UInt16 *aText, SldU16WordsArray& aTextWords, SldU16WordsArray& aAltrnativeWords) const
{
	aTextWords.clear();

	if (!aText)
		return eMemoryNullPointer;

	Int32 TextLen = CSldCompare::StrLen(aText);
	if (!TextLen)
		return eOK;

	// Разбиваем исходный запрос на слова
	UInt32 isDelimiterSymbol = 0;
	UInt32 isHalfDelimiterSymbol = 0;
	UInt32 wordLen = 0;
	const UInt16* wordBegin = aText;
	Int32 alternativeIndex = -1;

	for (UInt32 charIndex = 0; charIndex < TextLen; charIndex++)
	{
		isDelimiterSymbol = IsDelimiter(aText[charIndex]);
		isHalfDelimiterSymbol = IsHalfDelimiter(aText[charIndex]);

		if (isDelimiterSymbol)
		{
			if (wordLen)
			{
				aTextWords.push_back(SldU16String(wordBegin, wordLen));
				aAltrnativeWords.push_back(SldU16String());

				if (isHalfDelimiterSymbol)
				{
					if (alternativeIndex == -1)
					{
						alternativeIndex = aAltrnativeWords.size() - 1;
					}
					aAltrnativeWords[alternativeIndex] += aTextWords.back();
				}
				else if (alternativeIndex != -1)
				{
					aAltrnativeWords[alternativeIndex] += aTextWords.back();
				}

				wordBegin = &aText[charIndex];
				wordLen = 0;
			}

			if (!isHalfDelimiterSymbol && alternativeIndex != -1)
			{
				for (UInt32 i = alternativeIndex + 1; i < aAltrnativeWords.size(); i++)
				{
					aAltrnativeWords[i] = aAltrnativeWords[alternativeIndex];
				}
				alternativeIndex = -1;
			}

			wordBegin++;
		}
		else
		{
			wordLen++;
		}
	}

	if (wordLen)
	{
		aTextWords.push_back(SldU16String(wordBegin, wordLen));
		aAltrnativeWords.push_back(SldU16String());
		if (alternativeIndex != -1)
		{
			aAltrnativeWords[alternativeIndex] += aTextWords.back();
			for (UInt32 i = alternativeIndex + 1; i < aAltrnativeWords.size(); i++)
			{
				aAltrnativeWords[i] = aAltrnativeWords[alternativeIndex];
			}
		}
	}

	return eOK;
}

/** *********************************************************************
* Возвращает тип алфавита для введенного текста в контексте текущего языка.
* В случае, если язык представлен несколькими алфавитами (например, китайский имеет два варианта алфавита: иероглифы и pinjin(латиница))
* функция позволяет по введенному тексту определить конкретный тип. 
* 
* @param[in]	aText	- исходная юникодная строка
*
* @return тип алфавита (см. enum EAlphabetType)
************************************************************************/
UInt32 CSldCompare::GetAlphabetTypeByText(const UInt16 *aText) const 
{
	const UInt16 *ptr = aText;
	
	UInt32 languageCode = m_CMPTableInfo[m_DefaultTable].LanguageCode;
	if (languageCode == SldLanguage::Chinese)
	{
		UInt32 hieroglyph = 0;
		UInt32 pinyin = 0;

		// Для китайского словаря вначале нужно определить какой алфавит ввода и 
		// соответственно этому алфавиту произвести переключение контекста.
		// Варианты алфавита: иероглифы(0x3000-0xFB00) или pinjin(латиница и 
		// расширенная латиница)
		while (*ptr && hieroglyph == 0)
		{
			if (*ptr >= 0x3000 && *ptr < 0xFB00)
				hieroglyph++;
			else
				// Проверяем, а не игнорируемый ли это символ
			if (GetMass(*ptr, m_CMPTable[m_DefaultTable].SimpleMassTable.data(), CMP_IGNORE_SYMBOL) != CMP_IGNORE_SYMBOL)
					pinyin++;
		
			ptr++;
		}
	
		if (hieroglyph != 0)
			return EAlphabet_Chin_Hierogliph;
		else if (pinyin != 0)
			return EAlphabet_Chin_Pinyin;
		else
			return EAlphabet_Unknown;
	}
	else if (languageCode == 'apaj')
	{
		UInt32 kana = 0;
		UInt32 kanji = 0;
		UInt32 romanji = 0;

		// Для японского словаря вначале нужно определить какой алфавит ввода и 
		// соответственно этому алфавиту произвести переключение контекста.
		// Варианты алфавита: kanji(0x3100-0xFB00) или kana(0x3042-0x30FE)
		// Основной язык ввода - kana(фонетические иероглифы)
		// Подробнее - в кана имеется 2 алфавита(katakana и hirogana) которые имеют 
		// между собой точное соответствие. Кроме того возможна запись romanji - 
		// запись звучания японских слов латиницей. Таблицы сравнения японского в 
		// СловоЕде имеют поддержку romanji - соответственно мы должны считать 
		// такие символы относящимися к kana.
		// Дополнительный kanji.
		
		while (*ptr && kana + kanji == 0)
		{
			if (*ptr >= 0x3042 && *ptr <= 0x30FE)
				kana++;
			else if (*ptr >= 0x3100 && *ptr <0xFB00)
				kanji++;
			else if ((*ptr >= 0x0041 && *ptr <= 0x005A) || (*ptr >= 0x0061 && *ptr <= 0x007A))
				romanji++;
		
			ptr++;
		}

		if (kana + kanji != 0)
		{
			if (kana > kanji)
				return EAlphabet_Japa_Kana;
			else
				return EAlphabet_Japa_Kanji;
		}	
		else if (romanji != 0)
			return EAlphabet_Japa_Romanji;
		else
			return EAlphabet_Unknown;
	}
	else if (languageCode == 'erok')
	{
		UInt32 hieroglyph = 0;
		UInt32 pinyin = 0;
		// Для корейского словаря вначале нужно определить какой алфавит ввода и 
		// соответственно этому алфавиту произвести переключение контекста.
		// Варианты алфавита:
		// hangul (иероглифы) - Hangul Syllables			(0xAC00-0xD7A3)
		//						Hangul Jamo					(0x1100-0x11FF)
		//						Hangul Compatibility Jamo	(0x3130-0x318F)
		//						Hangul Jamo Extended-A		(0xA960-0xA97F)
		//						Hangul Jamo Extended-B		(0xD7B0-0xD7FF)
		// pinjin - латиница и расширенная латиница
		while (*ptr && hieroglyph == 0)
		{	
			if ((*ptr>=0x1100 && *ptr<=0x11FF)||(*ptr>=0x3130 && *ptr<=0x318F)||(*ptr>=0xAC00 && *ptr<=0xD7A3)||
				(*ptr>=0xA960 && *ptr<=0xA97F)||(*ptr>=0xD7B0 && *ptr<=0xD7FF))
				hieroglyph++;
			else
				// Проверяем, а не игнорируемый ли это символ
			if (GetMass(*ptr, m_CMPTable[m_DefaultTable].SimpleMassTable.data(), CMP_IGNORE_SYMBOL) != CMP_IGNORE_SYMBOL)
					pinyin++;
			
			ptr++;
		}

		if (hieroglyph != 0)
			return EAlphabet_Kore_Hangul;
		else if(pinyin != 0)
			return EAlphabet_Kore_Pinyin;	
		else
			return EAlphabet_Unknown;
	}
	else
	{
		while (*ptr)
		{
			if (GetMass(*ptr, m_CMPTable[m_DefaultTable].SimpleMassTable.data(), CMP_IGNORE_SYMBOL) != CMP_IGNORE_SYMBOL)
				return EAlphabet_Standard;
			++ptr;
		}
		return EAlphabet_Unknown;
	}
}
/** *********************************************************************
* Проверяет, является ли данный символ игнорируемым
*
* @param[in]	aChar		- проверяемый символ
* @param[in]	aTableIndex	- индекс таблицы сортировки
*
* @return результат выполнения функции:
*						0 - символ не нулевой или его нет в таблице сортировки
*						1 - символ есть в таблице сортировке и его масса равна 0
************************************************************************/
Int8 CSldCompare::IsZeroSymbol(UInt16 aChar, UInt32 aTableIndex) const
{
	if(aTableIndex >= m_CMPTable.size())
		return 0;

	if(m_CMPTable[aTableIndex].SimpleMassTable[aChar])
		return 0;

	return 1;
}

/** *********************************************************************
* Проверяет, является ли данный символ игнорируемым
*
* @param[in]	aChar	- проверяемый символ
*
* @return результат выполнения функции:
*						0 - символ не нулевой или его нет в таблице сортировки
*						1 - символ есть в таблице сортировке и его масса равна 0
************************************************************************/
Int8 CSldCompare::IsZeroSymbol(UInt16 aChar) const
{
	return IsZeroSymbol(aChar, m_DefaultTable);
}

/** *********************************************************************
* Проверяет, является ли данный символ разделителем в заданной таблице сортировки
*
* @param[in]	aChar		- проверяемый символ
* @param[in]	aTableIndex	- индекс таблицы сортировки
*
* @return результат выполнения функции:
*						0 - символ не разделитель
*						1 - символ разделитель
************************************************************************/
Int8 CSldCompare::IsDelimiter(UInt16 aChar, UInt32 aTableIndex) const
{
	if(aTableIndex >= m_CMPTable.size())
		return 0;

	if(m_CMPTable[aTableIndex].Header->DelimiterCount == 0)
	{
		for (auto&& table : m_LanguageDelimiterSymbolsTable)
		{
			const auto langCode = table.GetLanguageCode();
			if (langCode == m_CMPTableInfo[aTableIndex].LanguageCode || langCode == SldLanguage::Delimiters)
			{
				if (table.IsSymbolBelongToLanguage(aChar))
					return 1;

				break;
			}
		}
	}

	if (aChar < CMP_MOST_USAGE_SYMBOL_RANGE)
		return m_CMPTable[aTableIndex].MostUsageCharTable[aChar] == eCmpDelimiterCharType;
	else
		return sld2::binary_search(m_CMPTable[aTableIndex].Delimiter, m_CMPTable[aTableIndex].Header->DelimiterCount, aChar);
}

/** *********************************************************************
* Проверяет, является ли данный символ разделителем в текущей дефолтной таблице сортировки
*
* @param[in]	aChar		- проверяемый символ
*
* @return результат выполнения функции:
*						0 - символ не разделитель
*						1 - символ разделитель
************************************************************************/
Int8 CSldCompare::IsDelimiter(UInt16 aChar) const
{
	return IsDelimiter(aChar, m_DefaultTable);
}

/** *********************************************************************
* Проверяет, является ли данный символ частичным разделителем в заданной таблице сортировки
*
* @param[in]	aChar		- проверяемый символ
* @param[in]	aTableIndex	- индекс таблицы сортировки
*
* @return результат выполнения функции:
*						0 - символ не разделитель
*						1 - символ разделитель
************************************************************************/
Int8 CSldCompare::IsHalfDelimiter(UInt16 aChar, UInt32 aTableIndex) const
{
	if(aTableIndex >= m_CMPTable.size())
		return 0;

	return sld2::binary_search(m_CMPTable[aTableIndex].HalfDelimiter, m_CMPTable[aTableIndex].Header->HalfDelimiterCount, aChar);
}

/** *********************************************************************
* Проверяет, является ли данный символ частичным разделителем в текущей дефолтной таблице сортировки
*
* @param[in]	aChar		- проверяемый символ
*
* @return результат выполнения функции:
*						0 - символ не разделитель
*						1 - символ разделитель
************************************************************************/
Int8 CSldCompare::IsHalfDelimiter(UInt16 aChar) const
{
	return IsHalfDelimiter(aChar, m_DefaultTable);
}

/** *********************************************************************
* Проверяет, является ли данный символ Emoji-символом
*
* @param[in]	aChar		- проверяемый символ
* @param[in]	aType		- тип Emoji
*
* @return результат выполнения функции:
*						0 - символ не Emoji
*						1 - символ Emoji
************************************************************************/
Int8 CSldCompare::IsEmoji(const UInt16 aChar, const EEmojiTypes aType)
{
	sld2::Span<const UInt16> table;
	switch (aType)
	{
		case eSlovoedEmoji:
			table = sld2::symbolsTable::GetSldEmojiTable();
			break;
		case eFullEmoji:
			table = sld2::symbolsTable::GetFullEmojiTable();
			break;
		default:
			table = sld2::symbolsTable::GetSldEmojiTable();
			break;
	}
	return sld2::binary_search(table.data(), table.size(), aChar);
}

/** *********************************************************************
* Проверяет, является ли данный символ пробельным
*
* @param[in]	aChar		- проверяемый символ
*
* @return результат выполнения функции:
*						0 - символ не пробельный
*						1 - символ пробельный
************************************************************************/
Int8 CSldCompare::IsWhitespace(const UInt16 aChar)
{
	sld2::Span<const UInt16> table = sld2::symbolsTable::GetSpaceTable();
	return sld2::binary_search(table.data(), table.size(), aChar);
}

/** *********************************************************************
* Проверяет, является ли данный символ значимым хотя бы в одной из таблиц
*
* @param[in]	aChar		- проверяемый символ
*
* @return результат выполнения функции:
*						0 - у символа есть масса в одной из таблиц
*						1 - у символа нет массы
************************************************************************/
Int8 CSldCompare::IsMarginalSymbol(const UInt16 aChar) const
{
	for (auto & table : m_CMPTable)
	{
		const auto charMass = table.SimpleMassTable[aChar];
		if (charMass != 0 && charMass != CMP_NOT_FOUND_SYMBOL)
			return 0;
	}
	return 1;
}

/** *********************************************************************
* Получает число из его текстового представления, основная функция
*
* @param[in]	aStr	- строка с текстовым представлением числа
* @param[in]	aRadix	- основание системы счисления
* @param[out]	aNumber	- сюда сохраняется результат
*
* @return код ошибки
************************************************************************/
ESldError CSldCompare::StrToInt32Base(const UInt16* aStr, UInt32 aRadix, Int32* aNumber)
{
	if (!aStr || !aNumber)
		return eMemoryNullPointer;

	*aNumber = 0;
	Int32 sign = 1;
	Int32 n = 0;
	
	if (*aStr && *aStr == 0x2D)								// L"-"
	{
		sign = -1;
		aStr++;
	}
	
	while (*aStr)
	{
		if (*aStr >= 0x30 && *aStr <= 0x39)					// L"0" L"9"
		{
			n = n * aRadix + (*aStr - 0x30);				// L"0"
		}
		else if (aRadix==16 && *aStr>=0x41 && *aStr<=0x46)	// L"A" L"F"
		{
			n = n * aRadix + 10 + (*aStr - 0x41);			// L"A"
		}
		else if (aRadix==16 && *aStr>=0x61 && *aStr<=0x66)	// L"a" L"f"
		{
			n = n * aRadix + 10 + (*aStr - 0x61);			// L"a"
		}
		else if (*aStr==0x2E || *aStr==0x2C)				// "," или "." - знак отделения дробной части; т.к. мы конвертируем в Int, выходим сразу
		{
			*aNumber = n;
			return eOK;
		}
		else
		{
			*aNumber = n*sign;
			return eMetadataErrorToIntConvertion;
		}
		
		++aStr;
	}
	
	*aNumber = n*sign;
	
	return eOK;
}

/** *********************************************************************
* Получает число из его текстового представления
*
* @param[in]	aStr	- строка с текстовым представлением числа
* @param[in]	aRadix	- основание системы счисления
* @param[out]	aNumber	- сюда сохраняется результат
*
* @return код ошибки
************************************************************************/
ESldError CSldCompare::StrToInt32(const UInt16* aStr, UInt32 aRadix, Int32* aNumber)
{
	ESldError error = StrToInt32Base(aStr, aRadix, aNumber);
	if(error == eMetadataErrorToIntConvertion)
		*aNumber = 0;
	return error;
}

/** *********************************************************************
* Получает число из его текстового представления
*
* @param[in]	aStr	- строка с текстовым представлением числа
* @param[in]	aRadix	- основание системы счисления
* @param[out]	aNumber	- сюда сохраняется результат
*
* @return код ошибки
************************************************************************/
ESldError CSldCompare::StrToUInt32(const UInt16* aStr, UInt32 aRadix, UInt32* aNumber)
{
	if (!aStr || !aNumber)
		return eMemoryNullPointer;
	
	*aNumber = 0;
	
	Int32 sign = 1;
	UInt32 n = 0;
	
	if (*aStr && *aStr==0x2D)								// L"-"
	{
		sign = -1;
		aStr++;
	}
	
	while (*aStr)
	{
		if (*aStr>=0x30 && *aStr<=0x39)						// L"0" L"9"
		{
			n = n * aRadix + (*aStr - 0x30);				// L"0"
		}
		else if (aRadix==16 && *aStr>=0x41 && *aStr<=0x46)	// L"A" L"F"
		{
			n = n * aRadix + 10 + (*aStr - 0x41);			// L"A"
		}
		else if (aRadix==16 && *aStr>=0x61 && *aStr<=0x66)	// L"a" L"f"
		{
			n = n * aRadix + 10 + (*aStr - 0x61);			// L"a"
		}
		else
		{
			*aNumber = 0;
			return eMetadataErrorToUIntConvertion;
		}
		
		++aStr;
	}
	
	*aNumber = n*sign;
	
	return eOK;
}

/** *********************************************************************
* Получает число с которого начинается строка
*
* @param[in]	aStr	- строка с текстовым представлением числа
* @param[in]	aRadix	- основание системы счисления
* @param[out]	aNumber	- сюда сохраняется результат
*
* @return код ошибки
************************************************************************/
ESldError CSldCompare::StrToBeginInt32(const UInt16* aStr, UInt32 aRadix, Int32* aNumber)
{
	return StrToInt32Base(aStr, aRadix, aNumber);
}

/** *********************************************************************
* Проверяет целостность скобок
*
* @param[in]	aText	- проверяемая строка вместе с открывающей скобкой
*
* @return результат выполнения функции:
*					0 - нет открывающей или закрывыющей скобки
*					1 - закрывающая скобка стоит в конце слова
*					2 - закрывающая скобка стоит в середине слова
*					3 - скобки пустые
*					4 - вложенные скобки
************************************************************************/
Int8 CSldCompare::CheckBracket(const UInt16 *aText) const
{
	if(!aText || *aText != CMP_QUERY_SPECIAL_SYMBOL_OPEN_BR)
		return 0;

	Int32 textLen = StrLen(aText);
	if(textLen == 1)
		return 0;

	if(textLen == 2 && aText[1] == CMP_QUERY_SPECIAL_SYMBOL_CLOSE_BR)
		return 3;

	for(UInt32 ch = 1; ch < textLen; ch++)
	{
		if(aText[ch] == CMP_QUERY_SPECIAL_SYMBOL_CLOSE_BR)
		{
			if(ch + 1 == textLen)
			{
				return 1;
			}
			else if(IsZeroSymbol(aText[ch+1], m_DefaultTable))
			{
				return 1;
			}
			else
			{
				return 2;
			}
		}
		else if(aText[ch] == CMP_QUERY_SPECIAL_SYMBOL_OPEN_BR)
		{
			return 4;
		}
	}

	return 0;
}

/** *********************************************************************
 * Получает дестичное число с плавающей точкой из строки
 *
 * @param[in]	aStr	- строка с текстовым представлением числа
 * @param[out]  aEnd    - сюда сохраняется начало строки сразу после числа
 * @param[out]	aNumber	- сюда сохраняется результат
 *
 * @return код ошибки
 ************************************************************************/
ESldError CSldCompare::StrToFloat32(const UInt16 *aStr, const UInt16 **aEnd, Float32 *aNumber)
{
	if (!aStr || !aNumber)
		return eMemoryNullPointer;

	Float32 n = 0.0f;

	bool negative = false;
	if (*aStr == '-')
	{
		negative = true;
		aStr++;
	}

	int fracSize = 1;
	bool inFrac = false;
	for (int chr = *aStr; chr; chr = *(++aStr))
	{
		if ((unsigned)(chr - '0') <= 9)
		{
			n = n * 10 + (chr - '0');
			if (inFrac)
				fracSize *= 10;
		}
		else if (chr == L'.')
		{
			if (inFrac)
				break;
			inFrac = true;
		}
		else
			break;
	}

	n /= fracSize;
	*aNumber = negative ? -n : n;
	if (aEnd)
		*aEnd = aStr;

	return eOK;
}

/** *********************************************************************
* Переворачивает кусок строки на месте (основано на Kernighan & Ritchie's "Ansi C")
*
* @param	aBegin	- начало переворачиваемого участка
* @param	aEnd	- конец переворачиваемого участка
*
* @return код ошибки
************************************************************************/
ESldError CSldCompare::StrReverse(UInt16* aBegin, UInt16* aEnd)
{
	if (!aBegin || !aEnd)
		return eMemoryNullPointer;
	sld2::StrReverse(aBegin, aEnd);
	return eOK;
}

/** *********************************************************************
* Получает строку из числа (основано на Kernighan & Ritchie's "Ansi C")
*
* @param[in]	aNumber	- сюда сохраняется результат
* @param[in]	aRadix	- основание системы счисления
* @param[out]	aStr	- строка с текстовым представлением числа
*
* @return код ошибки
************************************************************************/
ESldError CSldCompare::UInt32ToStr(UInt32 aNumber, UInt16* aStr, UInt32 aRadix)
{
	if (!aStr)
		return eMemoryNullPointer;

	// Validate base
	if (aRadix<2 || aRadix>35)
	{ 
		*aStr=0; 
		return eInputErrorBase; 
	}

	static char num[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	UInt16* wstr=aStr;

	// Conversion. Number is reversed.

	do *wstr++ = num[aNumber%aRadix]; while(aNumber/=aRadix);
	*wstr=0;

	// Reverse string
	return StrReverse(aStr, wstr-1);
}

/** *********************************************************************
 * "Возвращает" строку разделителей в таблице сортировки заданного языка
 *
 * @param[in]   aCMPTable        - указатель на таблицу сравнения, из которой будем получать разделители
 * @param[out]  aDelimitersStr   - указатель куда записывать адрес строки разделителей
 *                                 не zero-terminated
 * @param[out]  aDelimitersCount - длина строки с разделителями
 *
 * @return код ошибки
 ************************************************************************/
static ESldError GetDelimiters(const TCompareTableSplit *aCMPTable, const UInt16 **aDelimitersStr, UInt32 *aDelimitersCount)
{
	if (aCMPTable->Header->DelimiterCount == 0)
		return eCommonTooOldCompareTable;

	*aDelimitersStr = aCMPTable->Delimiter;
	*aDelimitersCount = aCMPTable->Header->DelimiterCount;

	return eOK;
}

/** *********************************************************************
 * "Возвращает" строку разделителей в таблице сортировки заданного языка
 *
 * @param[in]   aLangCode        - код языка
 * @param[out]  aDelimitersStr   - указатель куда записывать адрес строки разделителей
 *                                 не zero-terminated
 * @param[out]  aDelimitersCount - длина строки с разделителями
 *
 * @return код ошибки
 ************************************************************************/
ESldError CSldCompare::GetDelimiters(ESldLanguage aLangCode, const UInt16 **aDelimitersStr, UInt32 *aDelimitersCount) const
{
	if (!aDelimitersStr || !aDelimitersCount)
		return eMemoryNullPointer;

	for (UInt32 tableIdx = 0; tableIdx < m_CMPTable.size(); tableIdx++)
	{
		if (m_CMPTableInfo[tableIdx].LanguageCode == aLangCode)
			return ::GetDelimiters(&m_CMPTable[tableIdx], aDelimitersStr, aDelimitersCount);
	}

	return eCommonWrongLanguage;
}

/** *********************************************************************
 * "Возвращает" строку разделителей в текущей дефолтной таблице сортировки
 *
 * @param[out] aDelimitersStr   - указатель куда записывать адрес строки разделителей
 *                                не zero-terminated
 * @param[out] aDelimitersCount - длина строки с разделителями
 *
 * @return код ошибки
 ************************************************************************/
ESldError CSldCompare::GetDelimiters(const UInt16 **aDelimitersStr, UInt32 *aDelimitersCount) const
{
	if (!aDelimitersStr || !aDelimitersCount)
		return eMemoryNullPointer;

	return ::GetDelimiters(&m_CMPTable[m_DefaultTable], aDelimitersStr, aDelimitersCount);
}

/** *********************************************************************
* Добавляет выбранный селектор после всех Emoji-символов в строке
*
* @param[in/out] aString	- строка, в которой мы хотим добавить селектор
* @param[in]	 aType		- тип Emoji
* @param[in]	 aSelector	- код селектора (по дефолту 0xFE0E)
*
* @return код ошибки
************************************************************************/
ESldError CSldCompare::AddEmojiSelector(SldU16String & aString, const EEmojiTypes aType, const UInt16 aSelector)
{
	UInt32 pos = 0;
	const UInt16 * ptr = aString.c_str();
	while (*ptr)
	{
		if (IsEmoji(*ptr, aType) == 1)
		{
			pos++;
			ptr++;
			aString.insert(pos, aSelector);
		}

		pos++;
		ptr++;
	}

	return eOK;
}

/// Проверяет, является ли данный символ Emoji-селектором
static inline Int8 IsEmojiSelector(const UInt16 aChar)
{
	return (aChar >= 0xFE00 && aChar <= 0xFE0F) ? 1 : 0;
}

/** *********************************************************************
* Удаляет селекторы после всех Emoji-символов в строке
*
* @param[in/out] aString	- строка, в которой мы хотим удалить селекторы
* @param[in]	 aType		- тип Emoji
*
* @return код ошибки
************************************************************************/
ESldError CSldCompare::ClearEmojiSelector(SldU16String & aString, const EEmojiTypes aType)
{
	UInt32 pos = 0;
	const UInt16 * ptr = aString.c_str();
	while (*ptr)
	{
		if (IsEmoji(*ptr, aType) == 1 && IsEmojiSelector(*(ptr + 1)) == 1)
		{
			aString.erase(pos + 1, 1);
		}

		pos++;
		ptr++;
	}

	return eOK;
}

/** *********************************************************************
* Конвертирует 4-х символьную строчку в 4-х байтное число (например код языка, ID базы)
*
* @param[out] aString	- строка, которую мы хотим сконвертировать в число
*
* @return код ошибки
************************************************************************/
UInt32 CSldCompare::UInt16StrToUInt32Code(const SldU16StringRef aString)
{
	UIntUnion code;
	code.ui_32 = 0;
	const UInt32 langCodeSize = aString.length() <= sizeof(UInt32) ? aString.length() : sizeof(UInt32);
	for (UInt32 charIndex = 0; charIndex < langCodeSize; charIndex++)
	{
		code.ui_8[charIndex] = (UInt8)aString[charIndex];
	}

	return code.ui_32;
}

/** *********************************************************************
* Возвращает следующий по массе простой символ
* В случае, если передан самый тяжелый символ - возвращает MAX_UINT16_VALUE
*
* @param[in] aSymbol	- код текущего символа
*
* @return код символа
************************************************************************/
UInt16 CSldCompare::GetNextMassSymbol(const UInt16 aSymbol) const
{
	const auto simpleCount = m_CMPTable[m_DefaultTable].Header->SimpleCount;
	const auto simple = m_CMPTable[m_DefaultTable].Simple;
	const auto & sortedMass = m_CMPTable[m_DefaultTable].SortedMass;

	UInt32 resultIndex = 0;
	if (sld2::binary_search(simple, simpleCount, aSymbol, &resultIndex) && simple[resultIndex].mass + 1 < sortedMass.size())
	{
		return sortedMass[simple[resultIndex].mass + 1];
	}
	return MAX_UINT16_VALUE;
}

/** *********************************************************************
* Возвращает слово по позиции во фразе
*
* @param[in] aPhrase	- фраза
* @param[in] aPos		- позиция, для которой мы хотим получить слово
*
* @return SldU16StringRef с соответсвующим словом
************************************************************************/
SldU16StringRef CSldCompare::GetWordByPosition(const SldU16StringRef aPhrase, const UInt32 aPos) const
{
	UInt32 startPos = aPos;
	UInt32 endPos = aPos;

	while (startPos > 0 && !IsWhitespace(aPhrase[startPos]))
		startPos--;

	while (endPos < aPhrase.size() && !IsWhitespace(aPhrase[endPos]))
		endPos++;

	return aPhrase.substr(startPos, endPos - startPos);
}

/** *********************************************************************
* Заменяет выбранное слово во фразе
*
* @param[in/out] aPhrase	- фраза
* @param[in]     aWordIndex	- номер заменяемого слова
* @param[in]     aNewWord	- слово, на которое заменяем
*
* @return код ошибки
************************************************************************/
void CSldCompare::ReplaceWordInPhraseByIndex(SldU16String& aPhrase, const SldU16StringRef aNewWord, const UInt32 aWordIndex) const
{
	UInt32 wordIndex = 0;
	UInt32 charIndex = 0;
	UInt32 startPos = 0;
	UInt32 endPos = ~0;

	while (charIndex < aPhrase.length() && IsDelimiter(aPhrase[charIndex]))
		charIndex++;

	for (; charIndex < aPhrase.length() + 1; charIndex++)
	{
		if (charIndex == aPhrase.length() || IsDelimiter(aPhrase[charIndex]))
		{
			startPos = endPos + 1;
			endPos = charIndex;
			wordIndex++;
		}

		if (aWordIndex < wordIndex)
			break;
	}

	aPhrase.erase(startPos, endPos - startPos);
	aPhrase.insert(startPos, aNewWord);
}

SldU16WordsArray CSldCompare::ExpandBrackets(SldU16StringRef aText) const
{
  const SldU16String kBracketOpenChars  = SldU16StringLiteral("([<");
  const SldU16String kBracketCloseChars = SldU16StringLiteral(")]>");

  CSldVector<UInt32> validBracketsOpen;
  CSldVector<UInt32> validBracketsClose;

  {
    CSldVector<UInt32> bracketStack;
    for (UInt32 i = 0; i < aText.size(); ++i)
    {
      if (kBracketOpenChars.find(aText[i]) != kBracketOpenChars.npos)
      {
        bracketStack.push_back(i);
        continue;
      }

      const UInt32 kBracketType = kBracketCloseChars.find(aText[i]);
      if (kBracketType == kBracketCloseChars.npos)
        continue;

      for (UInt32 j = 0; j < bracketStack.size(); ++j)
      {
        UInt32 openPosition = bracketStack[bracketStack.size() - j - 1];
        if (kBracketType != kBracketOpenChars.find(openPosition))
          continue;

        validBracketsOpen.push_back(openPosition);
        validBracketsClose.push_back(i);

        bracketStack.resize(bracketStack.size() - j - 1);
        break;
      }
    }
  }

  auto IsBracketsStart = [validBracketsOpen](UInt32 aPos) 
  {
    return sld2::find(validBracketsOpen, aPos) != validBracketsOpen.npos;
  };

  auto IsBracketsEnd = [validBracketsClose](UInt32 aPos) 
  {
    return sld2::find(validBracketsClose, aPos) != validBracketsClose.npos;
  };

  const SldU16String kInPhraseDelimiters = SldU16StringLiteral(",");
  const SldU16String kWordDelimiters = SldU16StringLiteral("/");

  struct TBracketText
  {
    enum EBracketContentType
    {
      eBracketAlternativeVariants,
      eBracketSolidPhrasePart
    };

    TBracketText()
    {
      m_Variants.push_back(SldU16String());
    }

    void SetContentType(EBracketContentType aType)
    {
      m_ContentType = aType;
    }

    void AddEnding(UInt16 aChar)
    {
      if (m_Variants.empty())
      {
        m_Variants.push_back(SldU16String(1, aChar));
      }
      else
      {
        for (SldU16String& variant : m_Variants)
          variant.push_back(aChar);
      }
    }

    void SplitVariantsByDelimiter(SldU16StringRef aDelimiter)
    {
      SldU16WordsArray newVariants;
      for (SldU16String variant : m_Variants)
      {
        /// этот костыль применяется для случаев text[, some addition]text,
        /// в которых контент скобок считаем цельным несмотря на:
        /// 1) запятую
        /// 2) то, что скобки не отделены от текста разделителем

        /// @todo обобщить, сейчас только запятая
        /// (других случаев пока не встречалось)
        if (!variant.empty() && variant[0] == SldU16Char(','))
        {
          newVariants.push_back(variant);
        }
        else
        {
          for (UInt32 i = 0; i = variant.find(aDelimiter), i != variant.npos; ++i)
          {
            newVariants.push_back(variant.substr(0, i));
            variant.erase(0, i + aDelimiter.size());
          }

          if (!variant.empty())
            newVariants.push_back(variant);
        }
      }

      sld2::swap(m_Variants, newVariants);
    }

    void MergeWithStringsOfVariant(const TBracketText& aSourceVariant)
    {
      if (m_Variants.empty())
      {
        m_Variants = aSourceVariant.m_Variants;
        return;
      }

      /// подозрительно. что оригинальный варианты остаются, но так
      /// в FTSGenerator'е
      m_Variants.reserve(m_Variants.size()*(1 + aSourceVariant.m_Variants.size()));
      for (const SldU16String& left : m_Variants)
      {
        for (const SldU16String& right : aSourceVariant.m_Variants)
          m_Variants.emplace_back(left + right);
      }
    }

    EBracketContentType m_ContentType = eBracketSolidPhrasePart;
    SldU16WordsArray m_Variants;
  };

  sld2::Stack<TBracketText> bracketsContent;
  bracketsContent.push(TBracketText());

  UInt16 previousSymbol = 0;
  for (Int32 symbolIndex = 0; symbolIndex < aText.size(); ++symbolIndex)
  {
    if (IsBracketsStart(symbolIndex))
    {
      bracketsContent.push(TBracketText());
      if (IsDelimiter(previousSymbol))
        bracketsContent.top().SetContentType(TBracketText::eBracketSolidPhrasePart);
      else
        bracketsContent.top().SetContentType(TBracketText::eBracketAlternativeVariants);

      continue;
    }

    previousSymbol = aText[symbolIndex];

    if (!IsBracketsEnd(symbolIndex))
    {
      bracketsContent.top().AddEnding(aText[symbolIndex]);
      continue;
    }

    /// И в случае разделителя перед скобкой, и в случае разделителя после,
    /// считаем контент скобки отдельной альтернативной (необязательной частью) фразы
    if (symbolIndex + 1 < aText.size())
    {
      if (!IsDelimiter(aText[symbolIndex + 1]))
        bracketsContent.top().SetContentType(TBracketText::eBracketAlternativeVariants);
    }

    TBracketText completedBracket = bracketsContent.top();
    bracketsContent.pop();

    if (completedBracket.m_ContentType == TBracketText::eBracketAlternativeVariants)
    {
      for (UInt32 i = 0; i < kInPhraseDelimiters.size(); i++)
      {
        // сначала с пробелом, потом без пробела
        SldU16String delimiter = 
          SldU16String(1, kInPhraseDelimiters[i]) + SldU16Char(' ');
        completedBracket.SplitVariantsByDelimiter(delimiter);
        delimiter.pop_back();
        completedBracket.SplitVariantsByDelimiter(delimiter);
      }
    }

    for (size_t index = 0; index < kWordDelimiters.length(); index++)
      completedBracket.SplitVariantsByDelimiter(SldU16String(1, kWordDelimiters[index]));

    bracketsContent.top().MergeWithStringsOfVariant(completedBracket);
  }

  return bracketsContent.top().m_Variants;
}
