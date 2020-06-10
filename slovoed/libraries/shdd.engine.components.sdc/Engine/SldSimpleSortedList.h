#ifndef _SLD_SIMPLE_SORTED_LIST_H_
#define _SLD_SIMPLE_SORTED_LIST_H_

#include "ISldList.h"

class CISldList;

class CSldSimpleSortedList
{
public:
	CSldSimpleSortedList(void)
	{
		Clear();
	};

	/// Инициализация списка
	ESldError Init(ISldList* aSortedListPtr, Int32 aWordCount);

	/// Очистка списка
	void Clear(void);

	/// Возвращает индекс слова, которое больше или равно заданному
	ESldError GetSortedIndexByText(const UInt16* aText, Int32* aSortedIndex);

	/// Возвращает индекс несортированного списка по индексу сортированного
	ESldError GetSortedIndexByIndex(const Int32 aIndex, Int32* aSortedIndex);

	/// Возвращает индекс следующего слова
	ESldError GetNextWordSortedIndex(Int32* aSortedIndex);

	/// Возвращает последний декодированный индекс для несортированного списка
	ESldError GetCurrentSortedIndex(Int32* aSortedIndex) const;

	/// Возвращает текущий индекс сортированного списка
	ESldError GetCurrentIndex(Int32* aIndex) const;

	/// Возвращает show-вариант последнего декодированного слова
	ESldError GetCurrentWord(UInt16** aWord) const;

	/// Возвращает флаг того, что нам доступны функции сортированного списка
	Int8 HasSortedList() const;

	/// Возвращает количество записей в таблице быстрого доступа
	ESldError QAGetNumberOfQAItems(Int32 *aNumberOfQAItems);

	/// Возвращает текст для записи в таблице быстрого доступа.
	ESldError QAGetItemText(Int32 aIndex, const UInt16 **aQAText);

	/// По номеру точки входа возвращает номер слова в списке, соответствующий этой точке
	ESldError QAGetItemIndex(Int32 aIndex, Int32 *aQAIndex);

	/// Производит переход по таблице быстрого доступа по номеру записи
	ESldError JumpByQAIndex(Int32 aIndex);

	/// Возвращает флаг, определяющий реальное это слово или виртуальное
	Int8 IsNormalWord();

	/// Возвращает количество слов в списке
	ESldError GetNumberOfWords(Int32* aWordsCount) { return m_data->GetTotalWordCount(aWordsCount); }

	/// Ищет точное вхождение слова в Show-вариантах и альтернативных заголовках
	ESldError FindBinaryMatch(const UInt16* aText, UInt32* aResult);
	
private:
	/// Синхронизирует список в соответсвии с текущим словом
	ESldError UpdateSortedIndex();


private:
	/// Указатель на сортированный список
	ISldList* m_data;

	/// Флаг того, что нам доступны функции сортированного списка
	bool m_SimpleSortedListFlag;

	/// Количество слов в сортированном списке
	Int32 m_NumbersOfWords;

	/// Текущий индекс в сортированном списке
	Int32 m_CurrentIndex;

	/// Последний декодированный индекс для несортированного списка
	Int32 m_CurrentSortedIndex;

	/// Флаг, определяющий реальное это слово или виртуальное
	Int8 m_IsNormalWord;

	/// Последнее декодированное слово
	UInt16* m_CurrentWord;

	/// Номер варианта написания, соответсвующий индексам из несортированного списка слов
	UInt32 m_IndexVariant;
};

#endif
