#include "SldSimpleSortedList.h"

/** *********************************************************************
* Инициализация списка
*
* @param[in]	aSortedListPtr	- указатель на сортированный список слов
* @param[in]	aWordCount		- количество слов в несортированном списке
*
* @return код ошибки
************************************************************************/
ESldError CSldSimpleSortedList::Init(ISldList* aSortedListPtr, Int32 aWordCount)
{
	if(!aSortedListPtr)
		return eMemoryNullPointer;

	m_data = aSortedListPtr;

	ESldError error = m_data->GetTotalWordCount(&m_NumbersOfWords);
	if (error != eOK)
		return error;

	if(aWordCount > m_NumbersOfWords)
		return eCommonWrongIndex;

	m_SimpleSortedListFlag = true;
	m_IndexVariant = 1;
	m_CurrentIndex = 0;

	UInt32 variantCount = 0;
	error = m_data->GetNumberOfVariants(&variantCount);
	if (error != eOK)
		return error;

	const CSldListInfo* pListInfo = NULL;
	error = m_data->GetWordListInfo(&pListInfo);
	if (error != eOK)
		return error;

	const Int32 index = pListInfo->GetVariantIndexByType(eVariantNotSortedRealGlobalIndex);
	if (index != SLD_DEFAULT_VARIANT_INDEX)
		m_IndexVariant = index;

	error = UpdateSortedIndex();
	if (error != eOK)
		return error;

	return eOK;
}


/** *********************************************************************
* Заканчиваем работу со списком
*
* @return void
************************************************************************/
void CSldSimpleSortedList::Clear(void)
{
	m_data = NULL;
	m_CurrentWord = NULL;
	m_SimpleSortedListFlag = false;
	m_NumbersOfWords = 0;
	m_CurrentIndex = 0;
	m_CurrentSortedIndex = 0;
	m_IsNormalWord = SLD_SIMPLE_SORTED_NORMAL_WORD;
	m_IndexVariant = 0;
}


/** *********************************************************************
* Возвращает индекс слова, которое больше или равно заданному
*
* @param[in]	aText			- запрашиваемое слово
* @param[out]	aSortedIndex	- указатель, по которому будет записан
*								  индекс слова для несортированного списка
*
* @return код ошибки
************************************************************************/
ESldError CSldSimpleSortedList::GetSortedIndexByText(const UInt16* aText, Int32* aSortedIndex)
{
	if(!aText)
		return eMemoryNullPointer;

	ESldError error = m_data->GetWordByText(aText);
	if (error != eOK)
		return error;

	error = UpdateSortedIndex();
	if (error != eOK)
		return error;

	*aSortedIndex = m_CurrentSortedIndex;

	return eOK;
}


/** *********************************************************************
* Возвращает индекс несортированного списка по индексу сортированного
*
* @param[in]	aIndex			- запрашиваемый индекс в сортированном списке
* @param[out]	aSortedIndex	- указатель, по которому будет записан
*								  индекс слова для несортированного списка
*
* @return код ошибки
************************************************************************/
ESldError CSldSimpleSortedList::GetSortedIndexByIndex(const Int32 aIndex, Int32* aSortedIndex)
{
	ESldError error = m_data->GetWordByIndex(aIndex);
	if (error != eOK)
		return error;

	error = UpdateSortedIndex();
	if (error != eOK)
		return error;

	*aSortedIndex = m_CurrentSortedIndex;

	return eOK;
}


/** *********************************************************************
* Возвращает индекс следующего слова
*
* @param[out]	aSortedIndex	- указатель, по которому будет записан
*								  индекс слова для несортированного списка
*
* @return код ошибки
************************************************************************/
ESldError CSldSimpleSortedList::GetNextWordSortedIndex(Int32* aSortedIndex)
{
	ESldError error = m_data->GetNextWord();
	if (error != eOK)
		return error;

	error = UpdateSortedIndex();
	if (error != eOK)
		return error;

	*aSortedIndex = m_CurrentSortedIndex;

	return eOK;
}


/** *********************************************************************
* Возвращает последний декодированный индекс для несортированного списка
*
* @param[out]	aSortedIndex	- указатель, по которому будет записан
*								  индекс слова для несортированного списка
*
* @return код ошибки
************************************************************************/
ESldError CSldSimpleSortedList::GetCurrentSortedIndex(Int32* aSortedIndex) const
{
	if(!aSortedIndex)
		return eMemoryNullPointer;

	*aSortedIndex = m_CurrentSortedIndex;

	return eOK;
}



/** *********************************************************************
* Возвращает флаг того, что нам доступны функции сортированного списка
*
* @return true, если функции доступны, иначе false
************************************************************************/
Int8 CSldSimpleSortedList::HasSortedList() const
{
	return m_SimpleSortedListFlag;
}


/** *********************************************************************
* Синхронизирует список в соответсвии с текущим словом
*
* @return код ошибки
************************************************************************/
ESldError CSldSimpleSortedList::UpdateSortedIndex()
{
	UInt16* sortedIndexStr = NULL;

	ESldError error = m_data->GetCurrentWord(m_IndexVariant, &sortedIndexStr);
	if (error != eOK)
		return error;

	Int8* ptrToStr = (Int8*)sortedIndexStr;
	m_IsNormalWord = *ptrToStr++;

	sortedIndexStr = (UInt16*)ptrToStr;

	if(*sortedIndexStr)
	{
		sldMemMove(&m_CurrentSortedIndex, ptrToStr, sizeof(Int32));
	}
	else
	{
		UInt16* tmpStr = NULL;
		ESldError error = m_data->GetCurrentWord(m_IndexVariant, &tmpStr);
		if (error != eOK)
			return error;

		if(CSldCompare::StrLen(tmpStr) == 1)
		{
			m_CurrentSortedIndex = 0;
		}
		else
		{
			sldMemMove(&m_CurrentSortedIndex, ptrToStr, sizeof(Int32));
		}
	}

	if(m_CurrentSortedIndex < 0 || m_CurrentSortedIndex >= m_NumbersOfWords)
		return eCommonWrongIndex;

	error = m_data->GetCurrentIndex(&m_CurrentIndex);
	if (error != eOK)
		return error;

	error = m_data->GetCurrentWord(0, &m_CurrentWord);
	if (error != eOK)
		return error;

	return eOK;
}


/** *********************************************************************
* Возвращает текущий индекс сортированного списка
*
* @param[out]	aIndex	- указатель, по которому будет записан текущий индекс
*
* @return код ошибки
************************************************************************/
ESldError CSldSimpleSortedList::GetCurrentIndex(Int32* aIndex) const
{
	*aIndex = m_CurrentIndex;

	return eOK;
}


/** *********************************************************************
* Возвращает show-вариант последнего декодированного слова
*
* @param[out]	aWord	- указатель, по которому будет записан результат
*
* @return код ошибки
************************************************************************/
ESldError CSldSimpleSortedList::GetCurrentWord(UInt16** aWord) const
{
	if(!aWord)
		return eMemoryNullPointer;

	*aWord = m_CurrentWord;

	return eOK;
}


/** *********************************************************************
* Возвращает количество записей в таблице быстрого доступа
*
* @param[out]	aNumberOfQAItems - указатель на переменную, в которую нужно будет
*								   поместить количество записей в таблице быстрого доступа.
*
* @return код ошибки
************************************************************************/
ESldError CSldSimpleSortedList::QAGetNumberOfQAItems(Int32 *aNumberOfQAItems)
{
	return m_data->QAGetNumberOfQAItems(aNumberOfQAItems);
}

/** *********************************************************************
* Возвращает текст для записи в таблице быстрого доступа.
*
* @param[in]	aIndex	- номер элемента в таблице быстрого доступа
* @param[out]	aQAText	- указатель на буфер для текста элемента таблицы быстрого доступа
*
* @return код ошибки
************************************************************************/
ESldError CSldSimpleSortedList::QAGetItemText(Int32 aIndex, const UInt16 **aQAText)
{
	return m_data->QAGetItemText(aIndex, aQAText);
}

/** *********************************************************************
* По номеру точки входа возвращает номер слова в списке, соответствующий этой точке
*
* @param[in]	aIndex		- номер элемента в таблице быстрого доступа
* @param[out]	aQAIndex	- указатель на буфер, куда сохраняется результат
*
* @return код ошибки
************************************************************************/
ESldError CSldSimpleSortedList::QAGetItemIndex(Int32 aIndex, Int32 *aQAIndex)
{
	return m_data->QAGetItemIndex(aIndex, aQAIndex);
}

/** *********************************************************************
* Производит переход по таблице быстрого доступа по номеру записи
*
* @param[in]	aIndex	- номер элемента в таблице быстрого доступа по которому нужно произвести переход.
*
* @return код ошибки
************************************************************************/
ESldError CSldSimpleSortedList::JumpByQAIndex(Int32 aIndex)
{
	ESldError error = m_data->JumpByQAIndex(aIndex);
	if (error != eOK)
		return error;

	return m_data->GetCurrentIndex(&m_CurrentSortedIndex);
}

/** *********************************************************************
* Возвращает флаг, определяющий реальное это слово или виртуальное
*
* @return результат выполнения функции: 1 - текущее слово реальное
*										-1 - текущее слово виртуальное
************************************************************************/
Int8 CSldSimpleSortedList::IsNormalWord()
{
	return m_IsNormalWord;
}

/** *********************************************************************
* Ищет точное вхождение слова в Show-вариантах и альтернативных заголовках
*
* @param[in]	aText		- искомое слово
* @param[out]	aResult		- сюда сохраняется флаг результата:
*							  0 - точное совпадение не найдено
*							  1 - точное совпадение найдено
*							  2 - точное совпадение найдено среди альтернативных заголовков.
*							  Если в списке нет совпадающих по массе show-вариантов, а альтернативный заголовок
*							  по массе совпал, так же вернется это значение
*
* @return код ошибки
************************************************************************/
ESldError CSldSimpleSortedList::FindBinaryMatch(const UInt16* aText, UInt32* aResult)
{
	ESldError error = m_data->GetMostSimilarWordByText(aText, aResult);
	if (error != eOK)
		return error;

	if(!*aResult)
		return eOK;

	UInt16* tmpStr = NULL;
	error = m_data->GetCurrentWord(m_IndexVariant, &tmpStr);
	if (error != eOK)
		return error;

	Int8 isRealWord = *(Int8*)tmpStr;
	*aResult = (isRealWord == SLD_SIMPLE_SORTED_VIRTUAL_WORD) ? 2 : 1;

	return UpdateSortedIndex();
}