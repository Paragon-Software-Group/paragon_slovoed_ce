#include "SldSearchWordResult.h"

#include "SldList.h"

namespace {
namespace impl {

inline void resultsOR(UInt32 *lhs, const UInt32 *rhs, Int32 size)
{
	for (Int32 j = 0; j < size; j++, lhs++, rhs++)
		*lhs |= *rhs;
}

inline void resultsAND(UInt32 *lhs, const UInt32 *rhs, Int32 size)
{
	for (Int32 j = 0; j < size; j++, lhs++, rhs++)
		*lhs &= *rhs;
}

inline void resultsNOT(UInt32 *data, Int32 size)
{
	for (Int32 j = 0; j < size; j++, data++)
		*data ^= ~0u;
}

} // namespace impl

void addWord(UInt32 *data, Int32 wordIndex, Int32 *wordsCount)
{
	if (!sld2::bitset::test(data, wordIndex))
		(*wordsCount)++;

	sld2::bitset::set(data, wordIndex);
}

bool isWordPresent(const UInt32 *data, Int32 wordIndex)
{
	return sld2::bitset::test(data, wordIndex);
}

ESldError resultsOR(sld2::Span<UInt32> lhs, sld2::Span<const UInt32> rhs)
{
	if (lhs.size() != rhs.size())
		return eCommonWrongSizeOfData;

	impl::resultsOR(lhs.data(), rhs.data(), lhs.size());
	return eOK;
}

ESldError resultsAND(sld2::Span<UInt32> lhs, sld2::Span<const UInt32> rhs)
{
	if (lhs.size() != rhs.size())
		return eCommonWrongSizeOfData;

	impl::resultsAND(lhs.data(), rhs.data(), lhs.size());
	return eOK;
}

void resultsNOT(sld2::Span<UInt32> data)
{
	impl::resultsNOT(data.data(), data.size());
}

Int32 countWords(sld2::Span<const UInt32> data)
{
	Int32 wordCount = 0;
	for (UInt32 v : data)
	{
		if (v)
		{
			// https://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel
			// that's not *the* fastest one in existence (especially in the presence
			// of popcnt-like intrinsics) but it's much faster than a naive loop
			v = v - ((v >> 1) & 0x55555555);                    // reuse input as temporary
			v = (v & 0x33333333) + ((v >> 2) & 0x33333333);     // temp
			wordCount += (((v + (v >> 4)) & 0xF0F0F0F) * 0x1010101) >> 24; // count
		}
	}
	return wordCount;
}

Int32 fillWordVector(sld2::Span<const UInt32> data, UInt32 listIndex,
					 sld2::Span<TSldSearchWordStruct> aWordVector, Int32 addedWordCount)
{
	for (UInt32 i = 0; i < data.size(); i++)
	{
		// TODO: this can be optimized with ffs()
		UInt32 v = data[i];
		if (v == 0)
			continue;

		for (Int32 j = 0; j < SLD_SEARCH_RESULT_BUFFER_SIZE; j++)
		{
			if (v & (1 << j))
			{
				if (addedWordCount >= aWordVector.size())
					return addedWordCount;

				TSldSearchWordStruct word;
				word.ListIndex = listIndex;
				word.WordIndex = i * SLD_SEARCH_RESULT_BUFFER_SIZE + j;

				aWordVector[addedWordCount++] = word;
			}
		}
	}
	return addedWordCount;
}

}

/***********************************************************************
* Конструктор
************************************************************************/
CSldSearchWordResult::CSldSearchWordResult()
	: m_WordCount(0), m_staleWordCount(false)
{}

/***********************************************************************
* Инициализация
*
* @param aList			- указатель на массив списков слов словаря
* @param aListCount		- количество списков слов в массиве
*
* @return код ошибки
************************************************************************/
ESldError CSldSearchWordResult::Init(ISldList** aList, Int32 aListCount)
{
	if (!aList)
		return eMemoryNullPointer;
	if (aListCount <= 0)
		return eCommonWrongInitSearchList;

	decltype(m_Lists) lists(aListCount);
	if (lists.empty())
		return eMemoryNotEnoughMemory;

	UInt32 totalSize = 0;
	for (UInt32 i = 0; i < lists.size(); i++)
	{
		const CSldListInfo *listInfo;
		ESldError error = aList[i]->GetWordListInfo(&listInfo);
		if (error != eOK)
			return error;

		lists[i].size = sld2::bitset::size(listInfo->GetNumberOfGlobalWords());
		totalSize += lists[i].size;
	}

	if (!m_DataBuf.resize(totalSize))
		return eMemoryNotEnoughMemory;

	UInt32 *data = m_DataBuf.data();
	for (auto&& info : lists)
	{
		info.data = data;
		data += info.size;
	}

	m_Lists = sld2::move(lists);
	return eOK;
}

/***********************************************************************
* Проверяет, инициализирован класс или нет
*
* @return true, если инициализирован, false в противном случае
************************************************************************/
bool CSldSearchWordResult::IsInit() const
{
	return m_DataBuf.size() && m_Lists.size();
}

/// Очищает все результаты поиска
void CSldSearchWordResult::Clear()
{
	m_WordCount = 0;
	m_staleWordCount = false;
	sldMemZero(m_DataBuf.data(), m_DataBuf.size() * sizeof(m_DataBuf[0]));
}

/***********************************************************************
* Добавляет к результатам поиска ссылку на слово
*
* @param aListIndex			- номер списка, который содержит найденное слово
* @param aWordIndex			- номер слова
* @param aTranslationIndex	- номер перевода
* @param aShiftIndex		- смещение
*
* @return код ошибки
************************************************************************/
ESldError CSldSearchWordResult::AddWord(Int32 aListIndex, Int32 aWordIndex, Int32 aTranslationIndex, Int32 aShiftIndex)
{
	if (aListIndex<0 || aWordIndex<0 || aTranslationIndex<0 || aShiftIndex<0)
		return eCommonWrongInitSearchList;

	addWord(m_Lists[aListIndex].data, aWordIndex, &m_WordCount);
	return eOK;
}

/***********************************************************************
* Проверяет, содержится ли в результатах поиска список
*
* @param aListIndex[in]		- номер списка
*
* @return статус
************************************************************************/
bool CSldSearchWordResult::IsListPresent(Int32 aListIndex) const
{
	if (aListIndex >= m_Lists.size())
		return false;

	for (UInt32 i = 0; i < m_Lists[aListIndex].size; i++)
	{
		if (m_Lists[aListIndex].data[i])
			return true;
	}

	return false;
}

/***********************************************************************
* Проверяет, содержится ли в результатах поиска слово
*
* @param aListIndex[in]		- номер списка
* @param aWordIndex[in]		- номер слова
*
* @return статус
************************************************************************/
bool CSldSearchWordResult::IsWordPresent(Int32 aListIndex, Int32 aWordIndex) const
{
	if (aListIndex >= m_Lists.size())
		return false;
	return isWordPresent(m_Lists[aListIndex].data, aWordIndex);
}

/// Возвращает количество слов в наборе
Int32 CSldSearchWordResult::GetWordCount()
{
	if (m_staleWordCount)
	{
		ReCountWords();
		m_staleWordCount = false;
	}

	return m_WordCount;
}

/***********************************************************************
* Заполняет итоговый массив найденными словами
*
* @param aWordVector[in]		- указатель на массив (память выделена снаружи)
* @param aMaximumWords[in]		- максимальное количество слов, которое можно добавить
*
* @return количество слов, которое добавили
************************************************************************/
Int32 CSldSearchWordResult::FillWordVector(sld2::Span<TSldSearchWordStruct> aWordVector, Int32 aMaximumWords) const
{
	aWordVector = aWordVector.first(sld2::min<UInt32>(aWordVector.size(), aMaximumWords));
	Int32 addedWordCount = 0;
	for (UInt32 i = 0; i < m_Lists.size() && addedWordCount < aMaximumWords; i++)
		addedWordCount = fillWordVector(m_Lists[i], i, aWordVector, addedWordCount);
	return addedWordCount;
}

/***********************************************************************
* Объединяет результаты 2 поисков через оператор AND (т.е. получается пересечение двух поисков)
*
* @param aSearchResults	- указатель на другой экземпляр класса с резульататами поиска.
*
* @return код ошибки
************************************************************************/
ESldError CSldSearchWordResult::ResultsAND(const CSldSearchWordResult *aSearchResults)
{
	if (!aSearchResults)
		return eMemoryNullPointer;

	if (m_Lists.size() != aSearchResults->m_Lists.size())
		return eCommonWrongListCount;

	for (Int32 i = 0; i < m_Lists.size(); i++)
	{
		ESldError error = resultsAND(m_Lists[i], aSearchResults->m_Lists[i]);
		if (error != eOK)
			return error;
	}

	m_staleWordCount = true;
	return eOK;
}

/***********************************************************************
* Объединяет результаты 2 поисков через оператор OR (т.е. получается сложение двух поисков)
*
* @param aSearchResults	- указатель на другой экземпляр класса с резульататами поиска.
*
* @return код ошибки
************************************************************************/
ESldError CSldSearchWordResult::ResultsOR(const CSldSearchWordResult *aSearchResults)
{
	if (!aSearchResults)
		return eMemoryNullPointer;

	if (m_Lists.size() != aSearchResults->m_Lists.size())
		return eCommonWrongListCount;

	for (Int32 i = 0; i < m_Lists.size(); i++)
	{
		ESldError error = resultsOR(m_Lists[i], aSearchResults->m_Lists[i]);
		if (error != eOK)
			return error;
	}

	m_staleWordCount = true;
	return eOK;
}

/// Применяет к результатам поиска операцию отрицания
void CSldSearchWordResult::ResultsNOT()
{
	for (UInt32 i = 0; i < m_Lists.size(); i++)
	{
		if (IsListPresent(i))
			resultsNOT(m_Lists[i]);
	}

	m_staleWordCount = true;
}

/// Пересчитывает количество уникальных слов
void CSldSearchWordResult::ReCountWords(void)
{
	m_WordCount = 0;
	for (auto&& info : m_Lists)
		m_WordCount += countWords(info);
}

/***********************************************************************
* Конструктор
************************************************************************/
CSldSimpleSearchWordResult::CSldSimpleSearchWordResult()
	: m_RealListIndex(0), m_WordCount(0), m_staleWordCount(false)
{}

/***********************************************************************
* Инициализация
*
* @param aList			- указатель на обычный список слов
* @param aRealListIndex	- настоящий номер списка слов в словаре
*
* @return код ошибки
************************************************************************/
ESldError CSldSimpleSearchWordResult::Init(Int32 aMaximumNumberOfWords, Int32 aRealListIndex)
{
	m_RealListIndex = aRealListIndex;
	
	// Выделяем память для битовой таблицы
	if (!m_Data.resize(sld2::bitset::size(aMaximumNumberOfWords)))
		return eMemoryZeroSizeMemoryRequest;

	return eOK;
}

/***********************************************************************
* Проверяет, инициализирован класс или нет
*
* @return true, если инициализирован, false в противном случае
************************************************************************/
bool CSldSimpleSearchWordResult::IsInit() const
{
	return !m_Data.empty();
}

/// Очищает все результаты поиска
void CSldSimpleSearchWordResult::Clear()
{
	m_WordCount = 0;
	m_staleWordCount = false;
	if (m_Data.size())
		sldMemZero(m_Data.data(), m_Data.size() * sizeof(m_Data[0]));
}

/***********************************************************************
* Добавляет к результатам поиска ссылку на слово
*
* @param aWordIndex	- номер слова
*
* @return код ошибки
************************************************************************/
ESldError CSldSimpleSearchWordResult::AddWord(Int32 aWordIndex)
{
	if (aWordIndex < 0)
		return eCommonWrongInitSearchList;

	addWord(m_Data.data(), aWordIndex, &m_WordCount);
	return eOK;
}

/***********************************************************************
* Проверяет, содержится ли в результатах поиска слово
*
* @param aWordIndex[in]		- номер слова
*
* @return статус
************************************************************************/
bool CSldSimpleSearchWordResult::IsWordPresent(Int32 aWordIndex) const
{
	return isWordPresent(m_Data.data(), aWordIndex);
}

/// Возвращает количество слов в наборе
Int32 CSldSimpleSearchWordResult::GetWordCount()
{
	if (m_staleWordCount)
	{
		m_WordCount = countWords(m_Data);
		m_staleWordCount = false;
	}
	return m_WordCount;
}

/***********************************************************************
* Заполняет итоговый массив найденными словами
*
* @param aWordVector[in]		- указатель на массив (память выделена снаружи)
* @param aMaximumWords[in]		- максимальное количество слов, которое можно добавить
* @param aLocalListIndex[in]	- локальный (внутренний) номер списка
*
* @return количество слов, которое добавили
************************************************************************/
Int32 CSldSimpleSearchWordResult::FillWordVector(sld2::Span<TSldSearchWordStruct> aWordVector, Int32 aMaximumWords, Int32 aLocalListIndex) const
{
	aWordVector = aWordVector.first(sld2::min<UInt32>(aWordVector.size(), aMaximumWords));
	return fillWordVector(m_Data, aLocalListIndex, aWordVector, 0);
}

/***********************************************************************
* Объединяет результаты 2 поисков через оператор AND (т.е. получается пересечение двух поисков)
*
* @param aSearchResult	- указатель на другой экземпляр класса с резульататами поиска.
*
* @return код ошибки
************************************************************************/
ESldError CSldSimpleSearchWordResult::ResultsAND(const CSldSimpleSearchWordResult* aSearchResult)
{
	if (!aSearchResult)
		return eMemoryNullPointer;

	ESldError error = resultsAND(m_Data, aSearchResult->m_Data);
	if (error == eOK)
		m_staleWordCount = true;
	return error;
}

/***********************************************************************
* Объединяет результаты 2 поисков через оператор OR (т.е. получается сложение двух поисков)
*
* @param aSearchResult	- указатель на другой экземпляр класса с резульататами поиска.
*
* @return код ошибки
************************************************************************/
ESldError CSldSimpleSearchWordResult::ResultsOR(const CSldSimpleSearchWordResult* aSearchResult)
{
	if (!aSearchResult)
		return eMemoryNullPointer;

	ESldError error = resultsOR(m_Data, aSearchResult->m_Data);
	if (error == eOK)
		m_staleWordCount = true;
	return error;
}

/// Применяет к результатам поиска операцию отрицания
void CSldSimpleSearchWordResult::ResultsNOT()
{
	resultsNOT(m_Data);
	m_staleWordCount = true;
}
