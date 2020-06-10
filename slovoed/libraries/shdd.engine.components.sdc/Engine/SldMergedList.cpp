#include "SldMergedList.h"

/** *********************************************************************
* Инициализация
*
* @param[in]	aLists	- вектор указателей на одиночные списки
*
* @return код ошибки
************************************************************************/
ESldError CSldIndexesStore::Init(CSldVector<ISldList*> & aLists)
{
	m_ListCount = aLists.size();
	return eOK;
}

/** *********************************************************************
* Добавляет новый индекс
*
* @param[in]	aSingleIndexes	- массив одиночных индексов
*
* @return код ошибки
************************************************************************/
void CSldIndexesStore::AddIndex(const Int32 * aSingleIndexes)
{
	if (m_ListCount == 0)
		return;

	const UInt32 blockIndex = m_IndexCount / MemBlockSize;
	const UInt32 localIndex = m_ListCount * (m_IndexCount % MemBlockSize);

	if (m_Indexes.size() <= blockIndex)
	{
		m_Indexes.emplace_back(MemBlockSize * m_ListCount);
	}

	sldMemMove(&m_Indexes[blockIndex][localIndex], aSingleIndexes, m_ListCount * sizeof(Int32));
	m_IndexCount++;
}

/** *********************************************************************
* Получает набор одиночных индексов для индекса в ядре слияния
*
* @param[in]	aIndex			- запрашиваемый индекс в ядре слияния
* @param[out]	aSingleIndexes	- массив одиночных индексов
*
* @return код ошибки
************************************************************************/
ESldError CSldIndexesStore::GetIndex(const Int32 aIndex, const Int32 ** aSingleIndexes) const
{
	if (aIndex >= m_IndexCount)
		return eCommonWrongIndex;

	const UInt32 blockIndex = aIndex / MemBlockSize;
	const UInt32 localIndex = m_ListCount * (aIndex % MemBlockSize);

	*aSingleIndexes = &m_Indexes[blockIndex][localIndex];

	return eOK;
}

/** *********************************************************************
* Конструктор
*
* @param[in]	aLists			- вектор указателей на одиночные списки
* @param[in]	aMergedMetaInfo	- дополнительная информация о слиянии
*
* @return код ошибки
************************************************************************/
CSldMergedList::CSldMergedList(CSldVector<TSldPair<ISldList*, Int32>> & aLists, const TMergedMetaInfo& aMergedMetaInfo) :
m_CurrentGlobalIndex(0)
{
	m_Lists.reserve(aLists.size());
	m_DictionaryIndexes.reserve(aLists.size());
	for (auto & list : aLists)
	{
		m_Lists.push_back(list.first);
		m_DictionaryIndexes.push_back(list.second);
	}
};

/** *********************************************************************
* Инициализация
*
* @param[out] aWordIndexes	- вектор соответствия локальных и глобальных индексов в словаре
* @param[in]  aMergedList	- если у нас уже есть полностью инициализированное список слияния
*							  мы можем воспользоваться им и ускорить загрузку
*
* @return код ошибки
************************************************************************/
ESldError CSldMergedList::InitMergedList(CSldVector<CSldVector<Int32>> & aWordIndexes, CSldMergedList * aMergedList)
{
	CSldVector<Int32> showVariantIndex;
	CSldVector<Int32> IndexBuf(m_Lists.size());
	CSldVector<bool> minIndexes;
	bool isFullTextSearchList = false;

	UInt32 maxWordLenght = 0;
	ESldError error = eOK;
	for (auto list : m_Lists)
	{
		error = list->GetWordBySortedIndex(0);
		if (error != eOK)
			return error;

		const CSldListInfo* listInfo;
		error = list->GetWordListInfo(&listInfo);
		if (error != eOK)
			return error;

		maxWordLenght = maxWordLenght < listInfo->GetMaximumWordSize() ? listInfo->GetMaximumWordSize() : maxWordLenght;

		if (!isFullTextSearchList && listInfo->IsFullTextSearchList())
			isFullTextSearchList = true;

		if (list == *m_Lists.begin())
		{
			error = m_ListInfo.Init(*listInfo);
			if (error != eOK)
				return error;
		}
		else
		{
			for (UInt32 variantIndex = 0; variantIndex < listInfo->GetNumberOfVariants(); variantIndex++)
			{
				EListVariantTypeEnum variantType = listInfo->GetVariantType(variantIndex);
				if (m_ListInfo.GetVariantIndexByType(variantType) == SLD_DEFAULT_VARIANT_INDEX)
				{
					TListVariantProperty newProperty;
					newProperty.Type = variantType;
					newProperty.LangCode = listInfo->GetVariantLangCode(variantIndex);
					error = m_ListInfo.AddVariantProperty(newProperty);
					if (error != eOK)
						return error;
				}
			}		
		}
		showVariantIndex.push_back(listInfo->GetShowVariantIndex());
	}

	if (!isFullTextSearchList)
	{
		Int32 listIndex = 0;
		for (auto list : m_Lists)
		{
			const CSldListInfo* listInfo;
			error = list->GetWordListInfo(&listInfo);
			if (error != eOK)
				return error;

			m_LocalVariantIndexes.emplace_back();

			for (UInt32 variantIndex = 0; variantIndex < m_ListInfo.GetNumberOfVariants(); variantIndex++)
			{
				EListVariantTypeEnum variantType = m_ListInfo.GetVariantType(variantIndex);
				m_LocalVariantIndexes.back().push_back(listInfo->GetVariantIndexByType(variantType));
			}

			Int32 wordsCount = 0;
			error = list->GetTotalWordCount(&wordsCount);
			if (error != eOK)
				return error;

			aWordIndexes[m_DictionaryIndexes[listIndex]].resize(wordsCount, SLD_DEFAULT_WORD_INDEX);
			listIndex++;
		}

		if (aMergedList != NULL)
		{
			m_SingleIndexes = aMergedList->m_SingleIndexes;
		}
		else
		{
			minIndexes.resize(m_Lists.size());
			error = m_SingleIndexes.Init(m_Lists);
			if (error != eOK)
				return error;

			CSldVector<Int8> listEnd(m_SingleIndexes.GetListCount(), 1);

			while (sld2::find(listEnd, 1) != CSldVector<Int8>::npos)
			{
				error = GetMinWordIndexes(showVariantIndex, listEnd, minIndexes);
				if (error != eOK)
					return error;

				for (UInt32 i = 0; i < m_SingleIndexes.GetListCount(); i++)
				{
					if (minIndexes[i])
					{
						error = m_Lists[i]->GetCurrentGlobalIndex(&IndexBuf[i]);
						if (error != eOK)
							return error;

						aWordIndexes[m_DictionaryIndexes[i]][IndexBuf[i]] = m_SingleIndexes.GetTotalIndexesCount();

						error = m_Lists[i]->GetNextRealSortedWord(&listEnd[i]);
						if (error != eOK)
							return error;
					}
					else
					{
						IndexBuf[i] = SLD_DEFAULT_WORD_INDEX;
					}
				}

				m_SingleIndexes.AddIndex(IndexBuf.data());
			}
		}
	}

	m_StringBuf.assign(SldU16String());
	m_StringBuf.push_back(0);
	m_ListInfo.m_Header.NumberOfWords = m_SingleIndexes.GetTotalIndexesCount();
	m_NumberOfWords = m_SingleIndexes.GetTotalIndexesCount();
	m_ListInfo.m_Header.MaximumWordSize = maxWordLenght;

	return eOK;
}

template<typename VectorElement>
void FillVector(CSldVector<VectorElement> & aVector, const VectorElement aElement)
{
	for (auto & element : aVector)
	{
		element = aElement;
	}
}

/** *********************************************************************
* Определяет в каких одиночных списках текущее слово активно
*
* @param[in]	aShowVariantIndexes	- вектор индексов ShowVariant-ов в одиночных списках
* @param[in]	aEndList			- вектор флагов, закончили ли мы декодировать данный список
* @param[out]	aIndexes			- вектор булевых значений, активно данное слово или нет
*
* @return код ошибки
************************************************************************/
ESldError CSldMergedList::GetMinWordIndexes(const CSldVector<Int32> & aShowVariantIndexes, const CSldVector<Int8> & aEndList, CSldVector<bool> & aIndexes)
{
	ESldError error = eOK;
	UInt16* wordPtr = NULL;
	m_StringBuf.clear();
	FillVector(aIndexes, false);

	Int32 counter = 0;
	for (auto list : m_Lists)
	{
		if (aEndList[counter] == 0)
		{
			counter++;
			continue;
		}

		error = list->GetCurrentWord(aShowVariantIndexes[counter], &wordPtr);
		if (error != eOK)
			return error;

		if (m_StringBuf.empty())
		{
			m_StringBuf = wordPtr;
			aIndexes[counter] = true;
			counter++;
			continue;
		}

		Int32 compareResult = GetCMP()->StrICmp(m_StringBuf.c_str(), wordPtr);
		if (compareResult == 1)
		{
			m_StringBuf = wordPtr;
			FillVector(aIndexes, false);
			aIndexes[counter] = true;
		}
		else if (compareResult == 0)
		{
			compareResult = CSldCompare::StrCmp(m_StringBuf.c_str(), wordPtr);
			if (compareResult == 1)
			{
				m_StringBuf = wordPtr;
				FillVector(aIndexes, false);
				aIndexes[counter] = true;
			}
			else if (compareResult == 0)
			{
				aIndexes[counter] = true;
			}
		}
		counter++;
	}

	return eOK;
}

/** *********************************************************************
* Получает слово по его глобальному номеру
*
* ВНИМАНИЕ: в случае, когда запрашиваемое слово находится внутри папки,
* ни класс с информацией о каталоге, ни индекс начала папки (m_baseIndex) не изменяются
*
* @param[in]	aGlobalIndex	- глобальный номер слова
* @param[in]	aInLocalization	- флаг учета локализации. true - мы запрашиваем индекс с учетом локализации
*								  false - мы запрашиваем индекс без учета локализации
*
* @return код ошибки
************************************************************************/
ESldError CSldMergedList::GetWordByGlobalIndex(Int32 aGlobalIndex, const bool aInLocalization)
{
	if (aGlobalIndex >= m_NumberOfWords)
		return eCommonWrongIndex;

	m_CurrentGlobalIndex = aGlobalIndex;
	return eOK;
}

/** ********************************************************************
* Возвращает указатель на последнее найденное слово.
*
* @param[in]	aVariantIndex	- номер варианта написания для текущего слова.
* @param[out]	aWord			- указатель на буфер для указателя на слово
*
* @return код ошибки
************************************************************************/
ESldError CSldMergedList::GetCurrentWord(Int32 aVariantIndex, UInt16** aWord)
{
	const Int32 * indexes = NULL;
	ESldError error = m_SingleIndexes.GetIndex(m_CurrentGlobalIndex, &indexes);
	if (error != eOK)
		return error;

	for (UInt32 i = 0; i < m_SingleIndexes.GetListCount(); i++)
	{
		if (indexes[i] != SLD_DEFAULT_WORD_INDEX)
		{
			error = m_Lists[i]->GoToByGlobalIndex(indexes[i]);
			if (error != eOK)
				return error;

			if (m_LocalVariantIndexes[i][aVariantIndex] != SLD_DEFAULT_VARIANT_INDEX)
			{
				return m_Lists[i]->GetCurrentWord(m_LocalVariantIndexes[i][aVariantIndex], aWord);
			}
		}
	}

	*aWord = m_StringBuf.data();

	return eOK;
}


/** *********************************************************************
* Возвращает номер статьи по номеру слова и номеру перевода в обычном списке слов
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
ESldError CSldMergedList::GetTranslationIndex(Int32 aGlobalIndex, Int32 aTranslationIndex, Int32* aArticleIndex)
{
	*aArticleIndex = -1;

	const Int32 * indexes = NULL;
	ESldError error = m_SingleIndexes.GetIndex(aGlobalIndex, &indexes);
	if (error != eOK)
		return error;

	Int32 translationIndex = 0;
	for (UInt32 i = 0; i < m_SingleIndexes.GetListCount(); i++)
	{
		if (indexes[i] != SLD_DEFAULT_WORD_INDEX)
		{
			if (aTranslationIndex == translationIndex)
			{
				error = m_Lists[i]->GetTranslationIndex(indexes[i], 0, aArticleIndex);
				if (error != eOK)
					return error;

				return eOK;
			}

			translationIndex++;
		}
	}

	return eOK;
}

/** *********************************************************************
* Возвращает количество переводов у указанного слова.
*
* @param[in]	aGlobalIndex		- номер слова из списка слов для которого требуется узнать
*									  количество переводов
* @param[out]	aTranslationCount	- указатель на переменную, в которую будет помещено
*									  количество переводов у последнего декодированного слова.
*
* @return код ошибки
************************************************************************/
ESldError CSldMergedList::GetTranslationCount(Int32 aGlobalIndex, Int32* aTranslationCount)
{
	*aTranslationCount = 0;

	const Int32 * indexes = NULL;
	ESldError error = m_SingleIndexes.GetIndex(aGlobalIndex, &indexes);
	if (error != eOK)
		return error;

	for (UInt32 i = 0; i < m_SingleIndexes.GetListCount(); i++)
	{
		if (indexes[i] != SLD_DEFAULT_WORD_INDEX)
		{
			(*aTranslationCount)++;
		}
	}

	return eOK;
}

/** *********************************************************************
* Ищем ближайшее слово, которое больше или равно заданному
*
* @param[in]	aText	- искомое слово
*
* @return код ошибки
************************************************************************/
ESldError CSldMergedList::GetWordByText(const UInt16* aText)
{
	ESldError error = eOK;
	Int32 low = 0;
	Int32 hi = m_NumberOfWords;

	while (low < hi)
	{
		Int32 med = (hi + low) >> 1;
		error = GetWordBySortedIndex(med);
		if (error != eOK)
			return error;

		UInt16* currentWord = NULL;
		error = GetCurrentWord(GetListInfo()->GetShowVariantIndex(), &currentWord);
		if (error != eOK)
			return error;

		if (GetCMP()->StrICmp(currentWord, aText) < 0)
			low = med + 1;
		else
			hi = med;
	}

	if (low >= m_NumberOfWords)
		low = m_NumberOfWords - 1;

	error = GetWordBySortedIndex(low);
	if (error != eOK)
		return error;

	return eOK;
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
ESldError CSldMergedList::GetWordBySortedText(const UInt16* aText, UInt32* aResultFlag)
{
	if (!aResultFlag)
		return eMemoryNullPointer;

	*aResultFlag = 1;
	return GetWordByText(aText);
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
ESldError CSldMergedList::FindBinaryMatch(const UInt16* aText, UInt32* aResult)
{
	return eOK;
}

/** *********************************************************************
* Возвращает вектор индексов озвучек текущего слова
*
* @param[out]	aSoundIndexes	- вектор с индексами озвучки для текущего слова
*
* @return код ошибки
************************************************************************/
ESldError CSldMergedList::GetSoundIndex(CSldVector<Int32> & aSoundIndexes)
{
	if (m_SingleIndexes.GetListCount() == 0)
		return eOK;

	const Int32 * indexes = NULL;
	ESldError error = m_SingleIndexes.GetIndex(m_CurrentGlobalIndex, &indexes);
	if (error != eOK)
		return error;

	for (UInt32 i = 0; i < m_SingleIndexes.GetListCount(); i++)
	{
		const Int32 currentIndex = indexes[i];
		if (currentIndex != SLD_DEFAULT_WORD_INDEX)
		{
			error = m_Lists[i]->GoToByGlobalIndex(currentIndex);
			if (error != eOK)
				return error;

			error = m_Lists[i]->GetSoundIndex(aSoundIndexes);
			if (error != eOK)
				return error;
		}
	}

	return eOK;
}

/** *********************************************************************
* Возвращает вектор индексов изображений для текущего слова
*
* @param[out]	aPictureIndexes	- вектор индексов изображений для текущего слова
*
* @return код ошибки
************************************************************************/
ESldError CSldMergedList::GetPictureIndex(CSldVector<Int32> & aPictureIndexes)
{
	if (m_SingleIndexes.GetListCount() == 0)
		return eOK;

	const Int32 * indexes = NULL;
	ESldError error = m_SingleIndexes.GetIndex(m_CurrentGlobalIndex, &indexes);
	if (error != eOK)
		return error;

	for (UInt32 i = 0; i < m_SingleIndexes.GetListCount(); i++)
	{
		const Int32 currentIndex = indexes[i];
		if (currentIndex != SLD_DEFAULT_WORD_INDEX)
		{
			error = m_Lists[i]->GoToByGlobalIndex(currentIndex);
			if (error != eOK)
				return error;

			error = m_Lists[i]->GetPictureIndex(aPictureIndexes);
			if (error != eOK)
				return error;
		}
	}

	return eOK;
}

/** *********************************************************************
* Получает следующее сортированное слово
*
* @return код ошибки
************************************************************************/
ESldError CSldMergedList::GetNextSortedWord()
{
	const Int32 nextIndex = GetCurrentWordIndex() + 1;
	if (nextIndex < m_NumberOfWords)
		return GetWordByGlobalIndex(nextIndex);

	return eCommonWrongIndex;
}

/** *********************************************************************
* Возвращает указатель на класс, хранящий информацию о свойствах списка слов
*
* @param[out]	aListInfo	- указатель на переменную, в которую будет возвращен указатель
* @param[in]	aDictIndex	- индекс словаря в ядре слияния, имеет дефолтное значение SLD_DEFAULT_DICTIONARY_INDEX
*
* @return код ошибки
************************************************************************/
ESldError CSldMergedList::GetWordListInfo(const CSldListInfo** aListInfo, Int32 aDictIndex) const
{
	if (aDictIndex == SLD_DEFAULT_DICTIONARY_INDEX)
	{
		*aListInfo = GetListInfo();
		return eOK;
	}

	for (Int32 listIndex = 0; listIndex < m_DictionaryIndexes.size(); listIndex++)
	{
		if (m_DictionaryIndexes[listIndex] == aDictIndex)
			return m_Lists[listIndex]->GetWordListInfo(aListInfo);
	}

	return eMergeWrongDictionaryIndex;
}

/// Возвращает индекс слова из одиночного словаря по смерженому индексу
ESldError CSldMergedList::GetSingleIndex(Int32 aMergedIndex, Int32 aTranslationIndex, Int32 & aSingleIndex)
{
	const Int32 * indexes = NULL;
	ESldError error = m_SingleIndexes.GetIndex(aMergedIndex, &indexes);
	if (error != eOK)
		return error;

	Int32 translationIndex = 0;
	for (UInt32 i = 0; i < m_SingleIndexes.GetListCount(); i++)
	{
		if (indexes[i] != SLD_DEFAULT_WORD_INDEX)
		{
			if (translationIndex == aTranslationIndex)
			{
				aSingleIndex = indexes[i];
				return eOK;
			}
			translationIndex++;
		}
	}

	return eOK;
}

/** *********************************************************************
* Получает следующее реальное сортированное слово
*
* @param[out]	aResult		- указатель, по которому будет возвращен результат
*							  выполнения функции	0 - мы не встретили ни 1 реального слова
*													или список закончился
*													1 - мы смогли получить следующее слово
*
* @return код ошибки
************************************************************************/
ESldError CSldMergedList::GetNextRealSortedWord(Int8* aResult)
{
	if (!aResult)
		return eMemoryNullPointer;

	*aResult = 0;
	if (m_CurrentGlobalIndex + 1 < m_NumberOfWords)
	{
		*aResult = 1;
		m_CurrentGlobalIndex++;
	}

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
ESldError CSldMergedList::GetDictionaryIndexesByGlobalIndex(const Int32 aGlobalIndex, CSldVector<Int32> & aDictIndexes)
{
	aDictIndexes.clear();
	const Int32 * indexes = NULL;
	if (aGlobalIndex >= m_SingleIndexes.GetTotalIndexesCount())
		return eMergeWrongWordIndex;

	ESldError error = m_SingleIndexes.GetIndex(aGlobalIndex, &indexes);
	if (error != eOK)
		return error;

	for (Int32 i = 0; i < m_SingleIndexes.GetListCount(); i++)
	{
		if (indexes[i] != SLD_DEFAULT_WORD_INDEX)
			aDictIndexes.push_back(m_DictionaryIndexes[i]);
	}

	return eOK;
}