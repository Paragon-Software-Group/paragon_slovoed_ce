#include "SldCustomList.h"

/** *********************************************************************
* Возвращает указатель на запрашиваемый вариант написания
*
* @param[in]	aVariantIndex	- номер запрашиваемого варианта написания
* @param[out]	aWord			- указатель на буфер для указателя на слово
*
* @return код ошибки
************************************************************************/
ESldError TSldCustomListWordInfo::GetCurrentWordPtr(Int32 aVariantIndex, UInt16** aWord, UInt16* aDefaultWord)
{
	if (!aWord)
		return eMemoryNullPointer;

	if (CustomWords && aVariantIndex >= NumberOfVariants)
		return eCommonWrongVariantIndex;

	*aWord = aDefaultWord;

	if (CustomWords && CustomWords[aVariantIndex])
	{
		*aWord = CustomWords[aVariantIndex];
	}
	else if (RealWordIndex != SLD_DEFAULT_WORD_INDEX)
	{
		ESldError error = RealListPtr->GetWordByGlobalIndex(RealWordIndex);
		if (error != eOK)
			return error;

		error = RealListPtr->GetCurrentWord(aVariantIndex, aWord);
		if (error != eOK)
			return error;
	}

	return eOK;
}

/** *********************************************************************
* Освобождение памяти
*
************************************************************************/
void TSldCustomListWordInfo::Close()
{
	if (CustomWords)
	{
		for (Int32 wordIndex = 0; wordIndex < NumberOfVariants; wordIndex++)
		{
			if (CustomWords[wordIndex])
			{
				sldMemFree(CustomWords[wordIndex]);
			}
		}

		sldMemFree(CustomWords);
		CustomWords = NULL;
	}
}

ESldError TSldRealLists::Init(ISldList** aWordLists, const UInt32 aNumberOfLists)
{
	Close();
	Clear();

	WordLists = (ISldList**)sldMemNewZero(aNumberOfLists * sizeof(ISldList*));
	if (!WordLists)
		return eMemoryNotEnoughMemory;

	NumberOfLists = aNumberOfLists;
	sldMemMove(WordLists, aWordLists, NumberOfLists * sizeof(ISldList*));

	return eOK;
}

/// Возвращает указатель на словарный список слов
ESldError TSldRealLists::GetListPtr(const Int32 aListIndex, ISldList** aListPtr)
{
	if (!aListPtr)
		return eMemoryNullPointer;

	if (aListIndex >= NumberOfLists)
		return eCommonWrongList;

	*aListPtr = WordLists[aListIndex];

	return eOK;
}


/// Деструктор
CSldCustomList::~CSldCustomList()
{
	if (m_WordVector)
	{
		for (Int32 wordIndex = 0; wordIndex < m_NumbersOfWords; wordIndex++)
			m_WordVector[wordIndex].Close();

		sldMemFree(m_WordVector);
	}

	if (m_SortedWordIndexes)
		sldMemFree(m_SortedWordIndexes);

	if (m_TmpWordBuff)
	{
		for (UInt32 wordIndex = 0; wordIndex < m_TmpWordSize; wordIndex++)
		{
			if (m_TmpWordBuff[wordIndex])
				sldMemFree(m_TmpWordBuff[wordIndex]);
		}
		sldMemFree(m_TmpWordBuff);
	}
}

/** *********************************************************************
* Инициализация списка слов
*
* @param[in]	aData			- указатель на объект, отвечающий за получение данных из контейнера
* @param[in]	aLayerAccess	- указатель на класс для взаимодействия с оболочкой
* @param[in]	aListInfo		- указатель на класс, хранящий информацию о списке слов
* @param[in]	aCMP			- указатель на класс сравнения строк
* @param[in]	aHASH			- число необходимое для декодирования списка слов (в поисковом списке не используется)
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::Init(CSDCReadMy* aData, ISldLayerAccess* aLayerAccess, const CSldListInfo* aListInfo, CSldCompare* aCMP, UInt32 aHASH)
{
	if (!aLayerAccess || !aListInfo)
		return eMemoryNullPointer;

	ESldError error;
	m_LayerAccess = aLayerAccess;

	TListHeader listHeader;
	sldMemZero(&listHeader, sizeof(listHeader));

	const TListHeader* pSourceListHeader = aListInfo->GetHeader();
	if (!pSourceListHeader)
		return eMemoryNullPointer;
	
	sldMemMove(&listHeader, pSourceListHeader, pSourceListHeader->HeaderSize);
	
	listHeader.HeaderSize = sizeof(listHeader);
	listHeader.WordListUsage = eWordListType_CustomList;
	
	m_ListInfo = sldNew<CSldListInfo>();
	
	if(aData)
	{
		error = m_ListInfo->Init(*aData, &listHeader);
		if (error != eOK)
			return error;
	}
	else
	{
		error = m_ListInfo->Init(listHeader);
		if (error != eOK)
			return error;
	}

	m_CMP = aCMP;

	return eOK;
}

/** *********************************************************************
* Проверяем, проинициализирован ли список слов
*
* @return true - если инициализация проведена, иначе false
************************************************************************/
bool CSldCustomList::isInit(void)
{
	if (m_LayerAccess && m_ListInfo)
		return true;
		
	return false;
}


/** *********************************************************************
* Получает слово по его глобальному номеру
*
* @param[in]	aGlobalIndex	- глобальный номер слова
* @param[in]	aInLocalization	- флаг учета локализации. true - мы запрашиваем индекс с учетом локализации
*								  false - мы запрашиваем индекс без учета локализации
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::GetWordByGlobalIndex(Int32 aGlobalIndex, const bool aInLocalization)
{
	// Глобальный индекс равен локальному	
	return GetWordByIndex(aGlobalIndex);
}

/** *********************************************************************
* Получаем слово по локальному номеру (номеру слова в пределах текущего списка слов и текущего уровня вложенности)
*
* @param[in]	aWordIndex - номер слова по порядку, начиная с 0.
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::GetWordByIndex(Int32 aWordIndex)
{	
	if(aWordIndex >= m_NumbersOfWords || aWordIndex < 0)
		return eCommonWrongIndex;

	m_CurrentIndex = aWordIndex;

	return eOK;
}

/** *********************************************************************
* Ищем ближайшее слово, которое больше или равно заданному
*
* @param[in]	aText	- искомое слово
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::GetWordByText(const UInt16* aText)
{
	if (!aText)
		return eMemoryNullPointer;

	ESldError error = eOK;
	if (m_SearchSourceListIndex != SLD_DEFAULT_LIST_INDEX)
	{
		UInt32 result = 0;
		return GetWordByTextInRealList(aText, &result, 2);
	}
	else
	{
		if (!m_SortedWordIndexes)
		{
			error = SortListByVariant(m_SortedVariantIndex);
			if (error != eOK)
				return error;
		}

		UInt32 low = 0;
		UInt32 hi = m_NumbersOfWords;
		UInt32 med = low;
		UInt16 *currentText = NULL;
		Int32 compareResult = 0;

		while (hi - low > 1)
		{
			error = GetWordByGlobalIndex(m_SortedWordIndexes[med]);
			if (error != eOK)
				return error;

			error = GetCurrentWord(m_SortedVariantIndex, &currentText);
			if (error != eOK)
				return error;

			compareResult = m_CMP->StrICmp(currentText, aText, 0);

			if (compareResult < 0)
			{
				low = med;
			}
			else if (compareResult > 0)
			{
				hi = med;
			}
			else
			{
				break;
			}

			med = (hi + low) >> 1;
		}

		if (compareResult == 0)
		{
			if (low == 0)
			{
				error = GetWordByGlobalIndex(m_SortedWordIndexes[low]);
				if (error != eOK)
					return error;

				error = GetCurrentWord(m_SortedVariantIndex, &currentText);
				if (error != eOK)
					return error;

				if (m_CMP->StrICmp(currentText, aText, 0) == 0)
					med = low;
			}
		}
		else
		{
			med = hi;
		}

		error = GetWordByGlobalIndex(m_SortedWordIndexes[med]);
		if (error != eOK)
			return error;

	}

	return eOK;
}

/** *********************************************************************
* Ищем наиболее похожее слово, которое равно заданному
*
* @param[in]	aText		- искомое слово
* @param[out]	aResultFlag	- сюда сохраняется флаг результата:
*							  1	- похожее слово найдено,
*							  0	- похожее слово не найдено
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::GetMostSimilarWordByText(const UInt16* aText, UInt32* aResultFlag)
{
	if (!aResultFlag)
		return eMemoryNullPointer;

	*aResultFlag = 0;

	ESldError error = GetWordByText(aText);
	if (error != eOK)
		return error;

	UInt16* currentWord = NULL;
	error = GetCurrentWord(m_SortedVariantIndex, &currentWord);
	if (error != eOK)
		return error;

	if (m_CMP->StrICmp(aText, currentWord) == 0)
	{
		*aResultFlag = 1;
	}

	return eOK;
}

/** *********************************************************************
* Возвращает указатель на последнее найденное слово. 
*
* @param[in]	aVariantIndex	- номер варианта написания для текущего слова.
* @param[out]	aWord			- указатель на буфер для указателя на слово
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::GetCurrentWord(Int32 aVariantIndex, UInt16** aWord)
{	
	if(!aWord)
		return eMemoryNullPointer;

	const auto & currentWordInfo = m_WordVector[m_CurrentIndex];

	if (currentWordInfo.CustomWords && aVariantIndex < currentWordInfo.NumberOfVariants && currentWordInfo.CustomWords[aVariantIndex])
	{
		*aWord = currentWordInfo.CustomWords[aVariantIndex];
		return eOK;
	}
	else if (currentWordInfo.RealListPtr && currentWordInfo.RealWordIndex != SLD_DEFAULT_WORD_INDEX)
	{
		ISldList* pList = currentWordInfo.RealListPtr;
		/// Считаем, что в ядре активным является CSldCustomList, значит при переключении на базовый список - состояние восстановится
		ESldError error = pList->GetWordByGlobalIndex(currentWordInfo.RealWordIndex);
		if(error != eOK)
			return error;

		const EListVariantTypeEnum variantType = m_ListInfo->GetVariantType(aVariantIndex);
		if (variantType == eVariantUnknown)
			return eCommonWrongVariantIndex;

		const CSldListInfo* listInfo = NULL;
		error = pList->GetWordListInfo(&listInfo);
		if(error != eOK)
			return error;

		const Int32 variantIndex = listInfo->GetVariantIndexByType(variantType);
		if (variantIndex != SLD_DEFAULT_VARIANT_INDEX)
			return pList->GetCurrentWord(variantIndex, aWord);
	}

	*aWord = &m_DefaultChar;
	return eOK;
}

/** ********************************************************************
* Получает локальный номер текущего слова (номер слова в пределах текущего списка слов и текущего уровня вложенности)
*
* @param[out]	aIndex	- указатель на переменную, в которую будет сохранен номер текущего слова
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::GetCurrentIndex(Int32* aIndex) const
{
	if (!aIndex)
		return eMemoryNullPointer;

	*aIndex = m_CurrentIndex;
	
	return eOK;
}

/** *********************************************************************
* Получает глобальный номер текущего слова (номер слова в пределах всего списка слов)
*
* @param[out]	aIndex	- указатель на переменную, в которую будет сохранен номер текущего слова
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::GetCurrentGlobalIndex(Int32* aIndex) const
{
	if (!aIndex)
		return eMemoryNullPointer;

	*aIndex = m_CurrentIndex;
	
	return eOK;
}

/** *********************************************************************
* Конвертирует локальный номер слова в пределах текущего списка слов и уровня вложенности
* в глобальный номер слова в пределах текущего списка слов
*
* @param[in]	aLocalIndex		- локальный номер слова в пределах текущего уровня вложенности
* @param[out]	aGlobalIndex	- указатель на переменную, куда сохраняется глобальный номер слова
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::LocalIndex2GlobalIndex(Int32 aLocalIndex, Int32* aGlobalIndex)
{
	if (!aGlobalIndex)
		return eMemoryNullPointer;

	Int32 NumberOfWords = 0;
	ESldError error = GetNumberOfWords(&NumberOfWords);
	if (error != eOK)
		return error;

	if (aLocalIndex < 0 || aLocalIndex >= NumberOfWords)
		return eCommonWrongIndex;

	*aGlobalIndex = aLocalIndex;

	return eOK;
}

/** *********************************************************************
* Получает номер списка слов по локальному номеру слова
*
* @param[in]	aLocalWordIndex	- локальный номер слова
* @param[out]	aListIndex		- указатель на переменную, в которую сохраняется номер списка слов
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::GetRealListIndex(Int32 aLocalWordIndex, Int32* aListIndex)
{
	if (!aListIndex)
		return eMemoryNullPointer;

	if (aLocalWordIndex >= m_NumbersOfWords)
		return eCommonWrongIndex;

	*aListIndex = m_WordVector[aLocalWordIndex].RealListIndex;

	return eOK;
}

/** *********************************************************************
* Получает глобальный номер слова по локальному номеру слова
*
* @param[in]	aLocalWordIndex		- локальный номер слова
* @param[out]	aGlobalWordIndex	- указатель на переменную, в которую сохраняется глобальный номер слова
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::GetRealGlobalIndex(Int32 aLocalWordIndex, Int32* aGlobalWordIndex)
{	
	if (!aGlobalWordIndex)
		return eMemoryNullPointer;

	if (aLocalWordIndex >= m_NumbersOfWords)
		return eCommonWrongIndex;

	*aGlobalWordIndex = m_WordVector[aLocalWordIndex].RealWordIndex;

	return eOK;
}

/** *********************************************************************
* Возвращает полное количество слов в списке слов
*
* @param[out]	aCount	- указатель на переменную, в которую сохраняется полное количество слов в списке слов
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::GetTotalWordCount(Int32* aCount)
{
	if (!aCount)
		return eMemoryNullPointer;
	*aCount = 0;
	
	ESldError error = GetNumberOfWords(aCount);
	if (error != eOK)
		return error;
	
	return eOK;
}

/** *********************************************************************
* Возвращает количество слов у списка слов
*
* @param[out]	aNumberOfWords	- указатель на переменную, в которую будет сохранено количество слов
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::GetNumberOfWords(Int32* aNumberOfWords) const
{
	if (!aNumberOfWords)
		return eMemoryNullPointer;

	*aNumberOfWords = m_NumbersOfWords;
	return eOK;
}

/** *********************************************************************
* Возвращает количество вариантов написания для списка слов
*
* @param[out]	aNumberOfVariants	- указатель на буфер для количества вариантов
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::GetNumberOfVariants(UInt32* aNumberOfVariants)
{
	return eOK;
}

/** *********************************************************************
* Устанавливаем базу для списка слов 
*
* @param[in]	aBase	- Номер текущего слова которое должно будет стать базой.
*						  MAX_UINT_VALUE - сбросить базу.
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::SetBase(Int32 aBase)
{
	return eOK;
}


/** *********************************************************************
* Возвращает количество кодов использования списка слов
* Для поискового списка подсчитываются коды использования списков слов,
* в которых производился поиск + свой собственный код использования
*
* @param[out]	aCount	- указатель на переменную, в которую будет сохранен результат
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::GetUsageCount(Int32* aCount) const
{
	return eOK;
}

/** *********************************************************************
* Возвращает код использования списка слов по индексу кода
*
* @param[in]	aIndex	- индекс кода использования
* @param[out]	aUsage	- указатель на переменную, в которую будет сохранен код (см. #EWordListTypeEnum)
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::GetListUsage(Int32 aIndex, UInt32* aUsage) const
{
	return eOK;
}

/** *********************************************************************
* Возвращает указатель на класс, хранящий информацию о свойствах списка слов
*
* @param[out]	aListInfo	- указатель на переменную, в которую будет возвращен указатель
* @param[in]	aDictIndex	- индекс словаря в ядре слияния, имеет дефолтное значение SLD_DEFAULT_DICTIONARY_INDEX
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::GetWordListInfo(const CSldListInfo** aListInfo, Int32 aDictIndex) const
{
	if (!aListInfo)
		return eMemoryNullPointer;

	if (!m_ListInfo)
		return eMemoryNullPointer;

	*aListInfo = m_ListInfo;
	
	return eOK;
}

/** *********************************************************************
* Возвращает флаг того, сортированный или нет данный список слов
*
* @param[out]	aIsSorted	- указатель на переменную, в которую будет возвращен флаг
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::IsListSorted(UInt32* aIsSorted)
{
	if (!aIsSorted)
		return eMemoryNullPointer;

	if (!m_ListInfo)
		return eMemoryNullPointer;

	*aIsSorted = m_ListInfo->IsSortedList();
	return eOK;
}

/** *********************************************************************
* Возвращает флаг того, сопоставлены или нет некоторым словам из списка слов картинки
*
* @param[out]	aIsPicture	- указатель на переменную, в которую будет возвращен флаг
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::IsListHasPicture(UInt32* aIsPicture)
{
	if (!aIsPicture)
		return eMemoryNullPointer;

	if (!m_ListInfo)
		return eMemoryNullPointer;

	*aIsPicture = m_ListInfo->IsPicture();
	return eOK;
}

/** *********************************************************************
* Возвращает флаг того, сопоставлено или нет некоторым словам из списка слов видео
*
* @param[out]	aIsVideo	- указатель на переменную, в которую будет возвращен флаг
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::IsListHasVideo(UInt32* aIsVideo)
{
	if (!aIsVideo)
		return eMemoryNullPointer;

	if (!m_ListInfo)
		return eMemoryNullPointer;

	*aIsVideo = m_ListInfo->IsVideo();
	return eOK;
}

/** *********************************************************************
* Возвращает флаг того, сопоставлена или нет некоторым словам из списка слов озвучка
*
* @param[out]	aIsSound	- указатель на переменную, в которую будет возвращен флаг
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::IsListHasSound(UInt32* aIsSound)
{
	if (!aIsSound)
		return eMemoryNullPointer;

	if (!m_ListInfo)
		return eMemoryNullPointer;

	*aIsSound = m_ListInfo->IsSound();
	return eOK;
}

/** *********************************************************************
* Возвращает флаг того, имеет или нет указанное слово поддерево иерархии
* Меняет состояние списка
*
* @param[in]	aIndex			- номер слова на текущем уровне иерархии
* @param[out]	aIsHierarchy	- указатель на переменную, в которую будет возвращен флаг
* @param[out]	aLevelType		- указатель, по которому будет записан тип поддерева (см #EHierarchyLevelType)
*								  (можно передать NULL)
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::isWordHasHierarchy(Int32 aIndex, UInt32* aIsHierarchy, EHierarchyLevelType* aLevelType)
{
	if(!aIsHierarchy)
		return eMemoryNullPointer;

	*aIsHierarchy = 0;

	ISldList* pList = m_WordVector[aIndex].RealListPtr;
	if(!pList)
		return eOK;

	if (m_WordVector[aIndex].RealWordIndex == SLD_DEFAULT_WORD_INDEX)
		return eOK;

	TCatalogPath path;
	ESldError error = pList->GetPathByGlobalIndex(m_WordVector[aIndex].RealWordIndex, &path);
	if(error != eOK)
		return error;

	error = pList->GoToByPath(&path, eGoToWord);
	if(error != eOK)
		return error;

	error = pList->isWordHasHierarchy(path.BaseList[path.BaseListCount - 1], aIsHierarchy, NULL);
	if(error != eOK)
		return error;

	return eOK;
}

/** *********************************************************************
* Возвращает вектор индексов изображений для текущего слова
*
* @param[out]	aPictureIndexes	- вектор индексов изображений для текущего слова
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::GetPictureIndex(CSldVector<Int32> & aPictureIndexes)
{
	if(m_NumbersOfWords == 0)
		return eOK;

	ISldList* pList = m_WordVector[m_CurrentIndex].RealListPtr;
	if(!pList)
		return eOK;

	if (m_WordVector[m_CurrentIndex].RealWordIndex == SLD_DEFAULT_WORD_INDEX)
		return eOK;

	/// Считаем, что в ядре активным является CSldCustomList, значит при переключении на базовый список - состояние восстановится
	ESldError error = pList->GetWordByGlobalIndex(m_WordVector[m_CurrentIndex].RealWordIndex);
	if(error != eOK)
		return error;

	error = pList->GetPictureIndex(aPictureIndexes);
	if(error != eOK)
		return error;

	return eOK;
}

/** *********************************************************************
* Возвращает номер видео текущего слова
*
* @param[out]	aVideoIndex	- указатель на переменную, в которую будет записан номер видео,
*							  либо SLD_INDEX_VIDEO_NO в случае его отсутствия
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::GetVideoIndex(Int32* aVideoIndex)
{
	if(!aVideoIndex)
		return eMemoryNullPointer;

	*aVideoIndex = SLD_INDEX_VIDEO_NO;
	if(m_NumbersOfWords == 0)
		return eOK;

	ISldList* pList = m_WordVector[m_CurrentIndex].RealListPtr;
	if(!pList)
		return eOK;

	if (m_WordVector[m_CurrentIndex].RealWordIndex == SLD_DEFAULT_WORD_INDEX)
		return eOK;

	/// Считаем, что в ядре активным является CSldCustomList, значит при переключении на базовый список - состояние восстановится
	ESldError error = pList->GetWordByGlobalIndex(m_WordVector[m_CurrentIndex].RealWordIndex);
	if(error != eOK)
		return error;

	error = pList->GetVideoIndex(aVideoIndex);
	if(error != eOK)
		return error;

	return eOK;
}

/** *********************************************************************
* Возвращает вектор индексов озвучек текущего слова
*
* @param[out]	aSoundIndexes	- вектор с индексами озвучки для текущего слова
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::GetSoundIndex(CSldVector<Int32> & aSoundIndexes)
{
	if(m_NumbersOfWords == 0)
		return eOK;

	ISldList* pList = m_WordVector[m_CurrentIndex].RealListPtr;
	if(!pList)
		return eOK;

	if (m_WordVector[m_CurrentIndex].RealWordIndex == SLD_DEFAULT_WORD_INDEX)
		return eOK;

	/// Считаем, что в ядре активным является CSldCustomList, значит при переключении на базовый список - состояние восстановится
	ESldError error = pList->GetWordByGlobalIndex(m_WordVector[m_CurrentIndex].RealWordIndex);
	if(error != eOK)
		return error;

	error = pList->GetSoundIndex(aSoundIndexes);
	if(error != eOK)
		return error;

	return eOK;
}

/** ********************************************************************
* Возвращает количество переводов у указанного слова.
*
* @param[in]	aGlobalIndex				- номер слова из списка слов для которого требуется узнать
*									  количество переводов
* @param[out]	aTranslationCount	- указатель на переменную, в которую будет помещено
*									  количество переводов у последнего декодированного слова.
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::GetTranslationCount(Int32 aGlobalIndex, Int32* aTranslationCount)
{
	if(!aTranslationCount)
		return eMemoryNullPointer;

	if (aGlobalIndex >= m_NumbersOfWords)
		return eCommonWrongIndex;

	*aTranslationCount = 0;

	ISldList* pList = m_WordVector[aGlobalIndex].RealListPtr;
	if (!pList)
		return eOK;

	if (m_WordVector[aGlobalIndex].RealWordIndex == SLD_DEFAULT_WORD_INDEX)
		return eOK;

	const CSldListInfo* listInfo = NULL;
	ESldError error = pList->GetWordListInfo(&listInfo);
	if(error != eOK)
		return error;

	Int32 realWordIndex = m_WordVector[aGlobalIndex].RealWordIndex;

	if (listInfo->GetUsage() == eWordListType_FullTextAuxiliary)
	{
		Int32 realListIndex = SLD_DEFAULT_LIST_INDEX;
		Int32 realIndexesCount = 0;

		error = pList->GetReferenceCount(realWordIndex, &realIndexesCount);
		if (error != eOK)
			return error;

		if (realIndexesCount == 1)
		{
			error = pList->GetRealIndexes(realWordIndex, 0, &realListIndex, &realWordIndex);
			if (error != eOK)
				return error;

			error = m_RealListVector.GetListPtr(realListIndex, &pList);
			if (error != eOK)
				return error;

			UInt32 usage = 0;
			error = pList->GetListUsage(0, &usage);
			if (error != eOK)
				return error;

			if (usage == eWordListType_FullTextAuxiliary)
				return eOK;
		}
		else
			return eOK;
	}
	else if (listInfo->IsFullTextSearchList())
		return eOK;

	/// Считаем, что в ядре активным является CSldCustomList, значит при переключении на базовый список - состояние восстановится
	error = pList->GoToByGlobalIndex(realWordIndex);
	if(error != eOK)
		return error;

	Int32 globalIndex = SLD_DEFAULT_WORD_INDEX;
	error = pList->GetCurrentGlobalIndex(&globalIndex);
	if(error != eOK)
		return error;

	error = pList->GetTranslationCount(globalIndex, aTranslationCount);
	if(error != eOK)
		return error;

	return eOK;
}

/** ********************************************************************
* Возвращает количество ссылок у указанного слова из списка слов
*
* @param[in]	aGlobalIndex		- номер слова из списка слов для которого требуется узнать
*									  количество ссылок
* @param[out]	aTranslationCount	- указатель на переменную, в которую будет помещено
*									  количество переводов у последнего декодированного слова.
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::GetReferenceCount(Int32 aGlobalIndex, Int32* aTranslationCount)
{
	if (!aTranslationCount)
		return eMemoryNullPointer;

	*aTranslationCount = 0;

	if (m_WordVector[aGlobalIndex].RealWordIndex == SLD_DEFAULT_WORD_INDEX)
	{
		*aTranslationCount = 0;
	}
	else
	{
		*aTranslationCount = 1;
	}


	return eOK;
}

/** ********************************************************************
* Возвращает номер статьи с переводом, по номеру перевода
*
* @param[in]	aGlobalIndex		- номер слова из списка слов для которого требуется узнать
*									  количество переводов
* @param[in]	aTranslationIndex	- номер перевода текущего слова для которого 
*									  хотим получить номер статьи с переводом.
* @param[out]	aArticleIndex		- указатель на переменную в которую будет
*									  помещен номер статьи с переводом.
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::GetTranslationIndex(Int32 aGlobalIndex, Int32 aTranslationIndex, Int32* aArticleIndex)
{
	if(!aArticleIndex)
		return eMemoryNullPointer;

	if (aGlobalIndex >= m_NumbersOfWords)
		return eCommonWrongIndex;

	ISldList* pList = m_WordVector[aGlobalIndex].RealListPtr;

	const CSldListInfo* listInfo = NULL;
	ESldError error = pList->GetWordListInfo(&listInfo);
	if (error != eOK)
		return error;

	Int32 realWordIndex = m_WordVector[aGlobalIndex].RealWordIndex;

	if (listInfo->GetUsage() == eWordListType_FullTextAuxiliary)
	{
		Int32 realListIndex = SLD_DEFAULT_LIST_INDEX;
		Int32 realIndexesCount = 0;

		error = pList->GetReferenceCount(realWordIndex, &realIndexesCount);
		if (error != eOK)
			return error;

		if (realIndexesCount)
		{
			error = pList->GetRealIndexes(realWordIndex, 0, &realListIndex, &realWordIndex);
			if (error != eOK)
				return error;

			error = m_RealListVector.GetListPtr(realListIndex, &pList);
			if (error != eOK)
				return error;
		}
	}

	/// Считаем, что в ядре активным является CSldCustomList, значит при переключении на базовый список - состояние восстановится
	error = pList->GoToByGlobalIndex(realWordIndex);
	if(error != eOK)
		return error;

	Int32 globalIndex = SLD_DEFAULT_WORD_INDEX;
	error = pList->GetCurrentGlobalIndex(&globalIndex);
	if(error != eOK)
		return error;

	error = pList->GetTranslationIndex(globalIndex, aTranslationIndex, aArticleIndex);
	if(error != eOK)
		return error;

	return eOK;
}

/** *********************************************************************
* Возвращает путь к текущему положению в каталоге
*
* @param[out]	aPath	- указатель на структуру в которую будет помещен путь к 
*						  текущему положению в каталоге.
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::GetCurrentPath(TCatalogPath* aPath)
{
	if (!aPath)
		return eMemoryNullPointer;
	
	aPath->Clear();

	Int32 LocalWordIndex = 0;
	ESldError error;
	
	error = GetCurrentIndex(&LocalWordIndex);
	if (error != eOK)
		return error;
	
	error = aPath->PushList(LocalWordIndex);
	if (error != eOK)
		return error;
	
	return eOK;
}

/** ********************************************************************
* Возвращает путь в каталоге к элементу с глобальным номером(т.е. 
* "прямой" номер слова без учета иерархии)
*
* @param[in]	aIndex	- номер слова без учета иерархии
* @param[out]	aPath	- указатель на структуру, в которую будет прописан путь к 
*						  указанному элементу списка слов
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::GetPathByGlobalIndex(Int32 aIndex, TCatalogPath* aPath)
{	
	if (!aPath)
		return eMemoryNullPointer;
	
	aPath->Clear();

	ESldError error = aPath->PushList(aIndex);
	if (error != eOK)
		return error;
	
	return eOK;
}

/** *********************************************************************
* Переходит по указанному пути
*
* @param[in]	aPath			- указатель на структуру, в которой содержится путь к месту 
*								  в каталоге, куда нужно попасть
* @param[in]	aNavigationType	- тип перехода
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::GoToByPath(const TCatalogPath* aPath, ESldNavigationTypeEnum aNavigationType)
{
	if (!aPath)
		return eMemoryNullPointer;
	
	ESldError error;
	
	// Переходить некуда
	if (!aPath->BaseListCount)
		return eOK;
	
	// В поисковом списке нет иерархии - это ошибка
	if (aPath->BaseListCount > 1)
		return eCommonLastLevel;

	// Последнее (и единственное) число в списке - номер слова
	Int32 LocalWordIndex = (Int32)aPath->BaseList[aPath->BaseListCount-1];

	switch (aNavigationType)
	{
		// Переходим на нужное слово, не важно, обычная это статья или подраздел
		case eGoToWord:
		// В поисковом списке иерархии нет, поэтому поведение аналогично eGoToWord
		case eGoToSubSection:
		{
			error = GetWordByIndex(LocalWordIndex);
			if (error != eOK)
				return error;
				
			break;
		}
		default:
		{
			return eCommonErrorBase;
		}
	}

	return eOK;
}

/** *********************************************************************
* Поднимаемся в каталоге на уровень выше текущего или остаемся на текущем, если выше некуда
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::GoToLevelUp(void)
{
	return eOK;
}

/** *********************************************************************
* Возвращает список названий родительских категорий, разделенных строкой-разделителем, по глобальному номеру слова
* Если у слова нет родительских категорий, возвращает пустую строку
*
* ВНИМАНИЕ! Память для результирующей строки выделяется в этом методе
* и должна быть освобождена в вызывающем методе вызовом функции sldMemFree()
*
* @param[in]	aGlobalWordIndex	- глобальный номер слова
* @param[out]	aText				- указатель, по которому будет записан указатель на строку-результат
* @param[in]	aSeparatorText		- указатель на строку-разделитель, может быть NULL
* @param[in]	aMaxLevelCount		- максимальное количество родительских категорий (уровней иерархии),
*									  названия которых нужно узнать. Значение -1 означает все уровни.
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::GetParentWordNamesByGlobalIndex(Int32 aGlobalWordIndex, UInt16** aText, const UInt16* aSeparatorText, Int32 aMaxLevelCount)
{	
	return eOK;
}

/** *********************************************************************
* Производит поиск слов по шаблону, в пределах текущего уровня вложенности
*
* @param[in]	aText			- шаблон поиска
* @param[in]	aMaximumWords	- максимальное количество слов, которые могут быть найдены
* @param[in]	aList			- список слов, в котором производим поиск
* @param[in]	aRealListIndex	- реальный индекс списка слов, в котором производим поиск
* @param[in]	aRealList		- указатель на реальный список(в том случае, если для поиска использовался SimpleSorted)
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::DoWildCardSearch(TExpressionBox* aExpressionBox, Int32 aMaximumWords, ISldList* aList, Int32 aRealListIndex, ISldList* aRealList)
{
	return eOK;
}

/** *********************************************************************
* Производит поиск похожих слов, в пределах текущего уровня вложенности
*
* @param[in]	aText				- шаблон поиска
* @param[in]	aMaximumWords		- максимальное количество слов, которые могут быть найдены
* @param[in]	aMaximumDifference	- максимальная разница между искомыми словами
* @param[in]	aList				- список слов, в котором производим поиск
* @param[in]	aRealListIndex		- реальный индекс списка слов, в котором производим поиск
* @param[in]	aSearchMode			- тип поиска (см. #EFuzzySearchMode)
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::DoFuzzySearch(const UInt16* aText, Int32 aMaximumWords, Int32 aMaximumDifference, ISldList* aList, Int32 aRealListIndex, EFuzzySearchMode aSearchMode)
{
	return eOK;
}

/** *********************************************************************
* Производит поиск анаграмм, в пределах текущего уровня вложенности
*
* @param[in]	aText			- текст (набор символов), по которым будет производиться поиск слов
* @param[in]	aTextLen		- длина переданного текста (количество символов)
* @param[in]	aList			- список слов, в котором производим поиск
* @param[in]	aRealListIndex	- реальный индекс списка слов, в котором производим поиск
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::DoAnagramSearch(const UInt16* aText, Int32 aTextLen, ISldList* aList, Int32 aRealListIndex)
{
	return eOK;
}

/** *********************************************************************
* Производит поиск слов с учетом возможных опечаток в пределах текущего уровня вложенности
*
* @param[in]	aText			- текст (набор символов), по которым будет производиться поиск слов
* @param[in]	aList			- список слов, в котором производим поиск
* @param[in]	aRealListIndex	- реальный индекс списка слов, в котором производим поиск
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::DoSpellingSearch(const UInt16* aText, ISldList* aList, Int32 aRealListIndex)
{
	return eOK;
}

/** *********************************************************************
* Производит полнотекстовый поиск
*
* @param[in]	aText			- шаблон поиска
* @param[in]	aMaximumWords	- максимальное количество слов, которые могут быть найдены
* @param[in]	aList			- указатель на массив списков слов словаря
* @param[in]	aListCount		- количество списков слов в массиве
* @param[in]	aRealListIndex	- реальный индекс списка слов, в котором производим поиск
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::DoFullTextSearch(const UInt16* aText, Int32 aMaximumWords, ISldList** aList, Int32 aListCount, Int32 aRealListIndex)
{
	return eOK;
}

/** *********************************************************************
* Производит сортировку поискового списка слов по релевантности шаблону поиска
*
* @param[in]	aText		- шаблон поиска
* @param[in]	aMode	- true: сортировать по упрощенной схеме если возможно - более быстро, но менее качественно
*							  false: полная сортировка - более медленно, но более качественно
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::SortListRelevant(const UInt16* aText, const CSldVector<TSldMorphologyWordStruct>& aMorphologyForms, ESldFTSSortingTypeEnum aMode)
{		
	return eOK;
}

/** *********************************************************************
* Устанавливаем HASH для декодирования данного списка слов.
*
* @param[in]	aHASH	- HASH, используемый для декодирования
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::SetHASH(UInt32 aHASH)
{
	return eOK;
}

/** *********************************************************************
* Сохраняет текущее состояние списка слов
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::SaveCurrentState(void)
{
	return eOK;
}

/** *********************************************************************
* Восстанавливает ранее сохраненное состояние списка слов
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::RestoreState(void)
{
	return eOK;
}

/** ********************************************************************
* Получает следующее слово
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::GetNextWord()
{
	if ((m_CurrentIndex + 1) >= m_NumbersOfWords)
		return eCommonWrongIndex;

	m_CurrentIndex++;

	return eOK;
}

/** ********************************************************************
* Возвращает таблицу сравнения которая используется в данном списке слов
*
* @param[out]	aCompare	- указатель на переменную, в которую будет помещен указатель на объект сравнения строк
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::GetCompare(CSldCompare **aCompare)
{
	return eCommonWrongList;
}

/** ********************************************************************
* Возвращает количество записей в таблице быстрого доступа
*
* @param[out]	aNumberOfQAItems	- указатель на переменную,
*									  в которую нужно будет поместить количество записей в таблице быстрого доступа.
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::QAGetNumberOfQAItems(Int32 *aNumberOfQAItems)
{
	return eCommonWrongList;
}

/** ********************************************************************
* Возвращает текст для записи в таблице быстрого доступа.
*
* @param aIndex - номер элемента в таблице быстрого доступа
* @param aQAText - указатель на буфер для текста элемента таблицы быстрого доступа
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::QAGetItemText(Int32 aIndex, const UInt16 **aQAText)
{
	return eCommonWrongList;
}

/***********************************************************************
* Возвращает номер для записи в таблице быстрого доступа
*
* @param[in]	aIndex		- номер элемента в таблице быстрого доступа
* @param[out]	aQAIndex	- указатель на буфер для номера из элемента таблицы быстрого доступа
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::QAGetItemIndex(Int32 aIndex, Int32 *aQAIndex)
{
	return eCommonWrongList;
}

/***********************************************************************
* Производит переход по таблице быстрого доступа по номеру записи
*
* @param[in]	aIndex	- номер элемента в таблице быстрого доступа по которому нужно произвести переход
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::JumpByQAIndex(Int32 aIndex)
{
	return eCommonWrongList;
}

/***********************************************************************
* Определяет глобальные номера границ текущего уровня вложенности (включая все подуровни)
* Получается диапазон глобальных индексов слов вида [aLowGlobalIndex, aHighGlobalIndex) - левая граница включается, правая - нет
*
* @param[out]	aLowGlobalIndex		- указатель на переменную, в которую записывается нижняя (левая) граница
* @param[out]	aHighGlobalIndex	- указатель на переменную, в которую записывается верхняя (правая) граница
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::GetHierarchyLevelBounds(Int32* aLowGlobalIndex, Int32* aHighGlobalIndex)
{
	if (!aLowGlobalIndex || !aHighGlobalIndex)
		return eMemoryNullPointer;
	
	*aLowGlobalIndex = 0;
	*aHighGlobalIndex = m_NumbersOfWords;
	
	return eOK;
}

/** *********************************************************************
* Получает вектор индексов озвучек по введенному тексту
*
* @param[in]	aText - указатель на строку, по которой ищем озвучку
* @param[out]	aSoundIndex - указатель на переменную в которую будет помещен индекс звука
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::GetSoundIndexByText(const UInt16 *aText, CSldVector<Int32> & aSoundIndexes)
{
	aSoundIndexes.clear();
	return eOK;
}


/** *********************************************************************
* Устанавливает нужную локализацию записей по переданному языковому коду
*
* @param[in]	aLanguageCode	- код языка на который нужно локализовать записи в списке;
*								  если такой локализации нет, то локализация не меняется
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::SetLocalization(ESldLanguage aLocalizationLangCode)
{
	return eCommonWrongList;
}

/** *********************************************************************
* Устанавливает нужную локализацию записей по переданному языковому коду и списку настроек
*
* @param[in]	aLanguageCode			- код языка на который нужно локализовать записи в списке;
*										  если такой локализации нет, то локализация не меняется
* @param[in]	aLocalizationDetails	- дополнительные настройки локализации (например, название платформы)
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::SetLocalization(ESldLanguage aLocalizationLangCode, const UInt16* aLocalizationDetails)
{
	return eCommonWrongList;
}

/** *********************************************************************
* Инициализирует вспомогательный сортированный список
*
* @param[in]	aSortedListPtr			- указатель на сортированный список;
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::InitSortedList(ISldList* aSortedListPtr)
{
	return eCommonWrongList;
}

/** *********************************************************************
* Ищет ближайшее слово, которое больше или равно заданному
* Работает в несортированных списках, для сортированных по сорткею списках
* ищет по Show-варианту(начиная с баз версии 112+)
*
* @param[in]	aText		- искомое слово
* @param[out]	aResultFlag	- Флаг результата
*							0 - подмотаться не удалось
*							1 - мы подмотались к заданному слову
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::GetWordBySortedText(const UInt16* aText, UInt32* aResultFlag)
{
	*aResultFlag = 1;
	return GetWordByText(aText);
}

/** *********************************************************************
* Ищет максимально похожее слово с помощью GetWordBySortedText()
*
* @param[in]	aText		- искомое слово
* @param[out]	aResultFlag	- Флаг результата
*							0 - найти слово не удалось
*							1 - мы нашли само слово
*							2 - мы нашли альтернативный заголовок слова (запрос и Show-вариант не совпадают)
* @param[in]	aActionsOnFailFlag	- флаг, определяющий нужно ли подматываться к наиболее подходящем слову,
*							  если поиск завершился неудачно, в иерархических списках ищет только в корне списка
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::GetWordByTextExtended(const UInt16* aText, UInt32* aResultFlag, UInt32 aActionsOnFailFlag)
{
	if (!aResultFlag)
		return eMemoryNullPointer;

	ESldError error = eOK;
	if (m_SearchSourceListIndex != SLD_DEFAULT_LIST_INDEX)
	{
		return GetWordByTextInRealList(aText, aResultFlag, aActionsOnFailFlag);
	}
	else
	{
		Int32 currentIndex = m_CurrentIndex;

		*aResultFlag = 1;
		error = GetWordByText(aText);
		if (error != eOK)
			return error;

		UInt16* currentWord = NULL;
		error = GetCurrentWord(m_SortedVariantIndex, &currentWord);
		if (error != eOK)
			return error;

		if (m_CMP->StrICmp(aText, currentWord))
		{
			*aResultFlag = 0;

			if (!aActionsOnFailFlag)
			{
				error = GetWordByIndex(currentIndex);
				if (error != eOK)
					return error;
			}
		}
	}

	return eOK;
}

/** *********************************************************************
* Синхронизирует состояние вспомогательного сортированного списка с базовым
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::SynchronizeWithASortedList()
{
	return eCommonWrongList;
}

/** *********************************************************************
* Получает слово по его номеру в сортированном списке
*
* @param[in]	aIndex		- запрашиваемый индекс слова
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::GetWordBySortedIndex(Int32 aIndex)
{
	return GetWordByIndex(aIndex);
}

/** *********************************************************************
* Получает следующее сортированное слово
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::GetNextSortedWord()
{
	return GetNextWord();
}

/** *********************************************************************
* Получает следующее реальное сортированное слово
*
* @param[out]	aResult		- указатель, по которому будет возвращен результат
*							  выполнения функции	0 - мы не встретили ни 1 реального слова
*													1 - мы смогли получить следующее слово
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::GetNextRealSortedWord(Int8* aResult)
{
	*aResult = 1;
	return GetNextWord();
}

/** *********************************************************************
* Возвращает текущий индекс сортированного списка
*
* @param[in]	aIndex		- указатель, по которому будет записан
*							  текущий индекс сортированного списка
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::GetCurrentSortedIndex(Int32* aIndex) const
{
	return GetCurrentIndex(aIndex);
}

/** *********************************************************************
* Возвращает количество записей в таблице быстрого доступа для сортированного списка
*
* @param[out]	aNumberOfQAItems - указатель на переменную, в которую нужно будет
*								   поместить количество записей в таблице быстрого доступа.
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::QAGetNumberOfSortedQAItems(Int32 *aNumberOfQAItems)
{
	return eCommonWrongList;
}

/** *********************************************************************
* Возвращает текст для записи в таблице быстрого доступа для сортированного списка
*
* @param[in]	aIndex	- номер элемента в таблице быстрого доступа
* @param[out]	aQAText	- указатель на буфер для текста элемента таблицы быстрого доступа
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::QAGetSortedItemText(Int32 aIndex, const UInt16 **aQAText)
{
	return eCommonWrongList;
}

/** *********************************************************************
* По номеру точки входа в сортированном списке возвращает номер слова, соответствующий этой точке
*
* @param[in]	aIndex		- номер элемента в таблице быстрого доступа
* @param[out]	aQAIndex	- указатель на буфер, куда сохраняется результат
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::QAGetSortedItemIndex(Int32 aIndex, Int32 *aQAIndex)
{
	return eCommonWrongList;
}

/** *********************************************************************
* Производит переход по таблице быстрого доступа по номеру записи в сортированном списке
*
* @param[in]	aIndex	- номер элемента в таблице быстрого доступа по которому нужно произвести переход.
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::JumpByQASortedIndex(Int32 aIndex)
{
	return eCommonWrongList;
}

/** *********************************************************************
* Проверяет наличие вспомогательного сортированного списка
*
* @return 1, если вспомогательный список имеется, иначе 0
************************************************************************/
Int8 CSldCustomList::HasSimpleSortedList()
{
	return false;
}

/** *********************************************************************
* Возвращает флаг, проверяющий на реальном ли слове установлен список, или на альтернативном заголовке
*
* @return SLD_SIMPLE_SORTED_NORMAL_WORD, если вспомогательный список имеется, иначе SLD_SIMPLE_SORTED_VIRTUAL_WORD
************************************************************************/
Int8 CSldCustomList::IsNormalWord()
{
	return SLD_SIMPLE_SORTED_NORMAL_WORD;
}

/** *********************************************************************
* Проверяет наличие иерархии у списка
*
* @return 1, если есть иерархия, иначе 0
************************************************************************/
Int8 CSldCustomList::HasHierarchy()
{
	return 0;
}

/** *********************************************************************
* Переходит к ближайшей закешированной точке (само слово не декодируется,
* после необходимо вызвать GetNexWord())
*
* @param[in]	aQAPointIndex		-	индекс QA-точки, в которой мы находимся
* @param[in]	aNeedGlobalIndex	-	запрашиваемый индекс слова
*
* @return Код ошибки
************************************************************************/
ESldError CSldCustomList::GoToCachedPoint(UInt32 aQAPointIndex, Int32 aNeedGlobalIndex)
{
	return eCommonWrongList;
}

/** *********************************************************************
* Проверяет сортированность текущего уровня иерархии
*
* @param[out]	aIsSorted	- указатель, по которому будет записан результат
*							  0 - уровень не сортирован
*							  1 - уровень сортирван
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::IsCurrentLevelSorted(UInt32* aIsSorted)
{
	*aIsSorted = true;
	return eOK;
}

/** *********************************************************************
* Получает слово по элементу истории
*
* @param[in]	aHistoryElement	- структура, содержащая информацию, необходимую для восстановления слова.
* @param[out]	aResultFlag		- указатель, по которому будет записан результат выполнения функции:
*								  (см. #ESldHistoryResult)
* @param[out]	aGlobalIndex	- указатель, по которому будет записан индекс найденного слова
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::GetWordByHistoryElement(CSldHistoryElement* aHistoryElement, ESldHistoryResult* aResultFlag, Int32* aGlobalIndex)
{
	return eCommonWrongList;
}

/** *********************************************************************
* Возвращает количество слов из вспомогательного сортированного списка
*
* @return количество слов
************************************************************************/
UInt32 CSldCustomList::GetSortedWordsCount()
{
	return 0;
}

/** *********************************************************************
* Устанавливает значение флага синхронизации каталога с текущим индексом
*
* @param[in]	aFlag	- значение флага (0 или 1)
*
* @return eOK
************************************************************************/
ESldError CSldCustomList::SetCatalogSynchronizedFlag(UInt8 aFlag)
{
	return eOK;
}

/** *********************************************************************
* Ищет слово, бинарно совпадающее по переданным вариантам написания
*
* @param[in]	aWordVariantsSet	- набор вариантов написания слова
* @param[out]	aResultFlag			- флаг результата:
*									  1 - совпадающее слово найдено,
*									  0 - совпадающее слово не найдено					
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::GetExactWordBySetOfVariants(TWordVariantsSet* aWordVariantsSet, UInt32* aResultFlag)
{
	*aResultFlag = 0;
	return eOK;
}

/** *********************************************************************
* Ищет наиболее подходящее слово по переданным вариантам написания
*
* @param[in]	aWordVariantsSet	- набор вариантов написания слова
* @param[in]	aShowVariant		- show-вариант из набора
* @param[in]	aWordSearchType		- тип (правила) поиска слова (см. ESldWordSearchTypeEnum)
* @param[out]	aResultFlag			- флаг результата:
*										0 - подмотаться не удалось
*										1 - мы подмотались к заданному слову
*										2 - мы подмотались к ближайшему слову по виртуальному 
*											идентификатору (запрос и Show-вариант не совпадают)	
* @param[in]	aActionsOnFailFlag	- флаг, определяющий нужно ли подматываться к наиболее подходящем слову,
*							  если поиск завершился неудачно, в иерархических списках ищет только в корне списка
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::GetWordBySetOfVariants(TWordVariantsSet* aWordVariantsSet, UInt32 aWordSearchType, UInt32* aResultFlag, UInt32 aActionsOnFailFlag)
{
	*aResultFlag = 0;
	return eOK;
}


/** *********************************************************************
* Возвращает количество результатов полнотекстового поиска
*
* @param[in]	aRequest	- текст, по которому будет производиться поиск
* @param[out]	aCount		- указатель на переменную, в которую будет сохранено количество результатов
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::GetFullTextResultCount(const UInt16* aRequest, Int32* aCount)
{
	return eCommonWrongList;
}

/** *********************************************************************
* Получает индекс варианта написания в базовом списке слов
*
* @param[in]	aVariantIndex	- индекс варианта написания в пользовательском списке
*
* @return индекс варианта написания в базовом списке слов
************************************************************************/
Int32 CSldCustomList::GetRealVariantIndex(Int32 aVariantIndex) const
{
	if (m_NumbersOfWords == 0 || m_WordVector[m_CurrentIndex].RealWordIndex == SLD_DEFAULT_WORD_INDEX)
		return SLD_DEFAULT_VARIANT_INDEX;

	ISldList* pList = m_WordVector[m_CurrentIndex].RealListPtr;
	if (!pList)
		return SLD_DEFAULT_VARIANT_INDEX;

	EListVariantTypeEnum variantType = m_ListInfo->GetVariantType(aVariantIndex);
	if (variantType == eVariantUnknown)
		return SLD_INDEX_STYLIZED_VARIANT_NO;

	const CSldListInfo* listInfo = NULL;
	ESldError error = pList->GetWordListInfo(&listInfo);
	if (error != eOK)
		return SLD_DEFAULT_VARIANT_INDEX;

	return listInfo->GetVariantIndexByType((EListVariantTypeEnum)variantType);
}

/** *********************************************************************
* Возвращает индекс статьи, содержащей стилизованный текст варианта написания для текущего слова
*
* @param[in]	aVariantIndex	- Индекс варианта написания
*
* @return индекс статьи 
************************************************************************/
Int32 CSldCustomList::GetStylizedVariantArticleIndex(UInt32 aVariantIndex) const
{
	const Int32 realVariantIndex = GetRealVariantIndex(aVariantIndex);
	if (realVariantIndex == SLD_DEFAULT_VARIANT_INDEX)
		return SLD_INDEX_STYLIZED_VARIANT_NO;

	ISldList* pList = m_WordVector[m_CurrentIndex].RealListPtr;
	if (!pList)
		return SLD_INDEX_STYLIZED_VARIANT_NO;

	/// Считаем, что в ядре активным является CSldCustomList, значит при переключении на базовый список - состояние восстановится
	if(pList->GetWordByGlobalIndex(m_WordVector[m_CurrentIndex].RealWordIndex) != eOK)
		return SLD_INDEX_STYLIZED_VARIANT_NO;

	return pList->GetStylizedVariantArticleIndex(realVariantIndex);
}

/** *********************************************************************
* Возвращает индекс стиля для заданного варианта написания из eVariantLocalizationPreferences
*
* @param[in]	aVariantIndex	- номер варианта написания, для которого мы хотим получить стиль
*
* @return индекс стиля варианта написания
************************************************************************/
Int32 CSldCustomList::GetStyleVariantIndex(Int32 aVariantIndex)
{
	const Int32 realVariantIndex = GetRealVariantIndex(aVariantIndex);
	if (realVariantIndex == SLD_DEFAULT_VARIANT_INDEX)
		return 0;

	ISldList* pList = m_WordVector[m_CurrentIndex].RealListPtr;
	if (!pList)
		return 0;

	/// Считаем, что в ядре активным является CSldCustomList, значит при переключении на базовый список - состояние восстановится
	if (pList->GetWordByGlobalIndex(m_WordVector[m_CurrentIndex].RealWordIndex) != eOK)
		return 0;

	return pList->GetStyleVariantIndex(realVariantIndex);
}

/** *********************************************************************
* Переходит по пути, соответсвующему заданному глобальному индексу
*
* @param[in]	aGlobalIndex	- Глобальный индекс, к которому мы хотим перейти
*
* @return индекс статьи 
************************************************************************/
ESldError CSldCustomList::GoToByGlobalIndex(const Int32 aGlobalIndex)
{
	return GetWordByGlobalIndex(aGlobalIndex);
}

/** *********************************************************************
* Производит переход по таблице быстрого поиска
*
* @param[in]	aSearchIndex	- номер записи в таблице быстрого поиска
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::GoToBySearchIndex(Int32 aSearchIndex)
{
	return eCommonWrongList;
}

/** *********************************************************************
* Добавляет слово в список на основе уже существующей записи в словаре
*
* @param[in]	aRealListIndex		- индекс списка, из которого мы хотим добавить слово
* @param[in]	aWordIndex			- индекс добавляемого слова
* @param[in]	aUncoverHierarchy	- глубина раскрытия иерархии для слов, имеющих потомков
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::AddWord(const Int32 aRealListIndex, const Int32 aWordIndex, const UInt32 aUncoverHierarchy)
{
	m_NumbersOfWords++;
	ESldError error = CheckAllocateMemory();
	if(error!= eOK)
		return error;

	const Int32 currentWordIndex = m_NumbersOfWords - 1;
	TSldCustomListWordInfo* newWordPtr = &m_WordVector[currentWordIndex];
	newWordPtr->Clear();

	error = m_RealListVector.GetListPtr(aRealListIndex, &newWordPtr->RealListPtr);
	if (error != eOK)
		return error;

	newWordPtr->RealListIndex	= aRealListIndex;
	newWordPtr->RealWordIndex	= aWordIndex;
	error = m_RealListVector.GetListPtr(aRealListIndex, &newWordPtr->RealListPtr);
	if (error != eOK)
		return error;

	// В список добавлен первый элемент, установим его в качестве текущего
	if(m_CurrentIndex == SLD_DEFAULT_WORD_INDEX)
		m_CurrentIndex = 0;

	const CSldListInfo* listInfo;
	error = newWordPtr->RealListPtr->GetWordListInfo(&listInfo);
	if (error != eOK)
		return error;

	if (listInfo->GetUsage() == eWordListType_RegularSearch || listInfo->GetUsage() == eWordListType_CustomList)
	{
		error = newWordPtr->RealListPtr->GetRealListIndex(aWordIndex, &newWordPtr->RealListIndex);
		if (error != eOK)
			return error;

		error = newWordPtr->RealListPtr->GetRealGlobalIndex(aWordIndex, &newWordPtr->RealWordIndex);
		if (error != eOK)
			return error;

		error = m_RealListVector.GetListPtr(newWordPtr->RealListIndex, &newWordPtr->RealListPtr);
		if (error != eOK)
			return error;
	}
	else if(aUncoverHierarchy > 0)
	{
		error = newWordPtr->RealListPtr->LocalIndex2GlobalIndex(aWordIndex, &newWordPtr->RealWordIndex);
		if(error!= eOK)
			return error;

		UInt32 hasHierarchy = 0;
		error = newWordPtr->RealListPtr->isWordHasHierarchy(aWordIndex, &hasHierarchy, NULL);
		if(error!= eOK)
			return error;

		if(hasHierarchy)
		{
			error = newWordPtr->RealListPtr->SetBase(aWordIndex);
			if(error!= eOK)
				return error;

			Int32 wordsCount = 0;
			error = newWordPtr->RealListPtr->GetNumberOfWords(&wordsCount);
			if(error!= eOK)
				return error;

			for(Int32 index = 0; index < wordsCount; index++)
			{
				error = AddWord(aRealListIndex, index, aUncoverHierarchy);
				if(error!= eOK)
					return error;
			}

			/// newWordPtr мог быть перемещен
			newWordPtr = &m_WordVector[currentWordIndex];

			error = newWordPtr->RealListPtr->GoToLevelUp();
			if(error!= eOK)
				return error;

			newWordPtr->UncoverFlag = true;
		}
	}

	if (m_SortedWordIndexes)
	{
		sldMemFree(m_SortedWordIndexes);
		m_SortedWordIndexes = NULL;
	}

	return eOK;
}

/** *********************************************************************
* Добавляет слово в список 
*
* @param[in]	aRealListIndex		- индекс списка, из которого мы хотим добавить слово
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::AddWord(const Int32 aRealListIndex)
{
	m_NumbersOfWords++;
	ESldError error = CheckAllocateMemory();
	if (error != eOK)
		return error;

	TSldCustomListWordInfo* newWordPtr = &m_WordVector[m_NumbersOfWords - 1];
	newWordPtr->Clear();

	if (m_SortedWordIndexes)
	{
		sldMemFree(m_SortedWordIndexes);
		m_SortedWordIndexes = NULL;
	}

	newWordPtr->RealListIndex = aRealListIndex;
	error = m_RealListVector.GetListPtr(newWordPtr->RealListIndex, &newWordPtr->RealListPtr);
	if (error != eOK)
		return error;

	return eOK;
}

/** *********************************************************************
* Привязывает слово к записи из списка
*
* @param[in]	aWordIndex			- индекс записи в пользовательском списке, к которой мы хотим привязать слово
* @param[in]	aSubWordIndex		- индекс записи в словарном списке
* @param[in]	aSubWordListIndex	- индекс списка слов, при значении SLD_DEFAULT_LIST_INDEX считаем,
*									  что список слов соответствует списку, в котором находится слово-родитель
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::AddSubWord(const Int32 aWordIndex, const Int32 aSubWordIndex, const Int32 aSubWordListIndex)
{
	if (aWordIndex < 0 || aWordIndex >= m_NumbersOfWords)
		return eCommonWrongIndex;

	auto realListPtr = m_WordVector[aWordIndex].RealListPtr;
	if (aSubWordListIndex > 0 && aSubWordListIndex < m_RealListVector.NumberOfLists)
		realListPtr = m_RealListVector.WordLists[aSubWordListIndex];

	if (!realListPtr)
		return eMemoryNullPointer;

	Int32 realListWordCount = 0;
	ESldError error = realListPtr->GetTotalWordCount(&realListWordCount);
	if (error != eOK)
		return error;

	if (aSubWordIndex < 0 || aSubWordIndex >= realListWordCount)
		return eCommonWrongIndex;

	m_WordVector[aWordIndex].SubIndexes.emplace_back(aSubWordListIndex, aSubWordIndex);

	return eOK;
}

/** *********************************************************************
* Добавляет диапазон слов в список на основе диапазона слов в заданном списке в словаре
*
* @param[in]	aRealListIndex			- индекс списка, из которого мы хотим добавить слово
* @param[in]	aBeginRealWordIndex		- индекс первого добавляемого слова
* @param[in]	aEndRealWordIndex		- индекс последнего добавляемого слова
* @param[in]	aInsertAfterIndex		- индекс, после которого нужно добавить слово
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::InsertWords(const Int32 aRealListIndex, const Int32 aBeginRealWordIndex, const Int32 aEndRealWordIndex, Int32 aInsertAfterIndex)
{
	if(aBeginRealWordIndex > aEndRealWordIndex)
		return eOK;

	if (aInsertAfterIndex >= m_NumbersOfWords)
		aInsertAfterIndex = SLD_DEFAULT_WORD_INDEX;

	Int32 insertWordsCount = aEndRealWordIndex - aBeginRealWordIndex;
	Int32 oldNumberOfWords = m_NumbersOfWords;
	m_NumbersOfWords += insertWordsCount;
	ESldError error = CheckAllocateMemory();
	if(error!= eOK)
		return error;
	
	TSldCustomListWordInfo* newWordPtr = NULL;
	if(aInsertAfterIndex == SLD_DEFAULT_WORD_INDEX)
	{
		newWordPtr = &m_WordVector[oldNumberOfWords];
	}
	else
	{
		sldMemMove(&m_WordVector[aInsertAfterIndex + insertWordsCount + 1], &m_WordVector[aInsertAfterIndex + 1], (oldNumberOfWords - aInsertAfterIndex - 1) * sizeof(TSldCustomListWordInfo));
		newWordPtr = &m_WordVector[aInsertAfterIndex + 1];
	}

	for(Int32 wordIndex = aBeginRealWordIndex; wordIndex < aEndRealWordIndex; wordIndex++)
	{
		newWordPtr->Clear();
		newWordPtr->RealListIndex	= aRealListIndex;
		newWordPtr->RealWordIndex	= wordIndex;
		error = m_RealListVector.GetListPtr(newWordPtr->RealListIndex, &newWordPtr->RealListPtr);
		if (error != eOK)
			return error;

		newWordPtr++;
	}

	// В список добавлен первый элемент, установим его в качестве текущего
	if (m_CurrentIndex == SLD_DEFAULT_WORD_INDEX)
		m_CurrentIndex = 0;

	if (m_SortedWordIndexes)
	{
		sldMemFree(m_SortedWordIndexes);
		m_SortedWordIndexes = NULL;
	}

	return eOK;
}

/** *********************************************************************
* Выделяет память, если это необходимо
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::CheckAllocateMemory()
{
	if(m_NumbersOfWords && m_NumbersOfWords >= m_MaxNumbersOfWords)
	{
		UInt32 newNumberOfWords = m_MaxNumbersOfWords ? 3 * m_NumbersOfWords / 2 : SLD_DEFAULT_WORDS_COUNT_IN_CUSTOM_LIST;

		TSldCustomListWordInfo* tmpWordVector = m_WordVector;

		m_WordVector = NULL;
		m_WordVector = (TSldCustomListWordInfo*)sldMemNew(newNumberOfWords * sizeof(TSldCustomListWordInfo));
		if(!m_WordVector)
			return eMemoryNotEnoughMemory;

		if(tmpWordVector)
		{
			sldMemMove(m_WordVector, tmpWordVector, m_MaxNumbersOfWords * sizeof(TSldCustomListWordInfo));
			sldMemZero(m_WordVector + m_MaxNumbersOfWords, (newNumberOfWords - m_MaxNumbersOfWords) * sizeof(TSldCustomListWordInfo));
			sldMemFree(tmpWordVector);
		}
		else
		{
			sldMemZero(m_WordVector, newNumberOfWords * sizeof(TSldCustomListWordInfo));
		}
		m_MaxNumbersOfWords = newNumberOfWords;
	}

	return eOK;
}

/** *********************************************************************
* Устанавливает вариант написания для уже добавленного слова
*
* @param[in]	aWordIndex			- индекс слова, для которого мы хотим задать вариант написания
* @param[in]	aText				- указатель на текст варианта написания
* @param[in]	aVariantIndex		- индекс варианта написания
* @param[in]	aResetSearchVector	- флаг, определяющий, нужно ли пересортировать массив, использующийся
*									  для поиска по тексту. Если мы добавляем тот же вариант, что и в базовом списке
*									  (для ускорения работы, например), то передается 0
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::SetCurrentWord(const Int32 aWordIndex, const UInt16* aText, const Int32 aVariantIndex, const UInt32 aResetSearchVector)
{
	if(aWordIndex >= m_NumbersOfWords || aWordIndex < 0)
		return eCommonWrongIndex;

	return SetCurrentWord(&m_WordVector[aWordIndex], aText, aVariantIndex, aResetSearchVector);
}

/** *********************************************************************
* Устанавливает вариант написания для уже добавленного слова
*
* @param[in]	aWordInfoPtr		- указатель на запись в пользовательском списке
* @param[in]	aText				- указатель на текст варианта написания
* @param[in]	aVariantIndex		- индекс варианта написания
* @param[in]	aResetSearchVector	- флаг, определяющий, нужно ли пересортировать массив, использующийся
*									  для поиска по тексту. Если мы добавляем тот же вариант, что и в базовом списке
*									  (для ускорения работы, например), то передается 0
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::SetCurrentWord(TSldCustomListWordInfo* aWordInfoPtr, const UInt16* aText, const Int32 aVariantIndex, const UInt32 aResetSearchVector)
{
	if (!aText)
		return eMemoryNullPointer;

	const Int32 numberOfVariants = m_ListInfo->GetNumberOfVariants();

	if (aVariantIndex >= numberOfVariants)
		return eCommonWrongVariantIndex;

	UInt32 wordLen = (CSldCompare::StrLen(aText) + 1);

	if (!aWordInfoPtr->CustomWords)
	{
		aWordInfoPtr->NumberOfVariants = numberOfVariants;
		aWordInfoPtr->CustomWords = (UInt16**)sldMemNewZero(aWordInfoPtr->NumberOfVariants * sizeof(UInt16*));
		if (!aWordInfoPtr->CustomWords)
			return eMemoryNotEnoughMemory;
	}
	else if (numberOfVariants > aWordInfoPtr->NumberOfVariants)
	{
		UInt16** tmpWords = aWordInfoPtr->CustomWords;
		aWordInfoPtr->CustomWords = NULL;

		aWordInfoPtr->NumberOfVariants = numberOfVariants;
		aWordInfoPtr->CustomWords = (UInt16**)sldMemNewZero(numberOfVariants * sizeof(UInt16*));
		if (!aWordInfoPtr->CustomWords)
			return eMemoryNotEnoughMemory;

		sld2::memmove_n(aWordInfoPtr->CustomWords, tmpWords, aWordInfoPtr->NumberOfVariants);
		aWordInfoPtr->NumberOfVariants = numberOfVariants;

		sldMemFree(tmpWords);
	}

	UInt16* currentWord = aWordInfoPtr->CustomWords[aVariantIndex];

	UInt32 oldWordLen = 0;
	if (currentWord)
	{
		oldWordLen = (CSldCompare::StrLen(currentWord) + 1);
	}

	if (oldWordLen < wordLen)
	{
		if (currentWord)
		{
			sldMemFree(currentWord);
			currentWord = NULL;
		}
		currentWord = (UInt16*)sldMemNew(wordLen * sizeof(UInt16));
		if (!currentWord)
			return eMemoryNotEnoughMemory;
	}

	CSldCompare::StrCopy(currentWord, aText);
	aWordInfoPtr->CustomWords[aVariantIndex] = currentWord;

	if (aResetSearchVector && aVariantIndex == m_SortedVariantIndex && m_SortedWordIndexes)
	{
		sldMemFree(m_SortedWordIndexes);
		m_SortedWordIndexes = NULL;
	}

	return eOK;
}

/** *********************************************************************
* Удаляет записи из списка
*
* @param[in]	aBeginIndex	- индекс первого из удаляемых слов
* @param[in]	aEndIndex	- следующий индекс после последнего удаляемого слова [...)
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::EraseWords(const Int32 aBeginIndex, const Int32 aEndIndex)
{
	if(aEndIndex > m_NumbersOfWords || aBeginIndex < 0 || aBeginIndex > aEndIndex)
		return eCommonWrongIndex;

	for(Int32 wordIndex = aBeginIndex; wordIndex < aEndIndex; wordIndex++)
	{
		m_WordVector[wordIndex].Close();
	}

	Int32 newNumberOfWords = m_NumbersOfWords - aEndIndex + aBeginIndex;
	if(aEndIndex != m_NumbersOfWords)
	{
		sldMemMove(&m_WordVector[aBeginIndex], &m_WordVector[aEndIndex], (m_NumbersOfWords - aEndIndex) * sizeof(m_WordVector[0]));
	}
	sldMemZero(&m_WordVector[newNumberOfWords], (m_NumbersOfWords - newNumberOfWords) * sizeof(m_WordVector[0]));

	m_NumbersOfWords = newNumberOfWords;

	if (m_SortedWordIndexes)
	{
		sldMemFree(m_SortedWordIndexes);
		m_SortedWordIndexes = NULL;
	}

	return eOK;
}

/** *********************************************************************
* Индексирует все слова, содержащиеся в списке в соответсвии с заданным вариантом написания
*
* @param[in]	aVariantIndex		- номер индексируемого варианта написания
* @param[in]	aCreateWordsBuffer	- в процессе индексации все слова будут буфферизированы,
*									  данный флаг определяет, нужно ли оставить этот буфер для ускорения поиска
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::SortListByVariant(const Int32 aVariantIndex, const Int8 aCreateWordsBuffer)
{
	if (aVariantIndex >= (Int32)m_ListInfo->GetNumberOfVariants())
		return eCommonWrongVariantIndex;

	Int8 sortByIndex = 0;
	ESldError error = eOK;
	if (m_SearchSourceListIndex != SLD_DEFAULT_LIST_INDEX)
	{
		sortByIndex = 1;
		if (m_NumbersOfWords)
		{
			const EListVariantTypeEnum variantType = m_ListInfo->GetVariantType(aVariantIndex);
			if (variantType == eVariantUnknown)
				return eCommonWrongVariantIndex;

			const CSldListInfo* listInfo = NULL;
			error = m_WordVector[0].RealListPtr->GetWordListInfo(&listInfo);
			if (error != eOK)
				return error;

			const Int32 realVariantIndex = listInfo->GetVariantIndexByType(variantType);
			if (realVariantIndex == SLD_DEFAULT_VARIANT_INDEX)
				return eCommonWrongVariantIndex;

			m_SortedVariantIndex = aVariantIndex;
			error = m_WordVector[0].RealListPtr->SortListByVariant(realVariantIndex, aCreateWordsBuffer);
			if (error != eOK)
				return error;
		}
	}

	if (m_SortedWordIndexes)
	{
		sldMemFree(m_SortedWordIndexes);
		m_SortedWordIndexes = NULL;
	}

	m_SortedWordIndexes = (Int32*)sldMemNew(m_NumbersOfWords * 2 * sizeof(*m_SortedWordIndexes));
	if (!m_SortedWordIndexes)
		return eMemoryNotEnoughMemory;

	m_SortedVariantIndex = aVariantIndex;

	Int32 firstIndex = 0;
	Int32 secondIndex = m_NumbersOfWords;
	Int32 medIndex = m_NumbersOfWords >> 1;
	UInt16* tmpWord = NULL;

	if (!sortByIndex)
	{
		m_TmpWordSize = m_NumbersOfWords;
		m_TmpWordBuff = (UInt16**)sldMemNewZero(m_TmpWordSize * sizeof(UInt16*));
		if (!m_TmpWordBuff)
			return eMemoryNotEnoughMemory;

		error = GetWordByGlobalIndex(medIndex);
		if (error != eOK)
			return error;

		error = GetCurrentWord(m_SortedVariantIndex, &tmpWord);
		if (error != eOK)
			return error;

		m_TmpWordBuff[medIndex] = (UInt16*)sldMemNew((CSldCompare::StrLen(tmpWord) + 1) * sizeof(UInt16));
		if (!m_TmpWordBuff[medIndex])
			return eMemoryNotEnoughMemory;

		CSldCompare::StrCopy(m_TmpWordBuff[medIndex], tmpWord);
	}

	Int32 compareResult = 0;
	for (Int32 wordIndex = 0; wordIndex < m_NumbersOfWords; wordIndex++)
	{
		if (wordIndex == medIndex)
		{
			m_SortedWordIndexes[firstIndex] = wordIndex;
			firstIndex++;
			continue;
		}

		if (sortByIndex)
		{
			compareResult = m_WordVector[wordIndex].RealWordIndex - m_WordVector[medIndex].RealWordIndex;
		}
		else
		{
			error = GetWordByGlobalIndex(wordIndex);
			if (error != eOK)
				return error;

			error = GetCurrentWord(m_SortedVariantIndex, &tmpWord);
			if (error != eOK)
				return error;

			m_TmpWordBuff[wordIndex] = (UInt16*)sldMemNew((CSldCompare::StrLen(tmpWord) + 1) * sizeof(UInt16));
			if (!m_TmpWordBuff[wordIndex])
				return eMemoryNotEnoughMemory;

			CSldCompare::StrCopy(m_TmpWordBuff[wordIndex], tmpWord);

			error = CompareWords(m_TmpWordBuff[wordIndex], m_TmpWordBuff[medIndex], &compareResult);
			if (error != eOK)
				return error;
		}

		if (compareResult <= 0)
		{
			m_SortedWordIndexes[firstIndex] = wordIndex;
			firstIndex++;
		}
		else
		{
			m_SortedWordIndexes[secondIndex] = wordIndex;
			secondIndex++;
		}	
	}

	sldMemMove(&m_SortedWordIndexes[firstIndex], &m_SortedWordIndexes[m_NumbersOfWords], (secondIndex - m_NumbersOfWords) * sizeof(*m_SortedWordIndexes));

	if (firstIndex - 1 > 0)
	{
		error = QuickSortSearchVector(0, firstIndex - 1, sortByIndex);
		if (error != eOK)
			return error;
	}

	if (firstIndex < m_NumbersOfWords - 1)
	{
		error = QuickSortSearchVector(firstIndex, m_NumbersOfWords - 1, sortByIndex);
		if (error != eOK)
			return error;
	}

	if (m_TmpWordBuff && !aCreateWordsBuffer)
	{
		for (UInt32 wordIndex = 0; wordIndex < m_TmpWordSize; wordIndex++)
		{
			if (m_TmpWordBuff[wordIndex])
			{
				sldMemFree(m_TmpWordBuff[wordIndex]);
			}
		}
		sldMemFree(m_TmpWordBuff);
		m_TmpWordBuff = NULL;
		m_TmpWordSize = 0;
	}

	return eOK;
}

/** *********************************************************************
* Сортирует записи в поисковом индексе
*
* @param[in]	aFirstIndex		- первый индекс сортируемого диапазона
* @param[in]	aLastIndex		- последний индекс сортируемого диапазона
* @param[in]	aSortByIndex	- флаг того что нужно отсортировать
*								0 - записи сортируются по тексту
*								1 - записи сортируются по реальным индексам из базового списка
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::QuickSortSearchVector(const Int32 aFirstIndex, const Int32 aLastIndex, const Int8 aSortByIndex)
{
	ESldError error = eOK;
	Int32 medIndex = (aFirstIndex + aLastIndex) >> 1;

	UInt16* medWord = NULL;
	Int32 medWordIndex = SLD_DEFAULT_WORD_INDEX;
	if (aSortByIndex)
	{
		medWordIndex = m_WordVector[m_SortedWordIndexes[medIndex]].RealWordIndex;
	}
	else
	{
		medWord = m_TmpWordBuff[m_SortedWordIndexes[medIndex]];
	}

	Int32 firstIndex = aFirstIndex;
	Int32 secondIndex = m_NumbersOfWords;
	Int32 medCount = 0;

	Int32 compareResult = 0;
	for (Int32 currentIndex = aFirstIndex; currentIndex <= aLastIndex; currentIndex++)
	{
		if (currentIndex == medIndex)
		{
			compareResult = 0;
		}
		else if (aSortByIndex)
		{
			compareResult = m_WordVector[m_SortedWordIndexes[currentIndex]].RealWordIndex - medWordIndex;
		}
		else
		{
			error = CompareWords(m_TmpWordBuff[m_SortedWordIndexes[currentIndex]], medWord, &compareResult);
			if (error != eOK)
				return error;
		}

		if (compareResult < 0)
		{
			m_SortedWordIndexes[firstIndex] = m_SortedWordIndexes[currentIndex];
			firstIndex++;
		}
		else if (compareResult > 0)
		{
			m_SortedWordIndexes[secondIndex] = m_SortedWordIndexes[currentIndex];
			secondIndex++;
		}
		else
		{
			medCount++;
			m_SortedWordIndexes[2 * m_NumbersOfWords - medCount] = m_SortedWordIndexes[currentIndex];
		}
	}

	if (medCount > 1)
	{
		for (Int32 i = 0; i < medCount; i++)
		{
			m_SortedWordIndexes[firstIndex + i] = m_SortedWordIndexes[2 * m_NumbersOfWords - 1 - i];
		}
	}
	else
	{
		sldMemMove(&m_SortedWordIndexes[firstIndex], &m_SortedWordIndexes[2 * m_NumbersOfWords - medCount], medCount * sizeof(*m_SortedWordIndexes));
	}

	sldMemMove(&m_SortedWordIndexes[firstIndex + medCount], &m_SortedWordIndexes[m_NumbersOfWords], (secondIndex - m_NumbersOfWords) * sizeof(*m_SortedWordIndexes));

	if (firstIndex - 1 > aFirstIndex)
	{
		error = QuickSortSearchVector(aFirstIndex, firstIndex - 1, aSortByIndex);
		if (error != eOK)
			return error;
	}

	if (secondIndex - medCount > m_NumbersOfWords)
	{
		error = QuickSortSearchVector(firstIndex + medCount, aLastIndex, aSortByIndex);
		if (error != eOK)
			return error;
	}

	return eOK;
}

/** *********************************************************************
* Сравнивает два слова
*
* @param[in]	aFirstWord		- указатель на первое слово
* @param[in]	aSecondWord		- указатель на второе слово
* @param[in]	aResult			- указатель, по которому будет записан результат
*								  0 - строки равны, 
*								  1 первая строка больше второй, 
*								 -1 первая строка меньше второй
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::CompareWords(const UInt16* aFirstWord, const UInt16* aSecondWord, Int32* aResult)
{
	if (!aResult || !aFirstWord || !aSecondWord)
		return eMemoryNullPointer;

	// В списке могут быть слиты слова из разных языковых направлений
	*aResult = m_CMP->StrICmp(aFirstWord, aSecondWord, 0);
	if (*aResult)
		return eOK;

	*aResult = m_CMP->StrCmp(aFirstWord, aSecondWord);
	return eOK;
}

/** *********************************************************************
* Ищет максимально похожее слово с помощью GetWordBySortedText() в базовом списке слов
*
* @param[in]	aText			- искомое слово
* @param[out]	aResultFlag		- Флаг результата
*								0 - найти слово не удалось
*								1 - мы нашли само слово
*								2 - мы нашли альтернативный заголовок слова (запрос и Show-вариант не совпадают)
* @param[in]	aActionsOnFailFlag	- флаг, определяющий нужно ли подматываться к наиболее подходящем слову, если поиск завершился неудачно
*								0 - в случае неудачи вернуться на исходную позицию
*								1 - в случае неудачи подмотаться к максимально похожему слову в корне списка
*								2 - в случае неудачи подмотаться к слову, которое больше или равно заданному
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::GetWordByTextInRealList(const UInt16* aText, UInt32* aResultFlag, UInt32 aActionsOnFailFlag)
{
	if (!aText || !aResultFlag)
		return eMemoryNullPointer;

	if (!m_NumbersOfWords)
		return eOK;

	if (m_WordVector[0].RealListIndex != m_SearchSourceListIndex)
		return eOK;
	
	ESldError error = eOK;

	if (!m_SortedWordIndexes)
	{
		error = SortListByVariant(m_SortedVariantIndex);
		if (error != eOK)
			return error;
	}

	if (aActionsOnFailFlag == 2)
	{
		error = m_WordVector[0].RealListPtr->GetWordBySortedText(aText, aResultFlag);
	}
	else
	{
		error = m_WordVector[0].RealListPtr->GetWordByTextExtended(aText, aResultFlag, aActionsOnFailFlag);
	}
	if (error != eOK)
		return error;

	if (!*aResultFlag && !aActionsOnFailFlag)
		return eOK;

	TCatalogPath path;
	error = m_WordVector[0].RealListPtr->GetCurrentPath(&path);
	if (error != eOK)
		return error;

	struct pred {
		Int32 pathIndex;
		decltype(m_WordVector) words;
		bool operator()(Int32 index) const { return words[index].RealWordIndex < pathIndex; }
	};
	UInt32 index = sld2::lower_bound(m_SortedWordIndexes, m_NumbersOfWords,
									 pred{ (Int32)path.BaseList[0], m_WordVector });
	if (index == m_NumbersOfWords)
	{
		error = GetWordByIndex(m_NumbersOfWords - 1);
		*aResultFlag = 0;
		return error;
	}

	Int32 currentIndex = m_SortedWordIndexes[index];
	if (m_WordVector[currentIndex].RealWordIndex != path.BaseList[0])
	{
		error = GetWordByIndex(currentIndex);
		*aResultFlag = 0;
		return error;
	}

	ESubwordsState wordState = eSubwordsStateHasNotSubwords;
	for (UInt32 depth = 0; depth < path.BaseListCount; depth++)
	{
		if (depth)
		{
			currentIndex++;
			currentIndex += path.BaseList[depth];
		}

		error = CheckSubwordsState(currentIndex, &wordState);
		if (error != eOK)
			return error;

		m_CurrentIndex = currentIndex;
		if (wordState != eSubwordsStateUncovered)
		{
			break;
		}
	}

	return eOK;
}

/** *********************************************************************
* Возвращает количество связанных со словом записей
*
* @param[in]	aWordIndex		- индекс слова
* @param[out]	aSubWordsCount	- указатель, по которому будет записан результат
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::GetSubWordsCount(const Int32 aWordIndex, UInt32* aSubWordsCount)
{
	if (!aSubWordsCount)
		return eMemoryNullPointer;

	if (aWordIndex < 0 || aWordIndex >= m_NumbersOfWords)
		return eCommonWrongIndex;

	*aSubWordsCount = 0;

	if (m_WordVector[aWordIndex].SubIndexes.size())
		*aSubWordsCount = m_WordVector[aWordIndex].SubIndexes.size();
	else
		return isWordHasHierarchy(aWordIndex, aSubWordsCount, NULL);

	return eOK;
}

/** *********************************************************************
* Возвращает массив связанных со словом записей.
* Каждый вызов данной функции переключает состояние ESubwordsState
*
* @param[in]	aWordIndex		- индекс слова
* @param[out]	aSubWordsIndexes- указатель, по которому будет записан указатель на массив связанных со словом индексов
* @param[out]	aUncoverFlag	- указатель, по которому будет записано текущее состояние связанных с записью слов
*								  получение этих индексов предполагает их последующее использование, поэтому состояние списка меняется
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::GetSubWords(const Int32 aWordIndex, const CSldVector<TSldWordIndexes> ** aSubWordsIndexes, ESubwordsState* aUncoverFlag)
{
	if (!aSubWordsIndexes)
		return eMemoryNullPointer;

	if (aWordIndex < 0 || aWordIndex >= m_NumbersOfWords)
		return eCommonWrongIndex;

	*aSubWordsIndexes = NULL;
	*aUncoverFlag = eSubwordsStateCollapsed;

	if (m_WordVector[aWordIndex].SubIndexes.empty())
		return eOK;

	*aSubWordsIndexes = &m_WordVector[aWordIndex].SubIndexes;
	if (m_WordVector[aWordIndex].UncoverFlag)
		*aUncoverFlag = eSubwordsStateUncovered;

	m_WordVector[aWordIndex].UncoverFlag = m_WordVector[aWordIndex].UncoverFlag ? false : true;

	return eOK;
}

/** *********************************************************************
* Проверяет состояние слова и связанных с ним слов
*
* @param[in]	aWordIndex	- индекс слова
* @param[out]	aWordState	- указатель, по которому будет записан результат
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::CheckSubwordsState(const Int32 aWordIndex, ESubwordsState* aWordState)
{
	if (aWordIndex < 0 || aWordIndex >= m_NumbersOfWords)
		return eCommonWrongIndex;

	const auto & word = m_WordVector[aWordIndex];
	if (word.UncoverFlag)
	{
		*aWordState = eSubwordsStateUncovered;
		return eOK;
	}

	UInt32 hasHierarchy = 0;
	ESldError error = isWordHasHierarchy(aWordIndex, &hasHierarchy, NULL);
	if (error != eOK)
		return error;

	if(!word.SubIndexes.empty() || hasHierarchy)
		*aWordState = eSubwordsStateCollapsed;
	else
		*aWordState = eSubwordsStateHasNotSubwords;

	return eOK;
}

/** *********************************************************************
* Меняет состояние слова и связанных с ним слов
*
* @param[in]	aWordIndex	- индекс слова
* @param[out]	aWordState	- указатель, по которому будет записан результат
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::ChengeSubwordsState(const Int32 aWordIndex, ESubwordsState* aWordState)
{
	if (aWordIndex < 0 || aWordIndex >= m_NumbersOfWords)
		return eCommonWrongIndex;

	auto & word = m_WordVector[aWordIndex];
	UInt32 hasHierarchy = 0;
	ESldError error = isWordHasHierarchy(aWordIndex, &hasHierarchy, NULL);
	if (error != eOK)
		return error;

	if(!word.SubIndexes.empty() || hasHierarchy)
	{
		word.UncoverFlag = !word.UncoverFlag;
		if (word.UncoverFlag)
		{
			*aWordState = eSubwordsStateUncovered;
		}
		else
		{
			*aWordState = eSubwordsStateCollapsed;
		}
	}
	else
	{
		*aWordState = eSubwordsStateHasNotSubwords;
	}

	return eOK;
}

/** *********************************************************************
* Возвращает указатель на eVariantLabel последнего найденного слова
*
* @param[out]	aLabel			- указатель на буфер для указателя на слово
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::GetCurrentWordLabel(UInt16** aLabel)
{
	return eOK;
}

ESldError CSldCustomList::IsListHasScene(UInt32* aIsScene)
{
	return eOK;
}

ESldError CSldCustomList::GetSceneIndex(Int32* aSceneIndex)
{
	return eOK;
}

/** *********************************************************************
* Объединяет все записи для вспомогательных листов, относящиеся к одному заголовку
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::GroupWordsByHeadwords()
{
	if (!m_NumbersOfWords)
		return eOK;

	const CSldListInfo* listInfo;
	ESldError error = m_WordVector[0].RealListPtr->GetWordListInfo(&listInfo);
	if (error != eOK)
		return error;

	if (listInfo->GetUsage() != eWordListType_FullTextAuxiliary)
		return eOK;

	struct TRealWordIndexes
	{
		Int32 realIndex;
		Int32 listIndex;
		Int32 currentIndex;
	};

	Int32 realIndexesCount = 0;
	sld2::DynArray<TRealWordIndexes> realIndexes(m_NumbersOfWords);
	if (realIndexes.empty())
		return eMemoryNotEnoughMemory;

	while (realIndexesCount < m_NumbersOfWords)
	{
		auto &currentWord = m_WordVector[realIndexesCount];

		Int32 reallyRealWordIndex = SLD_DEFAULT_WORD_INDEX;
		Int32 reallyRealListIndex = SLD_DEFAULT_LIST_INDEX;
		error = currentWord.RealListPtr->GetRealIndexes(currentWord.RealWordIndex, 0, &reallyRealListIndex, &reallyRealWordIndex);
		if (error != eOK)
			return error;

		struct pred {
			Int32 realWordIndex;
			bool operator()(const TRealWordIndexes &idx) const { return idx.realIndex < realWordIndex; }
		};

		const UInt32 index = sld2::lower_bound(realIndexes.data(), realIndexesCount, pred{ reallyRealWordIndex });
		if (index != realIndexesCount && reallyRealWordIndex == realIndexes[index].realIndex && reallyRealListIndex == realIndexes[index].listIndex)
		{
			auto &realWord = m_WordVector[realIndexes[index].currentIndex];
			realWord.SubIndexes.emplace_back(SLD_DEFAULT_LIST_INDEX, currentWord.RealWordIndex);

			error = EraseWords(realIndexesCount, realIndexesCount + 1);
			if (error != eOK)
				return error;
		}
		else
		{
			sld2::memmove_n(&realIndexes[index + 1], &realIndexes[index], realIndexesCount - index);
			realIndexes[index].currentIndex = realIndexesCount;
			realIndexes[index].listIndex = reallyRealListIndex;
			realIndexes[index].realIndex = reallyRealWordIndex;
			realIndexesCount++;
		}
	}

    return eOK;
}

/** *********************************************************************
* Возвращает количество раскрытых слов в иерархии в данной папке
*
* @param[in]	aWordIndex              - индекс слова
* @param[out]	aUncoveredWordsCount	- указатель, по которому будет записан результат
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::GetUncoveredWordsCount(const Int32 aWordIndex, UInt32* aUncoveredWordsCount)
{
    if (!aUncoveredWordsCount)
        return eMemoryNullPointer;

    if (aWordIndex < 0 || aWordIndex >= m_NumbersOfWords)
        return eCommonWrongIndex;

    *aUncoveredWordsCount = 0;

    if (!m_WordVector[aWordIndex].UncoverFlag)
        return eOK;

    UInt32 offset = aWordIndex + 1;
    for (Int32 i = 0; i < m_WordVector[aWordIndex].SubIndexes.size(); ++i)
    {
        UInt32 uncoveredSubwordsCount = 0;
        ESldError error = GetUncoveredWordsCount(offset, &uncoveredSubwordsCount);
        if (error != eOK)
            return error;

        offset += (1 + uncoveredSubwordsCount);
    }

    *aUncoveredWordsCount = offset - aWordIndex - 1;

    return eOK;
}

/** *********************************************************************
* Получает реальные индексы списка и записи в реальном списке в по глобальному номеру слова в указанном списке слов
*
* @param[in]	aWordIndex			- глобальный номер слова
* @param[in]	aTrnslationIndex	- номер индексов которые мы хотим получить (можно получить через GetNumberOfTranslations() для списков типа IsFullTextSearchList())
* @param[out]	aRealListIndex		- указатель на переменную, в которую сохраняется индекс списка
* @param[out]	aGlobalWordIndex	- указатель на переменную, в которую сохраняется глобальный номер слова
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::GetRealIndexes(Int32 aWordIndex, Int32 aTrnslationIndex, Int32* aRealListIndex, Int32* aGlobalWordIndex)
{
	if (!aRealListIndex || !aGlobalWordIndex)
		return eMemoryNullPointer;

	if (aWordIndex < 0 || aWordIndex > m_NumbersOfWords || aTrnslationIndex != 0)
		return eCommonWrongIndex;

	*aRealListIndex = m_WordVector[aWordIndex].RealListIndex;
	*aGlobalWordIndex = m_WordVector[aWordIndex].RealWordIndex;

	return eOK;
}

/** *********************************************************************
* Заполняет массив словарных списков слов
*
* @param[in]	aWordLists		- массив указателей на все списки слов в словаре
* @param[in]	aNumberOfLists	- количество списков в словаре
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::SetRealLists(ISldList** aWordLists, const UInt32 aNumberOfLists)
{
	return m_RealListVector.Init(aWordLists, aNumberOfLists);
}

/** *********************************************************************
* Сортирует список, согласно заданному массиву индексов
*
* @param[in]	aWordIndexes	- соритрованный вектор с индексами
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::SortByVector(const CSldVector<Int32> & aWordIndexes)
{
	// Эта функция вызываться не должна
	return eCommonWrongList;
}

/** *********************************************************************
* Получает количество локализаций записей в списке слов
*
* @param[out]	aNumberOfLocalization	- количество локализаций
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::GetNumberOfLocalization(Int32 & aNumberOfLocalization)
{
	return eOK;
}

/** *********************************************************************
* Устанавливает нужную локализацию записей в списке слов по номеру локализации
*
* @param[in]	aIndex	- номер локализации, которую нужно установить
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::SetLocalizationByIndex(const Int32 aIndex)
{
	return eOK;
}

/** *********************************************************************
* Получает информацию о локализации записей в списке слов по номеру локализации
*
* @param[in]	aIndex				- номер локализации, о которой мы хотим получить информацию
* @param[out]	aLocalizationCode	- код языка локализации
* @param[out]	aLocalizationName	- строчка с названием локализации
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::GetLocalizationInfoByIndex(const Int32 aIndex, UInt32 & aLocalizationCode, SldU16String & aLocalizationName)
{
	return eOK;
}

/** *********************************************************************
* Возвращает индекс текущей локализации
*
* @param[out]	aIndex		- номер текущей локализации
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::GetCurrentLocalizationIndex(Int32 & aIndex)
{
	return eOK;
}

/** *********************************************************************
* Возвращает вектор индексов словаря для заданного слова
*
* @param[in]	aGlobalIndex	- номер слова в списке слияния
* @param[out]	aDictIndexes	- вектор с индексами словаря
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomList::GetDictionaryIndexesByGlobalIndex(const Int32 aGlobalIndex, CSldVector<Int32> & aDictIndexes)
{
	ISldList* pList = m_WordVector[m_CurrentIndex].RealListPtr;
	if (!pList)
		return eOK;

	if (m_WordVector[m_CurrentIndex].RealWordIndex == SLD_DEFAULT_WORD_INDEX)
		return eOK;

	const CSldListInfo* listInfo = NULL;
	ESldError error = pList->GetWordListInfo(&listInfo);
	if (error != eOK)
		return error;

	Int32 realWordIndex = m_WordVector[m_CurrentIndex].RealWordIndex;

	if (listInfo->GetUsage() == eWordListType_FullTextAuxiliary)
	{
		Int32 realListIndex = SLD_DEFAULT_LIST_INDEX;
		Int32 realIndexesCount = 0;

		error = pList->GetReferenceCount(realWordIndex, &realIndexesCount);
		if (error != eOK)
			return error;

		if (realIndexesCount)
		{
			error = pList->GetRealIndexes(realWordIndex, 0, &realListIndex, &realWordIndex);
			if (error != eOK)
				return error;

			error = m_RealListVector.GetListPtr(realListIndex, &pList);
			if (error != eOK)
				return error;
		}
	}

	return pList->GetDictionaryIndexesByGlobalIndex(aGlobalIndex, aDictIndexes);
}
