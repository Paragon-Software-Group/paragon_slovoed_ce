#include "SldCustomListControl.h"

#include "SldCustomList.h"

/** *********************************************************************
*	Освобождение памяти
*
************************************************************************/
void CSldCustomListControl::Close()
{
	Clear();
}

/** *********************************************************************
* Инициализация
*
* @param[in]	aWordLists		- Массив указателей на все списки слов в словаре
* @param[in]	aListIndex		- индекс пользовательского списка
* @param[in]	aNumberOfLists	- количество списков в словаре
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomListControl::Init(ISldList** aWordLists, const Int32 aListIndex, const UInt32 aNumberOfLists)
{
	if (!aWordLists)
		return eMemoryNullPointer;

	if (!m_WordLists.assign(aWordLists, aNumberOfLists))
		return eMemoryNotEnoughMemory;

	m_CustomListIndex = aListIndex;
	return eOK;
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
ESldError CSldCustomListControl::AddWord(const Int32 aRealListIndex, const Int32 aWordIndex, const UInt32 aUncoverHierarchy)
{
	if (aRealListIndex >= static_cast<Int32>(m_WordLists.size()))
		return eCommonWrongList;

	return customList()->AddWord(aRealListIndex, aWordIndex, aUncoverHierarchy);
}

/** *********************************************************************
* Добавляет слово в список
*
* @param[in]	aRealListIndex		- индекс списка, из которого мы хотим добавить слово
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomListControl::AddWord(const Int32 aRealListIndex)
{
	if (aRealListIndex >= static_cast<Int32>(m_WordLists.size()))
		return eCommonWrongList;

	return customList()->AddWord(aRealListIndex);
}

/** *********************************************************************
* Привязывает слово к записи из списка
*
* @param[in]	aWordIndex		- индекс записи в пользовательском списке, к которой мы хотим привязать слово
* @param[in]	aSubWordIndex	- индекс записи в словарном списке
* @param[in]	aSubWordListIndex	- индекс списка слов, при значении SLD_DEFAULT_LIST_INDEX считаем,
*									  что список слов соответствует списку, в котором находится слово-родитель
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomListControl::AddSubWord(const Int32 aWordIndex, const Int32 aSubWordIndex, const Int32 aSubWordListIndex)
{
	return customList()->AddSubWord(aWordIndex, aSubWordIndex, aSubWordListIndex);
}

/** *********************************************************************
* Удаляет записи из списка
*
* @param[in]	aBeginWordIndex	- индекс первого из удаляемых слов
* @param[in]	aEndWordIndex	- индекс последнего удаляемого слова
*								  при значении SLD_DEFAULT_WORD_INDEX будет удалено только одно слово
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomListControl::RemoveWords(const Int32 aBeginWordIndex, const Int32 aEndWordIndex)
{
	return customList()->EraseWords(aBeginWordIndex, aEndWordIndex == SLD_DEFAULT_WORD_INDEX ? aBeginWordIndex + 1 : aEndWordIndex);
}

/** *********************************************************************
* Добавляет вариант написания
*
* @param[in]	aVariantProperty	- указатель на добавляемый вариант написания
* @param[in]	aUnique				- флаг проверки уникальности нового варианта написания
*									  при aUnique=true и наличии добавляемого варианта написания
*									  в listInfo новый вариант не будет добавлен
*									  дефолтное значение - false
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomListControl::AddVariantProperty(const TListVariantProperty & aVariantProperty, bool aUnique)
{
	CSldListInfo* listInfo;
	ESldError error = m_WordLists[m_CustomListIndex]->GetWordListInfo((const CSldListInfo**)&listInfo);
	if(error != eOK)
		return error;

	return listInfo->AddVariantProperty(aVariantProperty, aUnique);
}

/** *********************************************************************
* Устанавливает вариант написания для уже добавленного слова
*
* @param[in]	aWordIndex		- индекс слова, для которого мы хотим задать вариант написания
* @param[in]	aText			- индекс последнего удаляемого слова
* @param[in]	aVariantIndex	- индекс варианта 
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomListControl::SetCurrentWord(const Int32 aWordIndex, const UInt16* aText, const Int32 aVariantIndex)
{
	return customList()->SetCurrentWord(aWordIndex, aText, aVariantIndex);
}

/** *********************************************************************
* Раскрывает/скрывает следующий уровень иерархии
*
* @param[in]	aWordIndex		- индекс слова, для которого мы хотим раскрыть/скрыть иерархию
* @param[out]	aResult			- указатель, по которому будет записан результат выполнения функции
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomListControl::UncoverHierarchy(const Int32 aWordIndex, EUncoverFolderResult* aResult)
{
	if(!aResult)
		return eMemoryNullPointer;

	*aResult = eFolderNoActions;

	CSldCustomList* customList = this->customList();

    UInt32 uncoveredCount = 0;
    ESldError error = customList->GetUncoveredWordsCount(aWordIndex, &uncoveredCount);
    if(error != eOK)
        return error;

	const CSldVector<TSldWordIndexes> * subwords = NULL;
	ESubwordsState subWordState = eSubwordsStateHasNotSubwords;
    error = customList->GetSubWords(aWordIndex, &subwords, &subWordState);
	if (error != eOK)
		return error;
	//*******************************************************************
	// TODO:	Закомментировал ниже прописанный код раскрытия/сокрытия
	//			 уровней, который срабатывал при условии связанности с
	//			 уровнем каких либо слов.
	//			Причина - сокрытие непоследнего уровня, при условии
	//			 раскрытых подуровней в количестве >= 2, некоректно,
	//			 выдает ошибку.
	//			Пока будет работать альтернативный(который используется
	//			 в большинстве случаев) код, идущий сразу за
	//			 закомментированным.
	//*******************************************************************
	/*
	if (subwords)
	{
		Int32 realListIndex = 0;
		error = customList->GetRealListIndex(aWordIndex, &realListIndex);
		if (error != eOK)
			return error;

		if (subWordState == eSubwordsStateUncovered)
		{
			Int32 realGlobalIndex = SLD_DEFAULT_WORD_INDEX;

			error = customList->GetRealGlobalIndex(aWordIndex + subwords->size(), &realGlobalIndex);
			if (error != eOK)
				return error;

			if (realGlobalIndex == (*subwords)[subwords->size() - 1].WordIndex)
			{
				error = customList->EraseWords(aWordIndex + 1, aWordIndex + subwords->size() + 1);
				if (error != eOK)
					return error;

				*aResult = eFolderCollapseeSubwords;
			}
			else
			{
				return eCommonWrongIndex;
			}
		}
		else
		{
			Int32 subwordIndex = 0;
			for (auto & subword :*subwords)
			{
				const Int32 listIndex = subword.ListIndex == SLD_DEFAULT_LIST_INDEX ? realListIndex : subword.ListIndex;
				error = customList->InsertWords(listIndex, subword.WordIndex, subword.WordIndex + 1, aWordIndex + subwordIndex);
				if (error != eOK)
					return error;
				subwordIndex++;
			}

			*aResult = eFolderUncoverSubwords;
		}

		return eOK;
	}
	*/

	Int32 realWordIndex = SLD_DEFAULT_WORD_INDEX;
	error = customList->GetRealGlobalIndex(aWordIndex, &realWordIndex);
	if(error != eOK)
		return error;

	if(realWordIndex == SLD_DEFAULT_WORD_INDEX)
		return eOK;

	Int32 realListIndex = SLD_DEFAULT_LIST_INDEX;
	error = customList->GetRealListIndex(aWordIndex, &realListIndex);
	if(error != eOK)
		return error;

	ISldList* pList = m_WordLists[realListIndex];
	if(!pList)
		return eOK;

	error = pList->SaveCurrentState();
	if(error != eOK)
		return error;

	TCatalogPath path;
	error = pList->GetPathByGlobalIndex(realWordIndex, &path);
	if(error != eOK)
		return error;

	error = pList->GoToByPath(&path, eGoToSubSection);
	if(error != eOK)
		return error;

	Int32 firstIndex = SLD_DEFAULT_WORD_INDEX;
	Int32 lastIndex = SLD_DEFAULT_WORD_INDEX;
	Int32 realWordsCount = 0;

	error = pList->GetHierarchyLevelBounds(&firstIndex, &lastIndex);
	if(error != eOK)
		return error;

	error = pList->GetNumberOfWords(&realWordsCount);
	if(error != eOK)
		return error;

	lastIndex = firstIndex + realWordsCount;

	if(realWordIndex >= firstIndex && realWordIndex < lastIndex)
	{
		return pList->RestoreState();
	}

	Int32 wordsCount = 0;
	error = customList->GetNumberOfWords(&wordsCount);
	if(error != eOK)
		return error;

	if(aWordIndex + 1 == wordsCount)
	{
		error = customList->InsertWords(realListIndex, firstIndex, lastIndex, aWordIndex);
		if(error != eOK)
			return error;

		if(!subwords)
		{
			for (int index = firstIndex; index < lastIndex; index++)
			{
				error = AddSubWord(aWordIndex, index, realListIndex);
				if(error != eOK)
					return error;
			}

			error = customList->ChengeSubwordsState(aWordIndex, &subWordState);
			if(error != eOK)
				return error;
		}

		*aResult = eFolderUncover;
	}
	else
	{
		Int32 nextRealIndex = SLD_DEFAULT_WORD_INDEX;
		error = customList->GetRealGlobalIndex(aWordIndex + 1, &nextRealIndex);
		if(error != eOK)
			return error;

		if(nextRealIndex == firstIndex)
		{
			error = pList->GoToByGlobalIndex(realWordIndex);
			if(error != eOK)
				return error;

            Int32 lastErasedIndex = aWordIndex + 1 + uncoveredCount;

			error = customList->EraseWords(aWordIndex + 1, lastErasedIndex);
			if(error != eOK)
				return error;

			*aResult = eFolderCollapse;
		}
		else
		{
			error = customList->InsertWords(realListIndex, firstIndex, lastIndex, aWordIndex);
			if(error != eOK)
				return error;

			if(!subwords)
			{
				for (int index = firstIndex; index < lastIndex; index++)
				{
					error = AddSubWord(aWordIndex, index, realListIndex);
					if(error != eOK)
						return error;
				}

				error = customList->ChengeSubwordsState(aWordIndex, &subWordState);
				if(error != eOK)
					return error;
			}

			*aResult = eFolderUncover;
		}
	}

	return pList->RestoreState();
}

/** *********************************************************************
* Устанавливает индекс базового списка, в котором нужно проводить поиск
*
* @param[in]	aListIndex	- индекс списка, в котором будет производиться поиск
*							  для этого все слова или их родители должны присутствовать в пользовательском списке
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomListControl::SetSearchSourceListIndex(const Int32 aListIndex)
{
	customList()->SetSearchSourceListIndex(aListIndex);
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
ESldError CSldCustomListControl::GetSubWordsCount(const Int32 aWordIndex, UInt32* aSubWordsCount)
{
	return customList()->GetSubWordsCount(aWordIndex, aSubWordsCount);
}

/** *********************************************************************
* Проверяет состояние слова и связанных с ним слов
*
* @param[in]	aWordIndex	- индекс слова
* @param[out]	aWordState	- указатель, по которому будет записан результат
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomListControl::CheckSubwordsState(const Int32 aWordIndex, ESubwordsState* aWordState)
{
	return customList()->CheckSubwordsState(aWordIndex, aWordState);
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
ESldError CSldCustomListControl::SortListByVariant(const Int32 aVariantIndex)
{
	return customList()->SortListByVariant(aVariantIndex);
}

/** *********************************************************************
* Объединяет все записи для вспомогательных листов, относящиеся к одному заголовку
*
* @return код ошибки
************************************************************************/
ESldError CSldCustomListControl::GroupWordsByHeadwords()
{
	return customList()->GroupWordsByHeadwords();
}
