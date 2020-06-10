#include "SldList.h"

#include "SldMacros.h"
#include "SldTools.h"

/// Деструктор
CSldList::~CSldList()
{
	if (m_CurrentWord)
	{
		for (UInt32 i = 0; i<m_ListInfo->GetNumberOfVariants(); i++)
			sldMemFree(m_CurrentWord[i]);

		sldMemFree(m_CurrentWord);
	}

	if (m_SortedWordIndexes)
		sldMemFree(m_SortedWordIndexes);

	if (m_WordsBuff)
	{
		for (UInt32 wordIndex = 0; wordIndex < m_ListInfo->GetNumberOfGlobalWords(); wordIndex++)
			sldMemFree(m_WordsBuff[wordIndex]);

		sldMemFree(m_WordsBuff);
	}
}

/** *********************************************************************
* Получает границы бинарного поиска
*
* @param[in]	 aText			- искомое слово
* @param[in]	 aCompare		- указатель на экземпляр таблицы сравнения
* @param[in/out] aHi			- указатель на верхнюю границу поиска
* @param[in/out] aLow			- указатель на нижнюю границу поиска
* @param[in/out] aDepth			- указатель на глубину последнего элемента быстрого поиска
* @param[out]	 aLastElement	- указатель на последний найденный элемент быстрого поиска
*
* @return код ошибки
************************************************************************/
ESldError TQuickSearchPoints::GetSearchBounds(const UInt16* aText, CSldCompare* aCompare, Int32* aHi, Int32* aLow, UInt32* aDepth, TBinaryTreeElement** aLastElement)
{
	if(!aText || !aCompare || !aHi || !aLow || !aDepth || !aLastElement)
		return eMemoryNullPointer;

	if(TextSearchTree == NULL)
		return eOK;

	*aLastElement = TextSearchTree;

	Int32 compareResult = 0;

	while(*aDepth < MaxSearchTreeDepth && (*aLastElement)->CurrentWord)
	{
		(*aDepth)++;

		if((*aLastElement)->WordIndex >= *aHi && (*aLastElement)->LeftLeafIndex)
		{
			(*aLastElement) = &TextSearchTree[(*aLastElement)->LeftLeafIndex];
			continue;
		}
		else if((*aLastElement)->WordIndex < *aLow && (*aLastElement)->RightLeafIndex)
		{
			(*aLastElement) = &TextSearchTree[(*aLastElement)->RightLeafIndex];
			continue;
		}

		compareResult = aCompare->StrICmp((*aLastElement)->CurrentWord, aText);

		if (compareResult < 0)
		{	
			*aLow = (*aLastElement)->WordIndex;

			if((*aLastElement)->RightLeafIndex)
				(*aLastElement) = &TextSearchTree[(*aLastElement)->RightLeafIndex];
			else
				break;
		}
		else if (compareResult > 0)
		{
			*aHi = (*aLastElement)->WordIndex;

			if((*aLastElement)->LeftLeafIndex)
				(*aLastElement) = &TextSearchTree[(*aLastElement)->LeftLeafIndex];
			else
				break;
		}
		else
		{
			*aHi = (*aLastElement)->WordIndex;
			*aLow = (*aLastElement)->WordIndex;
			return eOK;
		}
	}

	return eOK;
}

/** *********************************************************************
* Добавляет элемент быстрого поиска
*
* @param[in]	 aCurrentWord	- текущее слово
* @param[in]	 aCurrentIndex	- индекс текущего слова
* @param[in]	 aCompare		- указатель на экземпляр таблицы сравнения
* @param[in/out] aDepth			- указатель на глубину последнего элемента быстрого поиска
* @param[in]	 aLastElement	- указатель на последний найденный элемент быстрого поиска
*
* @return код ошибки
************************************************************************/
ESldError TQuickSearchPoints::AddElement(UInt16* aCurrentWord, Int32 aCurrentIndex, CSldCompare* aCompare, UInt32* aDepth, TBinaryTreeElement** aLastElement)
{
	if(!aCurrentWord || !aCompare || !aDepth || !aLastElement)
		return eMemoryNullPointer;

	if(TextSearchTree == NULL)
		return eOK;

	TBinaryTreeElement* nextBinaryTreeElement = NULL;

	if (aCurrentIndex == (*aLastElement)->WordIndex)
		return eOK;


	(*aDepth)++;
	nextBinaryTreeElement = &TextSearchTree[SearchTreeElementCount];

	nextBinaryTreeElement->WordIndex = aCurrentIndex;
	nextBinaryTreeElement->CurrentWord = (UInt16*)sldMemNew((aCompare->StrLen(aCurrentWord) + 1) * sizeof(UInt16));
	if(!nextBinaryTreeElement->CurrentWord)
		return eMemoryNotEnoughMemory;

	aCompare->StrCopy(nextBinaryTreeElement->CurrentWord, aCurrentWord);

	if (SearchTreeElementCount > 1)
	{
		if (aCurrentIndex > (*aLastElement)->WordIndex)
		{
			(*aLastElement)->RightLeafIndex = SearchTreeElementCount;
		}
		else
		{
			(*aLastElement)->LeftLeafIndex = SearchTreeElementCount;
		}
	}

	SearchTreeElementCount++;
	*aLastElement = nextBinaryTreeElement;

	return eOK;
}

/// Деструктор
TQuickSearchPoints::~TQuickSearchPoints()
{
	if(TextSearchTree)
	{
		for(UInt32 elementIndex = 0; elementIndex < SearchTreeElementCount; elementIndex++)
			TextSearchTree[elementIndex].Clear();

		sldMemFree(TextSearchTree);
	}
}

/** *********************************************************************
* Инициализация структуры быстрого поиска
*
* @param[in]	aWordsCount				- количество слов в списке
* @param[in]	aSearchTreePointsPtr	- указатель на бинарный массив данных с точками быстрого поиска,
*										  созданных при сборке базы
*
* @return код ошибки
************************************************************************/
ESldError TQuickSearchPoints::Init(UInt32 aWordsCount, UInt32* aSearchTreePointsPtr)
{
	if(aWordsCount < SLD_MIN_WORDS_COUNT_FOR_QUICK_SEARCH && !aSearchTreePointsPtr)
		return eOK;

	Int32 tmpWordsCount = 2;
	Int32 iter = 1;

	while(tmpWordsCount < aWordsCount)
	{
		tmpWordsCount *= 2;
		iter++;
	}

	iter /= 2;

	MaxSearchTreeDepth = iter;
	UInt32 searchTreeSize = 1;
	while(iter)
	{
		searchTreeSize *= 2;
		iter--;
	}

	UInt32 searchTreePointsCount = 0;
	if(aSearchTreePointsPtr)
	{
		searchTreePointsCount = *aSearchTreePointsPtr;
		aSearchTreePointsPtr++;
	}
	searchTreeSize = searchTreeSize > searchTreePointsCount ? searchTreeSize : searchTreePointsCount;

	TextSearchTree = (TBinaryTreeElement*)sldMemNewZero(searchTreeSize * sizeof(TextSearchTree[0]));
	if(!TextSearchTree)
		return eMemoryNotEnoughMemory;

	UInt16* searchTreePointsPtr16 = NULL;
	Int32 strLen = 0;

	for (UInt32 searchPointIndex = 0; searchPointIndex < searchTreePointsCount; searchPointIndex++)
	{
		TBinaryTreeElement* elementLeaf = &TextSearchTree[SearchTreeElementCount];

		elementLeaf->WordIndex = *aSearchTreePointsPtr++;
		searchTreePointsPtr16 = (UInt16*)aSearchTreePointsPtr;

		elementLeaf->LeftLeafIndex = *searchTreePointsPtr16++;
		elementLeaf->RightLeafIndex = *searchTreePointsPtr16++;

		strLen = CSldCompare::StrLen(searchTreePointsPtr16) + 1;
		elementLeaf->CurrentWord = (UInt16*)sldMemNew(strLen * sizeof(UInt16));
		CSldCompare::StrCopy(elementLeaf->CurrentWord, searchTreePointsPtr16);

		searchTreePointsPtr16 += strLen;
		aSearchTreePointsPtr = (UInt32*)searchTreePointsPtr16;

		SearchTreeElementCount++;
	}

	return eOK;
}

/** *********************************************************************
* Инициализация
*
* @param[in]	aData				- указатель на объект, отвечающий за получение данных из контейнера
* @param[in]	aDataSize			- размер данных
* @param[in]	aMaxHeadWordSize	- максимальная длинна слова в списке слов
*
* @return код ошибки
************************************************************************/
ESldError AltNameManager::Init(Int8* aData, UInt32 aDataSize, UInt32 aMaxHeadWordSize)
{
	const UInt32 count = *(UInt32*)aData;
	aData += sizeof(UInt32);

	if (!m_AltInfo.resize(count))
		return eMemoryNotEnoughMemory;
	sldMemCopy(m_AltInfo.data(), aData, m_AltInfo.size() * sizeof(m_AltInfo[0]));
	aData += m_AltInfo.size() * sizeof(m_AltInfo[0]);

	UInt32 IndexCount = m_AltInfo.back().RealWordIndex + m_AltInfo.back().RealWordCount;
	if (!m_Indexes.resize(IndexCount))
		return eMemoryNotEnoughMemory;
	sldMemMove(m_Indexes.data(), aData, IndexCount * sizeof(m_Indexes[0]));

	if(!m_AlternativeWord.resize(aMaxHeadWordSize))
		return eMemoryNotEnoughMemory;

	return eOK;
}

/// Конструктор
AltNameManager::AltNameManager()
	: m_LastShift(-1)
{}

/** *********************************************************************
* Получает информацию об альтернативном заголовке по смещению
*
* @param[in]	aShift				- смещение слова, информацию о котором мы хотим получить
* @param[in]	aRealWordCount		- указатель, по которому будет записано количество ссылок
*									  альтернативного заголовка на обычные записи в списке слов
* @param[in]	aRealWordIndexes	- указатель, по которому будет записан указатель на первый элемент
*									  массива реальных индексов
*
* @return код ошибки
************************************************************************/
ESldError AltNameManager::GetAltnameInfo(UInt32 aShift, UInt32* aRealWordCount, const UInt32** aRealWordIndexes) const
{
	if(!aRealWordCount || !aRealWordIndexes)
		return eMemoryNullPointer;

	*aRealWordCount = 0;

	struct cmp {
		UInt32 shift;
		bool operator()(const TAlternativeHeadwordInfo &info) const { return info.WordShift < shift; }
	};
	UInt32 resultIndex = sld2::lower_bound(m_AltInfo.data(), m_AltInfo.size(), cmp{ aShift });
	if (resultIndex != m_AltInfo.size() && m_AltInfo[resultIndex].WordShift == aShift)
	{
		*aRealWordCount = m_AltInfo[resultIndex].RealWordCount;
		*aRealWordIndexes = &m_Indexes[m_AltInfo[resultIndex].RealWordIndex];
	}

	return eOK;
}

/** *********************************************************************
* Возвращает смещение последнего закодированного слова
*
* @return смещение последнего закодированного слова
************************************************************************/
UInt32 AltNameManager::GetShiftForLastWord() const
{
	return m_AltInfo.size() ? m_AltInfo.back().WordShift : 0;
}

/** *********************************************************************
* Инициализация списка слов
*
* @param[in]	aData			- ссылка на объект, отвечающий за получение данных из контейнера
* @param[in]	aLayerAccess	- указатель на класс для взаимодействия с оболочкой
* @param[in]	aListInfo		- указатель на класс, хранящий информацию о списке слов
* @param[in]	aCMP			- указатель на класс сравнения строк
* @param[in]	aHASH			- число необходимое для декодрования текущего списка слов
*
* @return код ошибки
************************************************************************/
ESldError CSldList::Init(CSDCReadMy &aData, ISldLayerAccess* aLayerAccess, const CSldListInfo* aListInfo, CSldCompare* aCMP, UInt32 aHASH)
{
	if (!aLayerAccess)
		return eMemoryNullPointer;
	if (!aListInfo)
		return eMemoryNullPointer;

	ESldError error;

	m_data = &aData;
	m_ListInfo = aListInfo;
	m_LayerAccess = aLayerAccess;
	
	const TListHeader* pListHeader = m_ListInfo->GetHeader();
	if (!pListHeader)
		return eMemoryNullPointer;

	m_Input = sld2::CreateInput(pListHeader->CompressionMethod);
	if (!m_Input)
	{
		m_ListInfo = NULL;
		return eCommonWrongCompressionType;
	}
	
	error = m_Input->Init(*m_data, pListHeader->Type_CompressedData, pListHeader->Type_Tree, SLD_NUMBER_OF_TREES_FOR_LIST, pListHeader->Type_DirectWordsShifts, pListHeader->Version);
	if (error != eOK)
		return error;

	error = SetHASH(aHASH);
	if (error != eOK)
		return error;


	m_CurrentWord = (UInt16**)sldMemNew(sizeof(UInt16*)*pListHeader->NumberOfVariants);
	if (!m_CurrentWord)
		return eMemoryNotEnoughMemory;

	if (!m_StylizedVariant.resize(pListHeader->NumberOfVariants))
		return eMemoryNotEnoughMemory;

	UInt32 bufferSize = pListHeader->MaximumWordSize*sizeof(UInt16);
	for (UInt32 i=0;i<pListHeader->NumberOfVariants;i++)
	{
		m_CurrentWord[i] = (UInt16*)sldMemNewZero(bufferSize);
		if (!m_CurrentWord[i])
			return eMemoryNotEnoughMemory;

		m_StylizedVariant[i] = SLD_INDEX_STYLIZED_VARIANT_NO;
	}

	if (pListHeader->IsHierarchy)
	{
		m_Catalog = sld2::make_unique<CSldCatalog>();
		if (!m_Catalog)
			return eMemoryNotEnoughMemory;

		error = m_Catalog->Init(*m_data, pListHeader->Type_Hierarchy);
		if (error != eOK)
			return error;
	}

	if(pListHeader->Version == VERSION_LIST_DIRECT_ACCESS)
	{
		if (!m_DirectAccessShifts.resize(sld2::div_round_up(MAX_COMPRESSED_DATA_RESOURCE_SIZE, sizeof(m_DirectAccessShifts[0]))))
			return eMemoryNotEnoughMemory;

		error = m_data->GetResourceData(m_DirectAccessShifts.data(), m_ListInfo->GetHeader()->Type_DirectWordsShifts, 0, MAX_COMPRESSED_DATA_RESOURCE_SIZE);
		if (error != eOK)
			return error;

		m_CurrentDirectAccessResourceIndex = 0;
		
		error = GetWordByGlobalIndex(0);
		if (error != eOK)
			return error;

		CSDCReadMy::ResourceHandle res;
		if(pListHeader->Type_SearchTreePoints)
		{
			res = m_data->GetResource(pListHeader->Type_SearchTreePoints, 0);
			if (res != eOK)
				return res.error();
		}

		error = m_QuickSearchPoints.Init(m_ListInfo->GetNumberOfGlobalWords(), res && res.size() ? (UInt32*)res.ptr() : nullptr);
		if (error != eOK)
			return error;

		if(pListHeader->Type_AlternativeHeadwordsInfo)
		{
			res = m_data->GetResource(pListHeader->Type_AlternativeHeadwordsInfo, 0);
			if (res != eOK)
				return res.error();

			if (res.size())
				m_AltNameManager.Init((Int8*)res.ptr(), res.size(), pListHeader->MaximumWordSize);
		}
	}
	else if(pListHeader->Version == VERSION_LIST_BASE)
	{
		error = GoToBySearchIndex(0);
		if (error != eOK)
			return error;
	}

	if (!pListHeader->IsDirectList)
	{
		m_Indexes = sld2::make_unique<CSldIndexes>();
		if (!m_Indexes)
			return eMemoryNotEnoughMemory;

		error = m_Indexes->Init(*m_data, pListHeader->Type_IndexesCount,
										 pListHeader->Type_IndexesData,
										 pListHeader->Type_IndexesQA,
										 pListHeader->Type_IndexesHeader);
		if (error != eOK)
			return error;
	}

	if (pListHeader->IsLocalizedList)
	{
		// Найдем номер набора строк для языка по умолчанию
		error = SetLocalization(SldLanguage::Default);
		if (error != eOK)
			return error;
	}

	// Cравнение строк
	m_CMP = aCMP;
	
	// Кэш декодированных слов
	if ((pListHeader->WordListUsage == eWordListType_Dictionary || pListHeader->WordListUsage == eWordListType_SimpleSearch) && pListHeader->Version == VERSION_LIST_BASE)
	{
		Int32 qaInterleave = pListHeader->NumberOfWords;
		Int32 qaCount = 0;

		error = QAGetNumberOfQAItems(&qaCount);
		if (error != eOK)
			return error;

		if (qaCount > 1)
		{
			error = QAGetItemIndex(1, &qaInterleave);
			if (error != eOK)
				return error;
			qaInterleave++;
		}

		error = m_QAShift.Init(qaInterleave, pListHeader->MaximumWordSize);
		if (error != eOK)
			return error;

		const UInt16 * prefix = NULL;
		error = m_Input->GetSearchPointText(0, &prefix);
		if (error != eOK)
			return error;

		error = m_QAShift.SetNewQAPoint(0, prefix, 0, 0);
		if (error != eOK)
			return error;
	}

	return eOK;
}

/** ********************************************************************
* Проверяем, был ли проинициализирован данный экземпляр.
*
* @return true - если инициализация проведена, иначе false.
************************************************************************/
bool CSldList::isInit(void)
{
	if (m_data)
		return true;
		
	return false;
}

/** *********************************************************************
* Получаем слово по локальному номеру
* (номеру слова в пределах текущего списка слов и текущего уровня вложенности)
*
* @param[in]	aWordIndex	- номер слова по порядку, начиная с 0.
*
* @return код ошибки
************************************************************************/
ESldError CSldList::GetWordByIndex(Int32 aWordIndex)
{
	if (!m_IsCatalogSynchronized)
		return eCommonCatalogIsNotSynchronizedError;

	Int32 NumberOfWords;
	ESldError error;

	error = GetNumberOfWords(&NumberOfWords);
	if (error != eOK)
		return error;
		
	if (aWordIndex >= NumberOfWords || aWordIndex < 0)
		return eCommonWrongIndex;

	// Учитываем то, что мы можем находится в иерархии на каком-то уровне.
	aWordIndex += GetBaseIndex();

	return GetWordByGlobalIndex(aWordIndex);
}

/** *********************************************************************
* Ищем ближайшее слово, которое больше или равно заданному
*
* @param[in]	aText	- искомое слово
*
* @return код ошибки
************************************************************************/
ESldError CSldList::GetWordByText(const UInt16* aText)
{
	if (!m_IsCatalogSynchronized)
		return eCommonCatalogIsNotSynchronizedError;

	if (!aText)
		return eMemoryNullPointer;
	

	// Если список слов не предназначен для поиска - тогда поиск не возможен
	UInt32 sortFlag = 0;
	ESldError error = IsCurrentLevelSorted(&sortFlag);
	if (error != eOK)
		return error;

	if (!sortFlag)
		return eOK;
		
	// Проверяем, что в качестве указателя на текст для поиска нам не передали указатель на текущее слово (в нашем внутреннем буфере)
	// Такой указатель может быть получен снаружи после вызова функции GetCurrentWord()
	// В таком случае поиск будет работать неправильно
	for (UInt32 i = 0; i < GetListInfo()->GetNumberOfVariants(); i++)
	{
		if (aText == m_CurrentWord[i])
			return eMemoryBadInputPointer;
	}

	Int32 currentIndex = 0;
	Int32 numberOfWords = 0;

	if (GetListInfo()->GetHeader()->Version == VERSION_LIST_DIRECT_ACCESS)
	{
		Int32 low = 0;
		Int32 hi = 0;
		Int32 med = 0;
		Int32 numberOfWords = 0;
		UInt32 depth = 0;
		TBinaryTreeElement* binaryTreeElement = NULL;

		error = GetNumberOfWords(&numberOfWords);
		if (error != eOK)
			return error;

		error = GetSearchBounds(eSearchRangeCurrentLevel, &low, &hi);
		if (error != eOK)
			return error;

		Int8 needQuickSearch = hi - low > SLD_MIN_WORDS_COUNT_FOR_QUICK_SEARCH ? 1 : 0;

		if(needQuickSearch)
		{
			error = m_QuickSearchPoints.GetSearchBounds(aText, GetCMP(), &hi, &low, &depth, &binaryTreeElement);
			if (error != eOK)
				return error;
		}

		while (hi-low > 1)
		{
			med = (hi+low) >> 1;
			error = GetWordByShift(med);
			if (error != eOK)
				return error;

			if(depth < m_QuickSearchPoints.MaxSearchTreeDepth && needQuickSearch)
			{
				if (!GetListInfo()->IsHierarchy())
				{
					error = m_QuickSearchPoints.AddElement(m_CurrentWord[0], m_CurrentIndex, GetCMP(), &depth, &binaryTreeElement);
					if (error != eOK)
						return error;
				}
				else
				{
					// Расстановка точек бинарного поиска происходит на основе списка, без учета иерархии.
					// Для продолжения кэширования нужно убедиться, что точка соответсвует глобальному списку
					Int8 normalLow = 0;
					Int8 normalHi = 0;	

					if ((hi != numberOfWords) || (hi == GetListInfo()->GetNumberOfGlobalWords()))
					{
						normalHi = 1;
					}

					if ((low != GetBaseIndex()) || (low == 0))
					{
						normalLow = 1;
					}

					if(normalLow && normalHi)
					{
						error = m_QuickSearchPoints.AddElement(m_CurrentWord[0], m_CurrentIndex, GetCMP(), &depth, &binaryTreeElement);
						if (error != eOK)
							return error;
					}
				}
			}

			if (GetCMP()->StrICmp(m_CurrentWord[0], aText) < 0)
			{
				low = med;
			}
			else
			{
				hi = med;
			}
		}

		hi--;

		while (hi >= GetBaseIndex())
		{
			error = GetWordByShift(hi);
			if (error != eOK)
				return error;

			if (GetCMP()->StrICmp(m_CurrentWord[0], aText) >= 0)
			{
				hi--;
			}
			else
			{
				break;
			}
		}

		if (++hi == (numberOfWords + GetBaseIndex()))
			hi--;
		{
			error = GetWordByShift(hi);
			if (error != eOK)
				return error;
		}
	}
	else
	{
		if (GetListInfo()->IsHierarchy())
		{
			if (!m_Catalog->IsLevelSorted())
				return eOK;

			Int32 qaPoint = 0;

			error = GetClosestQAPointByText(aText, &qaPoint);
			if (error != eOK)
				return error;

			if (qaPoint == -1 && GetCMP()->StrICmp(m_CurrentWord[0], aText) >= 0)
			{
				error = GetWordByIndex(0);
				if (error != eOK)
					return error;
			}
			else if (qaPoint != -1)
			{
				error = JumpByQAIndex(qaPoint);
				if (error != eOK)
					return error;

				error = GetNextWord();
				if (error != eOK)
					return error;
			}

			error = GetNumberOfWords(&numberOfWords);
			if (error != eOK)
				return error;

			Int32 currentLocalIndex = 0;
			error = GetCurrentIndex(&currentLocalIndex);
			if (error != eOK)
				return error;

			while (currentLocalIndex < (numberOfWords - 1) && GetCMP()->StrICmp(m_CurrentWord[0], aText) < 0)
			{
				error = GetNextWord();
				if (error != eOK)
					return error;

				error = GetCurrentIndex(&currentLocalIndex);
				if (error != eOK)
					return error;
			}
		}
		else
		{
			UInt32 low = 0;
			UInt32 hi = m_Input->GetSearchPointCount();
			UInt32 med = 0;
			const UInt16 *qaText;
	
			while (hi-low > 1)
			{
				med = (hi+low)>>1;
				error = m_Input->GetSearchPointText(med, &qaText);
				if (error != eOK)
					return error;

				if (GetCMP()->StrICmp(qaText, aText) < 0)
				{
					low = med;
				}else
				{
					hi = med;
				}
			}

			error = GoToBySearchIndex(low);
			if (error != eOK)
				return error;

			error = GetNumberOfWords(&numberOfWords);
			if (error != eOK)
				return error;

			error = GetCurrentIndex(&currentIndex);
			if (error != eOK)
				return error;

			do{
				error = GetNextWord();
				if (error != eOK)
					return error;

				error = GetCurrentIndex(&currentIndex);
				if (error != eOK)
					return error;

			} while (currentIndex + 1 < numberOfWords && GetCMP()->StrICmp(m_CurrentWord[0], aText) < 0);
		}
	}
	
	if (GetCMP()->IsAddTableDefine())
	{
		// Запоминаем индекс найденного с помощью основной таблицы слова
		Int32 findWordIndex  = currentIndex;

		bool isFound = false;
		
		while (currentIndex + 1 < numberOfWords && GetCMP()->StrICmp(m_CurrentWord[0], aText) == 0)
		{
			if (GetCMP()->StrICmp(m_CurrentWord[0], aText, GetCMP()->GetAddTableIndex()) == 0)
			{
				isFound = true;
				break;
			}
			error = GetNextWord();
			if (error != eOK)
				return error;

			error = GetCurrentIndex(&currentIndex);
			if (error != eOK)
				return error;
		}
		// Если не нашли с помощью дополнительной таблицы сравнения, то отматываемся к найденному с помощью основной таблицы слову
		if (!isFound)
		{
			error = GetWordByIndex(findWordIndex);
			if (error != eOK)
				return error;
		}
	}

	return eOK;
}

/** *********************************************************************
* Ищем наиболее похожее слово, которое равно заданному:
* - среди совпадающих по таблице сравнения слов выбирается совпадающее бинарно
* - если бинарного совпадения не нашлось, выбираем слово с минимальным расстоянием редактирования
*
* Сравнение производится по первому встретившемуся варианту  из возможных:
* - eVariantShow, eVariantShowSecondary (т.к. это варианты, показываемые пользователю)
* - eVariantSortKey (т.к. это вариант для сортировки, следовательно отражающий реально положение статьи в списке);
* приоритет всех вариантов равный
*
* @param[in]	aText		- искомое слово
* @param[out]	aResultFlag	- сюда сохраняется флаг результата:
*							  0 - похожее слово не найдено
*							  1 - похожее слово найдено
*							  2 - мы подмотались к ближайшему слову по виртуальному 
*							    идентификатору (запрос и Show-вариант не совпадают)	
*
* @return код ошибки
************************************************************************/
ESldError CSldList::GetMostSimilarWordByText(const UInt16* aText, UInt32* aResultFlag)
{
	ESldError error;
	
	if (!aResultFlag)
		return eMemoryNullPointer;
	*aResultFlag = 0;
	
	Int32 TextLen = GetCMP()->StrLen(aText);
	if (!TextLen)
		return eOK;
	
	// Если список слов не предназначен для поиска по тексту, выходим
	UInt32 sortFlag = 0;
	error = IsCurrentLevelSorted(&sortFlag);
	if (error != eOK)
		return error;

	if (!sortFlag)
		return eOK;
	
	const CSldListInfo* pListInfo = NULL;
	error = GetWordListInfo(&pListInfo);
	if (error != eOK)
		return error;
	
	error = GetWordByText(aText);
	if (error != eOK)
		return error;

	// Поищем точное вхождение слова
	if(!m_Catalog)
	{
		error = FindBinaryMatch(aText, aResultFlag);
		if (error != eOK)
			return error;

		if(*aResultFlag)
			return eOK;
	}

	// Количество вариантов написания
	UInt32 NumberOfVariants;
	error = GetNumberOfVariants(&NumberOfVariants);
	if (error != eOK)
		return error;
	
	Int32 StartEditDistance = 0xFFFF;
	Int32 MinimumEditDistance = StartEditDistance;
	Int32 BestWordIndex = 0;

	error = GetCurrentIndex(&BestWordIndex);
	if (error != eOK)
		return error;
		
	Int32 FuzzyBuffer[ARRAY_DIM][ARRAY_DIM];
	Int32 PatternLen = TextLen + 1;
	if (PatternLen >= MAX_FUZZY_WORD)
		return eCommonTooLargeText;

	// Инициализация массива
	UInt32 i = 0;
	for (i=0;i<ARRAY_DIM;i++)
	{
		FuzzyBuffer[TIO(i, 0)] = i;
		FuzzyBuffer[TIO(0, i)] = i;
	}
	
	Int32 LowIndex = 0;
	Int32 HighIndex = 0;
	error = GetSearchBounds(eSearchRangeCurrentLevel, &LowIndex, &HighIndex);
	if (error != eOK)
		return error;

	Int32 edit_distance = 0;
	UInt32 maxLocalIndex = HighIndex - LowIndex;

	for (i = BestWordIndex; i < maxLocalIndex;i++)
	{
		error = GetWordByIndex(i);
		if (error != eOK)
			return error;

		bool stop_search = true;
		for (UInt32 v=0;v<NumberOfVariants;v++)
		{
			const EListVariantTypeEnum VariantType = pListInfo->GetVariantType(v);
			if (VariantType == eVariantUnknown)
				return eCommonWrongVariantIndex;
			
			// Вспомогательные варианты написания не учитываем
			if (!(VariantType==eVariantShow || VariantType==eVariantShowSecondary || VariantType==eVariantSortKey))
				continue;
					
			if (GetCMP()->StrICmp(GetWord(v), aText) == 0)
			{
				// Если слово нашлось с помощью дополнительной таблицы сравнения, то прекращаем поиск. 
				// Иначе, продолжаем искать подходящее слово с минимальным расстоянием редактирования.
				if (GetCMP()->IsAddTableDefine())
				{
					if (GetCMP()->StrICmp(GetWord(v), aText, GetCMP()->GetAddTableIndex()) == 0)
					{
						
						*aResultFlag = 1;
						return eOK;
					}
				}
				// Дополнительной таблицы сравнения просто нет. Поэтому продолжаем поиск наиболее похожих слов.
				stop_search = false;
				break;
			}
		}
		
		if (stop_search)
			break;
		
		for (UInt32 v=0;v<NumberOfVariants;v++)
		{
			const EListVariantTypeEnum VariantType = pListInfo->GetVariantType(v);
			if (VariantType == eVariantUnknown)
				return eCommonWrongVariantIndex;
			
			// Вспомогательные варианты написания не учитываем
			if (!(VariantType==eVariantShow || VariantType==eVariantShowSecondary || VariantType==eVariantSortKey))
				continue;
			
			// Если слово бинарно совпадает, то прекращаем поиск
			if (GetCMP()->StrCmp(GetWord(v), aText) == 0)
			{
				error = GetWordByIndex(i);
				if (error != eOK)
					return error;

				*aResultFlag = 1;
				return eOK;
			}
			
			// Ищем наиболее похожее слово без учета разделителей, но с учетом регистра, ударений, диакритиков и пр.
			Int32 len = GetCMP()->StrLen(GetWord(v));

			UInt16* currentWordWithoutDelimiters = NULL;
			currentWordWithoutDelimiters = (UInt16*)sldMemNew((len + 1)* sizeof(currentWordWithoutDelimiters[0]));
			if (!currentWordWithoutDelimiters)
					return eMemoryNotEnoughMemory;

			GetCMP()->StrEffectiveCopy(currentWordWithoutDelimiters, GetWord(v));
			UInt32 lenWithoutDelimiters = GetCMP()->StrLen(currentWordWithoutDelimiters);

			if (len < ARRAY_DIM)
				edit_distance = GetCMP()->FuzzyCompare(currentWordWithoutDelimiters, aText, lenWithoutDelimiters + 1, PatternLen, FuzzyBuffer);
			else
				edit_distance = len;
			
			if (edit_distance < MinimumEditDistance)
			{
				MinimumEditDistance = edit_distance;
				BestWordIndex = i;
			}

			sldMemFree(currentWordWithoutDelimiters);
		}
	}
	
	error = GetWordByIndex(BestWordIndex);
	if (error != eOK)
		return error;
	
	if (MinimumEditDistance != StartEditDistance)
		*aResultFlag = 1;
	
	return eOK;
}

/** ********************************************************************
* Метод декодирует следующее слово в списке слов. 
*
* @return код ошибки
************************************************************************/
ESldError CSldList::GetNextWord(void)
{
	UInt16 shiftBuf[2] = {0};
	ESldError error;

	const TListHeader* pListHeader = GetListInfo()->GetHeader();

	//////////////////////////////////////////////////////////////////////////
	// Получаем слова
	//////////////////////////////////////////////////////////////////////////
	if (pListHeader->Version == VERSION_LIST_BASE)
	{
		error = m_Input->GetText(SLD_DECORER_TYPE_SHIFT, sld2::make_span(shiftBuf));
		if (error != eOK)
			return error;

		if (shiftBuf[0] >= pListHeader->MaximumWordSize)
			return eCommonWrongShiftSize;
	}
	else
	{
		UInt32 shift;
		error = GetShiftByIndex(m_CurrentIndex + 1, &shift);
		if (error != eOK)
			return error;

		if(shift == m_Input->GetCurrentPosition())
		{
			m_AltNameManager.m_LastShift = -1;
		}
		else
		{
			m_AltNameManager.m_LastShift = m_Input->GetCurrentPosition();
			return m_Input->GetText(SLD_DECORER_TYPE_TEXT, m_AltNameManager.m_AlternativeWord);
		}
	}

	UInt16 *decodedWord = m_CurrentWord[0] + shiftBuf[0];
	for (UInt32 i=0;i<pListHeader->NumberOfVariants;i++)
	{
		if (i)
			decodedWord = m_CurrentWord[i];

		error = m_Input->GetText(SLD_DECORER_TYPE_TEXT, sld2::make_span(decodedWord, pListHeader->MaximumWordSize));
		if (error != eOK)
			return error;

		if(pListHeader->IsStylizedVariant)
		{
			UInt32 isStylizedVariant;
			m_Input->GetData(&isStylizedVariant, 1);
			if (isStylizedVariant)
			{
				m_Input->GetData((UInt32 *)&m_StylizedVariant[i], pListHeader->SizeOfArticleIndex);
			}
			else
			{
				m_StylizedVariant[i] = SLD_INDEX_STYLIZED_VARIANT_NO;
			}
		}
	}

	// Получаем номера "медиа ресурсов" для текущего слова
	const struct {
		bool active;
		Int32 *ptr;
		UInt32 size;
		Int32 deflt;
	} indices[] = {
		{ !!pListHeader->IsPicture, &m_PictureIndex, pListHeader->PictureIndexSize, SLD_INDEX_PICTURE_NO },
		{ !!pListHeader->IsSound,   &m_SoundIndex,   pListHeader->SoundIndexSize,   SLD_INDEX_SOUND_NO },
		{ !!pListHeader->IsVideo,   &m_VideoIndex,   pListHeader->VideoIndexSize,   SLD_INDEX_VIDEO_NO },
		{ !!pListHeader->IsScene,   &m_SceneIndex,   pListHeader->SceneIndexSize,   SLD_INDEX_SCENE_NO },
	};
	for (auto&& index : indices)
	{
		if (!index.active)
			continue;

		UInt32 hasIndex;
		m_Input->GetData(&hasIndex, 1);
		if (hasIndex)
			m_Input->GetData((UInt32*)index.ptr, index.size);
		else
			*index.ptr = index.deflt;
	}

	m_CurrentIndex++;

	if (m_QAShift.IsInit)
	{
		error = m_QAShift.AddCashedPoint(shiftBuf[0], m_CurrentWord[0], m_Input->GetCurrentPosition(), m_CurrentIndex + 1);
		if (error != eOK)
			return error;
	}

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
ESldError CSldList::GetCurrentWord(Int32 aVariantIndex, UInt16** aWord)
{
	if (!aWord)
		return eMemoryNullPointer;
		
	if ((UInt32)aVariantIndex >= GetListInfo()->GetNumberOfVariants() || aVariantIndex < 0)
		return eCommonWrongIndex;

	*aWord = m_CurrentWord[aVariantIndex];

	return eOK;
}

/** ********************************************************************
*  Возвращает указатель на eVariantLabel последнего найденного слова
*
* @param[out]	aLabel			- указатель на буфер для указателя на слово
*
* @return код ошибки
************************************************************************/
ESldError CSldList::GetCurrentWordLabel(UInt16** aLabel)
{
	if (!aLabel)
		return eMemoryNullPointer;

	const CSldListInfo* pListInfo = NULL;
	ESldError error = GetWordListInfo(&pListInfo);
	if (error != eOK)
		return error;

	const Int32 index = pListInfo->GetVariantIndexByType(eVariantLabel);
	if (index == SLD_DEFAULT_VARIANT_INDEX)
		return eOK;

	return GetCurrentWord(index, aLabel);
}

/** *********************************************************************
* Получает локальный номер текущего слова
* (номер слова в пределах текущего списка слов и текущего уровня вложенности)
*
* @param[out]	aIndex	- указатель на переменную, в которую следует поместить 
*						  номер текущего слова
*
* @return код ошибки
************************************************************************/
ESldError CSldList::GetCurrentIndex(Int32* aIndex) const
{
	if (!m_IsCatalogSynchronized)
		return eCommonCatalogIsNotSynchronizedError;

	if (!aIndex)
		return eMemoryNullPointer;
	
	*aIndex = GetCurrentWordIndex() - GetBaseIndex();

    if (m_LocalizedBaseGlobalIndex != SLD_DEFAULT_LIST_INDEX)
        *aIndex -= m_LocalizedBaseGlobalIndex;
	
	return eOK;
}


/** *********************************************************************
* Получает глобальный номер текущего слова (номер слова в пределах всего списка слов)
*
* @param[out]	aIndex	- указатель на переменную, в которую следует поместить
*						  номер текущего слова
*
* @return код ошибки
************************************************************************/
ESldError CSldList::GetCurrentGlobalIndex(Int32* aIndex) const
{
	if (!aIndex)
		return eMemoryNullPointer;

	*aIndex = GetCurrentWordIndex();

	if (m_LocalizedBaseGlobalIndex != SLD_DEFAULT_LIST_INDEX)
		*aIndex -= m_LocalizedBaseGlobalIndex;

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
ESldError CSldList::LocalIndex2GlobalIndex(Int32 aLocalIndex, Int32* aGlobalIndex)
{
	if (!m_IsCatalogSynchronized)
		return eCommonCatalogIsNotSynchronizedError;

	ESldError error;
	
	if (!aGlobalIndex)
		return eMemoryNullPointer;
	
	Int32 NumberOfWords;
	error = GetNumberOfWords(&NumberOfWords);
	if (error != eOK)
		return error;
	
	if (aLocalIndex < 0 || aLocalIndex >= NumberOfWords)
		return eCommonWrongIndex;
	
	*aGlobalIndex = aLocalIndex + GetBaseIndex();
	
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
ESldError CSldList::GetRealListIndex(Int32 aLocalWordIndex, Int32* aListIndex)
{
	if (!aListIndex)
		return eMemoryNullPointer;

	ESldError error;
	Int32 globalIndex = aLocalWordIndex;

	if (m_Catalog)
	{
		error = LocalIndex2GlobalIndex(aLocalWordIndex, &globalIndex);
		if (error != eOK)
			return error;
	}

	Int32 linkCount = 0;
	error = GetTranslationCount(globalIndex, &linkCount);
	if (error != eOK)
		return error;

	if (!linkCount)
	{
		*aListIndex = SLD_DEFAULT_LIST_INDEX;
		return eOK;
	}

	Int32 listEntryIndex = SLD_DEFAULT_WORD_INDEX;
	Int32 translationIndex = 0;
	Int32 shiftIndex = 0;

	return GetFullTextTranslationData(globalIndex, 0, aListIndex, &listEntryIndex, &translationIndex, &shiftIndex);
}

/** *********************************************************************
* Получает глобальный номер слова по локальному номеру слова
*
* @param[in]	aLocalWordIndex		- локальный номер слова
* @param[out]	aGlobalWordIndex	- указатель на переменную, в которую сохраняется глобальный номер слова
*
* @return код ошибки
************************************************************************/
ESldError CSldList::GetRealGlobalIndex(Int32 aLocalWordIndex, Int32* aGlobalWordIndex)
{
	if (!aGlobalWordIndex)
		return eMemoryNullPointer;

	ESldError error;
	Int32 globalIndex = aLocalWordIndex;

	if (m_Catalog)
	{
		error = LocalIndex2GlobalIndex(aLocalWordIndex, &globalIndex);
		if (error != eOK)
			return error;
	}

	Int32 linkCount = 0;
	error = GetTranslationCount(globalIndex, &linkCount);
	if (error != eOK)
		return error;

	if (!linkCount)
	{
		*aGlobalWordIndex = SLD_DEFAULT_WORD_INDEX;
		return eOK;
	}

	Int32 listIndex = SLD_DEFAULT_LIST_INDEX;
	Int32 translationIndex = 0;
	Int32 shiftIndex = 0;

	return GetFullTextTranslationData(globalIndex, 0, &listIndex, aGlobalWordIndex, &translationIndex, &shiftIndex);
}

/** *********************************************************************
* Возвращает полное количество слов в списке слов
*
* @param[out]	aCount	- указатель на переменную, в которую сохраняется полное количество слов в списке слов
*
* @return код ошибки
************************************************************************/
ESldError CSldList::GetTotalWordCount(Int32* aCount)
{
	if (!aCount)
		return eMemoryNullPointer;
	*aCount = 0;
	
	if (!GetListInfo())
		return eMemoryNullPointer;
	
	UInt32 Count = 0;
	ESldError error = GetNumberOfWordsAtCurrentLevel(0, &Count);
	if (error != eOK)
		return error;
	
	*aCount = (Int32)Count;
	return eOK;
}

/** *********************************************************************
* Возвращает количество слов у списка слов на текущем уровне вложенности
*
* @param[out]	aNumberOfWords	- указатель на буфер для количества слов
*
* @return код ошибки
************************************************************************/
ESldError CSldList::GetNumberOfWords(Int32* aNumberOfWords) const
{
	if (!m_IsCatalogSynchronized)
		return eCommonCatalogIsNotSynchronizedError;

	if (!aNumberOfWords)
		return eMemoryNullPointer;

	ESldError error;
	if (!m_Catalog)
	{
		*aNumberOfWords = GetListInfo()->GetNumberOfGlobalWords();
		error = eOK;
	}
	else
	{
		error = m_Catalog->GetNumberOfWords(aNumberOfWords);
	}

	return error;
}

/** *********************************************************************
* Возвращает количество вариантов написания для списка слов
*
* @param[out]	aNumberOfVariants	- указатель на буфер для количества вариантов
*
* @return код ошибки
************************************************************************/
ESldError CSldList::GetNumberOfVariants(UInt32* aNumberOfVariants)
{
	if (!aNumberOfVariants)
		return eMemoryNullPointer;
	
	if (!GetListInfo())
		return eMemoryNullPointer;
	
	*aNumberOfVariants = GetListInfo()->GetNumberOfVariants();
		
	return eOK;
}

/** ********************************************************************
* Устанавливаем базу для списка слов 
*
* @param[in]	aBase	- Номер текущего слова которое должно будет стать базой.
*						  -1 - сбросить базу.
*
* @return код ошибки
************************************************************************/
ESldError CSldList::SetBase(Int32 aBase)
{
	// флаг того, что переход внутрь некоторой папки является выбором локализации
	UInt16 applyingLocalization = false;
	Int32 NumberOfWords = 0;
	ESldError error;

	UInt8 savedFlag = m_IsCatalogSynchronized;
	// после успешного выполнения метода состояние каталога и базовый индекс синхронизированы c текущим индексом списка
	m_IsCatalogSynchronized = 1;

	Int32 baseIndex = aBase;
	if (aBase == MAX_UINT_VALUE)
	{
		m_Path.Clear();
		m_BaseIndex = 0;
		
		if (m_Catalog)
		{
			error = m_Catalog->SetBaseByIndex(aBase);
			if (error != eOK)
			{
				m_IsCatalogSynchronized = savedFlag;
				return error;
			}

			if (m_LocalizedBaseIndex == SLD_DEFAULT_LIST_INDEX)
			{
				error = GetWordByIndex(0);
				if (error != eOK)
				{
					m_IsCatalogSynchronized = savedFlag;
					return error;
				}
				return eOK;
			}

			applyingLocalization = true;
			baseIndex = m_LocalizedBaseIndex;
		}
		else
			return eOK;
	}

	error = GetNumberOfWords(&NumberOfWords);
	if (error != eOK)
	{
		m_IsCatalogSynchronized = savedFlag;
		return error;
	}
	if (baseIndex >= NumberOfWords)
	{
		m_IsCatalogSynchronized = savedFlag;
		return eCommonWrongIndex;
	}

	if (m_Catalog)
	{
		UInt32 newWordListBase = 0;
		error = m_Catalog->GetBaseByIndex(baseIndex, &newWordListBase);
		if (error != eOK)
		{
			m_IsCatalogSynchronized = savedFlag;
			return error;
		}

		error = m_Catalog->SetBaseByIndex(baseIndex);
		if (error != eOK)
		{
			m_IsCatalogSynchronized = savedFlag;
			return error;
		}

		// Добавляем элемент в путь.
		if (!applyingLocalization)
		{
			error = m_Path.PushList(baseIndex);
			if (error != eOK)
				return error;
		}

		m_BaseIndex = newWordListBase;
		
		error = GetWordByIndex(0);
		if (error != eOK)
		{
			m_IsCatalogSynchronized = savedFlag;
			return error;
		}
	}

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
ESldError CSldList::GetWordListInfo(const CSldListInfo** aListInfo, Int32 aDictIndex) const
{
	if (!aListInfo)
		return eMemoryNullPointer;
	
	if (!GetListInfo())
		return eMemoryNullPointer;

	*aListInfo = GetListInfo();
	
	return eOK;
}

/** *********************************************************************
* Возвращает количество кодов использования списка слов
*
* @param[out]	aCount	- указатель на переменную, в которую будет сохранен результат
*
* @return код ошибки
************************************************************************/
ESldError CSldList::GetUsageCount(Int32* aCount) const
{
	if (!aCount)
		return eMemoryNullPointer;
	
	*aCount = 1;

	return eOK;
}

/** *********************************************************************
* Возвращает код использования списка слов по индексу кода
*
* @param[in]	aIndex	- индекс кода использования
* @param[out]	aUsage	- указатель на переменную, в которую будет сохранен код
*
* @return код ошибки
************************************************************************/
ESldError CSldList::GetListUsage(Int32 aIndex, UInt32* aUsage) const
{
	if (!aUsage)
		return eMemoryNullPointer;
	
	if (aIndex)
		return eCommonWrongIndex;
	
	if (!GetListInfo())
		return eMemoryNullPointer;

	*aUsage = GetListInfo()->GetUsage();
	return eOK;
}

/** *********************************************************************
* Возвращает флаг того, сортированный или нет данный список слов
*
* @param[out]	aIsSorted	- указатель на переменную, в которую будет возвращен флаг
*
* @return код ошибки
************************************************************************/
ESldError CSldList::IsListSorted(UInt32* aIsSorted)
{
	if (!aIsSorted)
		return eMemoryNullPointer;
	
	if (!GetListInfo())
		return eMemoryNullPointer;

	*aIsSorted = GetListInfo()->IsSortedList();
	return eOK;
}

/** *********************************************************************
* Возвращает флаг того, сопоставлены или нет некоторым словам из списка слов картинки
*
* @param[out]	aIsPicture	- указатель на переменную, в которую будет возвращен флаг
*
* @return код ошибки
************************************************************************/
ESldError CSldList::IsListHasPicture(UInt32* aIsPicture)
{
	if (!aIsPicture)
		return eMemoryNullPointer;

	if (!GetListInfo())
		return eMemoryNullPointer;

	*aIsPicture = GetListInfo()->IsPicture();
	return eOK;
}

/** *********************************************************************
* Возвращает флаг того, сопоставлены или нет некоторым словам из списка слов видео
*
* @param[out]	aIsVideo	- указатель на переменную, в которую будет возвращен флаг
*
* @return код ошибки
************************************************************************/
ESldError CSldList::IsListHasVideo(UInt32* aIsVideo)
{
	if (!aIsVideo)
		return eMemoryNullPointer;

	if (!GetListInfo())
		return eMemoryNullPointer;

	*aIsVideo = GetListInfo()->IsVideo();
	return eOK;
}

/** *********************************************************************
* Возвращает флаг того, сопоставлены или нет некоторым словам из списка слов 3d сцены
*
* @param[out]	aIsScene	- указатель на переменную, в которую будет возвращен флаг
*
* @return код ошибки
************************************************************************/
ESldError CSldList::IsListHasScene(UInt32* aIsScene)
{
	if (!aIsScene)
		return eMemoryNullPointer;

	if (!GetListInfo())
		return eMemoryNullPointer;

	*aIsScene = GetListInfo()->IsScene();
	return eOK;
}

/** *********************************************************************
* Возвращает флаг того, сопоставлена или нет некоторым словам из списка слов озвучка
*
* @param[out]	aIsSound	- указатель на переменную, в которую будет возвращен флаг
*
* @return код ошибки
************************************************************************/
ESldError CSldList::IsListHasSound(UInt32* aIsSound)
{
	if (!aIsSound)
		return eMemoryNullPointer;

	if (!GetListInfo())
		return eMemoryNullPointer;

	*aIsSound = GetListInfo()->IsSound();
	return eOK;
}

/** ********************************************************************
* Возвращает флаг того, что указанное слово имеет или нет поддерево иерархии.
*
* @param[in]	aIndex			- номер слова на текущем уровне иерархии
* @param[out]	aIsHierarchy	- указатель на переменную в которую будет возвращен флаг.
* @param[out]	aLevelType		- указатель, по которому будет записан тип поддерева (см #EHierarchyLevelType)
*								  (можно передать NULL)
*
* @return код ошибки
************************************************************************/
ESldError CSldList::isWordHasHierarchy(Int32 aIndex, UInt32* aIsHierarchy, EHierarchyLevelType* aLevelType)
{
	if (!m_IsCatalogSynchronized)
		return eCommonCatalogIsNotSynchronizedError;

	if (!aIsHierarchy)
		return eMemoryNullPointer;
	if (!m_Catalog)
	{
		*aIsHierarchy = 0;
		return eOK;
	}

	ESldError error;
	UInt32 tmpBase;
	error = m_Catalog->GetBaseByIndex(aIndex, &tmpBase);
	if (error != eOK)
		return error;

	if (tmpBase == MAX_UINT_VALUE)
		*aIsHierarchy = 0;
	else
		*aIsHierarchy = 1;

	if (aLevelType)
	{
		if (*aIsHierarchy)
		{
			error = m_Catalog->SetBaseByIndex(aIndex);
			if (error != eOK)
				return error;

			*aLevelType = m_Catalog->GetLevelType();

			error = GoToByGlobalIndex(GetCurrentWordIndex());
			if (error != eOK)
				return error;
		}
		else
		{
			*aLevelType = eLevelTypeNormal;
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
ESldError CSldList::GetPictureIndex(CSldVector<Int32> & aPictureIndexes)
{
	if (m_PictureIndex == SLD_INDEX_PICTURE_NO)
		return eOK;

	aPictureIndexes.push_back(m_PictureIndex);
	if (m_MergedDictInfo)
	{
		aPictureIndexes.back() += m_MergedDictInfo->MetaInfo->PicturiesCount[m_MergedDictInfo->DictIndex];
	}

	return eOK;
}

/** *********************************************************************
* Возвращает номер видео текущего слова
*
* @param[out]	aVideoIndex - указатель на переменную, в которую будет записан номер видео
*
* @return код ошибки
************************************************************************/
ESldError CSldList::GetVideoIndex(Int32* aVideoIndex)
{
	if (!aVideoIndex)
		return eMemoryNullPointer;

	*aVideoIndex = m_VideoIndex;

	return eOK;
}

/** *********************************************************************
* Возвращает вектор озвучек текущего слова
*
* @param[out]	aSoundIndexes - указатель на переменную, в которую будет записан номер озвучки
*
* @return код ошибки
************************************************************************/
ESldError CSldList::GetSoundIndex(CSldVector<Int32> & aSoundIndexes)
{
	if (m_SoundIndex == SLD_INDEX_SOUND_NO)
		return eOK;

	aSoundIndexes.push_back(m_SoundIndex);
	if (m_MergedDictInfo)
	{
		aSoundIndexes.back() += m_MergedDictInfo->MetaInfo->SoundsCount[m_MergedDictInfo->DictIndex];
	}

	return eOK;
}

/** *********************************************************************
* Возвращает номер 3d сцены текущего слова
*
* @param[out]	aSceneIndex - указатель на переменную, в которую будет записан номер 3d сцены
*
* @return код ошибки
************************************************************************/
ESldError CSldList::GetSceneIndex(Int32* aSceneIndex)
{
	if (!aSceneIndex)
		return eMemoryNullPointer;

	*aSceneIndex = m_SceneIndex;

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
ESldError CSldList::GetTranslationCount(Int32 aGlobalIndex, Int32* aTranslationCount)
{
	if (!m_IsCatalogSynchronized)
		return eCommonCatalogIsNotSynchronizedError;

	if (!aTranslationCount)
		return eMemoryNullPointer;

	const CSldListInfo* listInfo = GetListInfo();

	// Если индексов нет, значит у нас список слов напрямую соответствует переводам.
	// Это значит, что на каждое слово 1 перевод.
	if (listInfo->IsDirectList())
	{
		*aTranslationCount = 1;
		return eOK;
	}
	
	if (!m_Indexes)
		return eMemoryNullPointer;
	
	if (!listInfo->GetHeader()->SizeOfNumberOfArticles)
	{
		*aTranslationCount = 0;
		return eOK;
	}

	if (listInfo->GetUsage() == eWordListType_Atomic)
	{
		*aTranslationCount = 1;
		return eOK;
	}
	else
	{
		if (m_LocalizedBaseGlobalIndex != SLD_DEFAULT_WORD_INDEX)
			aGlobalIndex += m_LocalizedBaseGlobalIndex;

		ESldError error = m_Indexes->GetNumberOfArticlesByIndex(aGlobalIndex, aTranslationCount);
		if (error != eOK)
			return error;
	}

	return eOK;
}

/** ********************************************************************
* Возвращает количество ссылок у указанного слова из списка слов
*
* @param[in]	aGlobalIndex		- номер слова из списка слов для которого требуется узнать
*									  количество переводов
* @param[out]	aTranslationCount	- указатель на переменную, в которую будет помещено
*									  количество переводов у последнего декодированного слова.
*
* @return код ошибки
************************************************************************/
ESldError CSldList::GetReferenceCount(Int32 aGlobalIndex, Int32* aTranslationCount)
{
	if (!m_IsCatalogSynchronized)
		return eCommonCatalogIsNotSynchronizedError;

	if (!aTranslationCount)
		return eMemoryNullPointer;

	const TListHeader* pHeader = GetListInfo()->GetHeader();
	if (!pHeader)
		return eMemoryNullPointer;

	if (!pHeader->SizeOfNumberOfArticles || !pHeader->IsFullTextSearchList)
	{
		*aTranslationCount = 0;
		return eOK;
	}
	else if (!m_Indexes)
	{
		*aTranslationCount = 1;
		return eOK;
	}

	if (m_LocalizedBaseGlobalIndex != SLD_DEFAULT_WORD_INDEX)
		aGlobalIndex += m_LocalizedBaseGlobalIndex;

	ESldError error = m_Indexes->GetNumberOfArticlesByIndex(aGlobalIndex, aTranslationCount);
	if (error != eOK)
		return error;

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
ESldError CSldList::GetTranslationIndex(Int32 aGlobalIndex, Int32 aTranslationIndex, Int32* aArticleIndex)
{
	if (!m_IsCatalogSynchronized)
		return eCommonCatalogIsNotSynchronizedError;

	if (!aArticleIndex)
		return eMemoryNullPointer;
	
	ESldError error;
		
	// Если список слов имеет прямое отображение
	if (GetListInfo()->IsDirectList())
	{
		// тогда номер перевода совпадает с запрашиваемым индексом
		Int32 NumberOfWords;
		error = GetNumberOfWords(&NumberOfWords);
		if (error != eOK)
			return error;
		if (aGlobalIndex >= NumberOfWords)
			return eCommonWrongIndex;
			
		*aArticleIndex = aGlobalIndex;

		if (m_MergedDictInfo)
		{
			*aArticleIndex += m_MergedDictInfo->MetaInfo->ArticlesCount[m_MergedDictInfo->DictIndex];
		}

		return eOK;
	}
	
	if (!m_Indexes)
		return eMemoryNullPointer;
	
	Int32 ListIndex;
	Int32 TranslationIndex;
	Int32 ShiftIndex;

	if (m_LocalizedBaseGlobalIndex != SLD_DEFAULT_WORD_INDEX)
		aGlobalIndex += m_LocalizedBaseGlobalIndex;

	error = m_Indexes->GetIndexData(aGlobalIndex, aTranslationIndex, &ListIndex, aArticleIndex, &TranslationIndex, &ShiftIndex);
	if (error != eOK)
		return error;

	if (m_MergedDictInfo)
	{
		*aArticleIndex += m_MergedDictInfo->MetaInfo->ArticlesCount[m_MergedDictInfo->DictIndex];
	}

	return eOK;
}

/** *********************************************************************
* Возвращает данные перевода по номеру слова и номеру перевода в списке слов полнотекстового поиска
*
* @param[in]	aGlobalIndex			- глобальный номер слова в списке слов
* @param[in]	aWordTranslationIndex	- номер перевода слова (от 0 до количества переводов, приходящихся на данное слово)
* @param[out]	aListIndex				- указатель на переменную, куда будет сохранен номер списка слов, на который ссылается слово из списка слов
* @param[out]	aListEntryIndex			- указатель на переменную, куда будет сохранен номер слова в списке слов
* @param[out]	aTranslationIndex		- указатель на переменную, куда будет сохранен номер перевода
* @param[out]	aShiftIndex				- указатель на переменную, куда будет сохранено смещение
*
* @return код ошибки
************************************************************************/
ESldError CSldList::GetFullTextTranslationData(Int32 aGlobalIndex, Int32 aWordTranslationIndex, Int32* aListIndex, Int32* aListEntryIndex, Int32* aTranslationIndex, Int32* aShiftIndex)
{
	if (!m_IsCatalogSynchronized)
		return eCommonCatalogIsNotSynchronizedError;

	if (!aListIndex || !aListEntryIndex || !aTranslationIndex || !aShiftIndex)
		return eMemoryNullPointer;
	
	ESldError error;
		
	if (!m_Indexes)
		return eMemoryNullPointer;
	
	if (m_LocalizedBaseGlobalIndex != SLD_DEFAULT_WORD_INDEX)
		aGlobalIndex += m_LocalizedBaseGlobalIndex;

	error = m_Indexes->GetIndexData(aGlobalIndex, aWordTranslationIndex, aListIndex, aListEntryIndex, aTranslationIndex, aShiftIndex);
	if (error != eOK)
		return error;
		
	return eOK;
}

/** ********************************************************************
* Производит переход по указанному поисковому номеру.
*
* @param[in]	aSearchIndex - номер записи в таблице быстрого поиска.
*
* @return код ошибки
************************************************************************/
ESldError CSldList::GoToBySearchIndex(Int32 aSearchIndex)
{
	ESldError error;
	error = m_Input->GoTo(aSearchIndex);
	if (error != eOK)
		return error;

	error = m_Input->GetSearchPointIndex(aSearchIndex, &m_CurrentIndex);
	if (error != eOK)
		return error;

	error = m_Input->GetSearchPointText(aSearchIndex, m_CurrentWord[0]);
	if (error != eOK)
		return error;

	if (m_QAShift.IsInit)
	{
		m_QAShift.SetNewQAPoint(aSearchIndex, m_CurrentWord[0], m_Input->GetCurrentPosition(), GetCurrentWordIndex() + 1);
		if (error != eOK)
			return error;
	}

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
ESldError CSldList::GetCurrentPath(TCatalogPath* aPath)
{
	if (!aPath)
		return eMemoryNullPointer;
	
	ESldError error;
	Int32 GlobalWordIndex = 0;
	error = GetCurrentGlobalIndex(&GlobalWordIndex);
	if (error != eOK)
		return error;
	
	error = GetPathByGlobalIndex(GlobalWordIndex, aPath);
	if (error != eOK)
		return error;
	
	/*
	* Старый вариант: копируются значения из m_Path, который заполняется при вызове функций SetBase().
	* Так как класс CSldSearchList использует функции данного класса CSldList напрямую, значения в m_Path
	* при вызовах из поискового списка могут не всегда соответствовать текущему глобальному индексу,
	* поэтому теперь путь вычисляется через глобальный индекс.
	
	Int32 LocalWordIndex = 0;
	ESldError error;
	
	error = GetCurrentIndex(&LocalWordIndex);
	if (error != eOK)
		return error;
	
	aPath->BaseListMax = aPath->BaseListCount = m_Path.BaseListCount + 1;
	aPath->BaseList = (UInt32*)sldMemNew(aPath->BaseListMax*sizeof(aPath->BaseList[0]));
	if (!aPath->BaseList)
		return eMemoryNotEnoughMemory;

	if (m_Path.BaseListCount)
	{
		sldMemMove(aPath->BaseList, m_Path.BaseList, m_Path.BaseListCount*sizeof(m_Path.BaseList[0]));
	}
	
	aPath->BaseList[aPath->BaseListCount-1] = (UInt32)LocalWordIndex;
	*/
	
	return eOK;
}

/** *********************************************************************
* Возвращает путь в каталоге к элементу с глобальным номером
* (т.е. "прямой" номер слова без учета иерархии)
*
* @param[in]	aIndex	- номер слова без учета иерархии
* @param[out]	aPath	- указатель на структуру, в которую будет прописан путь к 
*						  указанному элементу списка слов
*
* @return код ошибки
************************************************************************/
ESldError CSldList::GetPathByGlobalIndex(Int32 aIndex, TCatalogPath* aPath)
{
	if (!aPath)
		return eMemoryNullPointer;
	
	ESldError error;
	if (m_Catalog)
	{
		// учтем то, что мы можем находиться внутри локализованной категории
		if (m_LocalizedBaseGlobalIndex != SLD_DEFAULT_WORD_INDEX)
			aIndex += m_LocalizedBaseGlobalIndex;

		error = m_Catalog->GetPathByGlobalIndex(aIndex, aPath);
		if (error != eOK)
			return error;

		// если мы пользуемся локализованным списком слов, то категория верхнего уровня - скрытые настройки локализации, удалим их из пути
		if (aPath->BaseListCount && m_LocalizedBaseIndex != SLD_DEFAULT_LIST_INDEX)
			aPath->popFront();

		return eOK;
	}
	
	aPath->Clear();

	if (!GetListInfo())
		return eMemoryNullPointer;

	if (aIndex >= (Int32)GetListInfo()->GetNumberOfGlobalWords())
		return eCommonWrongIndex;
	
	error = aPath->PushList(aIndex);
	if (error != eOK)
		return error;
	
	return eOK;
}

/** *********************************************************************
* Переходит по указанному пути
*
* @param[out]	aPath			- указатель на структуру, в которой содержится путь к месту 
*								  в каталоге, куда нужно попасть
* @param[in]	aNavigationType	- тип перехода
*
* @return код ошибки
************************************************************************/
ESldError CSldList::GoToByPath(const TCatalogPath* aPath, ESldNavigationTypeEnum aNavigationType)
{
	if (!aPath)
		return eMemoryNullPointer;
	
	ESldError error;
	
	// Переходить некуда
	if (!aPath->BaseListCount)
		return eOK;

	// Переходим в самое начало
	error = SetBase(MAX_UINT_VALUE);
	if (error != eOK)
		return error;
	
	// Переходим на уровень, на котором находится искомое слово
	for (UInt32 i=0;i<aPath->BaseListCount-1;i++)
	{
		error = SetBase(aPath->BaseList[i]);
		if (error != eOK)
			return error;
	}
	
	// Последнее число в списке - номер слова
	Int32 LocalWordIndex = (Int32)aPath->BaseList[aPath->BaseListCount-1];
	
	// Имеет ли слово уровни вложенности
	UInt32 HasHierarchy = 0;
	error = isWordHasHierarchy(LocalWordIndex, &HasHierarchy, NULL);
	if (error != eOK)
		return error;
	
	switch (aNavigationType)
	{
		case eGoToWord:
		{
			// Переходим на нужное слово, не важно, обычная это статья или подраздел
			error = GetWordByIndex(LocalWordIndex);
			if (error != eOK)
				return error;
				
			break;
		}
		case eGoToSubSection:
		{
			if (HasHierarchy)
			{
				// Заходим в подраздел
				error = SetBase(LocalWordIndex);
				if (error != eOK)
					return error;
			}
			else
			{	// Заходить некуда, переходим на само слово
				error = GetWordByIndex(LocalWordIndex);
				if (error != eOK)
					return error;
			}
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
ESldError CSldList::GoToLevelUp(void)
{
	ESldError error;
	TCatalogPath Path;
	
	error = GetCurrentPath(&Path);
	if (error != eOK)
		return error;
	
	if (Path.isRoot())
		return eOK;
	
	error = SetBase(MAX_UINT_VALUE);
	if (error != eOK)
		return error;
	
	UInt32 wordIndex = Path.BaseList[Path.BaseListCount-2];

	for (UInt32 i=0;i<Path.BaseListCount-2;i++)
	{
		error = SetBase(Path.BaseList[i]);
		if (error != eOK)
			return error;
	}
	
	error = GetWordByIndex(wordIndex);
	if (error != eOK)
		return error;

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
ESldError CSldList::GetParentWordNamesByGlobalIndex(Int32 aGlobalWordIndex, UInt16** aText, const UInt16* aSeparatorText, Int32 aMaxLevelCount)
{
	if (!aText)
		return eMemoryNullPointer;
	*aText = NULL;
	
	ESldError error;
	TCatalogPath Path;
	TCatalogPath CurrPath;
	Int32 CurrGlobalIndex = 0;
	UInt16* Word = 0;
	
	error = GetCurrentGlobalIndex(&CurrGlobalIndex);
	if (error != eOK)
		return error;
	
	error = GetPathByGlobalIndex(CurrGlobalIndex, &CurrPath);
	if (error != eOK)
		return error;
	
	error = GetPathByGlobalIndex(aGlobalWordIndex, &Path);
	if (error != eOK)
		return error;
	
	Int32 StartIdx = 0;
	Int32 LastIdx = 0;
	// Есть родительские категории
	if (!Path.isRoot())
	{
		LastIdx = Path.BaseListCount - 1;
		if (!(aMaxLevelCount == -1 || aMaxLevelCount > Path.BaseListCount-1))
		{
			StartIdx = Path.BaseListCount - 1 - aMaxLevelCount;
		}
	}
	
	error = SetBase(MAX_UINT_VALUE);
	if (error != eOK)
		return error;
	
	for (Int32 i=0;i<StartIdx;i++)
	{
		error = SetBase(Path.BaseList[i]);
		if (error != eOK)
			return error;
	}
	
	Int32 ResultMaxSize = 32;
	*aText = (UInt16*)sldMemNewZero(ResultMaxSize*sizeof((*aText)[0]));
	if (!(*aText))
		return eMemoryNotEnoughMemory;

	// Номер варианта написания для отображения
	Int32 VariantIndex = GetListInfo()->GetVariantIndexByType(eVariantShow);
	if (VariantIndex == SLD_DEFAULT_VARIANT_INDEX)
		VariantIndex = 0;

	for (Int32 i=StartIdx;i<LastIdx;i++)
	{
		error = GetWordByIndex(Path.BaseList[i]);
		if (error != eOK)
			return error;
		
		error = GetCurrentWord(VariantIndex, &Word);
		if (error != eOK)
			return error;
		
		Int32 NeedSize = GetCMP()->StrLen(*aText) + GetCMP()->StrLen(Word) + 1;
		if (aSeparatorText)
			NeedSize += GetCMP()->StrLen(aSeparatorText);
		
		while (ResultMaxSize < NeedSize)
		{
			ResultMaxSize *= 2;
			UInt16* tmpText = (UInt16*)sldMemNewZero(ResultMaxSize*sizeof(tmpText[0]));
			if (!tmpText)
				return eMemoryNotEnoughMemory;
			GetCMP()->StrCopy(tmpText, *aText);
			sldMemFree(*aText);
			*aText = tmpText;
		}
		
		GetCMP()->StrCopy(*aText + GetCMP()->StrLen(*aText), Word);
		if (aSeparatorText && i < LastIdx-1)
			GetCMP()->StrCopy(*aText + GetCMP()->StrLen(*aText), aSeparatorText);
		
		error = SetBase(Path.BaseList[i]);
		if (error != eOK)
			return error;
	}

	error = GoToByPath(&CurrPath, eGoToWord);
	if (error != eOK)
		return error;

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
ESldError CSldList::GetWordByGlobalIndex(Int32 aGlobalIndex, const bool aInLocalization)
{
	Int32 wordIndex = 0;
	ESldError error;

	if (aInLocalization && m_LocalizedBaseGlobalIndex != SLD_DEFAULT_WORD_INDEX)
		aGlobalIndex += m_LocalizedBaseGlobalIndex;

	if ((UInt32)aGlobalIndex >= GetListInfo()->m_Header.NumberOfWords || aGlobalIndex < 0)
		return eCommonWrongIndex;

	// Запрашивается то же слово, что и в предыдущий раз - ничего делать не надо
	if (aGlobalIndex == GetCurrentWordIndex())
	{
		return eOK;
	}
	else if (GetListInfo()->m_Header.Version == VERSION_LIST_DIRECT_ACCESS)
	{
		return GetWordByShift(aGlobalIndex);
	}
	// Запрашивается следующее по порядку слово
	else if (aGlobalIndex - GetCurrentWordIndex() == 1)
	{
		return GetNextWord();
	}

	// Ищем точку входа по таблице быстрого доступа
	UInt32 low = 0;
	UInt32 hi = m_Input->GetSearchPointCount();
	UInt32 med = 0;
	while (hi-low > 1)
	{
		med = (hi+low)>>1;
		error = m_Input->GetSearchPointIndex(med, &wordIndex);
		if (error != eOK)
			return error;

		if (wordIndex < aGlobalIndex)
		{
			low = med;
		}else
		{
			hi = med;
		}
	}

	// wordIndex - такой индекс мы получим, если перейдем в таблице быстрого доступа в точку low
	error = m_Input->GetSearchPointIndex(low, &wordIndex);
	if (error != eOK)
		return error;

	// Если до запрашиваемого индекса от текущего индекса ближе, чем от индекса найденной точки быстрого доступа,
	// тогда переходить в найденную точку доступа не будем
	if (m_QAShift.IsInit)
	{
		error = GoToCachedPoint(low, aGlobalIndex);
		if (error != eOK)
			return error;
	}
	else if (!(aGlobalIndex > GetCurrentWordIndex() && (aGlobalIndex - GetCurrentWordIndex() < aGlobalIndex - wordIndex)))
	{
		error = GoToBySearchIndex(low);
		if (error != eOK)
			return error;
	}

	while (GetCurrentWordIndex() < aGlobalIndex)
	{
		error = GetNextWord();
		if (error != eOK)
			return error;
	}

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
ESldError CSldList::DoWildCardSearch(TExpressionBox* aExpressionBox, Int32 aMaximumWords, ISldList* aList, Int32 aRealListIndex, ISldList* aRealList)
{
	// Эта функция вызываться не должна
	return eCommonWrongList;
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
ESldError CSldList::DoFuzzySearch(const UInt16* aText, Int32 aMaximumWords, Int32 aMaximumDifference, ISldList* aList, Int32 aRealListIndex, EFuzzySearchMode aSearchMode)
{
	// Эта функция вызываться не должна
	return eCommonWrongList;
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
ESldError CSldList::DoAnagramSearch(const UInt16* aText, Int32 aTextLen, ISldList* aList, Int32 aRealListIndex)
{
	// Эта функция вызываться не должна
	return eCommonWrongList;
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
ESldError CSldList::DoSpellingSearch(const UInt16* aText, ISldList* aList, Int32 aRealListIndex)
{
	// Эта функция вызываться не должна
	return eCommonWrongList;
}

/** *********************************************************************
* Производит полнотекстовый поиск
*
* @param[in]	aText				- шаблон поиска
* @param[in]	aMaximumWords		- максимальное количество слов, которые могут быть найдены
* @param[in]	aList				- указатель на массив списков слов словаря
* @param[in]	aListCount			- количество списков слов в массиве
* @param[in]	aRealListIndex		- реальный индекс списка слов, в котором производим поиск
*
* @return код ошибки
************************************************************************/
ESldError CSldList::DoFullTextSearch(const UInt16* aText, Int32 aMaximumWords, ISldList** aList, Int32 aListCount, Int32 aRealListIndex)
{
	// Эта функция вызываться не должна
	return eCommonWrongList;
}

/** *********************************************************************
* Производит сортировку поискового списка слов по релевантности шаблону поиска
*
* @param[in]	aText		- шаблон поиска
* @param[in]	aSimpleMode	- true: сортировать по упрощенной схеме если возможно - более быстро, но менее качественно
*							  false: полная сортировка - более медленно, но более качественно
*
* @return код ошибки
************************************************************************/
ESldError CSldList::SortListRelevant(const UInt16* aText, const CSldVector<TSldMorphologyWordStruct>& aMorphologyForms, ESldFTSSortingTypeEnum aMode)
{
	// Эта функция вызываться не должна
	return eCommonWrongList;
}

/** *********************************************************************
* Определяет глобальные номера границ текущего уровня вложенности (включая все подуровни)
* Получается диапазон глобальных индексов слов вида [aLowGlobalIndex, aHighGlobalIndex) - левая граница включается, правая - нет
*
* @param[out]	aLowGlobalIndex		- указатель на переменную, в которую записывается нижняя (левая) граница
* @param[out]	aHighGlobalIndex	- указатель на переменную, в которую записывается верхняя (правая) граница
*
* @return код ошибки
************************************************************************/
ESldError CSldList::GetHierarchyLevelBounds(Int32* aLowGlobalIndex, Int32* aHighGlobalIndex)
{
	return GetSearchBounds(eSearchRangeCurrentLevelRecursive, aLowGlobalIndex, aHighGlobalIndex);
}

/** *********************************************************************
* Определяет границы, в которых будет производиться поиск (границы текущего уровня вложенности)
*
* @param[in]	aSearchRange- диапазон поиска в котором нужно определить границы
* @param[out]	aLowIndex	- указатель на переменную, в которую будет сохранена нижняя граница поиска
* @param[out]	aHighIndex	- указатель на переменную, в которую будет сохранена верхняя граница поиска
*
* @return код ошибки
************************************************************************/
ESldError CSldList::GetSearchBounds(ESldSearchRange aSearchRange, Int32* aLowIndex, Int32* aHighIndex)
{
	if (!m_IsCatalogSynchronized)
		return eCommonCatalogIsNotSynchronizedError;

	if (!aLowIndex || !aHighIndex)
		return eMemoryNullPointer;
	
	if (!GetListInfo())
		return eMemoryNullPointer;

	// Общее количество слов в текущем списке слов
	ESldError error;
	Int32 TotalNumberOfWords;
	error = GetTotalWordCount(&TotalNumberOfWords);
	if (error != eOK)
		return error;
	
	// Нижняя граница поиска - первый элемент в корне иерархии
	*aLowIndex = m_LocalizedBaseGlobalIndex == SLD_DEFAULT_LIST_INDEX ? 0 : m_LocalizedBaseGlobalIndex;
	
	// Мы находимся в словаре
	if (!GetListInfo()->IsHierarchy() || aSearchRange == eSearchRangeFullRecursive)
	{
		*aHighIndex = TotalNumberOfWords + *aLowIndex;
		return eOK;
	}

	if(aSearchRange == eSearchRangeRoot)
	{
		if (m_LocalizedBaseGlobalIndex == SLD_DEFAULT_LIST_INDEX)
		{
			error = m_Catalog->GetNumberOfWordsInRoot(aHighIndex);
			if (error != eOK)
				return error;

			*aHighIndex += *aLowIndex;
			return eOK;
		}
		else
		{
			error = GetNumberOfWordsAtCurrentLevel(0, (UInt32*)&TotalNumberOfWords);
			if (error != eOK)
				return error;

			*aHighIndex += TotalNumberOfWords;
			return eOK;
		}
	}

	// Для остальных типов поиска нижняя граница поиска - начало текущего уровня (базовое смещение)
	*aLowIndex = GetBaseIndex();

	Int32 globalIndex = SLD_DEFAULT_WORD_INDEX;
	error = GetCurrentGlobalIndex(&globalIndex);
	if (error != eOK)
		return error;

	error = GetNumberOfWordsAtCurrentLevel(globalIndex, (UInt32*)&TotalNumberOfWords);
	if (error != eOK)
		return error;

	if (aSearchRange == eSearchRangeCurrentLevel)
	{
		error = GetNumberOfWords(aHighIndex);
		if (error != eOK)
			return error;

		*aHighIndex += *aLowIndex;
		return eOK;
	}

	*aHighIndex = *aLowIndex + TotalNumberOfWords;

	return eOK;
}

/** ********************************************************************
* Устанавливаем HASH для декодирования данного списка слов.
*
* @param[in]	aHASH	- HASH код для декодирования списка слов.
*
* @return код ошибки
************************************************************************/
ESldError CSldList::SetHASH(UInt32 aHASH)
{
	m_Input->SetHASH(aHASH);
	return eOK;
}

/** *********************************************************************
* Сохраняет текущее состояние списка слов
*
* @return код ошибки
************************************************************************/
ESldError CSldList::SaveCurrentState(void)
{
	ESldError error;
	
	m_CurrentState.m_Path.Clear();
	
	Int32 GlobalIndex;
	error = GetCurrentGlobalIndex(&GlobalIndex);
	if (error != eOK)
		return error;
	
	if (GlobalIndex == (-1))
		return eOK;
	
	error = GetPathByGlobalIndex(GlobalIndex, &m_CurrentState.m_Path);
	if (error != eOK)
		return error;

	return eOK;
}

/** *********************************************************************
* Восстанавливает ранее сохраненное состояние списка слов
*
* @return код ошибки
************************************************************************/
ESldError CSldList::RestoreState(void)
{
	if (!m_CurrentState.m_Path.BaseList)
		return eOK;

	ESldError error;
	error = GoToByPath(&m_CurrentState.m_Path, eGoToWord);
	if (error != eOK)
		return error;

	return eOK;
}

/** *********************************************************************
* Возвращает таблицу сравнения которая используется в данном списке слов
*
* @param[out]	aCompare - указатель на переменную в которую будет помещен указатель на объект сравнения строк
*
* @return код ошибки
************************************************************************/
ESldError CSldList::GetCompare(CSldCompare **aCompare)
{
	if (!aCompare)
		return eMemoryNullPointer;

	*aCompare = GetCMP();
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
ESldError CSldList::QAGetNumberOfQAItems(Int32 *aNumberOfQAItems)
{
	if (!aNumberOfQAItems)
		return eMemoryNullPointer;
	*aNumberOfQAItems = m_Input->GetSearchPointCount();
	return eOK;
}

/** *********************************************************************
* Возвращает текст для записи в таблице быстрого доступа.
*
* @param[in]	aIndex	- номер элемента в таблице быстрого доступа
* @param[out]	aQAText	- указатель на буфер для текста элемента таблицы быстрого доступа
*
* @return код ошибки
************************************************************************/
ESldError CSldList::QAGetItemText(Int32 aIndex, const UInt16 **aQAText)
{
	return m_Input->GetSearchPointText(aIndex, aQAText);
}

/** *********************************************************************
* По номеру точки входа возвращает номер слова в списке, соответствующий этой точке
*
* @param[in]	aIndex		- номер элемента в таблице быстрого доступа
* @param[out]	aQAIndex	- указатель на буфер, куда сохраняется результат
*
* @return код ошибки
************************************************************************/
ESldError CSldList::QAGetItemIndex(Int32 aIndex, Int32 *aQAIndex)
{
	return m_Input->GetSearchPointIndex(aIndex, aQAIndex);
}


/** *********************************************************************
* Производит переход по таблице быстрого доступа по номеру записи.
*
* @param[in]	aIndex	- номер элемента в таблице быстрого доступа по которому нужно произвести переход.
*
* @return код ошибки
************************************************************************/
ESldError CSldList::JumpByQAIndex(Int32 aIndex)
{
	ESldError error = m_Input->GetSearchPointIndex(aIndex, &m_CurrentIndex);
	if (error != eOK)
		return error;

	const UInt16 *tmpText;
	error = m_Input->GetSearchPointText(aIndex, &tmpText);
	if (error != eOK)
		return error;

	if (!tmpText)
		return eMemoryNullPointer;

	CSldCompare::StrCopy(m_CurrentWord[0], tmpText);

	error = m_Input->GoTo(aIndex);
	if (error != eOK)
		return error;

	if (m_QAShift.IsInit)
	{
		m_QAShift.SetNewQAPoint(aIndex, m_CurrentWord[0], m_Input->GetCurrentPosition(), GetCurrentWordIndex() + 1);
		if (error != eOK)
			return error;
	}

	return eOK;
}

/** *********************************************************************
* Получает вектор индексов озвучек по введенному тексту
*
* @param[in]	aText			- указатель на строку, по которой ищем озвучку
* @param[out]	aSoundIndexes	- вектор с индексами озвучки для текущего слова
*
* @return код ошибки
************************************************************************/
ESldError CSldList::GetSoundIndexByText(const UInt16 *aText, CSldVector<Int32> & aSoundIndexes)
{
	if (!aText)
		return eMemoryNullPointer;

	aSoundIndexes.clear();

	UInt32 hasSounds = 0;
	ESldError error = IsListHasSound(&hasSounds);
	if (error != eOK)
		return error;

	if (!hasSounds)
		return eOK;

	error = GetWordByText(aText);
	if (error != eOK)
		return error;
	
	if (GetCMP()->IsAddTableDefine()) {
		if (GetCMP()->StrICmp(GetWord(0), aText, GetCMP()->GetAddTableIndex()) != 0)
			return eOK;
	}
	else {
		if (GetCMP()->StrICmp(GetWord(0), aText) != 0)
			return eOK;
	}

	return GetSoundIndex(aSoundIndexes);
}


/** *********************************************************************
* Устанавливает нужную локализацию записей по переданному языковому коду
*
* @param[in]	aLanguageCode	- код языка на который нужно локализовать записи в списке;
*								  если такой локализации нет, то локализация не меняется
*
* @return код ошибки
************************************************************************/
ESldError CSldList::SetLocalization(ESldLanguage aLocalizationLanguageCode)
{
	return SetLocalization(aLocalizationLanguageCode, NULL);
}

/** *********************************************************************
* Устанавливает нужную локализацию записей по переданному языковому коду и набору других настроек
*
* @param[in]	aLanguageCode			- код языка на который нужно локализовать записи в списке;
*										  если такой локализации нет, то локализация не меняется
* @param[in]	aLocalizationDetails	- дополнительные настройки локализации (например, название платформы)
*
* @return код ошибки
************************************************************************/
ESldError CSldList::SetLocalization(ESldLanguage aLocalizationLanguageCode, const UInt16* aLocalizationDetails)
{
	ESldError error;

	const CSldListInfo* pListInfo = GetListInfo();

	// список не поддерживает локализацию - никакого смещения для доступа к словам получать не нужно
	if (!pListInfo->GetLocalizedWordsFlag())
		return eOK;

	// ищем вариант написания, хранящий настройки локализации
	const Int32 localizationVariantIndex = GetListInfo()->GetVariantIndexByType(eVariantLocalizationPreferences);

	if (localizationVariantIndex == SLD_DEFAULT_VARIANT_INDEX)
	{
		// список не содержит варианта написания с настройками локализации
		// возможно, был ошибочно выставлен флаг в заголовке списка - это не помешает нам дальше с ним работать
		return eOK;
	}

	if (!GetCMP())
		return eOK;

	SldU16WordsArray RequestParts;
	// разделитель настроек локализации внутри текста варианта написания
	const UInt16* const localizationParamDelimiter = SldU16StringLiteral(";");

	if (aLocalizationDetails)
	{
		error = GetCMP()->DivideQueryByParts(aLocalizationDetails, localizationParamDelimiter, RequestParts);
		if (error != eOK)
			return error;
	}

	// сохраняем текущее значение локализации
	// (тогда мы сможем восстановить его в случае ошибки или если найти запрашиваемую локализацию не удалось)
	Int32 oldLocalizedBase = m_LocalizedBaseIndex;
	Int32 oldLocalizedGlobalBase = m_LocalizedBaseGlobalIndex;
	m_LocalizedBaseIndex = SLD_DEFAULT_LIST_INDEX;

	error = SetBase(-1);
	if (error != eOK)
		return error;

	Int32 numberOfLocalizations;
	error = GetNumberOfWords(&numberOfLocalizations);
	if (error != eOK)
		return error;
	
	// из всех возможных локализаций выбираем совпадающие по языку (обязательное условие, если не выполняется, то остальные настройки не смотрим);
	// среди них выбираем наиболее совпадающую по остальным настройкам (вес всех доп.ключей из aLocalizationDetails одинаков)

	// индекс лучшего из возможных вариантов локализации
	Int32 bestLocalizationIndex = SLD_DEFAULT_LIST_INDEX;
	// количество совпадающих доп.ключей у лучшего варианта локализации
	Int32 bestLocalizationDetailsMatch = 0;

	for (Int32 index=0;index<numberOfLocalizations;index++)
	{
		error = GetWordByIndex(index);
		if (error != eOK)
			break;

		UInt16* Word = 0;
		error = GetCurrentWord(localizationVariantIndex, &Word);
		if (error != eOK)
			break;

		SldU16WordsArray WordParts;
		error = GetCMP()->DivideQueryByParts(Word, localizationParamDelimiter, WordParts);
		if (error != eOK)
			break;

		Int32 languageCodeIndex = SLD_DEFAULT_LIST_INDEX;
		for (UInt16 wordPartIndex=0;wordPartIndex<WordParts.size();wordPartIndex++)
		{
			// преобразуем строку с текущим словом в код языка
			UInt32 langCodeSize = sizeof(ESldLanguage);
			UInt32 wordLength = WordParts[wordPartIndex].size();

			// длина кода языка в варианте написания отличается от стандартной - эта часть слова не может являеться кодом языка
			if (wordLength != langCodeSize)
				continue;

			UIntUnion code;
			code.ui_32 = 0;
			for (UInt32 n=0;n<langCodeSize;n++)
			{
				code.ui_8[n] = (UInt8)(WordParts[wordPartIndex].c_str()[n]);
			}

			if (code.ui_32 == aLocalizationLanguageCode)
			{
				languageCodeIndex = wordPartIndex;
				break;
			}
		}

		// проверим, насколько эта локализация совпадает по неязыковым настройкам с искомой
		if (languageCodeIndex != SLD_DEFAULT_LIST_INDEX)
		{
			Int32 currentDetailsMatch = 0;
			if (aLocalizationDetails && RequestParts.size())
			{
				for (UInt16 wordPartIndex=0;wordPartIndex<WordParts.size();wordPartIndex++)
				{
					// код языка (указанный в тексте варианта написания) доп.настройкой не считаем - он уже учтен выше
					if (wordPartIndex == languageCodeIndex)
						continue;

					for (UInt16 requestPartIndex=0;requestPartIndex<RequestParts.size();requestPartIndex++)
					{
						if (!GetCMP()->StrICmp(RequestParts[requestPartIndex].c_str(), WordParts[wordPartIndex].c_str()))
							currentDetailsMatch++;
					}
				}

				if (currentDetailsMatch > bestLocalizationDetailsMatch)
				{
					bestLocalizationIndex = index;
					bestLocalizationDetailsMatch = currentDetailsMatch;
				}
			}
			else
			{
				// если дополнительные настройки не указаны, то выберем локализацию с минимальным количеством настроек (считаем это наиболее общим вариантом для данного языка)
				if (bestLocalizationIndex == SLD_DEFAULT_LIST_INDEX || bestLocalizationDetailsMatch > currentDetailsMatch)
					bestLocalizationIndex = index;
			}
		}
	}

	// восстановим последнее корректное значение
	if (error != eOK)
	{
		m_LocalizedBaseIndex = oldLocalizedBase;
		m_LocalizedBaseGlobalIndex = oldLocalizedGlobalBase;
		// т.к. на этом этапе операции со списком слов уже вызвали ошибку, выполнение SetBase также может завершиться некорректно,
		// код ошибки при этом можно не проверять - вызывающей стороне важна ошибка, с которой все началось
		SetBase(SLD_DEFAULT_LIST_INDEX);

		return error;
	}

	if (bestLocalizationIndex == SLD_DEFAULT_LIST_INDEX)
	{
		m_LocalizedBaseIndex = oldLocalizedBase;
		m_LocalizedBaseGlobalIndex = oldLocalizedGlobalBase;
	}
	else
	{
		m_LocalizedBaseIndex = bestLocalizationIndex;
		UInt32 localizedGlobalIndex;
		error = m_Catalog->GetBaseByIndex(m_LocalizedBaseIndex, &localizedGlobalIndex);
		if (error != eOK)
			return error;

		m_LocalizedBaseGlobalIndex = (Int32)localizedGlobalIndex;
	}

	return SetBase(SLD_DEFAULT_LIST_INDEX);
}

/** *********************************************************************
* Инициализирует вспомогательный сортированный список
*
* @param[in]	aSortedListPtr			- указатель на сортированный список;
*
* @return код ошибки
************************************************************************/
ESldError CSldList::InitSortedList(ISldList* aSortedListPtr)
{
	if(!aSortedListPtr)
		return eMemoryNullPointer;

	Int32 wordCount = 0;
	ESldError error = GetTotalWordCount(&wordCount);
	if (error != eOK)
		return error;

	return m_SortedList.Init(aSortedListPtr, wordCount);
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
ESldError CSldList::GetWordBySortedText(const UInt16* aText, UInt32* aResultFlag)
{
	if (!aText || !aResultFlag)
		return eMemoryNullPointer;
	
	*aResultFlag = 0;
	ESldError error = eOK;

	if (m_SortedWordIndexes)
	{
		*aResultFlag = 1;

		if (!m_SortedWordIndexes)
		{
			return GetWordByText(aText);
		}

		ESldError error = eOK;
		UInt32 low = 0;
		UInt32 hi = GetListInfo()->GetNumberOfGlobalWords();
		UInt32 med = 0;
		UInt16 *currentText = NULL;

		while (hi - low > 1)
		{
			med = (hi + low) >> 1;

			error = GetWordByGlobalIndex(m_SortedWordIndexes[med], false);
			if (error != eOK)
				return error;

			error = GetCurrentWord(m_SortedVariantIndex, &currentText);
			if (error != eOK)
				return error;

			if (GetCMP()->StrICmp(currentText, aText, 0) < 0)
			{
				low = med;
			}
			else
			{
				hi = med;
			}
		}

		error = GetWordByGlobalIndex(m_SortedWordIndexes[low], false);
		if (error != eOK)
			return error;

		error = GetCurrentWord(m_SortedVariantIndex, &currentText);
		if (error != eOK)
			return error;

		if (GetCMP()->StrICmp(currentText, aText, 0) < 0)
		{
			error = GetWordByGlobalIndex(m_SortedWordIndexes[hi], false);
			if (error != eOK)
				return error;
		}

		return eOK;
	}

	if(HasSimpleSortedList())
	{
		Int32 sortedIndex = SLD_DEFAULT_LIST_INDEX;
		error = m_SortedList.GetSortedIndexByText(aText, &sortedIndex);
		if (error != eOK)
			return error;

		if(m_Catalog)
		{
			error = SetBase(MAX_UINT_VALUE);
			if (error != eOK)
				return error;
		}

		error = GetWordByGlobalIndex(sortedIndex, false);
		if (error != eOK)
			return error;

		*aResultFlag = 1;
	}
	else
	{
		UInt32 isSorted = 0;
		error = IsCurrentLevelSorted(&isSorted);
		if (error != eOK)
			return error;

		if (isSorted)
		{
			error = GetWordByText(aText);
			if (error != eOK)
				return error;

			*aResultFlag = 1;
		}
	}

	return eOK;
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
ESldError CSldList::GetWordByTextExtended(const UInt16* aText, UInt32* aResultFlag, UInt32 aActionsOnFailFlag)
{
	if (aActionsOnFailFlag && GetBaseIndex())
	{
		*aResultFlag = 0;
		return eOK;
	}

	ESldError error = GetWordBySortedText(aText, aResultFlag);
	if (error != eOK)
		return error;

	if(*aResultFlag == 0)
		return eOK;

	Int32 currentSortedIndex = 0;

	if (m_SortedWordIndexes)
	{
		if (GetCMP()->StrICmp(GetWord(m_SortedVariantIndex), aText))
		{
			*aResultFlag = 0;
			return eOK;
		}

		error = GetCurrentGlobalIndex(&currentSortedIndex);
		if (error != eOK)
			return error;

		const UInt32 numberOfWords = GetListInfo()->GetNumberOfGlobalWords();
		while (GetCMP()->StrICmp(GetWord(m_SortedVariantIndex), aText) == 0 && GetCurrentWordIndex() < numberOfWords)
		{
			if (GetCMP()->StrCmp(GetWord(m_SortedVariantIndex), aText) == 0)
			{
				return eOK;
			}

			error = GetNextWord();
			if (error != eOK)
				return error;
		}

		return GetWordByGlobalIndex(currentSortedIndex);
	}
	else if (HasSimpleSortedList())
	{
		error = m_SortedList.FindBinaryMatch(aText, aResultFlag);
		if (error != eOK)
			return error;

		if(!*aResultFlag && !aActionsOnFailFlag)
			return eOK;

		error = m_SortedList.GetCurrentSortedIndex(&currentSortedIndex);
		if (error != eOK)
			return error;

		if(aActionsOnFailFlag || m_LocalizedBaseGlobalIndex != SLD_DEFAULT_WORD_INDEX)
		{
			Int32 high = 0;
			Int32 low = 0;
			Int32 sortedWordsCount = 0;
			Int32 sortedIndex = 0;

			const ESldSearchRange searchRange = aActionsOnFailFlag == 0 ? eSearchRangeFullRecursive : eSearchRangeRoot;
			error = GetSearchBounds(searchRange, &low, &high);
			if (error != eOK)
				return error;
			
			error = m_SortedList.GetNumberOfWords(&sortedWordsCount);
			if (error != eOK)
				return error;

			error = m_SortedList.GetCurrentIndex(&sortedIndex);
			if (error != eOK)
				return error;

			bool changeResult = false;
			while ((sortedIndex + 1) < sortedWordsCount && !sld2::InRange<Int32>(currentSortedIndex, low, high))
			{
				changeResult = true;
				error = m_SortedList.GetNextWordSortedIndex(&currentSortedIndex);
				if (error != eOK)
					return error;

				error = m_SortedList.GetCurrentIndex(&sortedIndex);
				if (error != eOK)
					return error;

				if (m_LocalizedBaseGlobalIndex != SLD_DEFAULT_WORD_INDEX)
				{
					UInt16* currentWord = NULL;
					error = m_SortedList.GetCurrentWord(&currentWord);

					if (GetCMP()->StrICmp(currentWord, aText) != 0)
					{
						*aResultFlag = 0;
						return eOK;
					}
				}
			}

			if(sortedIndex + 1 >= sortedWordsCount)
			{
				currentSortedIndex = high - 1;
			}

			if (changeResult)
			{
				UInt16* currentWord = NULL;
				error = m_SortedList.GetCurrentWord(&currentWord);

				if (GetCMP()->StrICmp(currentWord, aText) != 0)
				{
					*aResultFlag = 0;
				}
			}
		}

		error = GetWordByGlobalIndex(currentSortedIndex, false);
		if (error != eOK)
			return error;
	}
	else
	{
		error = FindBinaryMatch(aText, aResultFlag);
		if (error != eOK)
			return error;
		
		if (GetCMP()->StrICmp(aText, GetWord(GetListInfo()->GetShowVariantIndex())) == 0)
			*aResultFlag = 1;

		return eOK;
	}

	return eOK;
}

/** *********************************************************************
* Синхронизирует состояние вспомогательного сортированного списка с базовым
*
* @return код ошибки
************************************************************************/
ESldError CSldList::SynchronizeWithASortedList()
{
	if (!HasSimpleSortedList())
		return eOK;

	Int32 currentSortedIndex = 0;
	ESldError error = m_SortedList.GetCurrentSortedIndex(&currentSortedIndex);
	if (error != eOK)
		return error;

	if (currentSortedIndex == GetCurrentWordIndex())
		return eOK;

	// XXX: That's kinda broken...
	Int32 currentVariant = GetListInfo()->GetVariantIndexByType(eVariantShow);
	if (currentVariant == SLD_DEFAULT_VARIANT_INDEX)
		currentVariant = GetListInfo()->m_Header.NumberOfVariants;

	UInt16* currentWord = NULL;
	error = GetCurrentWord(currentVariant, &currentWord);
	if (error != eOK)
		return error;

	UInt16* currentSortedWord = NULL;
	error = m_SortedList.GetSortedIndexByText(currentWord, &currentSortedIndex);
	if (error != eOK)
		return error;

	while (currentSortedIndex != GetCurrentWordIndex())
	{
		error = m_SortedList.GetNextWordSortedIndex(&currentSortedIndex);
		if (error != eOK)
			return error;

		error = m_SortedList.GetCurrentWord(&currentSortedWord);
		if (error != eOK)
			return error;

		if (GetCMP()->StrICmp(currentWord, currentSortedWord))
			break;
	}

	if (currentSortedIndex != GetCurrentWordIndex())
		return eCommonWrongIndex;

	return eOK;
}

/** *********************************************************************
* Получает слово по его номеру в сортированном списке
*
* @param[in]	aIndex		- запрашиваемый индекс слова
*
* @return код ошибки
************************************************************************/
ESldError CSldList::GetWordBySortedIndex(Int32 aIndex)
{
	if (HasSimpleSortedList())
	{
		Int32 sortedIndex = 0;
		ESldError error = m_SortedList.GetSortedIndexByIndex(aIndex, &sortedIndex);
		if (error != eOK)
			return error;

		return GoToByGlobalIndex(sortedIndex);
	}

	return GoToByGlobalIndex(aIndex);
}

/** *********************************************************************
* Получает следующее сортированное слово
*
* @return код ошибки
************************************************************************/
ESldError CSldList::GetNextSortedWord()
{
	if (HasSimpleSortedList())
	{
		Int32 sortedIndex = 0;
		ESldError error = m_SortedList.GetNextWordSortedIndex(&sortedIndex);
		if (error != eOK)
			return error;

		return GetWordByGlobalIndex(sortedIndex);
	}

	return GetNextWord();
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
ESldError CSldList::GetNextRealSortedWord(Int8* aResult)
{
	*aResult = 1;
	if (HasSimpleSortedList())
	{
		Int32 sortedIndex = 0;

		Int32 currentIndexInSortedList = -1;
		Int32 wordsCountInSortedList = 0;

		ESldError error = m_SortedList.GetCurrentIndex(&currentIndexInSortedList);
		if (error != eOK)
			return error;

		error = m_SortedList.GetNumberOfWords(&wordsCountInSortedList);
		if (error != eOK)
			return error;

		if ((currentIndexInSortedList + 1) == wordsCountInSortedList)
		{
			*aResult = 0;
			return eOK;
		}

		while ((currentIndexInSortedList + 1) < wordsCountInSortedList)
		{
			error = m_SortedList.GetNextWordSortedIndex(&sortedIndex);
			if (error != eOK)
				return error;

			if(m_SortedList.IsNormalWord() == SLD_SIMPLE_SORTED_NORMAL_WORD)
			{
				break;
			}

			ESldError error = m_SortedList.GetCurrentIndex(&currentIndexInSortedList);
			if (error != eOK)
				return error;
		}

		if (m_SortedList.IsNormalWord() != SLD_SIMPLE_SORTED_NORMAL_WORD)
		{
			*aResult = 0;
			return SynchronizeWithASortedList();
		}
		else
		{
			return GetWordByGlobalIndex(sortedIndex);
		}
	}

	if ((GetCurrentWordIndex() + 1) == GetListInfo()->GetNumberOfGlobalWords())
	{
		*aResult = 0;
		return eOK;
	}

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
ESldError CSldList::GetCurrentSortedIndex(Int32* aIndex) const
{
	if (m_SortedList.HasSortedList())
	{
		return m_SortedList.GetCurrentIndex(aIndex);
	}

	return GetCurrentGlobalIndex(aIndex);
}

/** *********************************************************************
* Возвращает количество записей в таблице быстрого доступа для сортированного списка
*
* @param[out]	aNumberOfQAItems - указатель на переменную, в которую нужно будет
*								   поместить количество записей в таблице быстрого доступа.
*
* @return код ошибки
************************************************************************/
ESldError CSldList::QAGetNumberOfSortedQAItems(Int32 *aNumberOfQAItems)
{
	if(HasSimpleSortedList())
	{
		return m_SortedList.QAGetNumberOfQAItems(aNumberOfQAItems);
	}

	return QAGetNumberOfQAItems(aNumberOfQAItems);
}

/** *********************************************************************
* Возвращает текст для записи в таблице быстрого доступа для сортированного списка
*
* @param[in]	aIndex	- номер элемента в таблице быстрого доступа
* @param[out]	aQAText	- указатель на буфер для текста элемента таблицы быстрого доступа
*
* @return код ошибки
************************************************************************/
ESldError CSldList::QAGetSortedItemText(Int32 aIndex, const UInt16 **aQAText)
{
	if(HasSimpleSortedList())
	{
		return m_SortedList.QAGetItemText(aIndex, aQAText);
	}

	return QAGetItemText(aIndex, aQAText);
}

/** *********************************************************************
* По номеру точки входа в сортированном списке возвращает номер слова, соответствующий этой точке
*
* @param[in]	aIndex		- номер элемента в таблице быстрого доступа
* @param[out]	aQAIndex	- указатель на буфер, куда сохраняется результат
*
* @return код ошибки
************************************************************************/
ESldError CSldList::QAGetSortedItemIndex(Int32 aIndex, Int32 *aQAIndex)
{
	if(HasSimpleSortedList())
	{
		return m_SortedList.QAGetItemIndex(aIndex, aQAIndex);
	}

	return QAGetItemIndex(aIndex, aQAIndex);
}

/** *********************************************************************
* Производит переход по таблице быстрого доступа по номеру записи в сортированном списке
*
* @param[in]	aIndex	- номер элемента в таблице быстрого доступа по которому нужно произвести переход.
*
* @return код ошибки
************************************************************************/
ESldError CSldList::JumpByQASortedIndex(Int32 aIndex)
{
	if(HasSimpleSortedList())
	{
		return m_SortedList.JumpByQAIndex(aIndex);
	}

	return JumpByQAIndex(aIndex);
}

/** *********************************************************************
* Возвращает флаг, проверяющий на реальном ли слове установлен список, или на альтернативном заголовке
*
* @return SLD_SIMPLE_SORTED_NORMAL_WORD, если вспомогательный список имеется, иначе SLD_SIMPLE_SORTED_VIRTUAL_WORD
************************************************************************/
Int8 CSldList::IsNormalWord()
{
	if (!HasSimpleSortedList())
		return SLD_SIMPLE_SORTED_NORMAL_WORD;

	return m_SortedList.IsNormalWord();
}

/** *********************************************************************
* Проверяет наличие вспомогательного сортированного списка
*
* @return 1, если вспомогательный список имеется, иначе 0
************************************************************************/
Int8 CSldList::HasSimpleSortedList()
{
	return m_SortedList.HasSortedList();
}

/** *********************************************************************
* Проверяет наличие иерархии у списка
*
* @return 1, если есть иерархия, иначе 0
************************************************************************/
Int8 CSldList::HasHierarchy()
{
	if (m_Catalog)
		return 1;

	return 0;
}

/** *********************************************************************
* Переходит к ближайшей закешированной точке (само слово не декодируется,
* после необходимо вызвать GetNextWord())
*
* @param[in]	aQAPointIndex		-	индекс QA-точки, в которой мы находимся
* @param[in]	aNeedGlobalIndex	-	запрашиваемый индекс слова
*
* @return Код ошибки
************************************************************************/
ESldError CSldList::GoToCachedPoint(UInt32 aQAPointIndex, Int32 aNeedGlobalIndex)
{
	ESldError error = eOK;

	if (m_QAShift.QAPointIndex == aQAPointIndex)
	{
		Int32 localCashedIndex;
		if (m_QAShift.IsCahed(aNeedGlobalIndex))
		{
			localCashedIndex = aNeedGlobalIndex - m_QAShift.BeginGlobalIndex;
			m_CurrentIndex = aNeedGlobalIndex - 1;
		}
		else
		{
			localCashedIndex = m_QAShift.LastIndex() - 1;
			m_CurrentIndex = m_QAShift.LastGlobalIndex - 2;
		}

		error = m_Input->SetCurrentPosition(m_QAShift.Shift[localCashedIndex]);
		if (error != eOK)
			return error;

		CSldCompare::StrCopy(m_CurrentWord[0], m_QAShift.Prefix.data() + m_QAShift.PrefixPos[localCashedIndex]);
	}
	else
	{
		error = GoToBySearchIndex(aQAPointIndex);
		if (error != eOK)
			return error;
	}

	return eOK;
}

/** *********************************************************************
* Инициализация структуры быстрого доступа
*
* @param[in]	aQAInterleaveSize	-	расстояние между ближайшими точками доступа
* @param[in]	aPrefixSize			-	размер массива, для кеширования префиксов
*
* @return Код ошибки
************************************************************************/
ESldError TCurrentQAWordsShift::Init(const Int32 aQAInterleaveSize, const UInt32 aMaxPrefixSize)
{
	IsInit = 1;

	if (!Shift.resize(aQAInterleaveSize))
		return eMemoryNotEnoughMemory;

	// Для PrefixPos понадобиться на 1 ячейку больше, так как она записывается со смещением +1
	if (!PrefixPos.resize(aQAInterleaveSize + 1))
		return eMemoryNotEnoughMemory;

	if (!Prefix.resize(aMaxPrefixSize * aQAInterleaveSize))
		return eMemoryNotEnoughMemory;

	return eOK;
}

/** *********************************************************************
* Переходит на новую QA-точку, очищая структуру быстрого поиска
*
* @param[in]	aQAPoint	-	номер точки, на которую мы перешли
*
* @return Код ошибки
************************************************************************/
ESldError TCurrentQAWordsShift::SetNewQAPoint(const Int32 aQAPointIndex, const UInt16 * aCurrentWord, const UInt32 aCurrentShift, const Int32 aCurrentGlobalIndex)
{
	QAPointIndex = aQAPointIndex;

	BeginGlobalIndex = aCurrentGlobalIndex;
	LastGlobalIndex = aCurrentGlobalIndex;

	Shift[0] = aCurrentShift;
	PrefixPos[0] = 0;
	PrefixPos[1] = CSldCompare::StrLen(aCurrentWord) + 1;
	CSldCompare::StrCopy(Prefix.data(), aCurrentWord);

	return eOK;
}

/** *********************************************************************
* Добавляет новую точку в структуру быстрого доступа
*
* @param[in]	aSize			-	размер префикса для данного слова
* @param[in]	aCurrentWord	-	указатель на последнее декодированное слово
* @param[in]	aInput			-	указатель на класс, отвечающий за получение данных
*
* @return Код ошибки
************************************************************************/
ESldError TCurrentQAWordsShift::AddCashedPoint(const UInt16 aSize, const UInt16* aCurrentWord, const UInt32 aCurrentShift, const Int32 aGlobalIndex)
{
	if (aGlobalIndex != LastGlobalIndex + 1 || LastIndex() + 1 >= Shift.size())
		return eOK;

	// настоящий размер префикса для 0 индекса мы можем узнать только при установке QA-точки
	// поэтому запишем только смещение для следующего слова
	if (LastIndex() == 0)
	{
		LastGlobalIndex++;
		Shift[LastIndex()] = aCurrentShift;
		return eOK;
	}

	const Int32 currentLocalIndex = LastIndex();
	PrefixPos[currentLocalIndex + 1] = PrefixPos[currentLocalIndex] + aSize + 1;
	CSldCompare::StrNCopy(Prefix.data() + PrefixPos[currentLocalIndex], aCurrentWord, aSize);

	LastGlobalIndex++;
	Shift[LastIndex()] = aCurrentShift;

	return eOK;
}

/** *********************************************************************
* Получает ближайшую точку быстрого доступа для заданного слова на текущем уровне
*
* @param[in]	aText		-	искомое слово
* @param[out]	aQAPoint	-	указатель на переменную, в которую будет записана
*							-	ближайшая точка доступа
*
* @return Код ошибки
************************************************************************/
ESldError CSldList::GetClosestQAPointByText(const UInt16* aText, Int32* aQAPoint)
{
	if (!m_IsCatalogSynchronized)
		return eCommonCatalogIsNotSynchronizedError;

	if (!aQAPoint)
		return eMemoryNullPointer;

	*aQAPoint = -1;
	if (m_Input->GetSearchPointCount() < 2)
		return eOK;

	Int32 qaInterleave = 0;
	Int32 wordsCount = 0;

	ESldError error = GetNumberOfWords(&wordsCount);
	if (error != eOK)
		return error;

	// Определяем расстояние между qa-точками
	error = m_Input->GetSearchPointIndex(1, &qaInterleave);
	if (error != eOK)
		return error;

	// Этот запас необходим из-за неравномерности распределения qa-точек
	qaInterleave++;

	if (qaInterleave * 2 > wordsCount)
		return eOK;

	// Ищем первую точку на текущем уровне
	UInt32 low = 0;
	UInt32 hi = m_Input->GetSearchPointCount();
	UInt32 med = low;
	Int32 qaWordIndex = 0;
	Int32 firstQAPointOnLevel = 0;
	Int32 lastLevelIndex = GetBaseIndex() + wordsCount;
	const UInt16* qaText;

	if (GetBaseIndex())
	{
		while (hi - low > 1)
		{
			med = (hi + low) >> 1;
			error = m_Input->GetSearchPointIndex(med, &qaWordIndex);
			if (error != eOK)
				return error;

			if (qaWordIndex + 1 < GetBaseIndex())
			{
				low = med;
			}
			else
			{
				hi = med;
			}
		}

		firstQAPointOnLevel = med;
	}
	else
	{
		firstQAPointOnLevel = 1;
	}

	error = m_Input->GetSearchPointText(firstQAPointOnLevel, &qaText);
	if (error != eOK)
		return error;

	if (GetCMP()->StrICmp(qaText, aText) > 0)
	{
		*aQAPoint = -1;
		return eOK;
	}

	// Определяем последнюю qa-точку на уровне
	low = firstQAPointOnLevel;
	hi = m_Input->GetSearchPointCount();
	while (hi - low > 1)
	{
		med = (hi + low) >> 1;
		error = m_Input->GetSearchPointIndex(med, &qaWordIndex);
		if (error != eOK)
			return error;

		if (qaWordIndex < lastLevelIndex)
		{
			low = med;
		}
		else
		{
			hi = med;
		}
	}

	hi = low + 1;
	// Ищем ближайшую по тексту qa-точку	
	low = firstQAPointOnLevel;
	while (hi - low > 1)
	{
		med = (hi + low) >> 1;
		error = m_Input->GetSearchPointText(med, &qaText);
		if (error != eOK)
			return error;

		if (GetCMP()->StrICmp(qaText, aText) < 0)
		{
			low = med;
		}
		else
		{
			hi = med;
		}
	}

	*aQAPoint = low;

	return eOK;
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
ESldError CSldList::IsCurrentLevelSorted(UInt32* aIsSorted)
{
	if (!m_IsCatalogSynchronized)
		return eCommonCatalogIsNotSynchronizedError;

	if (!aIsSorted)
		return eMemoryNullPointer;

	if (!GetListInfo()->IsHierarchy())
	{
		*aIsSorted = GetListInfo()->IsSortedList();
		return eOK;
	}

	*aIsSorted = m_Catalog->IsLevelSorted();
	return eOK;
}

/** *********************************************************************
* Получает слово по элементу истории.
* Функция не переходит по иерархии, требует дополнительного вызова GoToByGlobalIndex()
*
* @param[in]	aHistoryElement	- структура, содержащая информацию, необходимую для восстановления слова.
* @param[out]	aResultFlag		- указатель, по которому будет записан результат выполнения функции:
* @param[out]	aGlobalIndex	- указатель, по которому будет записан индекс найденного слова
*								  (см. #ESldHistoryResult)
* @return код ошибки
************************************************************************/
ESldError CSldList::GetWordByHistoryElement(CSldHistoryElement* aHistoryElement, ESldHistoryResult* aResultFlag, Int32* aGlobalIndex)
{
	if(!aHistoryElement || !aResultFlag)
		return eMemoryNullPointer;

	*aResultFlag = eHistoryNotCompare;
	*aGlobalIndex = SLD_DEFAULT_WORD_INDEX;

	ESldError error = eOK;
	UInt32 massCompare = 0;
	UInt16* currentElementShowVariant = NULL;
	UInt16* currentWordShowVariant = NULL;

	error = aHistoryElement->GetShowVariant(&currentElementShowVariant);
	if(error != eOK)
		return error;

	// Проверяем текущее слово
	if (aHistoryElement->GetWordIndex() < GetListInfo()->GetNumberOfGlobalWords())
	{
		error = GetWordByGlobalIndex(aHistoryElement->GetWordIndex());
		if(error != eOK)
			return error;

		error = GetCurrentWord(GetListInfo()->GetShowVariantIndex(), &currentWordShowVariant);
		if(error != eOK)
			return error;

		if (CompareHistoryElementWithCurrentWord(*aHistoryElement) == 0)
		{
			*aResultFlag = eHistoryFullCompare;
			return GetCurrentGlobalIndex(aGlobalIndex);
		}
	}

	// Если индекс слова сменился - пытаемся найти слово самостоятельно
	error = GetWordByTextExtended(currentElementShowVariant, &massCompare, 0);
	if(error != eOK)
		return error;

	// Если слова с данной массой не найдено в списке - заканчиваем поиск
	if(massCompare != 1)
		return eOK;

	error = GetCurrentWord(GetListInfo()->GetShowVariantIndex(), &currentWordShowVariant);
	if (error != eOK)
		return error;

	// Ищем нужное слово, среди всех show-вариантов с одинаковой массой
	Int8 isRealWord = 1;
	CSldVector<TSldPair<UInt32, CSldVector<Int32>>> weight_Index;
	while (isRealWord == 1 && GetCMP()->StrICmp(currentElementShowVariant, currentWordShowVariant) == 0)
	{
		UInt32 resultIndex = 0;
		TSldPair<UInt32, CSldVector<Int32>> pair(CompareHistoryElementWithCurrentWord(*aHistoryElement, true), CSldVector<Int32>());

		if (sld2::binary_search(&weight_Index[0], weight_Index.size(), pair, &resultIndex))
		{
			weight_Index[resultIndex].second.push_back(GetCurrentWordIndex());
		}
		else
		{
			pair.second.push_back(GetCurrentWordIndex());
			weight_Index.insert(resultIndex, sld2::move(pair));
		}

		error = GetNextRealSortedWord(&isRealWord);
		if (error != eOK)
			return error;

		error = GetCurrentWord(GetListInfo()->GetShowVariantIndex(), &currentWordShowVariant);
		if (error != eOK)
			return error;
	}

	if (weight_Index.empty())
		return eOK;

	auto & maxSimilarIndexes = weight_Index.front().second;
	CSldVector<TSldPair<UInt32, Int32>> binary_weight_Index;
	for (auto index : maxSimilarIndexes)
	{
		error = GetWordByGlobalIndex(index);
		if (error != eOK)
			return error;

		const Int32 checkResult = CompareHistoryElementWithCurrentWord(*aHistoryElement);
		if (checkResult == 0)
		{
			*aResultFlag = eHistoryFullCompare;
			return GetCurrentGlobalIndex(aGlobalIndex);
		}

		if (*aResultFlag == eHistoryNotCompare &&
			!GetCMP()->StrCmp(currentElementShowVariant, GetWord(GetListInfo()->GetShowVariantIndex())))
		{
			*aResultFlag = eHistoryDuplicateCompare;
		}

		Int32 currentIndex = SLD_DEFAULT_WORD_INDEX;
		error = GetCurrentGlobalIndex(&currentIndex);
		if (error != eOK)
			return error;

		sld2::sorted_insert(binary_weight_Index, TSldPair<UInt32, Int32>(checkResult, currentIndex));
	}

	if (*aResultFlag == eHistoryNotCompare)
	{
		*aResultFlag = eHistoryMassCompare;
	}
	*aGlobalIndex = binary_weight_Index.front().second;
	return eOK;
}

/** *********************************************************************
* Возвращает количество слов из вспомогательного сортированного списка
*
* @return количество слов
************************************************************************/
UInt32 CSldList::GetSortedWordsCount()
{
	Int32 wordsCount = 0;
	ESldError error;

	if(HasSimpleSortedList())
	{
		error =  m_SortedList.GetNumberOfWords(&wordsCount);
		if(error != eOK)
			return error;
	}
	else
	{
		error = GetNumberOfWords(&wordsCount);
		if(error != eOK)
			return error;
	}

	return wordsCount;
}

/** *********************************************************************
* Получает слово напрямую при наличии массива Индекс - Смещение
*
* @param[in]	aWordIndex	- номер слова по порядку, начиная с 0
*
* @return код ошибки
************************************************************************/
ESldError CSldList::GetWordByShift(UInt32 aWordIndex)
{
	ESldError error;

	UInt32 shift;
	error = GetShiftByIndex(aWordIndex, &shift);
	if (error != eOK)
		return error;

	error = m_Input->SetCurrentPosition(shift);
	if (error != eOK)
		return error;

	m_CurrentIndex = aWordIndex - 1;

	error = GetNextWord();	
	if (error != eOK)
		return error;

	return eOK;
}

/** *********************************************************************
* Получает смещение для текущего индекса
*
* @param[in]	aWordIndex	- номер слова по порядку, начиная с 0
* @param[out]	aShift		- указатель, по которому будет записано смещение
*							  для текущего слова
*
* @return код ошибки
************************************************************************/
ESldError CSldList::GetShiftByIndex(UInt32 aWordIndex, UInt32* aShift)
{
	if (!aShift)
		return eMemoryNullPointer;

	Int32 resIndex = aWordIndex / SLD_DEFAULT_DIRECT_ACCESS_POINT_COUNT_IN_RESOURCE;
	*aShift = 0;

	if(resIndex != m_CurrentDirectAccessResourceIndex)
	{
		ESldError error = m_data->GetResourceData(m_DirectAccessShifts.data(), GetListInfo()->GetHeader()->Type_DirectWordsShifts, resIndex, MAX_COMPRESSED_DATA_RESOURCE_SIZE);
		if (error != eOK)
			return error;

		m_CurrentDirectAccessResourceIndex = resIndex;
	}
	*aShift = m_DirectAccessShifts[aWordIndex - (resIndex * SLD_DEFAULT_DIRECT_ACCESS_POINT_COUNT_IN_RESOURCE)];

	return eOK;
}

/** *********************************************************************
* Устанавливает значение флага синхронизации каталога с текущим индексом
*
* @param[in]	aFlag	- значение флага (0 или 1)
*
* @return eOK
************************************************************************/
ESldError CSldList::SetCatalogSynchronizedFlag(UInt8 aFlag)
{
	m_IsCatalogSynchronized = aFlag;
	return eOK;
}

/** *********************************************************************
* Ищет слово, бинарно совпадающее по переданным вариантам написания
*
* @param[in]	aWordVariantsSet	- набор вариантов написания слова
* @param[out]	aResultFlag			- флаг результата:
*										0 - подмотаться не удалось
*										1 - мы подмотались к заданному слову
*										2 - мы подмотались к ближайшему слову по виртуальному 
*											идентификатору (запрос и Show-вариант не совпадают)
*
* @return код ошибки
************************************************************************/
ESldError CSldList::GetExactWordBySetOfVariants(TWordVariantsSet* aWordVariantsSet, UInt32* aResultFlag)
{
	if (!aWordVariantsSet || !aResultFlag)
		return eMemoryNullPointer;

	*aResultFlag = 0;
	ESldError error;

	if(HasSimpleSortedList())
	{
		// находим в наборе вариантов show-вариант
		const UInt16 *showVariant = NULL;
		for (UInt32 variantIndex = 0; variantIndex < aWordVariantsSet->GetCount(); variantIndex++)
		{
			if (aWordVariantsSet->GetVariantType(variantIndex) == eVariantShow)
			{
				showVariant = aWordVariantsSet->GetWordVariant(variantIndex);
				break;
			}
		}
		if (!showVariant)
			return eOK;

		if(m_Catalog)
		{
			error = SetBase(MAX_UINT_VALUE);
			if (error != eOK)
				return error;
		}

		// по show-варианту в SimpleSorted списке подматываемся к слову
		Int32 globalIndexInRealList = -1;
		error = m_SortedList.GetSortedIndexByText(showVariant, &globalIndexInRealList);
		if (error != eOK)
			return error;

		Int32 numberOfWordsInSortedList = 0;
		error = m_SortedList.GetNumberOfWords(&numberOfWordsInSortedList);
		if (error != eOK)
			return error;

		Int32 currentIndexInSortedList = -1;
		error = m_SortedList.GetCurrentIndex(&currentIndexInSortedList);
		if (error != eOK)
			return error;
		do
		{
			// GetNextWordSortedIndex использовать не получается, т.к. нам необходимо сравнить слово именно из SimpleSorted списка (оно может быть альтернативным/виртуальным),
			// а не из основного (альтернативного/виртуального слова в основном списке не существует). Поэтому переходим по SimpleSorted списку просто по индексу.
			error = m_SortedList.GetSortedIndexByIndex(currentIndexInSortedList++, &globalIndexInRealList);
			if (error != eOK)
				return error;

			// получаем слово, к которому подматались
			UInt16* currentWordInSortedList = NULL;
			error = m_SortedList.GetCurrentWord(&currentWordInSortedList);
			if (error != eOK)
				return error;

			// проверяем совпадают ли бинарно найденное слово и show-вариант из набора, если нет, то заканчиваем поиск с нулевым результатом
			if (GetCMP()->StrCmp(currentWordInSortedList, showVariant))
				return eOK;

			// если show-вариант подходит, то проверям остальные варианты из набора

			// подматываемся к слову в основном списке
			error = GetWordByGlobalIndex(globalIndexInRealList);
			if (error != eOK)
				return error;

			// флаг совпадения вариантов
			UInt8 AreVariantsMatching = 0;
			// перебираем переданные варианты
			for (UInt32 setVariantIndex = 0; setVariantIndex < aWordVariantsSet->GetCount(); setVariantIndex++)
			{
				const auto variantType = static_cast<EListVariantTypeEnum>(aWordVariantsSet->GetVariantType(setVariantIndex));

				// show-вариант пропускаем, так как его уже проверяли
				if (aWordVariantsSet->GetVariantType(setVariantIndex) == eVariantShow)
					continue;

				// ищем такой же вариант в списке
				const Int32 listVariantIndex = GetListInfo()->GetVariantIndexByType(variantType);

				// не нашли такого варианта - игнорируем его
				if (listVariantIndex == SLD_DEFAULT_VARIANT_INDEX)
					continue;

				// получаем вариант написания из реального списка
				UInt16* currentWordInRealList = NULL;
				error = GetCurrentWord(listVariantIndex, &currentWordInRealList);
				if (error != eOK)
					return error;

				// если варианты написания не совпадают бинарно, то данное слово из реального списка не подходит
				if (GetCMP()->StrCmp(currentWordInRealList, aWordVariantsSet->GetWordVariant(setVariantIndex)))
				{
					AreVariantsMatching = 0;
					break;
				}
				else
					AreVariantsMatching = 1;
			}
			// если вышли из цикла с 1 во флаге, значит все варианты совпали и результат поиска успешный
			if (AreVariantsMatching)
			{
				if (m_SortedList.IsNormalWord())
					*aResultFlag = 1;
				else
					*aResultFlag = 2;

				if(m_Catalog)
				{
					TCatalogPath path;
					error = GetPathByGlobalIndex(globalIndexInRealList, &path);
					if (error != eOK)
						return error;

					error = GoToByPath(&path, eGoToWord);
					if (error != eOK)
						return error;
				}
				return eOK;
			}
		}
		while (currentIndexInSortedList < numberOfWordsInSortedList);
	}

	return eOK;
}

/** *********************************************************************
* Ищет наиболее подходящее слово по переданным вариантам написания
*
* @param[in]	aWordVariantsSet	- набор вариантов написания слова
* @param[in]	aWordSearchType		- тип (правила) поиска слова (см. ESldWordSearchTypeEnum)
* @param[in]	aActionsOnFailFlag	- флаг, определяющий нужно ли подматываться к наиболее подходящем слову, если поиск завершился неудачно
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
ESldError CSldList::GetWordBySetOfVariants(TWordVariantsSet* aWordVariantsSet, UInt32 aWordSearchType, UInt32* aResultFlag, UInt32 aActionsOnFailFlag)
{
	if (!aWordVariantsSet || !aResultFlag)
		return eMemoryNullPointer;

	*aResultFlag = 0;
	ESldError error;

	// ищем в наборе либо show-вариант, либо sortkey (show-вариант приоритетнее)
	const UInt16 *showVariant = NULL;
	const UInt16 *sortKey = NULL;
	for (UInt32 variantIndex = 0; variantIndex < aWordVariantsSet->GetCount(); variantIndex++)
	{
		if (aWordVariantsSet->GetVariantType(variantIndex) == eVariantShow)
		{
			showVariant = aWordVariantsSet->GetWordVariant(variantIndex);
			break;
		}
		if (aWordVariantsSet->GetVariantType(variantIndex) == eVariantSortKey)
			sortKey = aWordVariantsSet->GetWordVariant(variantIndex);
	}
	if (!showVariant && !sortKey)
		return eOK;

	// получаем количество вариантов написания в списке
	UInt32 numberOfVariants = 0;
	error = GetNumberOfVariants(&numberOfVariants);
	if (error != eOK)
		return error;

	// подготовка к вычислению расстояний редактирования
	Int32 fuzzyBuffer[ARRAY_DIM][ARRAY_DIM];
	UInt32 showVariantLen = GetCMP()->StrLen(showVariant);
	UInt32 patternLen = showVariantLen + 1;
	if (patternLen >= MAX_FUZZY_WORD)
		return eCommonTooLargeText;
	for (UInt32 i = 0; i < ARRAY_DIM; i++)
	{
		fuzzyBuffer[TIO(i, 0)] = i;
		fuzzyBuffer[TIO(0, i)] = i;
	}

	// индекс наиболее подходящего слова
	Int32 bestIndex = -1;
	// минимальное расстояние редактирования show-варианта
	UInt32 minDistance = 0xFFFF;
	// максимальный вес слова по всем остальным вариантам
	UInt32 maxWeight = 0;

	// если передан show-вариант, то ищем по нему
	if (showVariant)
	{
		// ищем индекс show-варианта в списке
		const Int32 showVariantIndex = GetListInfo()->GetVariantIndexByType(eVariantShow);
		if (showVariantIndex == SLD_DEFAULT_VARIANT_INDEX)
			return eCommonWrongVariantIndex;

		if (aActionsOnFailFlag && GetBaseIndex())
		{
			*aResultFlag = 0;
			return eOK;
		}

		// ищем show-вариант в SimpleSorted списке, если он есть, или в основном списке
		UInt32 bySortedTextResult;
		error = GetWordBySortedText(showVariant, &bySortedTextResult);
		if (error != eOK)
			return error;

		// если нет SimpleSorted списка и основной список не сортирован, то найти мы ничего не сможем
		if (bySortedTextResult == 0)
			return eOK;

		// получаем индекс слова, к которому подматались, и само это слово
		Int32 currentIndex = -1;
		UInt16* currentWord = NULL;
		if (HasSimpleSortedList())
		{
			// перед получением индекса, проведем бинарный поиск слова в SimpleSorted списке.
			// Часть кода для этого поиска скопирована из функции GetWordByTextExtended
			Int32 currentSortedIndex = 0;

			error = m_SortedList.FindBinaryMatch(showVariant, aResultFlag);
			if (error != eOK)
				return error;

			if (!*aResultFlag && !aActionsOnFailFlag)
				return eOK;

			error = m_SortedList.GetCurrentSortedIndex(&currentSortedIndex);
			if (error != eOK)
				return error;

			if (aActionsOnFailFlag || m_LocalizedBaseGlobalIndex != SLD_DEFAULT_WORD_INDEX)
			{
				Int32 high = 0;
				Int32 low = 0;
				Int32 sortedWordsCount = 0;

				const ESldSearchRange searchRange = aActionsOnFailFlag == 0 ? eSearchRangeFullRecursive : eSearchRangeRoot;
				error = GetSearchBounds(searchRange, &low, &high);
				if (error != eOK)
					return error;

				error = m_SortedList.GetNumberOfWords(&sortedWordsCount);
				if (error != eOK)
					return error;

				error = m_SortedList.GetCurrentIndex(&currentIndex);
				if (error != eOK)
					return error;

				bool changeResult = false;
				while ((currentIndex + 1) < sortedWordsCount && !sld2::InRange<Int32>(currentSortedIndex, low, high))
				{
					changeResult = true;
					error = m_SortedList.GetNextWordSortedIndex(&currentSortedIndex);
					if (error != eOK)
						return error;

					error = m_SortedList.GetCurrentIndex(&currentIndex);
					if (error != eOK)
						return error;

					if (m_LocalizedBaseGlobalIndex != SLD_DEFAULT_WORD_INDEX)
					{
						error = m_SortedList.GetCurrentWord(&currentWord);

						if (GetCMP()->StrICmp(currentWord, showVariant) != 0)
						{
							*aResultFlag = 0;
							return eOK;
						}
					}
				}

				if (currentIndex + 1 >= sortedWordsCount)
				{
					currentSortedIndex = high - 1;
				}

				if (changeResult)
				{
					error = m_SortedList.GetCurrentWord(&currentWord);

					if (GetCMP()->StrICmp(currentWord, showVariant) != 0)
					{
						*aResultFlag = 0;
					}
				}
			}

			error = GetWordByGlobalIndex(currentSortedIndex, false);
			if (error != eOK)
				return error;

			// получаем индекс слова
			error = m_SortedList.GetCurrentIndex(&currentIndex);
			if (error != eOK)
				return error;

			error = m_SortedList.GetCurrentWord(&currentWord);
			if (error != eOK)
				return error;
		}
		else
		{
			error = GetCurrentIndex(&currentIndex);
			if (error != eOK)
				return error;

			error = GetCurrentWord(showVariantIndex, &currentWord);
			if (error != eOK)
				return error;
		}

		// если слово, к которому подмотались, не совпадает по массе с show-вариантом, то завершаем поиск
		if (GetCMP()->StrICmp(showVariant, currentWord))
		{
			// пытаемся подмотаться по sortkey'ю (если он передан), либо оставляем, что было найдено по show-варианту
			if (sortKey)
				return GetWordByText(sortKey);
			else
				return eOK;
		}

		bestIndex = currentIndex;
		// флаг, определяющий было ли уже найдено слово с полным совпадением вариантов по массе
		UInt32 isFullMassMatch = 0;

		// перебираем слова из диапазона
		while (GetCMP()->StrICmp(showVariant, currentWord) == 0 && currentIndex + 1 < GetSortedWordsCount())
		{
			UInt32 currentDistance = 0;
			UInt32 currentWeight = 0;

			// если слово бинарно не совпадает с show-вариантом, то вычисляем расстояние редактирования
			if (GetCMP()->StrCmp(showVariant, currentWord))
			{
				Int32 len = GetCMP()->StrLen(currentWord);
				// удаляем из слова разделители в начале и в конце
				const auto currentWordWithoutDelimiters = GetCMP()->TrimDelimiters(currentWord);

				// вычисляем расстояние редактирования
				if (len < ARRAY_DIM)
					currentDistance = GetCMP()->FuzzyCompare(currentWordWithoutDelimiters.c_str(), showVariant, currentWordWithoutDelimiters.length() + 1, patternLen, fuzzyBuffer);
				else
					currentDistance = len;
			}

			// флаг, определяющий возможность изменения значения индекса наиболее подходящего слова
			UInt32 probablyNeedToChangeBestIndex = 0;
			// в следующем случае нам необходимо, помимо веса, знать было ли полное совпадение вариантов по массе
			// поэтому вызывать функцию вычисления веса приходится вне зависимости от вычисленного расстояния
			if (aWordSearchType == eFullMassMatch && !isFullMassMatch)
			{
				// получаем вес и флаги совпадения вариантов
				UInt32 isCurrentWordFullBinaryMatch = 0;
				UInt32 isCurrentWordFullMassMatch = 0;
				error = GetCurrentWordVariantsWeight(aWordVariantsSet, &currentWeight, &isCurrentWordFullBinaryMatch, &isCurrentWordFullMassMatch);
				if (error != eOK)
					return error;

				// если было полное бинарное совпадение, то завершаем поиск успешно, лучшего слова мы уже не найдем
				if (isCurrentWordFullBinaryMatch && !currentDistance)
				{
					if (IsNormalWord() == SLD_SIMPLE_SORTED_NORMAL_WORD)
						*aResultFlag = 1;
					else
						*aResultFlag = 2;
					return eOK;
				}
				// если нашли (первый раз) слово с полным совпадением по массе, то лучшие расстояние и вес лучшие вне зависимости от предыдущих значений
				if (isCurrentWordFullMassMatch)
				{
					isFullMassMatch = 1;
					minDistance = currentDistance;
					maxWeight = currentWeight;
					bestIndex = currentIndex;
				}
				// иначе устанавливаем флаг возможности изменения индекса, только если расстояние было <= минимальному расстоянию
				else if (currentDistance <= minDistance)
					probablyNeedToChangeBestIndex = 1;
			}
			// в остальных случаях вес вычисляется только если расстояние было <= минимальному расстоянию (оптимизация)
			else if (currentDistance <= minDistance)
			{
				UInt32 isCurrentWordFullBinaryMatch = 0, isCurrentWordFullMassMatch = 0;
				error = GetCurrentWordVariantsWeight(aWordVariantsSet, &currentWeight, &isCurrentWordFullBinaryMatch, &isCurrentWordFullMassMatch);
				if (error != eOK)
					return error;
				// если было полное бинарное совпадение, то завершаем поиск успешно, лучшего слова мы уже не найдем
				if (isCurrentWordFullBinaryMatch && !currentDistance)
				{
					if (IsNormalWord() == SLD_SIMPLE_SORTED_NORMAL_WORD)
						*aResultFlag = 1;
					else
						*aResultFlag = 2;
					return eOK;
				}
				// при eFullMassMatch необходимо выполнять такие же действия, если это уже не первое слово с флагом полного совпадения по массе
				if (aWordSearchType != eFullMassMatch || (aWordSearchType == eFullMassMatch && isCurrentWordFullMassMatch))
					probablyNeedToChangeBestIndex = 1;
			}
			// проверяем, нужно ли все-таки изменять значение индекса наиболее подходящего слова
			if (probablyNeedToChangeBestIndex)
			{
				if (currentDistance < minDistance)
				{
					minDistance = currentDistance;
					maxWeight = currentWeight;
					bestIndex = currentIndex;
				}
				else if (currentWeight > maxWeight)
				{
					maxWeight = currentWeight;
					bestIndex = currentIndex;
				}
			}

			// получаем следующее слово
			error = GetNextSortedWord();
			if (error != eOK)
				return error;

			if (HasSimpleSortedList())
			{
				error = m_SortedList.GetCurrentIndex(&currentIndex);
				if (error != eOK)
					return error;

				error = m_SortedList.GetCurrentWord(&currentWord);
				if (error != eOK)
					return error;
			}
			else
			{
				error = GetCurrentIndex(&currentIndex);
				if (error != eOK)
					return error;

				error = GetCurrentWord(showVariantIndex, &currentWord);
				if (error != eOK)
					return error;
			}
		}

		// подматываемся к наиболее подходящему слову независимо от результата поиска
		error = GetWordBySortedIndex(bestIndex);
		if (error != eOK)
			return error;

		if ((aWordSearchType == eShowVariantBinaryMatch && !minDistance) || aWordSearchType == eShowVariantFuzzyMatch || (aWordSearchType == eFullMassMatch && isFullMassMatch))
		{
			if (IsNormalWord() == SLD_SIMPLE_SORTED_NORMAL_WORD)
				*aResultFlag = 1;
			else
				*aResultFlag = 2;
		}
	}
	else
	{
		// если передан только sortKey, то тип поиска может быть только полное бинарное совпадение, иначе ничего не делаем
		if (aWordSearchType != eFullBinaryMatch)
			return eOK;

		// ищем индекс sortKey'я в списке
		const Int32 sortKeyIndex = GetListInfo()->GetVariantIndexByType(eVariantSortKey);
		if (sortKeyIndex == SLD_DEFAULT_VARIANT_INDEX)
			return eCommonWrongVariantIndex;

		// подматывается к слову по sortKey'ю
		error = GetWordByText(sortKey);
		if (error != eOK)
			return error;

		// получаем индекс слова, к которому подматались, и само это слово
		Int32 currentIndex = -1;
		UInt16* currentWord = NULL;
		error = GetCurrentIndex(&currentIndex);
		if (error != eOK)
			return error;

		error = GetCurrentWord(sortKeyIndex, &currentWord);
		if (error != eOK)
			return error;

		// если слово, к которому подмотались, не совпадает по массе с sortKey'ем, то завершаем поиск
		if (GetCMP()->StrICmp(sortKey, currentWord))
			return eOK;

		// получаем количество слов на уровне
		Int32 numberOfWords = 0;
		error = GetNumberOfWords(&numberOfWords);
		if (error != eOK)
			return error;

		// перебираем слова из диапазона
		while (GetCMP()->StrICmp(sortKey, currentWord) == 0 && currentIndex++ < numberOfWords)
		{
			UInt32 currentDistance = 0;
			UInt32 currentWeight = 0;

			// если слово бинарно не совпадает с sortKey'ем, то вычисляем расстояние редактирования
			if (GetCMP()->StrCmp(sortKey, currentWord))
			{
				Int32 len = GetCMP()->StrLen(currentWord);
				// удаляем из слова разделители в начале и в конце
				const auto currentWordWithoutDelimiters = GetCMP()->TrimDelimiters(currentWord);

				// вычисляем расстояние редактирования
				if (len < ARRAY_DIM)
					currentDistance = GetCMP()->FuzzyCompare(currentWordWithoutDelimiters.c_str(), sortKey, currentWordWithoutDelimiters.length() + 1, patternLen, fuzzyBuffer);
				else
					currentDistance = len;
			}

			if (currentDistance <= minDistance)
			{
				UInt32 isCurrentWordFullBinaryMatch = 0, isCurrentWordFullMassMatch = 0;
				error = GetCurrentWordVariantsWeight(aWordVariantsSet, &currentWeight, &isCurrentWordFullBinaryMatch, &isCurrentWordFullMassMatch);
				if (error != eOK)
					return error;

				// если было полное бинарное совпадение, то завершаем поиск успешно, лучшего слова мы уже не найдем
				if (isCurrentWordFullBinaryMatch)
				{
					*aResultFlag = 1;
					return eOK;
				}
				if (currentDistance < minDistance)
				{
					minDistance = currentDistance;
					maxWeight = currentWeight;
					bestIndex = GetCurrentWordIndex();
				}
				else if (currentWeight > maxWeight)
				{
					maxWeight = currentWeight;
					bestIndex = GetCurrentWordIndex();
				}
			}

			error = GetWordByIndex(currentIndex);
			if (error != eOK)
				return error;

			error = GetCurrentWord(sortKeyIndex, &currentWord);
			if (error != eOK)
				return error;
		}

		// подматываемся к наиболее подходящему слову даже при неудаче
		return GetWordByGlobalIndex(bestIndex);
	}

	return eOK;
}

/** *********************************************************************
* Получает вес текущего слова при сравнении с переданными вариантами написания
*
* @param[in]	aWordVariantsSet	- набор вариантов написания слова
* @param[out]	aResultWeight		- итоговый вес слова
* @param[out]	aIsFullBinaryMatch	- флаг, определяющий, совпали ли все варианты написания бинарно
* @param[out]	aIsFullMassMatch	- флаг, определяющий, совпали ли все варианты написания по массе
*
* @return код ошибки
************************************************************************/
ESldError CSldList::GetCurrentWordVariantsWeight(TWordVariantsSet* aWordVariantsSet, UInt32* aResultWeight, UInt32* aIsFullBinaryMatch, UInt32* aIsFullMassMatch)
{
	if (!aWordVariantsSet || !aResultWeight || !aIsFullBinaryMatch || !aIsFullMassMatch)
		return eMemoryNullPointer;

	*aResultWeight = 0;
	*aIsFullBinaryMatch = 1;
	*aIsFullMassMatch = 1;

	UInt32 massMatchWeight = 1;
	UInt32 binaryMatchWeight = 2;
	UInt32 sortKeyCoefficient = binaryMatchWeight * aWordVariantsSet->GetCount() + 1;
	UInt32 showSecondaryCoefficient = binaryMatchWeight * sortKeyCoefficient + sortKeyCoefficient;

	// перебираем переданные варианты
	for (UInt32 setVariantIndex = 0; setVariantIndex < aWordVariantsSet->GetCount(); setVariantIndex++)
	{
		// show-вариант пропускаем
		const auto variantType = static_cast<EListVariantTypeEnum>(aWordVariantsSet->GetVariantType(setVariantIndex));
		if (variantType == eVariantShow)
			continue;

		// ищем такой же вариант в списке
		Int32 listVariantIndex = GetListInfo()->GetVariantIndexByType(variantType);
		// не нашли такого варианта - игнорируем его
		if (listVariantIndex == SLD_DEFAULT_VARIANT_INDEX)
			continue;

		// получаем вариант написания
		UInt16* currentWordVariant = NULL;
		ESldError error = GetCurrentWord(listVariantIndex, &currentWordVariant);
		if (error != eOK)
			return error;
		
		// вычисляем вес
		UInt32 matchWeight = 0;
		if (!GetCMP()->StrCmp(aWordVariantsSet->GetWordVariant(setVariantIndex), currentWordVariant))
			matchWeight = binaryMatchWeight;
		else if (!GetCMP()->StrICmp(aWordVariantsSet->GetWordVariant(setVariantIndex), currentWordVariant))
		{
			matchWeight = massMatchWeight;
			*aIsFullBinaryMatch = 0;
		}
		else
		{
			*aIsFullBinaryMatch = 0;
			*aIsFullMassMatch = 0;
		}

		switch (variantType)
		{
		case eVariantShowSecondary:
			*aResultWeight += (matchWeight * showSecondaryCoefficient);
			break;
		case eVariantSortKey:
			*aResultWeight += (matchWeight * sortKeyCoefficient);
			break;
		default:
			*aResultWeight += matchWeight;
		}
	}

	return eOK;
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
ESldError CSldList::FindBinaryMatch(const UInt16* aText, UInt32* aResult)
{
	if(!aText || !aResult)
		return eMemoryNullPointer;

	*aResult = 0;

	ESldError error;
	Int32 firstCompareIndex = GetCurrentWordIndex();
	UInt32 currentPos = 0;
	Int32 massCompareResult = 0;
	Int8 altCompare = 0;

	if(m_LastShift == -1)
	{
		if (GetListInfo()->GetNumberOfGlobalWords() > 1)
		{
			error = GetWordByGlobalIndex(GetListInfo()->GetNumberOfGlobalWords() - 2);
			if (error != eOK)
				return error;
			m_LastShift = m_Input->GetCurrentPosition();

			error = GetWordByGlobalIndex(firstCompareIndex);
			if (error != eOK)
				return error;
		}
		else
		{
			m_LastShift = 0;
		}

		m_LastShift = m_AltNameManager.GetShiftForLastWord() > m_LastShift ? m_AltNameManager.GetShiftForLastWord() : m_LastShift;
	}

	if (GetCurrentWordIndex() < 1)
	{
		currentPos = 0;
		error = m_Input->SetCurrentPosition(0);
		if (error != eOK)
			return error;

		m_CurrentIndex = SLD_DEFAULT_WORD_INDEX;
	}
	else
	{
		error = GetWordByGlobalIndex(GetCurrentWordIndex() - 1);
		if (error != eOK)
			return error;
	}

	currentPos = m_Input->GetCurrentPosition();

	while(currentPos <= m_LastShift)
	{
		error = GetNextWord();
		if (error != eOK)
			return error;

		if(m_AltNameManager.m_LastShift == -1)
		{
			massCompareResult = GetCMP()->StrICmp(GetWord(0), aText);
			if(massCompareResult <= 0)
			{
				if (GetCMP()->StrCmp(GetWord(0), aText) == 0 || (GetListInfo()->GetShowVariantIndex() && GetCMP()->StrCmp(GetWord(GetListInfo()->GetShowVariantIndex()), aText) == 0))
				{
					*aResult = 1;
					return eOK;
				}

				if(massCompareResult == 0 && altCompare > -1)
				{
					altCompare = -1;
					firstCompareIndex = GetCurrentWordIndex();
					*aResult = 0;
				}
			}
			else
			{
				break;
			}
		}
		else
		{
			massCompareResult = GetCMP()->StrICmp(m_AltNameManager.m_AlternativeWord.data(), aText);
			if(massCompareResult <= 0)
			{
				if (GetCMP()->StrCmp(m_AltNameManager.m_AlternativeWord.data(), aText) == 0)
				{
					UInt32 realWordCount = 0;
					const UInt32* realIndexes = NULL;

					error = m_AltNameManager.GetAltnameInfo(currentPos, &realWordCount, &realIndexes);
					if (error != eOK)
						return error;

					if(realWordCount)
					{
						firstCompareIndex = realIndexes[0];
						*aResult = 2;
						break;
					}
				}

				if(massCompareResult == 0 && !altCompare)
				{
					UInt32 realWordCount = 0;
					const UInt32* realIndexes = NULL;

					error = m_AltNameManager.GetAltnameInfo(currentPos, &realWordCount, &realIndexes);
					if (error != eOK)
						return error;

					if(realWordCount)
					{
						altCompare = 1;
						firstCompareIndex = realIndexes[0];
						*aResult = 2;
					}
				}
			}
			else
			{
				break;
			}
		}
		currentPos = m_Input->GetCurrentPosition();
	}

	error = GetWordByGlobalIndex(firstCompareIndex);
	if (error != eOK)
		return error;

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
ESldError CSldList::GetFullTextResultCount(const UInt16* aRequest, Int32* aCount)
{
	if(!aRequest || !aCount)
		return eMemoryNullPointer;

	*aCount = 0;
	ESldError error = GetWordByText(aRequest);
	if (error != eOK)
		return error;

	Int32 translationCount = 0;
	while (GetCMP()->StrICmp(aRequest, GetWord(0)) == 0)
	{
		error = GetTranslationCount(GetCurrentWordIndex(), &translationCount);
		if (error != eOK)
			return error;

		*aCount += translationCount;

		if (GetCurrentWordIndex() + 1 < GetListInfo()->GetNumberOfGlobalWords())
		{
			error = GetNextWord();
			if (error != eOK)
				return error;
		}
		else
		{
			break;
		}
	}

	return eOK;
}

/** *********************************************************************
* Возвращает индекс статьи, содержащей стилизованный текст варианта написания для текущего слова
*
* @param[in]	aVariantIndex	- Индекс варианта написания
*
* @return индекс статьи 
************************************************************************/
Int32 CSldList::GetStylizedVariantArticleIndex(UInt32 aVariantIndex) const
{
	const CSldListInfo *listInfo = GetListInfo();
	if (listInfo->IsStylizedVariant() != 0 && aVariantIndex < listInfo->GetNumberOfVariants())
	{
		return m_StylizedVariant[aVariantIndex];
	}

	return SLD_INDEX_STYLIZED_VARIANT_NO;
}

/** *********************************************************************
* Возвращает индекс стиля для заданного варианта написания из eVariantLocalizationPreferences
*
* @param[in]	aVariantIndex	- номер варианта написания, для которого мы хотим получить стиль
*
* @return индекс стиля варианта написания
************************************************************************/
Int32 CSldList::GetStyleVariantIndex(Int32 aVariantIndex)
{
	const CSldListInfo * listInfo;
	ESldError error = GetWordListInfo(&listInfo);
	if (error != eOK)
		return 0;

	const UInt32 variantStylePreferencesIndex = listInfo->GetVariantIndexByType(eVariantStylePreferences);
	if (variantStylePreferencesIndex == SLD_DEFAULT_VARIANT_INDEX)
		return 0;

	UInt16 * word;
	error = GetCurrentWord(variantStylePreferencesIndex, &word);
	if (error != eOK)
		return 0;

	SldU16WordsArray words;
	error = GetCMP()->DivideQueryByParts(word, words);
	if (error != eOK)
		return 0;

	Int32 variantIndex = SLD_DEFAULT_VARIANT_INDEX;
	UInt32 preferencesIndex = 0;
	for (preferencesIndex; preferencesIndex < words.size(); preferencesIndex += 2)
	{
		error = GetCMP()->StrToInt32(words[preferencesIndex].c_str(), 10, &variantIndex);
		if (error != eOK)
			return 0;

		if (variantIndex >= aVariantIndex)
			break;
	}

	if (aVariantIndex != variantIndex)
		return 0;

	auto & styleIndexString = words[preferencesIndex + 1];
	UInt32 charIndex = 0;
	while (charIndex < styleIndexString.size())
	{
		if (!CMP_IS_DIGIT(styleIndexString[charIndex]))
			styleIndexString.erase(charIndex, 1);
		else
			charIndex++;
	}

	Int32 styleIndex = 0;
	error = GetCMP()->StrToInt32(styleIndexString.c_str(), 10, &styleIndex);
	if (error != eOK)
		return 0;

	return styleIndex;
}

/** *********************************************************************
* Переходит по пути, соответсвующему заданному глобальному индексу
*
* @param[in]	aGlobalIndex	- Глобальный индекс, к которому мы хотим перейти
*
* @return код ошибки
************************************************************************/
ESldError CSldList::GoToByGlobalIndex(const Int32 aGlobalIndex)
{
	if (m_Catalog == NULL)
	{
		return GetWordByGlobalIndex(aGlobalIndex);
	}

	TCatalogPath path;
	ESldError error = GetPathByGlobalIndex(aGlobalIndex, &path);
	if(error != eOK)
		return error;

	error = GoToByPath(&path, eGoToWord);
	if(error != eOK)
		return error;

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
ESldError CSldList::SortListByVariant(const Int32 aVariantIndex, const Int8 aCreateWordsBuffer)
{
	if (aVariantIndex >= GetListInfo()->GetNumberOfVariants())
		return eCommonWrongVariantIndex;

	if (m_SortedWordIndexes && aVariantIndex == m_SortedVariantIndex)
		return eOK;

	if (m_SortedWordIndexes)
	{
		sldMemFree(m_SortedWordIndexes);
		m_SortedWordIndexes = NULL;
	}

	if (m_WordsBuff)
	{
		for (UInt32 wordIndex = 0; wordIndex < GetListInfo()->GetNumberOfGlobalWords(); wordIndex++)
		{
			sldMemFree(m_WordsBuff[wordIndex]);
		}
		sldMemFree(m_WordsBuff);
		m_WordsBuff = NULL;
	}

	if (!aVariantIndex && !aCreateWordsBuffer && !m_Catalog)
	{
		m_SortedVariantIndex = aVariantIndex;
		return eOK;
	}

	m_SortedVariantIndex = aVariantIndex;

	UInt32 wordsCount = GetListInfo()->GetNumberOfGlobalWords();
	if (wordsCount <= 1)
		return eOK;

	m_WordsBuff = (UInt16**)sldMemNewZero(wordsCount * sizeof(UInt16*));
	if (!m_WordsBuff)
		return eMemoryNotEnoughMemory;

	m_SortedWordIndexes = (UInt32*)sldMemNew(wordsCount * 2 * sizeof(UInt32));
	if (!m_SortedWordIndexes)
		return eMemoryNotEnoughMemory;

	Int32 firstIndex = 0;
	Int32 secondIndex = wordsCount;
	Int32 medIndex = wordsCount >> 1;

	ESldError error = InsetWordToBuffer(medIndex);
	if (error != eOK)
		return error;

	Int32 compareResult = 0;
	for (UInt32 wordIndex = 0; wordIndex < wordsCount; wordIndex++)
	{
		if (wordIndex == medIndex)
		{
			m_SortedWordIndexes[firstIndex] = wordIndex;
			firstIndex++;
			continue;
		}

		error = InsetWordToBuffer(wordIndex);
		if (error != eOK)
			return error;

		compareResult = GetCMP()->StrICmp(m_WordsBuff[wordIndex], m_WordsBuff[medIndex]);
		if (!compareResult)
		{
			compareResult = GetCMP()->StrCmp(m_WordsBuff[wordIndex], m_WordsBuff[medIndex]);
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

	sldMemMove(&m_SortedWordIndexes[firstIndex], &m_SortedWordIndexes[wordsCount], (secondIndex - wordsCount) * sizeof(UInt32));

	if (firstIndex - 1 > 0)
	{
		error = QuickSortSearchVector(0, firstIndex - 1);
		if (error != eOK)
			return error;
	}

	if (firstIndex < wordsCount - 1)
	{
		error = QuickSortSearchVector(firstIndex, wordsCount - 1);
		if (error != eOK)
			return error;
	}

	if (m_WordsBuff && !aCreateWordsBuffer)
	{
		for (UInt32 wordIndex = 0; wordIndex < wordsCount; wordIndex++)
		{
			if (m_WordsBuff[wordIndex])
			{
				sldMemFree(m_WordsBuff[wordIndex]);
			}
		}
		sldMemFree(m_WordsBuff);
		m_WordsBuff = NULL;
	}

	return eOK;
}

/** *********************************************************************
* Переходит по глобальному индексу, добавляет m_SortedVariantIndex в буфер m_WordsBuff
*
* @param[in]	aIndex	- индекс слова, которое мы хотим добавить в буфер
*
* @return код ошибки
************************************************************************/
ESldError CSldList::InsetWordToBuffer(const Int32 aIndex)
{
	ESldError error = GetWordByGlobalIndex(aIndex);
	if (error != eOK)
		return error; 

	if (m_WordsBuff[aIndex])
	{
		sldMemFree(m_WordsBuff[aIndex]);
		m_WordsBuff[aIndex] = NULL;
	}

	m_WordsBuff[aIndex] = (UInt16*)sldMemNew((CSldCompare::StrLen(GetWord(m_SortedVariantIndex)) + 1) * sizeof(UInt16));
	if (!m_WordsBuff[aIndex])
		return eMemoryNotEnoughMemory;

	CSldCompare::StrCopy(m_WordsBuff[aIndex], GetWord(m_SortedVariantIndex));

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
ESldError CSldList::QuickSortSearchVector(const Int32 aFirstIndex, const Int32 aLastIndex)
{
	ESldError error = eOK;
	Int32 medIndex = (aFirstIndex + aLastIndex) >> 1;
	UInt16* medWord = m_WordsBuff[m_SortedWordIndexes[medIndex]];

	UInt32 wordsCount = GetListInfo()->GetNumberOfGlobalWords();
	Int32 firstIndex = aFirstIndex;
	Int32 secondIndex = wordsCount;
	UInt32 medCount = 0;

	Int32 compareResult = 0;
	for (UInt32 currentIndex = aFirstIndex; currentIndex <= aLastIndex; currentIndex++)
	{
		if (currentIndex == medIndex)
		{
			compareResult = 0;
		}
		else
		{
			compareResult = GetCMP()->StrICmp(m_WordsBuff[m_SortedWordIndexes[currentIndex]], medWord);
			if (!compareResult)
			{
				compareResult = GetCMP()->StrCmp(m_WordsBuff[m_SortedWordIndexes[currentIndex]], medWord);
			}
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
			m_SortedWordIndexes[2 * wordsCount - medCount] = m_SortedWordIndexes[currentIndex];
		}
	}

	if (medCount > 1)
	{
		for (UInt32 i = 0; i < medCount; i++)
		{
			m_SortedWordIndexes[firstIndex + i] = m_SortedWordIndexes[2 * wordsCount - 1 - i];
		}
	}
	else
	{
		sldMemMove(&m_SortedWordIndexes[firstIndex], &m_SortedWordIndexes[2 * wordsCount - medCount], medCount* sizeof(UInt32));
	}

	sldMemMove(&m_SortedWordIndexes[firstIndex + medCount], &m_SortedWordIndexes[wordsCount], (secondIndex - wordsCount) * sizeof(UInt32));

	if (firstIndex - 1 > aFirstIndex)
	{
		error = QuickSortSearchVector(aFirstIndex, firstIndex - 1);
		if (error != eOK)
			return error;
	}

	if (secondIndex - medCount > wordsCount)
	{
		error = QuickSortSearchVector(firstIndex + medCount, aLastIndex);
		if (error != eOK)
			return error;
	}

	return eOK;
}

/** *********************************************************************
* Получает реальные индексы списка и записи в реальном списке в по глобальному номеру слова
*
* @param[in]	aWordIndex			- глобальный номер слова
* @param[in]	aTrnslationIndex	- номер индексов которые мы хотим получить (можно получить через GetNumberOfTranslations() для списков типа IsFullTextSearchList())
* @param[out]	aRealListIndex		- указатель на переменную, в которую сохраняется индекс списка
* @param[out]	aGlobalWordIndex	- указатель на переменную, в которую сохраняется глобальный номер слова
*
* @return код ошибки
************************************************************************/
ESldError CSldList::GetRealIndexes(Int32 aWordIndex, Int32 aTrnslationIndex, Int32* aRealListIndex, Int32* aGlobalWordIndex)
{
	if (!aRealListIndex || !aGlobalWordIndex)
		return eMemoryNullPointer;

	if (aWordIndex < 0 || aTrnslationIndex < 0)
		return eCommonWrongIndex;

	if (aWordIndex >= GetListInfo()->GetNumberOfGlobalWords())
		return eCommonWrongIndex;

	Int32 linkCount = 0;
	ESldError error = GetReferenceCount(aWordIndex, &linkCount);
	if (error != eOK)
		return error;

	if (aTrnslationIndex >= linkCount)
	{
		*aGlobalWordIndex = SLD_DEFAULT_WORD_INDEX;
		return eOK;
	}

	Int32 translationIndex = 0;
	Int32 shiftIndex = 0;

	return GetFullTextTranslationData(aWordIndex, aTrnslationIndex, aRealListIndex, aGlobalWordIndex, &translationIndex, &shiftIndex);
}

/** *********************************************************************
* Сортирует список, согласно заданному массиву индексов
*
* @param[in]	aWordIndexes	- соритрованный вектор с индексами
*
* @return код ошибки
************************************************************************/
ESldError CSldList::SortByVector(const CSldVector<Int32> & aWordIndexes)
{
	// Эта функция вызываться не должна
	return eCommonWrongList;
}

/** *********************************************************************
* Возвращает количество слов на текущем уровне включая подкаталоги
*
* @param[in]	aGlobalIndex	- глобальный индекс первой записи на нужном нам уровне
* @param[out]	aWordsCount		- количество слов
*
* @return код ошибки
************************************************************************/
ESldError CSldList::GetNumberOfWordsAtCurrentLevel(const Int32 aGlobalIndex, UInt32 * aWordsCount)
{
	if (GetListInfo()->IsHierarchy() == 0 || (m_LocalizedBaseIndex == SLD_DEFAULT_WORD_INDEX && aGlobalIndex == 0))
	{
		*aWordsCount = GetListInfo()->GetNumberOfGlobalWords();
		return eOK;
	}

	Int32 numberOfWordsInCurrentLevel = 0;
	Int32 lastLocalIndex = 0;
	UInt32 isHierarchy = 0;

	Int32 currentGlobalIndex = SLD_DEFAULT_WORD_INDEX;
	ESldError error = GetCurrentGlobalIndex(&currentGlobalIndex);
	if (error != eOK)
		return error;

	error = GoToByGlobalIndex(aGlobalIndex);
	if (error != eOK)
		return error;

	// Самый большой глобальный индекс на текущем уровне имеет последнее слово в последнем подкаталоге текущего уровня
	while (true)
	{
		error = GetNumberOfWords(&numberOfWordsInCurrentLevel);
		if (error != eOK)
			return error;

		UInt8 wasHierarchy = 0;
		lastLocalIndex = numberOfWordsInCurrentLevel - 1;
		for (Int32 i = lastLocalIndex; i >= 0; i--)
		{
			error = isWordHasHierarchy(i, &isHierarchy, NULL);
			if (error != eOK)
				return error;

			if (isHierarchy)
			{
				error = SetBase(i);
				if (error != eOK)
					return error;

				wasHierarchy = 1;
				break;
			}
		}

		if (wasHierarchy)
			continue;

		error = GetWordByIndex(lastLocalIndex);
		if (error != eOK)
			return error;

		error = GetCurrentGlobalIndex((Int32*)aWordsCount);
		if (error != eOK)
			return error;

		break;
	}

	error = GoToByGlobalIndex(aGlobalIndex);
	if (error != eOK)
		return error;

	*aWordsCount -= GetBaseIndex();
	(*aWordsCount)++;

	error = GoToByGlobalIndex(currentGlobalIndex);
	if (error != eOK)
		return error;

	return eOK;
}

/** *********************************************************************
* Получает количество локализаций записей в списке слов
*
* @param[out]	aNumberOfLocalization	- количество локализаций
*
* @return код ошибки
************************************************************************/
ESldError CSldList::GetNumberOfLocalization(Int32 & aNumberOfLocalization)
{
	aNumberOfLocalization = 0;
	ESldError error;

	const CSldListInfo* listInfo = GetListInfo();

	if (!listInfo->GetLocalizedWordsFlag() || !listInfo->IsHierarchy())
		return eOK;

	const UInt32 localizationVariantIndex = GetListInfo()->GetVariantIndexByType(eVariantLocalizationPreferences);
	if (localizationVariantIndex == SLD_DEFAULT_VARIANT_INDEX)
		return eOK;

	const Int32 oldLocalizedBase = m_LocalizedBaseIndex;
	const Int32 oldGlobalIndex = GetCurrentWordIndex() - m_LocalizedBaseGlobalIndex;
	m_LocalizedBaseIndex = SLD_DEFAULT_WORD_INDEX;

	error = SetBase(MAX_UINT_VALUE);
	if (error != eOK)
		return error;

	error = GetNumberOfWords(&aNumberOfLocalization);
	if (error != eOK)
		return error;

	m_LocalizedBaseIndex = oldLocalizedBase;
	error = m_Catalog->GetBaseByIndex(m_LocalizedBaseIndex, (UInt32*)&m_LocalizedBaseGlobalIndex);
	if (error != eOK)
		return error;

	error = GoToByGlobalIndex(oldGlobalIndex);
	if (error != eOK)
		return error;

	return eOK;
}

/** *********************************************************************
* Устанавливает нужную локализацию записей в списке слов по номеру локализации
*
* @param[in]	aIndex	- номер локализации, которую нужно установить
*
* @return код ошибки
************************************************************************/
ESldError CSldList::SetLocalizationByIndex(const Int32 aIndex)
{
	ESldError error;

	Int32 numberOfLocalization = 0;
	error = GetNumberOfLocalization(numberOfLocalization);
	if (error != eOK)
		return error;

	if (aIndex >= numberOfLocalization)
		return eCommonWrongIndex;

	const Int32 oldGlobalIndex = GetCurrentWordIndex() - m_LocalizedBaseGlobalIndex;
	m_LocalizedBaseIndex = SLD_DEFAULT_WORD_INDEX;

	error = SetBase(MAX_UINT_VALUE);
	if (error != eOK)
		return error;

	m_LocalizedBaseIndex = aIndex;
	error = m_Catalog->GetBaseByIndex(m_LocalizedBaseIndex, (UInt32*)&m_LocalizedBaseGlobalIndex);
	if (error != eOK)
		return error;

	error = GoToByGlobalIndex(0);
	if (error != eOK)
		return error;

	Int32 numberOfWords = 0;
	error = GetNumberOfWords(&numberOfWords);
	if (error != eOK)
		return error;

	error = GoToByGlobalIndex(oldGlobalIndex);
	if (error != eOK)
		return error;

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
ESldError CSldList::GetLocalizationInfoByIndex(const Int32 aIndex, UInt32 & aLocalizationCode, SldU16String & aLocalizationName)
{
	ESldError error;

	Int32 numberOfLocalization = 0;
	error = GetNumberOfLocalization(numberOfLocalization);
	if (error != eOK)
		return error;

	if (aIndex >= numberOfLocalization)
		return eCommonWrongIndex;

	const Int32 oldLocalizedBase = m_LocalizedBaseIndex;
	const Int32 oldGlobalIndex = GetCurrentWordIndex() - m_LocalizedBaseGlobalIndex;
	
	m_LocalizedBaseIndex = SLD_DEFAULT_WORD_INDEX;
	m_LocalizedBaseGlobalIndex = SLD_DEFAULT_WORD_INDEX;

	error = SetBase(MAX_UINT_VALUE);
	if (error != eOK)
		return error;

	error = GetWordByIndex(aIndex);
	if (error != eOK)
		return error;

	UInt16 * currentWord = NULL;
	error = GetCurrentWord(GetListInfo()->GetShowVariantIndex(), &currentWord);
	if (error != eOK)
		return error;
	aLocalizationName = currentWord;

	error = GetCurrentWord(GetListInfo()->GetVariantIndexByType(eVariantLocalizationPreferences), &currentWord);
	if (error != eOK)
		return error;
	aLocalizationCode = CSldCompare::UInt16StrToUInt32Code(currentWord);

	m_LocalizedBaseIndex = oldLocalizedBase;
	error = m_Catalog->GetBaseByIndex(m_LocalizedBaseIndex, (UInt32*)&m_LocalizedBaseGlobalIndex);
	if (error != eOK)
		return error;

	error = GoToByGlobalIndex(oldGlobalIndex);
	if (error != eOK)
		return error;

	return eOK;
}

/** *********************************************************************
* Возвращает индекс текущей локализации
*
* @param[out]	aIndex		- номер текущей локализации
*
* @return код ошибки
************************************************************************/
ESldError CSldList::GetCurrentLocalizationIndex(Int32 & aIndex)
{
	aIndex = m_LocalizedBaseIndex;
	return eOK;
}

/** *********************************************************************
* Сравнивает все варианты элемента истории и текущего слова
*
* @param[in]	aElement	- проверяемый элемент истории
* @param[in]	aByMass		- флаг, определяющий как будет проверятся слово
*							  true - на основе масс символов
*							  false - бинарно
*
* @return колличество расхождений среди результатов сравнения, 0 - полное совпадение
************************************************************************/
UInt32 CSldList::CompareHistoryElementWithCurrentWord(CSldHistoryElement & aElement, bool aByMass)
{
	ESldError error = eOK;
	UInt32 resultWeight = 0;

	UInt16* currentWord = NULL;
	EListVariantTypeEnum variantType = eVariantUnknown;
	for (UInt32 variantIndex = 0; variantIndex < aElement.GetWordsCount(); variantIndex++)
	{
		error = aElement.GetVariantType(variantIndex, &variantType);
		if (error != eOK)
			return ~0;

		//if (variantType != eVariantShow || variantType != eVariantSortKey)
		//	continue;

		error = aElement.GetCurrentWord(variantIndex, &currentWord);
		if (error != eOK)
			return ~0;

		const Int32 currentVariantIndex = GetListInfo()->GetVariantIndexByType(variantType);
		if (currentVariantIndex != SLD_DEFAULT_VARIANT_INDEX)
		{
			const Int32 compareResult = aByMass ? GetCMP()->StrICmp(currentWord, GetWord(currentVariantIndex)) : GetCMP()->StrCmp(currentWord, GetWord(currentVariantIndex));
			if (compareResult != 0)
				resultWeight++;
		}
	}

	return resultWeight;
}

/** *********************************************************************
* Возвращает вектор индексов словаря для заданного слова
*
* @param[in]	aGlobalIndex	- номер слова в списке слияния
* @param[out]	aDictIndexes	- вектор с индексами словаря
*
* @return код ошибки
************************************************************************/
ESldError CSldList::GetDictionaryIndexesByGlobalIndex(const Int32 aGlobalIndex, CSldVector<Int32> & aDictIndexes)
{
	aDictIndexes.clear();
	Int32 dictIndex = m_MergedDictInfo ? m_MergedDictInfo->DictIndex : 0;
	aDictIndexes.push_back(dictIndex);
	return eOK;
}
