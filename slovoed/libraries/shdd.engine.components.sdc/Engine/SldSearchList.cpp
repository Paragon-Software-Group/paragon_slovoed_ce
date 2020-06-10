#include "SldSearchList.h"

#include "SldLogicalExpression.h"
#include "SldLogicalExpressionImplementation.h"

struct CSldSearchList::WordGetter
{
	WordGetter() {} // <- required because clang throws "error: default initialization of an object of const type"
	const UInt16* operator()(CSldList *aList, UInt32 aIndex) const { return aList->GetWord(aIndex); }
};

/** *********************************************************************
* Инициализация списка слов
*
* @param[in]	aData			- ссылка на объект, отвечающий за получение данных из контейнера
* @param[in]	aLayerAccess	- указатель на класс для взаимодействия с оболочкой
* @param[in]	aListInfo		- указатель на класс, хранящий информацию о списке слов
* @param[in]	aCMP			- указатель на класс сравнения строк
* @param[in]	aHASH			- число необходимое для декодирования списка слов (в поисковом списке не используется)
*
* @return код ошибки
************************************************************************/
ESldError CSldSearchList::Init(CSDCReadMy &aData, ISldLayerAccess* aLayerAccess, const CSldListInfo* aListInfo, CSldCompare* aCMP, UInt32 aHASH)
{
	if (!aLayerAccess)
		return eMemoryNullPointer;
	if (!aListInfo)
		return eMemoryNullPointer;

	m_LayerAccess = aLayerAccess;
	
	TListHeader ListHeader;
	sldMemZero(&ListHeader, sizeof(ListHeader));
	
	const TListHeader* pSourceListHeader = aListInfo->GetHeader();
	if (!pSourceListHeader)
		return eMemoryNullPointer;
	
	sldMemMove(&ListHeader, pSourceListHeader, pSourceListHeader->HeaderSize);
	
	// Специфические свойства поискового списка
	ListHeader.HeaderSize = sizeof(ListHeader);
	ListHeader.WordListUsage = eWordListType_RegularSearch;
	ListHeader.IsSortedList = 0;
	ListHeader.IsDirectList = 0;
	ListHeader.IsFullTextSearchList = 0;
	ListHeader.IsSuffixFullTextSearchList = 0;
	ListHeader.LocalizedStringsRecordsCount = 0;
	ListHeader.LocalizedStringsResourceIndex = 0;
	ListHeader.NumberOfWords = 0;
	ListHeader.IsHierarchy = 0;
	
	m_ListInfo = sld2::make_unique<CSldListInfo>();
	if (!m_ListInfo)
		return eMemoryNotEnoughMemory;

	if (!aData.GetFileData())
		return m_ListInfo->Init(*aListInfo, &ListHeader);
	else
		return m_ListInfo->Init(aData, &ListHeader);
}

/** *********************************************************************
* Проверяем, проинициализирован ли список слов
*
* @return true - если инициализация проведена, иначе false
************************************************************************/
bool CSldSearchList::isInit(void)
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
ESldError CSldSearchList::GetWordByGlobalIndex(Int32 aGlobalIndex, const bool aInLocalization)
{
	// Глобальный индекс равен локальному
	ESldError error = GetWordByIndex(aGlobalIndex);
	if (error != eOK)
		return error;
	
	return eOK;
}

/** *********************************************************************
* Получаем слово по локальному номеру (номеру слова в пределах текущего списка слов и текущего уровня вложенности)
*
* @param[in]	aWordIndex - номер слова по порядку, начиная с 0.
*
* @return код ошибки
************************************************************************/
ESldError CSldSearchList::GetWordByIndex(Int32 aWordIndex)
{
	Int32 NumberOfWords = 0;
	ESldError error;

	error = GetNumberOfWords(&NumberOfWords);
	if (error != eOK)
		return error;
		
	if (aWordIndex >= NumberOfWords || aWordIndex < 0)
		return eCommonWrongIndex;
	
	m_CurrentWordIndex = aWordIndex;
	m_Word = GetWord(m_CurrentWordIndex);
	if (!m_Word)
		return eMemoryNullPointer;
	
	m_CurrentListIndex = m_Word->ListIndex;
	m_List = GetList(m_CurrentListIndex);
	if (!m_List)
		return eMemoryNullPointer;

	// Получаем слово
	error = m_List->pList->GetWordByGlobalIndex(m_Word->WordIndex);
	if (error != eOK)
		return error;
	
	return eOK;
}

/** *********************************************************************
* Ищем ближайшее слово, которое больше или равно заданному
*
* @param[in]	aText	- искомое слово
*
* @return код ошибки
************************************************************************/
ESldError CSldSearchList::GetWordByText(const UInt16* aText)
{
	if (!aText)
		return eMemoryNullPointer;
	
	if (!m_List)
		return eCommonWrongList;
	
	// Если список слов не предназначен для поиска - тогда поиск не возможен
	//if (!m_ListInfo->GetHeader()->IsSortedList)
	//	return eCommonWrongList;

	ESldError error = eOK;

	Int32 tWordCount = WordCount();
	if(!tWordCount)
		return eOK;

	const Int32 showVariantIndex = m_ListInfo->GetVariantIndexByType(eVariantShow);
	if (showVariantIndex == SLD_DEFAULT_VARIANT_INDEX)
		return eCommonWrongList;

	UInt32 NumberOfVariants;
	UInt16* Article;
	Int32 currentIndex = m_CurrentWordIndex;
	
	for (Int32 i=0;i<tWordCount;i++)
	{
		error = GetWordByIndex((Int16)i);
		if (error != eOK)
			return error; 
		
		error = GetNumberOfVariants(&NumberOfVariants);
		if (error != eOK)
			return error;
		
		if (!NumberOfVariants)
			continue;
		
		error = GetCurrentWord(showVariantIndex, &Article);
		if (error != eOK)
			return error;
		
		if (m_List->pList->GetCMP()->StrICmp(Article, aText) == 0)
			return eOK;
	}

	return GetWordByIndex(currentIndex);
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
ESldError CSldSearchList::GetMostSimilarWordByText(const UInt16* aText, UInt32* aResultFlag)
{
	if (!aResultFlag)
		return eMemoryNullPointer;
	
	*aResultFlag = 0;

	const UInt32 numberOfWords = m_ListInfo->GetNumberOfGlobalWords();

	// в пустом списке искать нечего, но это не ошибка
	if (!numberOfWords)
		return eOK;

	return GetWordByText(aText);
}

/** *********************************************************************
* Возвращает указатель на последнее найденное слово. 
*
* @param[in]	aVariantIndex	- номер варианта написания для текущего слова.
* @param[out]	aWord			- указатель на буфер для указателя на слово
*
* @return код ошибки
************************************************************************/
ESldError CSldSearchList::GetCurrentWord(Int32 aVariantIndex, UInt16** aWord)
{
	if (!aWord)
		return eMemoryNullPointer;
	
	ESldError error;
	
	if (!m_List)
		return eMemoryNullPointer;
	
	if (!m_Word)
		return eMemoryNullPointer;
	
	UInt32 NumberOfVariants = 0;
	error = m_List->pList->GetNumberOfVariants(&NumberOfVariants);
	if (error != eOK)
		return error;
	
	if (aVariantIndex >= NumberOfVariants || aVariantIndex < 0)
		return eCommonWrongIndex;
	
	return m_List->pList->GetCurrentWord(aVariantIndex, aWord);
}


/** *********************************************************************
* Возвращает указатель на eVariantLabel последнего найденного слова
*
* @param[out]	aLabel			- указатель на буфер для указателя на слово
*
* @return код ошибки
************************************************************************/
ESldError CSldSearchList::GetCurrentWordLabel(UInt16** aLabel)
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


/** ********************************************************************
* Получает локальный номер текущего слова (номер слова в пределах текущего списка слов и текущего уровня вложенности)
*
* @param[out]	aIndex	- указатель на переменную, в которую будет сохранен номер текущего слова
*
* @return код ошибки
************************************************************************/
ESldError CSldSearchList::GetCurrentIndex(Int32* aIndex) const
{
	if (!aIndex)
		return eMemoryNullPointer;

	*aIndex = m_CurrentWordIndex;
	
	return eOK;
}

/** *********************************************************************
* Получает глобальный номер текущего слова (номер слова в пределах всего списка слов)
*
* @param[out]	aIndex	- указатель на переменную, в которую будет сохранен номер текущего слова
*
* @return код ошибки
************************************************************************/
ESldError CSldSearchList::GetCurrentGlobalIndex(Int32* aIndex) const
{
	if (!aIndex)
		return eMemoryNullPointer;

	*aIndex = m_CurrentWordIndex;
	
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
ESldError CSldSearchList::LocalIndex2GlobalIndex(Int32 aLocalIndex, Int32* aGlobalIndex)
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
ESldError CSldSearchList::GetRealListIndex(Int32 aLocalWordIndex, Int32* aListIndex)
{
	ESldError error;
	Int32 GlobalIndex;
	
	if (!aListIndex)
		return eMemoryNullPointer;

	error = LocalIndex2GlobalIndex(aLocalWordIndex, &GlobalIndex);
	if (error != eOK)
		return error;
		
	TSldSearchWordStruct* pWord = GetWord(aLocalWordIndex);
	if (!pWord)
		return eMemoryNullPointer;
	
	TSldSearchListStruct* pList = GetList(pWord->ListIndex);
	if (!pList)
		return eMemoryNullPointer;
	
	*aListIndex = pList->RealListIndex;

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
ESldError CSldSearchList::GetRealGlobalIndex(Int32 aLocalWordIndex, Int32* aGlobalWordIndex)
{
	ESldError error = LocalIndex2GlobalIndex(aLocalWordIndex, aGlobalWordIndex);
	if (error != eOK)
		return error;
	
	TSldSearchWordStruct* pWord = GetWord(aLocalWordIndex);
	if (!pWord)
		return eMemoryNullPointer;
	
	*aGlobalWordIndex = pWord->WordIndex;
	
	return eOK;
}

/** *********************************************************************
* Возвращает полное количество слов в списке слов
*
* @param[out]	aCount	- указатель на переменную, в которую сохраняется полное количество слов в списке слов
*
* @return код ошибки
************************************************************************/
ESldError CSldSearchList::GetTotalWordCount(Int32* aCount)
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
ESldError CSldSearchList::GetNumberOfWords(Int32* aNumberOfWords) const
{
	if (!aNumberOfWords)
		return eMemoryNullPointer;
		
	*aNumberOfWords = WordCount();

	return eOK;
}

/** *********************************************************************
* Возвращает количество вариантов написания для списка слов
*
* @param[out]	aNumberOfVariants	- указатель на буфер для количества вариантов
*
* @return код ошибки
************************************************************************/
ESldError CSldSearchList::GetNumberOfVariants(UInt32* aNumberOfVariants)
{
	if (!aNumberOfVariants)
		return eMemoryNullPointer;

	if (!m_ListInfo)
		return eMemoryNullPointer;

	*aNumberOfVariants = m_ListInfo->GetNumberOfVariants();

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
ESldError CSldSearchList::SetBase(Int32 aBase)
{
	return eOK;
}

/** *********************************************************************
* Возвращает массив уникальных кодов использования списка слов
*
* @param[out]	aUsages	- указатель на переменную, куда будет записан указатель на созданный массив
* @param[out]	aCount	- указатель на переменную, куда будет записан размер созданного массива
*
* @return код ошибки
************************************************************************/
ESldError CSldSearchList::GetAllUsages(UInt32** aUsages, Int32* aCount) const
{
	if (!aUsages || !aCount)
		return eMemoryNullPointer;
	
	*aUsages = NULL;
	*aCount = 0;
	
	ESldError error;
	Int32 ListUsageCount = 0;
	Int32 NumberOfLists = ListCount();
	TSldSearchListStruct* pList = NULL;

	if (!m_ListInfo)
		return eMemoryNullPointer;

	UInt32 ListUsage = m_ListInfo->GetUsage();

	sld2::DynArray<UInt32> usages(NumberOfLists + 1);
	if (usages.empty())
		return eMemoryNotEnoughMemory;

	usages[ListUsageCount] = ListUsage;
	ListUsageCount++;

	for (Int32 i=0;i<NumberOfLists;i++)
	{
		pList = GetList(i);
		if (!pList)
			return eMemoryNullPointer;

		error = pList->pList->GetListUsage(0, &ListUsage);
		if (error != eOK)
			return error;

		UInt8 was = 0;
		for (Int32 j=0;j<ListUsageCount;j++)
		{
			if (usages[j] == ListUsage)
			{
				was = 1;
				break;
			}
		}

		if (!was)
		{
			usages[ListUsageCount] = ListUsage;
			ListUsageCount++;
		}
	}

	*aUsages = usages.release();
	*aCount = ListUsageCount;

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
ESldError CSldSearchList::GetUsageCount(Int32* aCount) const
{
	if (!aCount)
		return eMemoryNullPointer;
	
	ESldError error;
	UInt32* Usages = NULL;
	Int32 UsageCount = 0;
	
	error = GetAllUsages(&Usages, &UsageCount);
	if (error != eOK)
		return error;
	
	*aCount = UsageCount;
	sldMemFree(Usages);
	
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
ESldError CSldSearchList::GetListUsage(Int32 aIndex, UInt32* aUsage) const
{
	if (!aUsage)
		return eMemoryNullPointer;
	
	ESldError error;
	UInt32* Usages = NULL;
	Int32 UsageCount = 0;

	error = GetAllUsages(&Usages, &UsageCount);
	if (error != eOK)
		return error;

	if (aIndex >= UsageCount)
		return eCommonWrongIndex;

	*aUsage = Usages[aIndex];
	sldMemFree(Usages);
	
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
ESldError CSldSearchList::GetWordListInfo(const CSldListInfo** aListInfo, Int32 aDictIndex) const
{
	if (!aListInfo)
		return eMemoryNullPointer;

	if (!m_ListInfo)
		return eMemoryNullPointer;

	*aListInfo = m_ListInfo.get();
	
	return eOK;
}

/** *********************************************************************
* Возвращает флаг того, сортированный или нет данный список слов
*
* @param[out]	aIsSorted	- указатель на переменную, в которую будет возвращен флаг
*
* @return код ошибки
************************************************************************/
ESldError CSldSearchList::IsListSorted(UInt32* aIsSorted)
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
ESldError CSldSearchList::IsListHasPicture(UInt32* aIsPicture)
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
ESldError CSldSearchList::IsListHasVideo(UInt32* aIsVideo)
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
ESldError CSldSearchList::IsListHasSound(UInt32* aIsSound)
{
	if (!aIsSound)
		return eMemoryNullPointer;

	if (!m_ListInfo)
		return eMemoryNullPointer;

	*aIsSound = m_ListInfo->IsSound();
	return eOK;
}


/** *********************************************************************
* Возвращает флаг того, сопоставлено или нет некоторым словам из списка слов 3d
*
* @param[out]	aIsScene	- указатель на переменную, в которую будет возвращен флаг
*
* @return код ошибки
************************************************************************/
ESldError CSldSearchList::IsListHasScene(UInt32* aIsScene)
{
	if (!aIsScene)
		return eMemoryNullPointer;

	if (!m_ListInfo)
		return eMemoryNullPointer;

	*aIsScene = m_ListInfo->IsScene();
	return eOK;
}

/** *********************************************************************
* Возвращает флаг того, имеет или нет указанное слово поддерево иерархии
*
* @param[in]	aIndex			- номер слова на текущем уровне иерархии
* @param[out]	aIsHierarchy	- указатель на переменную, в которую будет возвращен флаг
* @param[out]	aLevelType		- указатель, по которому будет записан тип поддерева (см #EHierarchyLevelType)
*								  (можно передать NULL)
*
* @return код ошибки
************************************************************************/
ESldError CSldSearchList::isWordHasHierarchy(Int32 aIndex, UInt32* aIsHierarchy, EHierarchyLevelType* aLevelType)
{
	if (!aIsHierarchy)
		return eMemoryNullPointer;
		
	TSldSearchWordStruct* pWord = GetWord(aIndex);
	if (!pWord)
		return eCommonWrongIndex;
	
	TSldSearchListStruct* pList = GetList(pWord->ListIndex);
	if (!pList)
		return eCommonWrongIndex;
	
	ESldError error;
	TCatalogPath PrevPath;
	TCatalogPath Path;
	
	// Сохраняем текущее состояние
	Int32 GlobalIndex = 0;
	error = pList->pList->GetCurrentGlobalIndex(&GlobalIndex);
	if (error != eOK)
		return error;
	
	error = pList->pList->GetPathByGlobalIndex(GlobalIndex, &PrevPath);
	if (error != eOK)
		return error;
	
	// Получаем путь к слову, информацию о котором нужно узнать
	error = pList->pList->GetPathByGlobalIndex(pWord->WordIndex, &Path);
	if (error != eOK)
		return error;
	
	// Переходим к слову
	error = pList->pList->GoToByPath(&Path, eGoToWord);
	if (error != eOK)
		return error;
	
	Int32 LocalWordIndex = 0;
	error = pList->pList->GetCurrentIndex(&LocalWordIndex);
	if (error != eOK)
		return error;
	
	// Узнаем флаг
	error = pList->pList->isWordHasHierarchy(LocalWordIndex, aIsHierarchy, NULL);
	if (error != eOK)
		return error;
	
	// Восстанавливаем предыдущее состояние
	error = pList->pList->GoToByPath(&PrevPath, eGoToWord);
	if (error != eOK)
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
ESldError CSldSearchList::GetPictureIndex(CSldVector<Int32> & aPictureIndexes)
{
	if (!m_List)
		return eOK;

	if (!m_Word)
		return eOK;
	
	ESldError error;
	error = m_List->pList->GetPictureIndex(aPictureIndexes);
	if (error != eOK)
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
ESldError CSldSearchList::GetVideoIndex(Int32* aVideoIndex)
{
	if (!aVideoIndex)
		return eMemoryNullPointer;

	*aVideoIndex = SLD_INDEX_VIDEO_NO;

	if (!m_List)
		return eOK;

	if (!m_Word)
		return eOK;

	ESldError error;
	error = m_List->pList->GetVideoIndex(aVideoIndex);
	if (error != eOK)
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
ESldError CSldSearchList::GetSoundIndex(CSldVector<Int32> & aSoundIndexes)
{
	if (!m_List)
		return eOK;

	if (!m_Word)
		return eOK;
	
	ESldError error;
	error = m_List->pList->GetSoundIndex(aSoundIndexes);
	if (error != eOK)
		return error;

	return eOK;
}


/** *********************************************************************
* Возвращает номер 3d сцены текущего слова
*
* @param[out]	aSoundIndex	- указатель на переменную, в которую будет записан номер 3d сцены
*
* @return код ошибки
************************************************************************/
ESldError CSldSearchList::GetSceneIndex(Int32* aSceneIndex)
{
	if (!aSceneIndex)
		return eMemoryNullPointer;
	
	*aSceneIndex = SLD_INDEX_SCENE_NO;

	if (!m_List)
		return eOK;

	if (!m_Word)
		return eOK;
	
	ESldError error;
	error = m_List->pList->GetSceneIndex(aSceneIndex);
	if (error != eOK)
		return error;

	return eOK;
}

/** ********************************************************************
* Возвращает количество переводов у указанного слова.
*
* @param[in]	aGlobalIndex		- номер слова из списка слов для которого требуется узнать
*									  количество переводов
* @param[out]	aTranslationCount	- указатель на переменную, в которую будет помещено
*									  количество переводов у последнего декодированного слова.
*
* @return код ошибки
************************************************************************/
ESldError CSldSearchList::GetTranslationCount(Int32 aGlobalIndex, Int32* aTranslationCount)
{
	if (!aTranslationCount)
		return eMemoryNullPointer;
	
	TSldSearchWordStruct* tmpWord = GetWord(aGlobalIndex);
	if (!tmpWord)
		return eCommonWrongIndex;

	TSldSearchListStruct* tmpList = GetList(tmpWord->ListIndex);
	if (!tmpList)
		return eCommonWrongIndex;

	ESldError error;
	Int32 realWordIndex = tmpWord->WordIndex;
	if (tmpList->pList->GetListInfo()->GetUsage() == eWordListType_FullTextAuxiliary)
	{
		Int32 realListIndex = SLD_DEFAULT_LIST_INDEX;
		Int32 realIndexesCount = 0;

		error = tmpList->pList->GetReferenceCount(tmpWord->WordIndex, &realIndexesCount);
		if (error != eOK)
			return error;

		if (realIndexesCount)
		{
			error = tmpList->pList->GetRealIndexes(tmpWord->WordIndex, 0, &realListIndex, &realWordIndex);
			if (error != eOK)
				return error;

			tmpList = GetList(realListIndex);
		}
	}
		
	return tmpList->pList->GetTranslationCount(realWordIndex, aTranslationCount);
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
ESldError CSldSearchList::GetReferenceCount(Int32 aGlobalIndex, Int32* aTranslationCount)
{
	if (!aTranslationCount)
		return eMemoryNullPointer;

	*aTranslationCount = 1;
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
ESldError CSldSearchList::GetTranslationIndex(Int32 aGlobalIndex, Int32 aTranslationIndex, Int32* aArticleIndex)
{
	if (!aArticleIndex)
		return eMemoryNullPointer;
	
	TSldSearchWordStruct* tmpWord = GetWord(aGlobalIndex);
	if (!tmpWord)
		return eCommonWrongIndex;

	TSldSearchListStruct* tmpList = GetList(tmpWord->ListIndex);
	if (!tmpList)
		return eCommonWrongIndex;
	
	ESldError error;
	Int32 realWordIndex = tmpWord->WordIndex;
	// Если список слов имеет прямое отображение
	if (tmpList->pList->GetListInfo()->IsDirectList())
	{
		// тогда номер перевода совпадает с номером текущего слова.
		*aArticleIndex = tmpWord->WordIndex;
		return eOK;
	}
	else if (tmpList->pList->GetListInfo()->GetUsage() == eWordListType_FullTextAuxiliary)
	{
		Int32 realListIndex = SLD_DEFAULT_LIST_INDEX;
		Int32 realIndexesCount = 0;

		error = tmpList->pList->GetReferenceCount(tmpWord->WordIndex, &realIndexesCount);
		if (error != eOK)
			return error;

		if (realIndexesCount)
		{
			error = tmpList->pList->GetRealIndexes(tmpWord->WordIndex, 0, &realListIndex, &realWordIndex);
			if (error != eOK)
				return error;

			tmpList = GetList(realListIndex);
		}
	}

	return tmpList->pList->GetTranslationIndex(realWordIndex, aTranslationIndex, aArticleIndex);
}

/** *********************************************************************
* Возвращает путь к текущему положению в каталоге
*
* @param[out]	aPath	- указатель на структуру в которую будет помещен путь к 
*						  текущему положению в каталоге.
*
* @return код ошибки
************************************************************************/
ESldError CSldSearchList::GetCurrentPath(TCatalogPath* aPath)
{
	if (!aPath)
		return eMemoryNullPointer;
	
	aPath->Clear();

	Int32 LocalWordIndex = 0;
	ESldError error;
	
	error = GetCurrentIndex(&LocalWordIndex);
	if (error != eOK)
		return error;
	
	error = aPath->PushList((UInt32)LocalWordIndex);
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
ESldError CSldSearchList::GetPathByGlobalIndex(Int32 aIndex, TCatalogPath* aPath)
{
	if (!aPath)
		return eMemoryNullPointer;
	
	aPath->Clear();
	
	TSldSearchWordStruct* pWord = GetWord(aIndex);
	if (!pWord)
		return eCommonWrongIndex;

	TSldSearchListStruct* pList = GetList(pWord->ListIndex);
	if (!pList)
		return eCommonWrongIndex;

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
ESldError CSldSearchList::GoToByPath(const TCatalogPath* aPath, ESldNavigationTypeEnum aNavigationType)
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
ESldError CSldSearchList::GoToLevelUp(void)
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
ESldError CSldSearchList::GetParentWordNamesByGlobalIndex(Int32 aGlobalWordIndex, UInt16** aText, const UInt16* aSeparatorText, Int32 aMaxLevelCount)
{
	TSldSearchWordStruct* pWord = GetWord(aGlobalWordIndex);
	if (!pWord)
		return eCommonWrongIndex;
	
	TSldSearchListStruct* pList = GetList(pWord->ListIndex);
	if (!pList)
		return eCommonWrongIndex;
	
	ESldError error = pList->pList->GetParentWordNamesByGlobalIndex(pWord->WordIndex, aText, aSeparatorText, aMaxLevelCount);
	if (error != eOK)
		return error;
	
	return eOK;
}

/** *********************************************************************
* Производит поиск по шаблону одного слова/фразы, в пределах текущего уровня вложенности
* Предполагается, что текст запроса не содержит символов логических операций
*
* @param[in]	aText			- шаблон поиска
* @param[in]	aList			- список слов, в котором производим поиск
* @param[in]	aRealListIndex	- реальный индекс списка слов, в котором производим поиск
* @param[in]	aResultData		- указатель на класс, в котором сохраняются результаты поиска
*
* @return код ошибки
************************************************************************/
ESldError CSldSearchList::DoWordWildCardSearch(const UInt16* aText, ISldList* aList, Int32 aRealListIndex, CSldSimpleSearchWordResult* aResultData)
{
	if (!aText || !aList || !aResultData)
		return eMemoryNullPointer;
	
	ESldError error;
	
	// Обычный список, в котором производится поиск
	CSldList* pList = (CSldList*)aList;
	
	const CSldListInfo* pListInfo = NULL;
	error = pList->GetWordListInfo(&pListInfo);
	if (error != eOK)
		return error;

	// Количество вариантов написания
	const UInt32 NumberOfVariants = pListInfo->GetNumberOfVariants();

	TCatalogPath StartPath;
	error = pList->GetCurrentPath(&StartPath);
	if (error != eOK)
		return error;

	ESldSearchRange searchRange = pListInfo->GetSearchRange();
	if (searchRange == eSearchRangeDefault)
	{
		if (pListInfo->GetUsage() == eWordListType_Dictionary && pListInfo->IsHierarchy())
			searchRange = eSearchRangeRoot;
		else
			searchRange = eSearchRangeCurrentLevelRecursive;
	}

	// Границы поиска
	Int32 LowIndex = 0;
	Int32 HighIndex = 0;
	error = pList->GetSearchBounds(searchRange, &LowIndex, &HighIndex);
	if (error != eOK)
		return error;

	SldU16String EncodedWord;
	SldU16String TemplateWord;
	error = pList->GetCMP()->GetSearchPatternOfMass(aText, TemplateWord, 1);
	if (error != eOK)
		return error;

		// Результат сравнения двух строк по шаблону
	UInt32 res = 0;

	for (Int32 i=LowIndex;i<HighIndex;i++)
	{
		error = pList->GetWordByGlobalIndex(i);
		if (error != eOK)
			return error;

		for (UInt32 v=0;v<NumberOfVariants;v++)
		{
			// Вспомогательные варианты написания не учитываем
			if (pListInfo->GetVariantType(v) != eVariantShow)
				continue;
			
			error = pList->GetCMP()->GetStrOfMassWithDelimiters(pList->GetWord(v), EncodedWord, 0, 1);
			if (error != eOK)
				return error;

			res = pList->GetCMP()->WildCompare(TemplateWord.c_str(), EncodedWord.c_str());
			if (res)
			{
				error = aResultData->AddWord(i);
				if (error != eOK)
					return error;
				
				// Один из вариантов написания слова подходит - другие проверять не будем
				break;
			}
		}
		
		if ((i%SLD_SEARCH_CALLBACK_INTERLEAVE) == 0)
		{
			error = m_LayerAccess->WordFound(eWordFoundCallbackInterleave, i);
			if (error == eExceptionSearchStop)
				return eOK;
		}
	}
	
	error = pList->GoToByPath(&StartPath, eGoToWord);
	if (error != eOK)
		return error;
		
	return eOK;
}

/** *********************************************************************
* Производит поиск по шаблону одного слова/фразы, в пределах текущего уровня вложенности
* Предполагается, что текст запроса не содержит символов логических операций
*
* @param[in]	aExpressionBox	- шаблон поиска
* @param[in]	aList			- список слов, в котором производим поиск
* @param[in]	aMaximumWords	- максимальное количество слов, которые могут быть найдены
* @param[in]	aResultData		- указатель на класс, в котором сохраняются результаты поиска
* @param[in]	aSearchRange	- диапазон поиска в котором нужно определить границы
*
* @return код ошибки
************************************************************************/
ESldError CSldSearchList::DoExpressionWildCardSearch(TExpressionBox* aExpressionBox, ISldList* aList, Int32 aMaximumWords, CSldSimpleSearchWordResult* aResultData)
{
	if (!aExpressionBox || !aList || !aResultData)
		return eMemoryNullPointer;

	ESldError error;

	// Обычный список, в котором производится поиск
	CSldList* pList = (CSldList*)aList;

	const CSldListInfo* pListInfo = NULL;
	error = pList->GetWordListInfo(&pListInfo);
	if (error != eOK)
		return error;

	const UInt32 NumberOfVariants = pListInfo->GetNumberOfVariants();

	UInt32 listUsage = 0;
	error = pList->GetListUsage(0, &listUsage);
	if (error != eOK)
		return error;

	error = pList->SaveCurrentState();
	if (error != eOK)
		return error;

	ESldSearchRange searchRange = pListInfo->GetSearchRange();
	if (searchRange == eSearchRangeDefault)
	{
		if (pListInfo->GetUsage() == eWordListType_Dictionary && pListInfo->IsHierarchy())
			searchRange = eSearchRangeRoot;
		else
			searchRange = eSearchRangeCurrentLevelRecursive;
	}

	// Границы поиска
	Int32 LowIndex = 0;
	Int32 HighIndex = 0;
	error = pList->GetSearchBounds(searchRange, &LowIndex, &HighIndex);
	if (error != eOK)
		return error;

	if (!aExpressionBox->IsQueryWasPrepared)
	{
		bool onlyZeroSymbols = true;
		SldU16String strOfMass;
		for (UInt32 currentTemplateIndex = 0; currentTemplateIndex < aExpressionBox->Count; currentTemplateIndex++)
		{
			UInt16* currentOperand = aExpressionBox->Operands[currentTemplateIndex];

			if (currentOperand)
			{
				error = pList->GetCMP()->GetSearchPatternOfMass(currentOperand, strOfMass, 1);
				if (error != eOK)
					return error;

				const UInt32 operandLength = CSldCompare::StrLen(currentOperand);
				if (strOfMass.length())
				{
					onlyZeroSymbols = false;

					if (operandLength < strOfMass.length())
					{
						currentOperand = sldMemReallocT(currentOperand, strOfMass.length() + 1);
						if (!currentOperand)
							return eMemoryNotEnoughMemory;
						aExpressionBox->Operands[currentTemplateIndex] = currentOperand;
					}

					CSldCompare::StrCopy(currentOperand, strOfMass.c_str());
				}
				else if (operandLength)
				{
					*currentOperand = 0;
				}
			}
		}

		if (onlyZeroSymbols)
		{
			aExpressionBox->IsQueryWasPrepared = eExpressionBoxOnlyZeroSymbol;
			return eOK;
		}
		else
		{
			aExpressionBox->IsQueryWasPrepared = eExpressionBoxPrepared;
		}
	}
	else if (aExpressionBox->IsQueryWasPrepared == eExpressionBoxOnlyZeroSymbol)
	{
		return eOK;
	}

	SldU16String EncodedWord;
	// Результат сравнения двух строк по шаблону
	UInt32 result = 0;
	UInt32 resultIndex = SLD_DEFAULT_WORD_INDEX;
	for (Int32 i = LowIndex; i < HighIndex; i++)
	{
		error = pList->GetWordByGlobalIndex(i);
		if (error != eOK)
		{
			// Восстановление исходного состояния списка после поиска
			pList->RestoreState();
			return error;
		}

		for (UInt32 v=0;v<NumberOfVariants;v++)
		{
			// Вспомогательные варианты написания не учитываем
			if (pListInfo->GetVariantType(v) != eVariantShow)
				continue;

			error = pList->GetCMP()->GetStrOfMassWithDelimiters(pList->GetWord(v), EncodedWord, 0, 1);
			if (error != eOK)
			{
				// Восстановление исходного состояния списка после поиска
				pList->RestoreState();
				return error;
			}

			for(UInt32 currentTemplateIndex = 0; currentTemplateIndex < aExpressionBox->Count; currentTemplateIndex++)
			{
				if(result && aExpressionBox->Operators[currentTemplateIndex] == eTokenType_Operation_OR)
				{
					continue;
				}

				result = pList->GetCMP()->WildCompare(aExpressionBox->Operands[currentTemplateIndex], EncodedWord.c_str());

				if(aExpressionBox->Operators[currentTemplateIndex] == eTokenType_Operation_AND)
				{
					if(result == 0 && currentTemplateIndex < aExpressionBox->Count && aExpressionBox->Operators[currentTemplateIndex + 1] != eTokenType_Operation_OR)
					{
						break;
					}
				}
				else if(aExpressionBox->Operators[currentTemplateIndex] == eTokenType_Operation_OR)
				{
					if(result == 0 && currentTemplateIndex < aExpressionBox->Count && aExpressionBox->Operators[currentTemplateIndex + 1] != eTokenType_Operation_OR)
					{
						break;
					}
				}
				else if(aExpressionBox->Operators[currentTemplateIndex] == eTokenType_Operation_NOT)
				{
					if(result)
					{
						result = 0;
					}
					else
					{
						result = 1;
					}
				}
			}

			if (result)
			{
				if (listUsage == eWordListType_SimpleSearch)
				{
					Int8 realWordFlag = 0;
					Int8* ptr = (Int8*)(pList->GetWord(1));
					sldMemMove(&realWordFlag, ptr, sizeof(Int8));

					if (realWordFlag == SLD_SIMPLE_SORTED_VIRTUAL_WORD)
						break;

					ptr++;
					sldMemMove(&resultIndex, ptr, sizeof(Int32));
				}
				else
				{
					resultIndex = i;
				}

				error = aResultData->AddWord(resultIndex);
				if (error != eOK)
				{
					// Восстановление исходного состояния списка после поиска
					pList->RestoreState();
					return error;
				}

				error = m_LayerAccess->WordFound(eWordFoundCallbackFound, resultIndex);
				if (error == eExceptionSearchStop)
				{
					// Восстановление исходного состояния списка после поиска
					pList->RestoreState();
					return error;
				}

				// Один из вариантов написания слова подходит - другие проверять не будем
				break;
			}
		}

		if ((i%SLD_SEARCH_CALLBACK_INTERLEAVE) == 0)
		{
			error = m_LayerAccess->WordFound(eWordFoundCallbackInterleave, i);
			if (error == eExceptionSearchStop)
			{
				// Восстановление исходного состояния списка после поиска
				pList->RestoreState();
				return error;
			}
		}

		Int32 ResultCount = aResultData->GetWordCount();
		if (ResultCount >= aMaximumWords)
		{
			// Восстановление исходного состояния списка после поиска
			return pList->RestoreState();
		}
	}

	// Восстановление исходного состояния списка после поиска
	return pList->RestoreState();
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
ESldError CSldSearchList::DoWildCardSearch(TExpressionBox* aExpressionBox, Int32 aMaximumWords, ISldList* aList, Int32 aRealListIndex, ISldList* aRealList)
{
	if (!aExpressionBox)
		return eMemoryNullPointer;
	if (!aList)
		return eMemoryNullPointer;
	if (!isInit())
		return eCommonListNotInitialized;

	//SldU16String str = aExpressionBox->Operands[0];

	// Ничего искать не нужно
	if (!aMaximumWords)
		return eOK;

	ESldError error;

	// Обычный список, в котором производится поиск
	CSldList* pList = (CSldList*)aList;

	const CSldListInfo* pListInfo = NULL;
	error = pList->GetWordListInfo(&pListInfo);
	if (error != eOK)
		return error;

	const UInt32 MaximumNumberOfWordsInList = pListInfo->GetNumberOfGlobalWords();

	// Начало поиска
	error = m_LayerAccess->WordFound(eWordFoundCallbackStartSearch);
	if (error == eExceptionSearchStop)
		return eOK;

	CSldSimpleSearchWordResult resultData;

	error = resultData.Init(MaximumNumberOfWordsInList, aRealListIndex);
	if (error != eOK)
		return error;

	error = DoExpressionWildCardSearch(aExpressionBox, aList, aMaximumWords, &resultData);
	if(error != eOK)
		return error;

	if (aRealList)
	{
		pList = (CSldList*)aRealList;
	}

	error = AddWildCardSearchSearchResults(&resultData, pList, aRealListIndex, aMaximumWords);
	if (error != eOK)
		return error;

	// Конец поиска
	error = m_LayerAccess->WordFound(eWordFoundCallbackStopSearch);
	if (error == eExceptionSearchStop)
		return eOK;

	//SortWildCardResultList(str.c_str());

	return error;
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
ESldError CSldSearchList::DoFuzzySearch(const UInt16* aText, Int32 aMaximumWords, Int32 aMaximumDifference, ISldList* aList, Int32 aRealListIndex, EFuzzySearchMode aSearchMode)
{
	if (!aText)
		return eMemoryNullPointer;
	if (!aList)
		return eMemoryNullPointer;
	if (!isInit())
		return eCommonListNotInitialized;

	ESldError error;

	// Обычный список, в котором производится поиск
	CSldList* pList = (CSldList*)aList;
	
	const CSldListInfo* pListInfo = NULL;
	error = pList->GetWordListInfo(&pListInfo);
	if (error != eOK)
		return error;

	// Ничего искать не нужно
	if (!aMaximumWords)
		return eOK;
	
	TSldSearchListStruct* addList = NULL;
	error = MakeList(pList, aRealListIndex, &addList);
	if (error != eOK)
		return error;

	m_CurrentListIndex = addList->ListIndex;
	m_List = addList;

	// Количество вариантов написания
	const UInt32 NumberOfVariants = pListInfo->GetNumberOfVariants();

	// Границы поиска
	Int32 LowIndex;
	Int32 HighIndex;

	ESldSearchRange searchRange = pListInfo->GetSearchRange();
	if (searchRange == eSearchRangeDefault)
	{
		if (pListInfo->GetUsage() == eWordListType_Dictionary && pListInfo->IsHierarchy())
			searchRange = eSearchRangeRoot;
		else
			searchRange = eSearchRangeCurrentLevelRecursive;
	}

	error = pList->GetSearchBounds(searchRange, &LowIndex, &HighIndex);
	if (error != eOK)
		return error;

	if (aMaximumDifference == 0)
		aMaximumDifference = 0xFFFF;
	
	Int32 FuzzyBuffer[ARRAY_DIM][ARRAY_DIM];
	UInt32 TextLen = pList->GetCMP()->StrLen(aText);
	UInt32 PatternLen = TextLen + 1;
	if (PatternLen >= MAX_FUZZY_WORD)
		return eCommonTooLargeText;

	// Инициализация массива
	for (UInt32 i=0;i<ARRAY_DIM;i++)
	{
		FuzzyBuffer[TIO(i, 0)] = i;
		FuzzyBuffer[TIO(0, i)] = i;
	}

	// Начало поиска
	error = m_LayerAccess->WordFound(eWordFoundCallbackStartSearch);
	if (error == eExceptionSearchStop)
		return eOK;
	else if (error != eOK)
		return error;
	
	// Битовый массив флагов наличия символов в слове
	sld2::DynArray<UInt32> SymbolsCheckTable;
	SldU16String PatternOfMass;
	// Указатель на шаблон, который будем искать
	const UInt16* SearchPattern = NULL;
	// Указатель на функцию сравнения
	TFuncFuzzyCmp FuzzyCompFunc = NULL;
	switch (aSearchMode)
	{
		case eFuzzy_WithoutOptimization:
		{
			FuzzyCompFunc = (TFuncFuzzyCmp)&CSldSearchList::FuzzyCompareWithoutOptimization;
			SearchPattern = aText;
			break;
		}
		case eFuzzy_CompareDirect:
		{
			if (!SymbolsCheckTable.resize(sld2::default_init, sld2::bitset::size(0x10000)))
				return eMemoryNotEnoughMemory;
			
			FuzzyCompFunc = (TFuncFuzzyCmp)&CSldSearchList::FuzzyCompareDirect;
			SearchPattern = aText;
			break;
		}
		case eFuzzy_CompareSortTable:
		{
			if (!SymbolsCheckTable.resize(sld2::default_init, sld2::bitset::size(0x8000)))
				return eMemoryNotEnoughMemory;
			
			FuzzyCompFunc = (TFuncFuzzyCmp)&CSldSearchList::FuzzyCompareSortTable;

			error =  pList->GetCMP()->GetStrOfMass(aText, PatternOfMass);
			if (error != eOK)
				return error;
			
			SearchPattern = PatternOfMass.c_str();
			PatternLen = pList->GetCMP()->StrLen(SearchPattern) + 1;
			
			break;
		}
		default:
		{
			return eCommonUnknownFuzzySearchMode;
		}
	}

	UInt16 resultWhithoutZero[MAX_FUZZY_WORD] = {0};

	Int32 edit_distance = 0;
	UInt32 CmpFlag = 0;
	for (Int32 i = LowIndex; i < HighIndex; i++)
	{
		error = pList->GetWordByGlobalIndex(i);
		if (error != eOK)
			return error;

		for (UInt32 v=0;v<NumberOfVariants;v++)
		{
			EListVariantTypeEnum VariantType = pListInfo->GetVariantType(v);

			// Вспомогательные варианты написания не учитываем
			if (!(VariantType==eVariantShow || VariantType==eVariantShowSecondary))
				continue;
			// Находим длину запроса и результата без разделителей;
			UInt32 textLenWithoutZeroSymbols = pList->GetCMP()->StrEffectiveLen(aText);
			UInt32 currentWordLenWithoutZeroSymbols = pList->GetCMP()->StrEffectiveLen(pList->GetWord(v));

			if (currentWordLenWithoutZeroSymbols >= MAX_FUZZY_WORD)
				continue;
			
			// Если длина искомого и текущего слова различается больше чем на максимально допустимое расстояние редактирование,
			// то текущее слово точно не подходит
			Int32 LenDiff = (currentWordLenWithoutZeroSymbols < textLenWithoutZeroSymbols) ? (textLenWithoutZeroSymbols - currentWordLenWithoutZeroSymbols) : (currentWordLenWithoutZeroSymbols - textLenWithoutZeroSymbols);
			if (LenDiff > aMaximumDifference)
				continue;

			// Сравниваем текущее слово без разделителей с шаблоном поиска
			pList->GetCMP()->StrEffectiveCopy(resultWhithoutZero, pList->GetWord(v));	
			error = (this->*FuzzyCompFunc)(pList, resultWhithoutZero, SearchPattern, currentWordLenWithoutZeroSymbols+1, PatternLen, SymbolsCheckTable, FuzzyBuffer, aMaximumDifference, &CmpFlag, &edit_distance);
			if (error != eOK)
				return error;

			if (!CmpFlag)
				continue;
			
			if (edit_distance <= aMaximumDifference)
			{
				Int32 wCount = WordCount();
				if (wCount < m_WordVector.size())
				{
					// Место еще есть, просто добавляем
					TSldSearchWordStruct word;
					word.ListIndex = addList->ListIndex;
					word.WordIndex = i;
					word.WordDistance = edit_distance;

					// Добавляем слово, одновременно сортируя
					error = AddFuzzyWord(word);
					if (error != eOK)
						return error;
				}
				else
				{
					// Последнее слово с максимальным WordDistance
					if (GetWord(wCount - 1)->WordDistance > edit_distance)
					{
						RemoveLastWord();

						TSldSearchWordStruct word;
						word.ListIndex = addList->ListIndex;
						word.WordIndex = i;
						word.WordDistance = edit_distance;

						// Добавляем слово, одновременно сортируя
						error = AddFuzzyWord(word);
						if (error != eOK)
							return error;
					}
					else
					{
						aMaximumDifference = edit_distance;
					}
				}
				
				// Один из вариантов написания слова подходит - другие проверять не будем
				break;
			}
		}

		if ((i%SLD_SEARCH_CALLBACK_INTERLEAVE) == 0)
		{
			error = m_LayerAccess->WordFound(eWordFoundCallbackInterleave, i);
			if (error == eExceptionSearchStop)
				return eOK;
		}
	}

	// Конец поиска
	error = m_LayerAccess->WordFound(eWordFoundCallbackStopSearch);
	if (error == eExceptionSearchStop)
		return eOK;
	
	return error;
}

/** *********************************************************************
* Функция сравнения слов для поиска FuzzySearch для случая #EFuzzySearchMode::eFuzzy_WithoutOptimization
*
* @param[in]	aList					- указатель на обычный список слов, в котором производим поиск
* @param[in]	aWord					- слово из списка слов, с которым сравниваем искомое слово
* @param[in]	aText					- искомое слово
* @param[in]	aWordLen				- длина слова из списка слов + 1
* @param[in]	aPatternLen				- длина искомого слова + 1
* @param[in]	aSymbolsCheckTable		- битовый массив, который используется для предварительной проверки наличия одинаковых символов в сравниваемых словах
*										  память выделяется снаружи (может быть пустым)
* @param[in]	aFuzzyBuffer			- указатель на квадратную матрицу для обработки; память выделяется снаружи
* @param[in]	aMaxEditDistance		- максимально допустимое расстояние редактирования
* @param[out]	aFlag					- сюда записывается результат предварительной проверки:
*										  0 - слово не подходит,
*										  1 - слово подходит (aEditDistance содержит расстояние редактирования)
* @param[out]	aEditDistance			- сюда записывается расстояние редактирования в случае aFlag == 1
*
* @return код ошибки
************************************************************************/
ESldError CSldSearchList::FuzzyCompareWithoutOptimization(CSldList* aList, const UInt16* aWord, const UInt16* aText, Int32 aWordLen, Int32 aPatternLen, sld2::Span<UInt32> aSymbolsCheckTable, Int32 (*aFuzzyBuffer)[ARRAY_DIM], Int32 aMaxEditDistance, UInt32* aFlag, Int32* aEditDistance)
{
	if (!aList || !aWord || !aText || !aFuzzyBuffer || !aFlag || !aEditDistance)
		return eMemoryNullPointer;
	
	// Предварительную проверку не выполняем
	*aFlag = 1;
	*aEditDistance = aList->GetCMP()->FuzzyCompare(aWord, aText, aWordLen, aPatternLen, aFuzzyBuffer);
	
	return eOK;
}

/** *********************************************************************
* Функция сравнения слов для поиска FuzzySearch для случая #EFuzzySearchMode::eFuzzy_CompareDirect
*
* @param[in]	aList					- указатель на обычный список слов, в котором производим поиск
* @param[in]	aWord					- слово из списка слов, с которым сравниваем искомое слово
* @param[in]	aText					- искомое слово
* @param[in]	aWordLen				- длина слова из списка слов + 1
* @param[in]	aPatternLen				- длина искомого слова + 1
* @param[in]	aSymbolsCheckTable		- битовый массив, который используется для предварительной проверки наличия одинаковых символов в сравниваемых словах
* @param[in]	aFuzzyBuffer			- указатель на квадратную матрицу для обработки; память выделяется снаружи
* @param[in]	aMaxEditDistance		- максимально допустимое расстояние редактирования
* @param[out]	aFlag					- сюда записывается результат предварительной проверки:
*										  0 - слово не подходит,
*										  1 - слово подходит (aEditDistance содержит расстояние редактирования)
* @param[out]	aEditDistance			- сюда записывается расстояние редактирования в случае aFlag == 1
*
* @return код ошибки
************************************************************************/
ESldError CSldSearchList::FuzzyCompareDirect(CSldList* aList, const UInt16* aWord, const UInt16* aText, Int32 aWordLen, Int32 aPatternLen, sld2::Span<UInt32> aSymbolsCheckTable, Int32 (*aFuzzyBuffer)[ARRAY_DIM], Int32 aMaxEditDistance, UInt32* aFlag, Int32* aEditDistance)
{
	if (!aList || !aWord || !aText || aSymbolsCheckTable.empty() || !aFuzzyBuffer || !aFlag || !aEditDistance)
		return eMemoryNullPointer;

	*aFlag = 0;
	sldMemZero(aSymbolsCheckTable.data(), aSymbolsCheckTable.size() * sizeof(aSymbolsCheckTable[0]));

	// Заполняем битовый массив наличия символов в искомом слове
	const UInt16* textPtr = aText;
	while (*textPtr)
	{
		sld2::bitset::set(aSymbolsCheckTable.data(), *textPtr);
		textPtr++;
	}

	// Подсчитаем, сколько символов из проверяемого слова не хватает в искомом
	// Если таких символов больше, чем максимально допустимое расстояние редактирования, то проверяемое слово точно не подходит
	textPtr = aWord;
	Int32 diff_count = 0;
	while (*textPtr)
	{
		if (!sld2::bitset::test(aSymbolsCheckTable.data(), *textPtr))
			diff_count++;

		textPtr++;
	}
	
	if (diff_count > aMaxEditDistance)
		return eOK;
	
	*aFlag = 1;
	*aEditDistance = aList->GetCMP()->FuzzyCompare(aWord, aText, aWordLen, aPatternLen, aFuzzyBuffer);
	
	return eOK;
}

/** *********************************************************************
* Функция сравнения слов для поиска FuzzySearch для случая #EFuzzySearchMode::eFuzzy_CompareSortTable
*
* @param[in]	aList					- указатель на обычный список слов, в котором производим поиск
* @param[in]	aWord					- слово из списка слов, с которым сравниваем искомое слово
* @param[in]	aText					- искомое слово
* @param[in]	aWordLen				- длина слова из списка слов + 1
* @param[in]	aPatternLen				- длина искомого слова + 1
* @param[in]	aSymbolsCheckTable		- битовый массив, который используется для предварительной проверки наличия одинаковых символов (по их весам) в сравниваемых словах
* @param[in]	aFuzzyBuffer			- указатель на квадратную матрицу для обработки; память выделяется снаружи
* @param[in]	aMaxEditDistance		- максимально допустимое расстояние редактирования
* @param[out]	aFlag					- сюда записывается результат предварительной проверки:
*										  0 - слово не подходит,
*										  1 - слово подходит (aEditDistance содержит расстояние редактирования)
* @param[out]	aEditDistance			- сюда записывается расстояние редактирования в случае aFlag == 1
*
* @return код ошибки
************************************************************************/
ESldError CSldSearchList::FuzzyCompareSortTable(CSldList* aList, const UInt16* aWord, const UInt16* aText, Int32 aWordLen, Int32 aPatternLen, sld2::Span<UInt32> aSymbolsCheckTable, Int32 (*aFuzzyBuffer)[ARRAY_DIM], Int32 aMaxEditDistance, UInt32* aFlag, Int32* aEditDistance)
{
	if (!aList || !aWord || !aText || aSymbolsCheckTable.empty() || !aFuzzyBuffer || !aFlag || !aEditDistance)
		return eMemoryNullPointer;
	
	ESldError error;
	UInt16 Mass = 0;
	Int32 diff_count = 0;
	const UInt16* src = NULL;
	SldU16String PatternOfMass;
	
	*aFlag = 0;
	sldMemZero(aSymbolsCheckTable.data(), aSymbolsCheckTable.size() * sizeof(aSymbolsCheckTable[0]));
	
	// Заполняем битовый массив весов символов в искомом слове
	src = aText;
	while (*src)
	{
		// Сюда вместо самого слова приходит уже массив весов
		Mass = *src;
		if (Mass && Mass != CMP_IGNORE_SYMBOL)
			sld2::bitset::set(aSymbolsCheckTable.data(), Mass);
		src++;
	}
	
	// Подсчитаем, сколько символов (их весов) из проверяемого слова не хватает в искомом
	// Если таких символов больше, чем максимально допустимое расстояние редактирования, то проверяемое слово точно не подходит
	error =  aList->GetCMP()->GetStrOfMass(aWord, PatternOfMass);
	if (error != eOK)
		return error;
		
	src = PatternOfMass.c_str();
	while (*src)
	{
		Mass = *src;
		if (Mass && Mass != CMP_IGNORE_SYMBOL)
		{
			if (!sld2::bitset::test(aSymbolsCheckTable.data(), Mass))
				diff_count++;
		}
		src++;
	}
	
	if (diff_count > aMaxEditDistance)
		return eOK;
	
	*aFlag = 1;
	
	Int32 NewWordLen = PatternOfMass.length() + 1;
	*aEditDistance = aList->GetCMP()->FuzzyCompare(PatternOfMass.c_str(), aText, NewWordLen, aPatternLen, aFuzzyBuffer);
	
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
ESldError CSldSearchList::DoAnagramSearch(const UInt16* aText, Int32 aTextLen, ISldList* aList, Int32 aRealListIndex)
{
	if (!aText)
		return eMemoryNullPointer;
	if (!aList)
		return eMemoryNullPointer;
	if (!isInit())
		return eCommonListNotInitialized;

	ESldError error;

	// Обычный список, в котором производится поиск
	CSldList* pList = (CSldList*)aList;

	TSldSearchListStruct* addList = NULL;
	error = MakeList(pList, aRealListIndex, &addList);
	if (error != eOK)
		return error;

	m_CurrentListIndex = addList->ListIndex;
	m_List = addList;
	
	// Ничего искать не нужно
	if (aTextLen < 1)
		return eOK;

	const CSldListInfo* pListInfo = NULL;
	error = pList->GetWordListInfo(&pListInfo);
	if (error != eOK)
		return error;

	// Количество вариантов написания
	const UInt32 NumberOfVariants = pListInfo->GetNumberOfVariants();
	UInt32 MaximumWordSize = pListInfo->GetMaximumWordSize();

	if (aTextLen > MaximumWordSize)
		MaximumWordSize = aTextLen;

	// Массив флагов для каждого символа в слове, здесь сохраняются флаги, что символ с определенным индексом уже учтен при сравнении
	sld2::DynArray<UInt8> flag(sld2::default_init, MaximumWordSize);
	if (MaximumWordSize > 0 && flag.empty())
		return eMemoryNotEnoughMemory;

	sld2::DynArray<UInt16> PreparedText(aTextLen + 1);
	if (PreparedText.empty())
		return eMemoryNotEnoughMemory;

	sld2::DynArray<UInt16> PreparedWord(MaximumWordSize + 1);
	if (PreparedWord.empty())
		return eMemoryNotEnoughMemory;

	aTextLen = pList->GetCMP()->PrepareTextForAnagramSearch(PreparedText.data(), aText);
	if (!aTextLen)
		return eOK;

	// Границы поиска
	Int32 LowIndex;
	Int32 HighIndex;

	ESldSearchRange searchRange = pListInfo->GetSearchRange();
	if (searchRange == eSearchRangeDefault)
	{
		if (pListInfo->GetUsage() == eWordListType_Dictionary && pListInfo->IsHierarchy())
			searchRange = eSearchRangeRoot;
		else
			searchRange = eSearchRangeCurrentLevelRecursive;
	}

	error = pList->GetSearchBounds(searchRange, &LowIndex, &HighIndex);
	if (error != eOK)
		return error;

	// Начало поиска
	error = m_LayerAccess->WordFound(eWordFoundCallbackStartSearch);
	if (error == eExceptionSearchStop)
		return eOK;

	// Перебираем слова
	Int32 Len;
	for (Int32 i=LowIndex;i<HighIndex;i++)
	{
		error = pList->GetWordByGlobalIndex(i);
		if (error != eOK)
			return error;

		// Перебираем варианты написания слов
		for (UInt32 v=0;v<NumberOfVariants;v++)
		{
			EListVariantTypeEnum VariantType = pListInfo->GetVariantType(v);

			// Вспомогательные варианты написания не учитываем
			if (!(VariantType==eVariantShow || VariantType==eVariantShowSecondary || VariantType==eVariantSortKey))
				continue;
			
			Len = pList->GetCMP()->PrepareTextForAnagramSearch(PreparedWord.data(), pList->GetWord(v));
			if (!Len || Len != aTextLen)
			{
				continue;
			}
			
			// Добавляем слово в список
			if (pList->GetCMP()->AnagramCompare(PreparedText.data(), PreparedWord.data(), flag.data(), aTextLen))
			{
				TSldSearchWordStruct word;
				word.ListIndex = addList->ListIndex;
				word.WordIndex = i;

				// Добавляем слово
				error = AddWord(word);
				if (error != eOK)
					return error;

				error = m_LayerAccess->WordFound(eWordFoundCallbackFound, i);
				if (error != eOK)
				{
					if (error == eExceptionSearchStop)
						return eOK;
						
					return error;
				}
				
				// Один из вариантов написания слова подходит - другие проверять не будем
				break;
			}
		}

		if ((i%SLD_SEARCH_CALLBACK_INTERLEAVE) == 0)
		{
			error = m_LayerAccess->WordFound(eWordFoundCallbackInterleave, i);
			if (error == eExceptionSearchStop)
				return eOK;
		}

		// если уже найдено необходимое количество слов, то завершаем поиск
		if (m_WordCount >= m_WordVector.size())
			break;
	}

	// Конец поиска
	error = m_LayerAccess->WordFound(eWordFoundCallbackStopSearch);
	if (error == eExceptionSearchStop)
		return eOK;
		
	return error;
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
ESldError CSldSearchList::DoSpellingSearch(const UInt16* aText, ISldList* aList, Int32 aRealListIndex)
{
	if (!aText || !aList)
		return eMemoryNullPointer;
	if (!isInit())
		return eCommonListNotInitialized;

	ESldError error = eOK;
	Int32 ListUsageCount = 0;
	UInt32 ListUsage = 0;
	Int32 i = 0;
	Int32 symbolIndex = 0;
	Int8 isDouble = 0;
	Int32 NumberOfCheckedWords = 0;

	// Обычный список, в котором производится поиск
	CSldList* pList = (CSldList*)aList;

	const CSldListInfo* pListInfo = NULL;
	error = pList->GetWordListInfo(&pListInfo);
	if (error != eOK)
		return error;

	error = pList->GetUsageCount(&ListUsageCount);
	if (error != eOK)
		return error;

	// Мы не можем искать в каталоге
	for (Int32 usageIndex=0;usageIndex<ListUsageCount;usageIndex++)
	{
		error = pList->GetListUsage(usageIndex, &ListUsage);
		if (error != eOK)
			return error;

		if (ListUsage==eWordListType_Unknown || ListUsage==eWordListType_Catalog)
			return eCommonWrongList;
	}

	// Создаем поисковый список
	TSldSearchListStruct* addList = NULL;
	error = MakeList(pList, aRealListIndex, &addList);
	if (error != eOK)
		return error;

	m_CurrentListIndex = addList->ListIndex;
	m_List = addList;

	// Количество вариантов написания
	const UInt32 NumberOfVariants = pListInfo->GetNumberOfVariants();

	// Начало поиска
	error = m_LayerAccess->WordFound(eWordFoundCallbackStartSearch);
	if (error == eExceptionSearchStop)
		return eOK;

	// Длина переданного текста
	Int32 TextLen = CSldCompare::StrLen(aText);
	// Нечего искать
	if (!TextLen)
	{
		error = m_LayerAccess->WordFound(eWordFoundCallbackStopSearch);
		if (error == eExceptionSearchStop)
			return eOK;
		return error;
	}
	// Введенный текст слишком большой
	if (TextLen+1 > MAX_FUZZY_WORD)
		return eCommonTooLargeText;

	// Границы поиска
	Int32 LowIndex = 0;
	Int32 HighIndex = 0;
	// Количество слов в вспомогательном списке может отличаться
	UInt32 wordsCount = pList->GetSortedWordsCount();
	Int32 startRange = 0;

	ESldSearchRange searchRange = pListInfo->GetSearchRange();
	if (searchRange == eSearchRangeDefault)
	{
		if (pListInfo->GetUsage() == eWordListType_Dictionary && pListInfo->IsHierarchy())
			// Данный тип списка теоретически эмулирует обычный словарный список, а в папках находятся вспомогательные записи, которые не должны учавствовать в поиске
			searchRange = eSearchRangeRoot;
		else
			searchRange = eSearchRangeCurrentLevelRecursive;
	}

	error = pList->GetSearchBounds(searchRange, &LowIndex, &HighIndex);
	if (error != eOK)
		return error;

	// Битовый массив флагов наличия символов (по весам символов) в слове
	sld2::DynArray<UInt32> SymbolsCheckTable(sld2::default_init, sld2::bitset::size(0x8000));
	if (SymbolsCheckTable.empty())
		return eMemoryNotEnoughMemory;

	SldU16String PatternOfMass;
	// Указатель на функцию сравнения
	TFuncFuzzyCmp FuzzyCompFunc = (TFuncFuzzyCmp)&CSldSearchList::FuzzyCompareSortTable;

	error =  pList->GetCMP()->GetStrOfMass(aText, PatternOfMass);
	if (error != eOK)
		return error;

	// Указатель на шаблон, который будем искать
	const UInt16* SearchPattern = PatternOfMass.c_str();
	Int32 PatternLen = pList->GetCMP()->StrLen(SearchPattern) + 1;

	Int32 FuzzyBuffer[MAX_FUZZY_WORD][MAX_FUZZY_WORD];
	// Инициализация массива
	for (i=0;i<MAX_FUZZY_WORD;i++)
	{
		FuzzyBuffer[TIO(i, 0)] = i;
		FuzzyBuffer[TIO(0, i)] = i;
	}

	UInt16 cmpStr[2] = {0};
	UInt16 findStr[MAX_FUZZY_WORD] = {0};
	// Найдем в строке первый символ - не разделитель
	for (symbolIndex=0;symbolIndex<TextLen;symbolIndex++)
	{
		findStr[symbolIndex] = aText[symbolIndex];
		if (pList->GetCMP()->StrICmp(findStr, cmpStr) != 0)
			break;
	}

	// Нечего искать
	if (symbolIndex == TextLen)
	{
		error = m_LayerAccess->WordFound(eWordFoundCallbackStopSearch);
		if (error == eExceptionSearchStop)
			return eOK;
		return error;
	}

	Int32 startPos = symbolIndex;
	cmpStr[0] = aText[startPos];
	Int32 distance = 0;
	Int32 WordLen = 0;
	UInt32 CmpFlag = 0;
	Int32 currentGlobalIndex = SLD_DEFAULT_WORD_INDEX;

	UInt32 resultFlag = 0;
	error = pList->GetWordBySortedText(cmpStr, &resultFlag);
	if (error != eOK)
		return error;

	error = pList->GetCurrentSortedIndex(&startRange);
	if (error != eOK)
		return error;

	// Ищем в пределах диапазона слов, которые начинаются с первого символа неразделителя переданной строки
	// Добавляем слова, которые содержат только одну ошибку
	i = startRange;
	while (i < wordsCount)
	{
		if (i != startRange)
		{
			error = pList->GetNextSortedWord();
			if (error != eOK)
				return error;
		}

		i++;
		NumberOfCheckedWords++;

		error = pList->GetCurrentGlobalIndex(&currentGlobalIndex);
		if (error != eOK)
			return error;

		if (currentGlobalIndex < LowIndex || currentGlobalIndex >= HighIndex)
		{
			continue;
		}

		/// TODO когда все базы будут 113 версии и выше - от данную проверку можно удалить и всегда использовать ShowVariant
		const UInt16* resultWordPtr = pList->GetWord(0);
		if (pList->HasSimpleSortedList())
		{
			resultWordPtr = pList->GetWord(pList->GetListInfo()->GetShowVariantIndex());
		}

		// Найдем в строке первый символ - не разделитель
		sldMemZero(findStr, MAX_FUZZY_WORD*sizeof(findStr[0]));
		WordLen = pList->GetCMP()->StrLen(resultWordPtr);

		for (Int32 j = 0; j < WordLen; j++)
		{
			findStr[j] = resultWordPtr[j];
			// Сравниваем с пустой строкой
			if (pList->GetCMP()->StrICmp(findStr, cmpStr + 1) != 0)
				break;
		}

		// Конец диапазона
		if (pList->GetCMP()->StrICmp(findStr, cmpStr) != 0)
			break;

		// Перебираем варианты написания слова
		for (UInt32 v = 0; v < NumberOfVariants; v++)
		{
			EListVariantTypeEnum VariantType = pListInfo->GetVariantType(v);

			// Вспомогательные варианты написания не учитываем
			if (!(VariantType == eVariantShow || VariantType == eVariantShowSecondary))
				continue;

			// Пропускаем слишком длинные слова, которые не можем сравнить
			WordLen = pList->GetCMP()->StrLen(pList->GetWord(v));
			if (MAX_FUZZY_WORD < WordLen + 1)
				continue;

			// Сравниваем
			error = (this->*FuzzyCompFunc)(pList, pList->GetWord(v), SearchPattern, WordLen + 1, PatternLen, SymbolsCheckTable, FuzzyBuffer, 1, &CmpFlag, &distance);
			if (error != eOK)
				return error;

			if (!CmpFlag)
				continue;

			if (distance == 1)
			{
				TSldSearchWordStruct word;
				word.ListIndex = addList->ListIndex;
				word.WordIndex = currentGlobalIndex;

				error = AddWord(word);
				if (error != eOK)
					return error;

				error = m_LayerAccess->WordFound(eWordFoundCallbackFound, currentGlobalIndex);
				if (error == eExceptionSearchStop)
					return eOK;

				// Один из вариантов написания слова подходит - другие проверять не будем
				break;
			}
		}

		if ((NumberOfCheckedWords%SLD_SEARCH_CALLBACK_INTERLEAVE) == 0)
		{
			error = m_LayerAccess->WordFound(eWordFoundCallbackInterleave, NumberOfCheckedWords);
			if (error == eExceptionSearchStop)
				return eOK;
		}
		// если уже найдено необходимое количество слов, то завершаем поиск
		if (m_WordCount >= m_WordVector.size())
		{
			error = m_LayerAccess->WordFound(eWordFoundCallbackStopSearch);
			if (error == eExceptionSearchStop)
				return eOK;
			return error;
		}
	}

	UInt16 tmpSymbolStr1[2];
	UInt16 tmpSymbolStr2[2];
	sldMemZero(tmpSymbolStr1, sizeof(tmpSymbolStr1[0]) * 2);
	sldMemZero(tmpSymbolStr2, sizeof(tmpSymbolStr2[0]) * 2);

	// Проверяем слова, которые получаются из исходного заменой первого символа неразделителя по очереди на все символы алфавита словаря
	pList->GetCMP()->StrCopy(findStr, &aText[startPos]);
	Int32 SymbolPairTableElementsCount = pList->GetCMP()->GetSymbolPairTableElementsCount(eSymbolPairTableType_Dictionary);

	// Подставляем символы верхнего регистра
	for (symbolIndex = 0; symbolIndex < SymbolPairTableElementsCount; symbolIndex++)
	{
		// Заменяем символ
		findStr[0] = pList->GetCMP()->GetUpperSymbolFromSymbolPairTable(symbolIndex, eSymbolPairTableType_Dictionary);

		// Само исходное слово нам не нужно
		if (pList->GetCMP()->StrICmp(findStr, &aText[startPos]) == 0)
			continue;

		// Если символ с этой массой уже участвовал в поиске - пропускаем
		Int8 doubleMass = 0;
		*tmpSymbolStr1 = findStr[0];
		for (UInt32 doubleSymbolIndex = 0; doubleSymbolIndex < symbolIndex; doubleSymbolIndex++)
		{
			*tmpSymbolStr2 = pList->GetCMP()->GetUpperSymbolFromSymbolPairTable(doubleSymbolIndex, eSymbolPairTableType_Dictionary);
			if (pList->GetCMP()->StrICmp(tmpSymbolStr1, tmpSymbolStr2) == 0)
			{
				doubleMass = 1;
				break;
			}
		}
		if(doubleMass)
			continue;

		UInt32 resultFlag = 0;
		error = pList->GetWordBySortedText(findStr, &resultFlag);
		if (error != eOK)
			return error;

		if (resultFlag)
		{
			error = pList->GetCurrentSortedIndex(&startRange);
			if (error != eOK)
				return error;

			Int32 j = startRange;
			while (j < wordsCount)
			{
				if (j != startRange)
				{
					error = pList->GetNextSortedWord();
					if (error != eOK)
						return error;
				}

				j++;
				NumberOfCheckedWords++;

				error = pList->GetCurrentGlobalIndex(&currentGlobalIndex);
				if (error != eOK)
					return error;

				if (currentGlobalIndex < LowIndex || currentGlobalIndex >= HighIndex)
				{
					continue;
				}

				UInt8 added = 0;
				// Перебираем варианты написания слова
				for (UInt32 v = 0; v < NumberOfVariants; v++)
				{
					EListVariantTypeEnum VariantType = pListInfo->GetVariantType(v);

					// Вспомогательные варианты написания не учитываем
					if (!(VariantType == eVariantShow || VariantType == eVariantShowSecondary))
						continue;

					if (pList->GetCMP()->StrICmp(findStr, pList->GetWord(v)) == 0)
					{
						TSldSearchWordStruct word;
						word.ListIndex = addList->ListIndex;
						word.WordIndex = currentGlobalIndex;

						error = AddSpellingWord(word, &isDouble);
						if (error != eOK)
							return error;

						if (isDouble == 0)
						{
							error = m_LayerAccess->WordFound(eWordFoundCallbackFound, currentGlobalIndex);
							if (error == eExceptionSearchStop)
								return eOK;
						}
						// Один из вариантов написания слова подходит - другие проверять не будем
						added = 1;
						break;
					}
				}
				if (!added)
					break;
			}
			if ((NumberOfCheckedWords%SLD_SEARCH_CALLBACK_INTERLEAVE) == 0)
			{
				error = m_LayerAccess->WordFound(eWordFoundCallbackInterleave, NumberOfCheckedWords);
				if (error == eExceptionSearchStop)
					return eOK;
			}
			// если уже найдено необходимое количество слов, то завершаем поиск
			if (m_WordCount >= m_WordVector.size())
			{
				error = m_LayerAccess->WordFound(eWordFoundCallbackStopSearch);
				if (error == eExceptionSearchStop)
					return eOK;
				return error;
			}
		}
	}

	// Проверяем слова, которые получаются из исходного добавлением в начало по очереди всех символов алфавита словаря
	pList->GetCMP()->StrCopy(findStr+1, &aText[startPos]);

	// Добавляем символы верхнего регистра
	for (symbolIndex = 0; symbolIndex < SymbolPairTableElementsCount; symbolIndex++)
	{
		// Добавляем символ
		findStr[0] = pList->GetCMP()->GetUpperSymbolFromSymbolPairTable(symbolIndex, eSymbolPairTableType_Dictionary);

		// Само исходное слово нам не нужно
		if (pList->GetCMP()->StrICmp(findStr, &aText[startPos]) == 0)
			continue;

		// Если символ с этой массой уже учавствовал в поиске - пропускаем
		Int8 doubleMass = 0;
		*tmpSymbolStr1 = findStr[0];
		for (UInt32 doubleSymbolIndex = 0; doubleSymbolIndex < symbolIndex; doubleSymbolIndex++)
		{
			*tmpSymbolStr2 = pList->GetCMP()->GetUpperSymbolFromSymbolPairTable(doubleSymbolIndex, eSymbolPairTableType_Dictionary);
			if (pList->GetCMP()->StrICmp(tmpSymbolStr1, tmpSymbolStr2) == 0)
			{
				doubleMass = 1;
				break;
			}
		}
		if(doubleMass)
			continue;

		error = pList->GetWordBySortedText(findStr, &resultFlag);
		if (error != eOK)
			return error;

		if (resultFlag)
		{
			error = pList->GetCurrentSortedIndex(&startRange);
			if (error != eOK)
				return error;

			Int32 j = startRange;
			while (j < wordsCount)
			{
				if (j != startRange)
				{
					error = pList->GetNextSortedWord();
					if (error != eOK)
						return error;
				}

				j++;
				NumberOfCheckedWords++;

				error = pList->GetCurrentGlobalIndex(&currentGlobalIndex);
				if (error != eOK)
					return error;

				if (currentGlobalIndex < LowIndex || currentGlobalIndex >= HighIndex)
				{
					continue;
				}

				UInt8 added = 0;
				// Перебираем варианты написания слова
				for (UInt32 v = 0; v < NumberOfVariants; v++)
				{
					EListVariantTypeEnum VariantType = pListInfo->GetVariantType(v);

					// Вспомогательные варианты написания не учитываем
					if (!(VariantType == eVariantShow || VariantType == eVariantShowSecondary))
						continue;

					if (pList->GetCMP()->StrICmp(findStr, pList->GetWord(v)) == 0)
					{
						TSldSearchWordStruct word;
						word.ListIndex = addList->ListIndex;
						word.WordIndex = currentGlobalIndex;

						error = AddSpellingWord(word, &isDouble);
						if (error != eOK)
							return error;

						if (isDouble == 0)
						{
							error = m_LayerAccess->WordFound(eWordFoundCallbackFound, j);
							if (error == eExceptionSearchStop)
								return eOK;
						}

						// Один из вариантов написания слова подходит - другие проверять не будем
						added = 1;
						break;
					}
				}
				if (!added)
					break;
			}
			if ((NumberOfCheckedWords%SLD_SEARCH_CALLBACK_INTERLEAVE) == 0)
			{
				error = m_LayerAccess->WordFound(eWordFoundCallbackInterleave, NumberOfCheckedWords);
				if (error == eExceptionSearchStop)
					return eOK;
			}
			// если уже найдено необходимое количество слов, то завершаем поиск
			if (m_WordCount >= m_WordVector.size())
			{
				error = m_LayerAccess->WordFound(eWordFoundCallbackStopSearch);
				if (error == eExceptionSearchStop)
					return eOK;
				return error;
			}
		}
	}

	// Проверяем слова, которые получаются из исходного удалением первого символа
	WordLen = pList->GetCMP()->StrLen(&aText[startPos+1]);
	if (WordLen)
	{
		error = pList->GetWordBySortedText(&aText[startPos + 1], &resultFlag);
		if (error != eOK)
			return error;
		if (resultFlag)
		{
			error = pList->GetCurrentSortedIndex(&startRange);
			if (error != eOK)
				return error;

			Int32 j = startRange;
			while (j < wordsCount)
			{
				if (j != startRange)
				{
					error = pList->GetNextSortedWord();
					if (error != eOK)
						return error;
				}

				j++;
				NumberOfCheckedWords++;

				error = pList->GetCurrentGlobalIndex(&currentGlobalIndex);
				if (error != eOK)
					return error;

				if (currentGlobalIndex < LowIndex || currentGlobalIndex >= HighIndex)
				{
					continue;
				}

				UInt8 added = 0;
				// Перебираем варианты написания слова
				for (UInt32 v = 0; v < NumberOfVariants; v++)
				{
					EListVariantTypeEnum VariantType = pListInfo->GetVariantType(v);

					// Вспомогательные варианты написания не учитываем
					if (!(VariantType == eVariantShow || VariantType == eVariantShowSecondary))
						continue;

					if (pList->GetCMP()->StrICmp(&aText[startPos + 1], pList->GetWord(v)) == 0)
					{
						TSldSearchWordStruct word;
						word.ListIndex = addList->ListIndex;
						word.WordIndex = currentGlobalIndex;

						error = AddSpellingWord(word, &isDouble);
						if (error != eOK)
							return error;

						if (isDouble == 0)
						{
							error = m_LayerAccess->WordFound(eWordFoundCallbackFound, j);
							if (error == eExceptionSearchStop)
								return eOK;
						}

						// Один из вариантов написания слова подходит - другие проверять не будем
						added = 1;
						break;
					}
				}
				if (!added)
					break;
			}
			if ((NumberOfCheckedWords%SLD_SEARCH_CALLBACK_INTERLEAVE) == 0)
			{
				error = m_LayerAccess->WordFound(eWordFoundCallbackInterleave, NumberOfCheckedWords);
				if (error == eExceptionSearchStop)
					return eOK;
			}
			// если уже найдено необходимое количество слов, то завершаем поиск
			if (m_WordCount >= m_WordVector.size())
			{
				error = m_LayerAccess->WordFound(eWordFoundCallbackStopSearch);
				if (error == eExceptionSearchStop)
					return eOK;
				return error;
			}
		}
	}

	// Конец поиска
	error = m_LayerAccess->WordFound(eWordFoundCallbackStopSearch);
	if (error == eExceptionSearchStop)
		return eOK;

	return error;
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
ESldError CSldSearchList::DoFullTextSearch(const UInt16* aText, Int32 aMaximumWords, ISldList** aList, Int32 aListCount, Int32 aRealListIndex)
{
	if (!aText || !aList)
		return eMemoryNullPointer;
	if (!isInit())
		return eCommonListNotInitialized;
		
	ESldError error;
	
	// Начало поиска
	error = m_LayerAccess->WordFound(eWordFoundCallbackStartSearch);
	if (error == eExceptionSearchStop)
		return eOK;
	
	// Реализация поиска
	CFullTextSearchImplementation SearchImplementation;

	if (!SearchImplementation.Init(this, aList, aListCount, aRealListIndex, aMaximumWords))
		return eCommonFullTextSearchLogicalExpressionError;

	// Препроцессинг запроса
	//************************************************************************
	/*
	UInt16* PreparedText = NULL;
	
	UInt32 isSmartQuery = CSldCompare::IsSmartFullTextSearchQuery(aText);
	if (isSmartQuery)
		error = CSldCompare::CorrectSmartFullTextSearchQuery(aText, &PreparedText);
	else
		error = CSldCompare::CorrectNonSmartFullTextSearchQuery(aText, &PreparedText);
	
	if (error != eOK)
	{
		if (PreparedText)
			sldMemFree(PreparedText);
		return error;
	}
	
	if (!PreparedText)
		return eMemoryNullPointer;
	*/
	//************************************************************************
	
	if (!SearchImplementation.SetExpression(aText))
	{
		//sldMemFree(PreparedText);
		return eCommonFullTextSearchLogicalExpressionError;
	}
	
	// Производим поиск
	if (!SearchImplementation.Calculate())
	{
		//sldMemFree(PreparedText);
		return eCommonFullTextSearchLogicalExpressionError;
	}
	
	//sldMemFree(PreparedText);
	
	auto ResultOperand = SearchImplementation.GetResult();
	if (!ResultOperand)
		return eCommonFullTextSearchLogicalExpressionError;
	
	// Результаты поиска
	CSldSearchWordResult* Result = ((const CFullTextSearchImplementation::Operand*)ResultOperand)->GetSearchWordResult();
	if (!Result)
		return eCommonFullTextSearchLogicalExpressionError;
	
	error = AddWordFullTextSearchResult(Result, aList, aListCount, aMaximumWords);
	if (error != eOK)
		return error;
	
	// Конец поиска
	error = m_LayerAccess->WordFound(eWordFoundCallbackStopSearch);
	if (error == eExceptionSearchStop)
		return eOK;
		
	return error;
}

namespace {
namespace sort {

static const CSldSearchList::WordGetter getWord;

CSldList* getList(sld2::Span<const TSldSearchListStruct> aLists, UInt32 aIndex)
{
	return aIndex < aLists.size() ? aLists[aIndex].pList : nullptr;
}

using MorphoFormsArray = sld2::Span<const TSldMorphologyWordStruct>;

/// Структура, в которую записываются коэффициенты параметров вычисления веса слова при сортировке по релевантности
struct TWeightParametersPriority
{
	/// коэффициент совпадения слова со словоформой запроса
	UInt32 gradeEqualMorpho;
	/// коэффициент различия в количестве слов
	UInt32 gradeDifferenceWord;
	/// коэффициент порядкового номера совпавшего слова
	UInt32 gradeStart;
	/// коэффициент полного совпадения
	UInt32 gradeEqual;
};

struct WordWeightCalculator
{
	/**
	 * Вычисляет вес строчки. Т.е. то, насколько строчка соответствует запросу.
	 *
	 * @param[in]  aCMP              - таблица сравнения
	 * @param[in]  aText             - фраза которую нужно проверить
	 * @param[in]  aTextWords        - поисковый запрос разбитый на слова
	 * @param[in]  aMorphologyForms  - словоформы слов поискового запроса
	 *
	 * @return вес фразы - чем больше, тем лучше фраза подходит запросу.
	 */
	UInt32 complex(const CSldCompare *aCMP, SldU16StringRef aText, sld2::Span<const SldU16StringRef> aTextWords, MorphoFormsArray aMorphologyForms)
	{
		UInt32 MinDistancePhrase, MinDistanceMixed, MinDistanceMorpho;
		UInt32 FirstOccurrence = MinDistancePhrase = MinDistanceMixed = MinDistanceMorpho = aText.size();
		UInt32 Equal, EqualMorphologyBase, EqualMorphologyWordform;
		UInt32 FirstOccurrenceMorphology = aText.size();
		Equal = EqualMorphologyBase = EqualMorphologyWordform = 0;

		aCMP->DivideQueryByParts(aText, phraseWords);

		auto textWordsResults = resize(textWordsResults_, phraseWords.size(), aTextWords.size());
		for (UInt32 t = 0; t < aTextWords.size(); t++)
		{
			for (UInt32 p = 0; p < phraseWords.size(); p++)
				textWordsResults[index(t, p)] = aCMP->StrICmp(phraseWords[p], aTextWords[t]);
		}

		auto morphoWordsResults = resize(morphoWordsResults_, phraseWords.size(), aMorphologyForms.size());
		for (UInt32 t = 0; t < aMorphologyForms.size(); t++)
		{
			for (UInt32 p = 0; p < phraseWords.size(); p++)
				morphoWordsResults[index(t, p)] = aCMP->StrICmp(phraseWords[p], aMorphologyForms[t].MorphologyForm);
		}

		for (UInt32 t = 0; t < aTextWords.size(); t++)
		{
			for (UInt32 p = 0; p < phraseWords.size(); p++)
			{
				if (textWordsResults[index(t, p)] == 0)
				{
					Equal++; // Считаем количество одинаковых слов.
					if (FirstOccurrence > p) // Считаем первое вхождение слова во фразу.
						FirstOccurrence = p;

					break;
				}
			}
		}

		for (UInt32 t = 0; t<aMorphologyForms.size(); t++)
		{
			for (UInt32 p = 0; p < phraseWords.size(); p++)
			{
				if (morphoWordsResults[index(t, p)] == 0)
				{
					// Считаем количество словоформ, совпадающих со словами фразы
					if (aMorphologyForms[t].MorphologyFormType == eMorphologyWordTypeBase)
						EqualMorphologyBase++;
					else
						EqualMorphologyWordform++;

					if (FirstOccurrenceMorphology > p) // Считаем первое вхождение словоформы во фразу.
						FirstOccurrenceMorphology = p;

					break;
				}
			}
		}

		// Для фраз и запросов, состоящих из одного слова минимальное расстояние не устанавливается, т.к. нет второго слова для сравнения.
		if (phraseWords.size() == 1 || aTextWords.size() == 1)
		{
			MinDistancePhrase = 0;
			MinDistanceMixed = 0;
			MinDistanceMorpho = 0;
		}
		else
		{
			// Считаем минимальное расстояние между словами
			UInt32 firstIndex = (FirstOccurrence < FirstOccurrenceMorphology) ? FirstOccurrence : FirstOccurrenceMorphology;
			for (UInt32 phraseWordIndex = firstIndex; phraseWordIndex < phraseWords.size(); phraseWordIndex++)
			{
				// флаг, указывающий, что слово в результате поиска совпало со словом исходного запроса
				bool queryMatch = false;

				UInt32 textWordIndex;
				for (textWordIndex = 0; textWordIndex < aTextWords.size(); textWordIndex++)
				{
					if (textWordsResults[index(textWordIndex, phraseWordIndex)] == 0)
					{
						queryMatch = true;
						break;
					}
				}

				if (queryMatch)
				{
					// Нашли совпадение по основному слову, от него отсчитываем смещение.
					for (UInt32 i = phraseWordIndex + 1; i < phraseWords.size(); i++)
					{
						for (UInt32 s = 0; s<aTextWords.size(); s++)
						{
							if (textWordsResults[index(s, i)] == 0 && s != textWordIndex)
							{
								if (MinDistancePhrase > i - phraseWordIndex)
									MinDistancePhrase = i - phraseWordIndex;
								break;
							}
						}


						for (UInt32 s = 0; s < aMorphologyForms.size(); s++)
						{
							if (morphoWordsResults[index(s, i)] == 0)
							{
								if (MinDistanceMixed > i - phraseWordIndex)
									MinDistanceMixed = i - phraseWordIndex;
								break;
							}
						}
					}
				}
				else
				{
					// флаг, указывающий, что слово в результате поиска совпало со словоформой исходного запроса
					bool morphologyMatch = false;

					for (textWordIndex = 0; textWordIndex < aMorphologyForms.size(); textWordIndex++)
					{
						if (morphoWordsResults[index(textWordIndex, phraseWordIndex)] == 0)
						{
							morphologyMatch = true;
							break;
						}
					}

					if (!morphologyMatch)
						continue;

					// Нашли совпадение по словоформе, от него отсчитываем смещение.
					for (UInt32 i = phraseWordIndex + 1; i < phraseWords.size(); i++)
					{
						for (UInt32 s = 0; s<aTextWords.size(); s++)
						{
							if (textWordsResults[index(s, i)] == 0)
							{
								if (MinDistanceMixed > i - phraseWordIndex)
									MinDistanceMixed = i - phraseWordIndex;
								break;
							}
						}

						for (UInt32 s = 0; s<aMorphologyForms.size(); s++)
						{
							if (morphoWordsResults[index(s, i)] == 0 && s != textWordIndex)
							{
								if (MinDistanceMorpho > i - phraseWordIndex)
									MinDistanceMorpho = i - phraseWordIndex;
								break;
							}
						}
					}
				}
			}
		}

		// Считаем разницу между фразой и запросом (учитываем значащие символы)
		UInt32 SignificantPhraseLen = 0;
		for (auto&& word : phraseWords)
			SignificantPhraseLen += word.size();

		UInt32 TextLen = 0;
		for (auto&& word : aTextWords)
			TextLen += word.size();

		Int32 SignificantLenDiff = 0;
		// считаем абсолютное значение
		if (SignificantPhraseLen>TextLen)
			SignificantLenDiff = SignificantPhraseLen - TextLen;
		else
			SignificantLenDiff = TextLen - SignificantPhraseLen;

		UInt32 LenDiff = 0;
		if (aText.size() > TextLen)
			LenDiff = aText.size() - TextLen;
		else
			LenDiff = TextLen - aText.size();
		// разница в значащих символах уже учтена
		if (LenDiff >= SignificantLenDiff)
			LenDiff -= SignificantLenDiff;

		// на этом этапе мы не знаем, сколько разделителей было в исходной фразе
		// поэтому в качестве приближенной оценки считаем, что между словами было по 1 разделителю
		if (LenDiff >= aTextWords.size() - 1)
			LenDiff -= aTextWords.size() - 1;

		Int32 DiffWords = 0;
		if (phraseWords.size() > aTextWords.size())
			DiffWords = phraseWords.size() - aTextWords.size();
		else
			DiffWords = aTextWords.size() - phraseWords.size();

		// Считаем вес строчки
		if (!Equal && !EqualMorphologyBase && !EqualMorphologyWordform)
			return 0;

		// устанавливаем базовый вес фразы на основе количества совпадений слов/словоформ
		// повторения слов не учитываем
		Int32 result = 0;
		if ((Equal + EqualMorphologyBase + EqualMorphologyWordform) > aTextWords.size())
			result = aTextWords.size() * SLD_FTS_GRADE_EQUAL;
		else
			result = (Equal + EqualMorphologyBase + EqualMorphologyWordform)*SLD_FTS_GRADE_EQUAL;

		UInt32 minDistance = (MinDistancePhrase > MinDistanceMixed) ? MinDistanceMixed : MinDistancePhrase;
		minDistance = (minDistance > MinDistanceMorpho) ? MinDistanceMorpho : minDistance;

		FirstOccurrence = (FirstOccurrence < FirstOccurrenceMorphology) ? FirstOccurrence : FirstOccurrenceMorphology;

		// совпадения по словоформам менее приоритетны, чем совпадения по исходным словам
		if (EqualMorphologyBase >= aTextWords.size() - Equal - EqualMorphologyWordform)
			result -= 2 * EqualMorphologyBase*SLD_FTS_GRADE_EQUAL_MORPHO_BASE;
		if (EqualMorphologyWordform >= aTextWords.size() - Equal - EqualMorphologyBase)
			result -= 2 * EqualMorphologyWordform*SLD_FTS_GRADE_EQUAL_MORPHO_FORM;

		result -= minDistance*SLD_FTS_GRADE_DISTANCE;

		result -= FirstOccurrence*SLD_FTS_GRADE_START;

		result -= DiffWords*SLD_FTS_GRADE_DIFFERENCE_WORDS;

		result -= 2 * SignificantLenDiff*SLD_FTS_GRADE_DIFFERENCE_LENGTH;
		result -= LenDiff*SLD_FTS_GRADE_DIFFERENCE_LENGTH;

		return result;
	}

	/**
	 * Вычисляет вес строчки, используя меньшую часть параметров для оценки.
	 *
	 * @param[in]  aCMP              - таблица сравнения
	 * @param[in]  aText             - фраза которую нужно проверить
	 * @param[in]  aTextWords        - поисковый запрос разбитый на слова
	 * @param[in]  aMorphologyForms  - словоформы слов поискового запроса
	 * @param[in]  aWeights          - коэффициенты параметров вычисления веса слова
	 *
	 * @return вес фразы - чем больше, тем лучше фраза подходит запросу.
	 */
	UInt32 simple(const CSldCompare *aCMP, SldU16StringRef aText, sld2::Span<const SldU16StringRef> aTextWords, MorphoFormsArray aMorphologyForms, const TWeightParametersPriority *aWeights)
	{
		UInt32 ifNotFirstOccurrence = 1;
		UInt32 ifFirstOccurrenceMorphology = 1;
		UInt32 Equal = 0;
		UInt32 EqualMorphology = 0;

		aCMP->DivideQueryByParts(aText, phraseWords);

		for (UInt32 t = 0; t<aTextWords.size(); t++)
		{
			for (UInt32 p = 0; p<phraseWords.size(); p++)
			{
				if (aCMP->StrICmp(phraseWords[p], aTextWords[t]) == 0)
				{
					Equal++; // Считаем количество одинаковых слов.
					if (p == 0) // Считаем первое вхождение слова во фразу.
						ifNotFirstOccurrence = 0;

					break;
				}
			}
		}

		for (UInt32 t = 0; t<aMorphologyForms.size(); t++)
		{
			for (UInt32 p = 0; p < phraseWords.size(); p++)
			{
				if (aCMP->StrICmp(phraseWords[p], aMorphologyForms[t].MorphologyForm) == 0)
				{
					EqualMorphology++;

					if (p == 0) // Считаем первое вхождение словоформы во фразу.
						ifFirstOccurrenceMorphology = 0;

					break;
				}
			}
		}

		Int32 ifDiffWords = 0;
		if (phraseWords.size() != aTextWords.size())
			ifDiffWords = 1;

		// Считаем вес строчки
		if (!Equal && !EqualMorphology)
			return 0;

		// устанавливаем базовый вес фразы на основе количества совпадений слов/словоформ
		// повторения слов не учитываем

		Int32 result = 0;
		if ((Equal + EqualMorphology) > aTextWords.size())
			result = aTextWords.size() * aWeights->gradeEqual;
		else
			result = (Equal + EqualMorphology) * aWeights->gradeEqual;

		ifNotFirstOccurrence = (ifNotFirstOccurrence < ifFirstOccurrenceMorphology) ? ifNotFirstOccurrence : ifFirstOccurrenceMorphology;

		// совпадения по словоформам менее приоритетны, чем совпадения по исходным словам
		result -= EqualMorphology * aWeights->gradeEqualMorpho;

		result -= ifNotFirstOccurrence * aWeights->gradeStart;

		result -= ifDiffWords * aWeights->gradeDifferenceWord;

		return result;
	}

private:
	UInt32 index(UInt32 i, UInt32 j) const { return i * phraseWords.size() + j; }

	sld2::Span<Int32> resize(sld2::DynArray<Int32> &array, UInt32 i, UInt32 j)
	{
		const UInt32 size = i * j;
		if (size > array.size())
			array.resize(size);
		return sld2::make_span(array.data(), size);
	}

	CSldVector<SldU16StringRef> phraseWords;
	sld2::DynArray<Int32> textWordsResults_;
	sld2::DynArray<Int32> morphoWordsResults_;
};

namespace impl {

void doQuickSort(TSldSearchWordStruct *aWords, Int32 aFirstIndex, Int32 aLastIndex)
{
	Int32 i = aFirstIndex;
	Int32 j = aLastIndex;
	Int32 p = aWords[(i + j) >> 1].WordDistance;

	do
	{
		while (aWords[i].WordDistance > p)
			i++;

		while (aWords[j].WordDistance < p)
			j--;

		if (i <= j)
		{
			if (i < j)
				sld2::swap(aWords[i], aWords[j]);

			i++;
			j--;
		}
	} while (i <= j);

	if (i < aLastIndex)
		doQuickSort(aWords, i, aLastIndex);

	if (aFirstIndex < j)
		doQuickSort(aWords, aFirstIndex, j);

}

template <typename Compare>
void doStableQuickSort(TSldSearchWordStruct *aWords, Int32 aFirstIndex, Int32 aLastIndex, Compare&& compare)
{
	Int32 i = aFirstIndex;
	Int32 j = aLastIndex;
	const TSldSearchWordStruct pivot = aWords[(i + j) >> 1];

	do
	{
		Int32 cmp;
		while ((cmp = compare(aWords[i], pivot)) < 0 || (cmp == 0 && aWords[i].WordIndex < pivot.WordIndex))
			i++;

		while ((cmp = compare(aWords[j], pivot)) > 0 || (cmp == 0 && aWords[j].WordIndex > pivot.WordIndex))
			j--;

		if (i <= j)
		{
			if (i < j)
				sld2::swap(aWords[i], aWords[j]);

			i++;
			j--;
		}
	} while (i <= j);

	if (i < aLastIndex)
		doStableQuickSort(aWords, i, aLastIndex, compare);

	if (aFirstIndex < j)
		doStableQuickSort(aWords, aFirstIndex, j, compare);
}

} // namespace impl

// Производит сортировку вектора слов по дистанции между ними
void doQuickSort(sld2::Span<TSldSearchWordStruct> aWords)
{
	// XXX: We could write a generic quick_sort algo for this...
	if (aWords.size() > 1)
		impl::doQuickSort(aWords.data(), 0, aWords.size() - 1);
}

// Производит устойчивую сортировку вектора слов используя компаратор
template <typename Compare>
void doStableQuickSort(sld2::Span<TSldSearchWordStruct> aWords, Compare&& compare)
{
	if (aWords.size() > 1)
		impl::doStableQuickSort(aWords.data(), 0, aWords.size() - 1, sld2::forward<Compare>(compare));
}

namespace alphabetical {

using Word = sld2::Array<sld2::DynArray<UInt16>, 2>;

/**
 * Загружает слово
 *
 * @param[in]  aWord        - слово для загрузки
 * @param[in]  aLists       - массив листов
 * @param[in]  aLoadedWord  - ссылка на структуру куда загружать слово
 *
 * @return код ошибки
 */
ESldError loadWord(const TSldSearchWordStruct &aWord, sld2::Span<const TSldSearchListStruct> aLists, Word &aLoadedWord)
{
	CSldList *list = getList(aLists, aWord.ListIndex);
	if (!list)
		return eMemoryNullPointer;

	ESldError error = list->GetWordByGlobalIndex(aWord.WordIndex);
	if (error != eOK)
		return error;

	const CSldListInfo* pListInfo = NULL;
	error = list->GetWordListInfo(&pListInfo);
	if (error != eOK)
		return error;

	static const EListVariantTypeEnum variants[] = { eVariantShow, eVariantSortKey };
	for (UInt32 idx = 0; idx < sld2::array_size(variants); idx++)
	{
		const Int32 v = pListInfo->GetVariantIndexByType(variants[idx]);
		if (v == SLD_DEFAULT_VARIANT_INDEX)
			continue;

		const UInt16 *word = getWord(list, static_cast<UInt32>(v));
		if (!aLoadedWord[idx].assign(word, sld2::StrLen(word) + 1))
			return eMemoryNotEnoughMemory;
	}
	return eOK;
}

/**
 * Сравнение 2-х слов по show- и sortkey- вариантам
 *
 * @param[in]  aCMP   - таблица сравнения
 * @param[in]  word1  - указатель на массив вариантов написания 1-го слова
 * @param[in]  word2  - указатель на массив вариантов написания 2-го слова
 *
 * @return  0 - слова равны
 *          1 - первое слово больше второго
 *         -1 - первое слово меньше второго
 */
Int32 compare(const CSldCompare *aCMP, const Word &word1, const Word &word2)
{
	if (word1[0].size() && word2[0].size())
	{
		Int32 result = aCMP->StrICmp(word1[0].data(), word2[0].data());
		if (result)
			return result;
	}
	if (word1[1].size() && word2[1].size())
	{
		Int32 result = aCMP->StrICmp(word1[1].data(), word2[1].data());
		return result ? result : aCMP->StrCmp(word1[1].data(), word2[1].data());
	}
	return 0;
}

} // namespace alphabetical

/**
 * Устойчивая быстрая сортировка по алфавиту
 *
 * @param[inout]  aWords  - массив слов для сортировки
 * @param[in]     aLists  - массив листов
 *
 * @return код ошибки
 */
ESldError doAlphabeticalSort(sld2::Span<TSldSearchWordStruct> aWords,
							 sld2::Span<const TSldSearchListStruct> aLists)
{
	CSldList *pList = getList(aLists, aWords[0].ListIndex);
	if (!pList)
		return eMemoryNullPointer;

	const CSldCompare *CMP;
	ESldError error = pList->GetCompare((CSldCompare**)&CMP);
	if (error != eOK)
		return error;

	sld2::DynArray<alphabetical::Word> variants(aWords.size());
	for (UInt32 i = 0; i < aWords.size(); i++)
	{
		error = alphabetical::loadWord(aWords[i], aLists, variants[i]);
		if (error != eOK)
			return error;
	}

	// reuse WordDistance to store the initial word index so we don't have to swap variants
	for (UInt32 i = 0; i < aWords.size(); i++)
		aWords[i].WordDistance = static_cast<Int32>(i);

	auto compare = [&](const TSldSearchWordStruct &lhs, const TSldSearchWordStruct &rhs) {
		return alphabetical::compare(CMP, variants[lhs.WordDistance], variants[rhs.WordDistance]);
	};
	doStableQuickSort(aWords, compare);

	return eOK;
}

namespace simple {

struct LoadedWordsCache
{
	enum : UInt32 { VariantIndex = 0 };

	sld2::Span<const TSldSearchWordStruct> words;
	CSldList *list;

	sld2::DynArray<sld2::DynArray<UInt16>> cache;

	SldU16StringRef at(UInt32 n, ESldError *error)
	{
		if (cache[n].empty())
		{
			*error = list->GetWordByGlobalIndex(words[n].WordIndex);
			if (*error != eOK)
				return nullptr;

			const UInt16 *word = getWord(list, VariantIndex);
			if (!cache[n].assign(word, sld2::StrLen(word)))
			{
				*error = eMemoryNotEnoughMemory;
				return nullptr;
			}
		}
		*error = eOK;
		return SldU16StringRef(cache[n].data(), cache[n].size());
	}
};

struct WordComparator
{
	const CSldCompare *cmp;

	Int32 operator()(SldU16StringRef lhs, SldU16StringRef rhs) const
	{
		lhs = lhs.substr(0, (sld2::min)(lhs.size(), rhs.size()));
		return cmp->StrICmp(lhs, rhs);
	}

	WordComparator(const CSldCompare *aCMP) : cmp(aCMP) {}
};

// returns an array of variants indexes to be used for weight calculation
sld2::Span<const UInt32> getVariants(ISldList *aList, sld2::DynArray<UInt32> &aVariants, ESldError *aError)
{
	const CSldListInfo* pListInfo = nullptr;
	*aError = aList->GetWordListInfo(&pListInfo);
	if (*aError != eOK)
		return nullptr;

	const UInt32 NumberOfVariants = pListInfo->GetNumberOfVariants();
	if (!aVariants.resize(sld2::default_init, NumberOfVariants))
	{
		*aError = eMemoryNotEnoughMemory;
		return nullptr;
	}

	UInt32 count = 0;
	for (UInt32 v = 0; v < NumberOfVariants; v++)
	{
		const EListVariantTypeEnum variantType = pListInfo->GetVariantType(v);
		// вспомогательные варианты написания не учитываем
		if (variantType == eVariantShow || variantType == eVariantShowSecondary || variantType == eVariantSortKey || variantType == eVariantPhrase)
			aVariants[count++] = v;
	}

	*aError = eOK;
	return sld2::Span<UInt32>(aVariants).first(count);
}

// pretty straightforward helper for calculating word weights
template <typename Compare, typename CalcualteWeight>
ESldError calculateWeigths(Int32 index, Int32 count, Compare&& compare, CalcualteWeight&& calculateWeight)
{
	// Найдем начало диапазона, в котором находятся слова, равные шаблону
	Int32 idx = index;
	while (idx >= 0)
	{
		if (compare(idx))
		{
			ESldError error = calculateWeight(idx);
			if (error != eOK)
				return error;

			idx--;
			continue;
		}
		break;
	}

	// Найдем конец диапазона, в котором находятся слова, равные шаблону
	idx = index + 1;
	while (idx < count)
	{
		if (compare(idx))
		{
			ESldError error = calculateWeight(idx);
			if (error != eOK)
				return error;

			idx++;
			continue;
		}
		break;
	}
	return eOK;
}

// бинарный поиск шаблона поиска в результатах
template <typename Compare>
Int32 findMatch(UInt32 count, Compare&& compare, ESldError *error)
{
	UInt32 low = 0;
	UInt32 hi = count;
	while (low < hi)
	{
		UInt32 mid = (hi + low) >> 1;

		Int32 cmp = compare(mid, error);
		if (*error != eOK)
			return -1;

		if (cmp == 0)
			return static_cast<Int32>(mid);

		if (cmp < 0)
			low = mid + 1;
		else
			hi = mid;
	}
	return -1;
}

} // namespace simple

/**
 * Быстрая упрощенная сортировка по релевантности шаблону поиска
 *
 * Может применяться только в том случае, если поисковый список содержит результаты поиска в списке слов
 * полнотекстового поиска, который в свою очередь ссылается только на один отсортированный список слов
 *
 * ВНИМАНИЕ! Результат сортировки данной функцией отличается от результата, получаемого с помощью функции QuickSort,
 * а именно всего навсего происходит следующее: диапазон слов, которые равны шаблону поиска, перемещается в начало списка.
 * Отличие состоит в том, что при работе данной функции слова, которые содержат в себе шаблон поиска в качестве подстроки,
 * не учитываются (не перемещаются выше слов, которые шаблон поиска в себе не содержат)
 *
 * @param[inout]  aWords           - массив слов для сортировки
 * @param[in]     aLists           - массив листов
 * @param[in]     aText            - шаблон текста, по которому производился поиск
 * @param[in]     aMorphologyForms - массив словоформ
 *
 * @return код ошибки
 */
ESldError doSimpleRelevantSort(sld2::Span<TSldSearchWordStruct> aWords,
							   sld2::Span<const TSldSearchListStruct> aLists,
							   SldU16StringRef aText, MorphoFormsArray aMorphologyForms)
{
	if (aWords.size() < 2 || aText.empty())
		return eOK;

	ESldError error;

	// Список слов, на который ссылаются все результаты поиска, находящиеся в массиве найденных слов
	CSldList *pList = getList(aLists, aWords[0].ListIndex);

	// Кэш первых варинтов написания слов
	simple::LoadedWordsCache wordsCache;
	wordsCache.words = aWords;
	wordsCache.list = pList;
	if (!wordsCache.cache.resize(aWords.size()))
		return eMemoryNotEnoughMemory;

	const CSldCompare *CMP;
	error = pList->GetCompare((CSldCompare**)&CMP);
	if (error != eOK)
		return error;

	CSldVector<SldU16StringRef> TextWords;
	CMP->DivideQueryByParts(aText, TextWords);

	// Ищем индекс шаблона поиска в результатах (полное совпадение)
	const Int32 Index = simple::findMatch(aWords.size(),
							[&](UInt32 index, ESldError *error) {
								SldU16StringRef word = wordsCache.at(index, error);
								return *error == eOK ? CMP->StrICmp(word, aText) : 0;
							}, &error);
	if (error != eOK)
		return error;

	// массив индексов вариантов написания используемых для подсчета весов слов
	sld2::DynArray<UInt32> _variants;
	const auto variants = sort::simple::getVariants(pList, _variants, &error);
	if (error != eOK)
		return error;

	// расставляем веса для найденных слов
	for (UInt32 resultIndex = 0; resultIndex < aWords.size(); resultIndex++)
		aWords[resultIndex].WordDistance = 0;

	WordWeightCalculator weightCalc;
	// calculates weight for a word at the given index
	auto calculateWeight = [&](Int32 index) {
		ESldError error = pList->GetWordByGlobalIndex(aWords[index].WordIndex);
		if (error == eOK)
		{
			for (UInt32 v : variants)
				aWords[index].WordDistance += weightCalc.complex(CMP, getWord(pList, v), TextWords, aMorphologyForms);
		}
		return error;
	};

	if (Index >= 0)
	{
		auto cmp = [&](Int32 index) {
			SldU16StringRef word = wordsCache.at(index, &error);
			return error == eOK && CMP->StrICmp(word, aText) == 0;
		};
		error = simple::calculateWeigths(Index, aWords.size(), cmp, calculateWeight);
		if (error != eOK)
			return error;
	}

	// TODO: try to split it out into it's own function?
	//       that's pretty hard though as it pulls *a lot* of locals
	if (TextWords.size() > 1)
	{
		// инициализируем объект для сравнения "подстрок"
		simple::WordComparator StrICmp(CMP);

		// здесь хранятся найденные бинарным поиском индексы слов, начинающихся с одного из слов запроса
		// i-й элемент соответствует i-му слову запроса
		sld2::DynArray<Int32> similarResultIndexes(sld2::default_init, TextWords.size());
		if (similarResultIndexes.empty())
			return eMemoryNotEnoughMemory;

		// если в запросе только одно слово, то наиболее релевантные результаты будут оценены с помощью проверки на совпадение
		for (UInt32 queryWordIndex = 0; queryWordIndex < TextWords.size(); queryWordIndex++)
		{
			auto cmp = [&](UInt32 index, ESldError *error) {
				SldU16StringRef word = wordsCache.at(index, error);
				return *error == eOK ? StrICmp(word, TextWords[queryWordIndex]) : 0;
			};
			similarResultIndexes[queryWordIndex] = simple::findMatch(aWords.size(), cmp, &error);
			if (error != eOK)
				return error;
		}

		// для результатов, начинающихся с одного из слов запроса, расставляем веса
		for (UInt32 queryWordIndex = 0; queryWordIndex < TextWords.size(); queryWordIndex++)
		{
			Int32 idx = similarResultIndexes[queryWordIndex];
			if (idx < 0)
				continue;

			auto cmp = [&](Int32 index) {
				SldU16StringRef word = wordsCache.at(index, &error);
				return error == eOK && StrICmp(word, TextWords[queryWordIndex]) == 0;
			};
			error = simple::calculateWeigths(idx, aWords.size(), cmp, calculateWeight);
			if (error != eOK)
				return error;
		}
	}

	// для результатов, не получивших вес ранее, выставляем веса на основе позиции в исходной выдаче
	// (так мы после сортировки сохраним порядок результатов)
	// во избежание конфликтов с уже рассмотренными словами (имеющими положительный вес) веса отрицательны
	Int32 currentWeight = -static_cast<Int32>(aWords.size());
	for (Int32 i = aWords.size() - 1; i >= 0; i--)
	{
		if (!aWords[i].WordDistance)
			aWords[i].WordDistance = currentWeight++;
	}

	doQuickSort(aWords);

	return eOK;
}

/**
 * Полная сортировка по релевантности шаблону поиска.
 * Используется быстрая сортировка.
 *
 * @param[inout]  aWords           - массив слов для сортировки
 * @param[in]     aLists           - массив листов
 * @param[in]     aText            - шаблон текста, по которому производился поиск
 * @param[in]     aMorphologyForms - массив словоформ
 *
 * @return код ошибки
 */
ESldError doFullRelevantSort(sld2::Span<TSldSearchWordStruct> aWords,
							 sld2::Span<const TSldSearchListStruct> aLists,
							 SldU16StringRef aText, MorphoFormsArray aMorphologyForms)
{
	if (aWords.size() < 2 || aText.empty())
		return eOK;

	CSldList* pList = getList(aLists, aWords[0].ListIndex);
	if (!pList)
		return eMemoryNullPointer;

	const CSldCompare *CMP;
	ESldError error = pList->GetCompare((CSldCompare**)&CMP);
	if (error != eOK)
		return error;

	CSldVector<SldU16StringRef> TextWords;
	CMP->DivideQueryByParts(aText, TextWords);

	sort::WordWeightCalculator weightCalc;
	for (auto&& word : aWords)
	{
		// Получаем первое слово
		pList = getList(aLists, word.ListIndex);
		if (!pList)
			return eMemoryNullPointer;

		error = pList->GetWordByGlobalIndex(word.WordIndex);
		if (error != eOK)
			return error;

		const CSldListInfo* pListInfo = NULL;
		error = pList->GetWordListInfo(&pListInfo);
		if (error != eOK)
			return error;

		word.WordDistance = 0;
		for (UInt32 v = 0; v < pListInfo->GetNumberOfVariants(); v++)
		{
			// вспомогательные варианты написания не учитываем
			const EListVariantTypeEnum VariantType = pListInfo->GetVariantType(v);
			if (!(VariantType == eVariantShow || VariantType == eVariantShowSecondary || VariantType == eVariantSortKey || VariantType == eVariantPhrase))
				continue;

			word.WordDistance += weightCalc.complex(CMP, getWord(pList, v), TextWords, aMorphologyForms);
		}
	}

	doQuickSort(aWords);

	return eOK;
}

namespace group {

// количество вариантов написания актуальных для сортировки
// при изменении необходимо проверить все места использования
enum : UInt32 { RelevantVariantsCount = 4 };
static const sld2::Array<EListVariantTypeEnum, RelevantVariantsCount> g_variantPriority = {
	eVariantShow, eVariantSortKey, eVariantPhrase, eVariantShowSecondary
};

struct Context
{
	// TODO: merge in a single structure
	sld2::DynArray<sld2::Array<Int32, RelevantVariantsCount>> wordWeights;
	sld2::DynArray<alphabetical::Word> loadedWords;

	WordWeightCalculator weightCalc;

	sld2::Span<const TSldSearchListStruct> lists;

	MorphoFormsArray morphologyForms;
	CSldVector<SldU16StringRef> queryWords;

	const CSldCompare *CMP;
	TWeightParametersPriority weightParams;
};

/// Загружает слово для алфавитного сравнения
ESldError loadWord(Context &ctx, const TSldSearchWordStruct &word)
{
	CSldList *pList = getList(ctx.lists, word.ListIndex);
	if (!pList)
		return eMemoryNullPointer;

	CSldCompare *CMP;
	ESldError error = pList->GetCompare(&CMP);
	if (error != eOK)
		return error;

	const CSldListInfo* pListInfo = NULL;
	error = pList->GetWordListInfo(&pListInfo);
	if (error != eOK)
		return error;

	error = pList->GetWordByGlobalIndex(word.WordIndex);
	if (error != eOK)
		return error;

	for (UInt32 variantIndex = 0; variantIndex < g_variantPriority.size(); variantIndex++)
	{
		const EListVariantTypeEnum variantType = g_variantPriority[variantIndex];
		const Int32 v = pListInfo->GetVariantIndexByType(variantType);
		if (v == SLD_DEFAULT_VARIANT_INDEX)
			continue;

		const UInt16 *wordText = getWord(pList, static_cast<UInt32>(v));
		if (variantType == eVariantShow || variantType == eVariantSortKey)
		{
			const UInt32 idx = variantType == eVariantShow ? 0 : 1;
			if (!ctx.loadedWords[word.WordDistance][idx].assign(wordText, sld2::StrLen(wordText) + 1))
				return eMemoryNotEnoughMemory;
		}
		ctx.wordWeights[word.WordDistance][variantIndex] =
			ctx.weightCalc.simple(CMP, wordText, ctx.queryWords, ctx.morphologyForms, &ctx.weightParams);
	}

	return eOK;
}

/**
 * Сравнение 2-х слов по весам.
 *
 * @param[in]   lhs  - первое слово
 * @param[in]   rhs  - второе слово
 *
 * @return результат
 *          0 - слова равны
 *         >0 - первое слово больше второго
 *         <0 - первое слово меньше второго
 */
Int32 compare(const Context &ctx, const TSldSearchWordStruct &lhs, const TSldSearchWordStruct &rhs)
{
	const UInt32 lhsIndex = lhs.WordDistance;
	const UInt32 rhsIndex = rhs.WordDistance;

	for (UInt32 variantType = 0; variantType < g_variantPriority.size(); variantType++)
	{
		const Int32 weight1 = ctx.wordWeights[lhsIndex][variantType];
		const Int32 weight2 = ctx.wordWeights[rhsIndex][variantType];

		if (weight1 == WORD_WEIGHT_IS_UNDEFINED || weight2 == WORD_WEIGHT_IS_UNDEFINED)
			continue;

		if (weight1 != weight2)
			return weight2 - weight1;
	}

	return alphabetical::compare(ctx.CMP, ctx.loadedWords[lhsIndex], ctx.loadedWords[rhsIndex]);
}

} // namespace group

/**
 * Cортировка по релевантности шаблону поиска, разбивающая на группы.
 * Используется устойчивая сортировка слиянием.
 *
 * @param[inout]  aWords           - массив слов для сортировки
 * @param[in]     aLists           - массив листов
 * @param[in]     aText            - шаблон текста, по которому производился поиск
 * @param[in]     aMorphologyForms - массив словоформ
 * @param[in]     aListInfo        - указатель на лист инфо *поискового* списка
 *
 * @return код ошибки
 */
ESldError doRelevantSortByGroups(sld2::Span<TSldSearchWordStruct> aWords,
								 sld2::Span<const TSldSearchListStruct> aLists,
								 SldU16StringRef aText, MorphoFormsArray aMorphologyForms,
								 const CSldListInfo *aListInfo)
{
	if (aWords.size() < 2)
		return eOK;

	group::Context ctx;

	ctx.lists = aLists;

	CSldList* pList = getList(aLists, aWords[0].ListIndex);
	if (!pList)
		return eMemoryNullPointer;

	CSldCompare *CMP;
	ESldError error = pList->GetCompare(&CMP);
	if (error != eOK)
		return error;

	// заполняем структуру запроса
	CMP->DivideQueryByParts(aText, ctx.queryWords);
	ctx.morphologyForms = aMorphologyForms;

	ctx.CMP = CMP;

	const UInt32 maxWordLen = aListInfo->GetMaximumWordSize();

	// Определяем коэффициенты параметров
	ctx.weightParams.gradeEqualMorpho = 1;
	ctx.weightParams.gradeDifferenceWord = maxWordLen;
	ctx.weightParams.gradeStart = ctx.weightParams.gradeDifferenceWord * maxWordLen;
	ctx.weightParams.gradeEqual = ctx.weightParams.gradeStart * maxWordLen;

	// расставляем веса для найденных слов
	static const sld2::Array<Int32, group::RelevantVariantsCount> initialWeight = {
		WORD_WEIGHT_IS_UNDEFINED, WORD_WEIGHT_IS_UNDEFINED, WORD_WEIGHT_IS_UNDEFINED, WORD_WEIGHT_IS_UNDEFINED
	};
	if (!ctx.wordWeights.resize(aWords.size(), initialWeight))
		return eMemoryNotEnoughMemory;

	if (!ctx.loadedWords.resize(aWords.size()))
		return eMemoryNotEnoughMemory;

	for (UInt32 i = 0; i < aWords.size(); i++)
	{
		// reuse WordDistance to store the initial word index so we don't have to swap weights
		aWords[i].WordDistance = static_cast<Int32>(i);

		// load all of the words upfront
		error = group::loadWord(ctx, aWords[i]);
		if (error != eOK)
			return error;
	}

	auto compare = [&](const TSldSearchWordStruct &lhs, const TSldSearchWordStruct &rhs) {
		return group::compare(ctx, lhs, rhs);
	};
	doStableQuickSort(aWords, compare);

	return eOK;
}


} // namespace sort
} // anon namespace

/** *********************************************************************
* Производит сортировку поискового списка слов по релевантности шаблону поиска
*
* @param[in]	aText		- шаблон поиска
* @param[in]	aMode	- true: сортировать по упрощенной схеме если возможно - более быстро, но менее качественно
*							  false: полная сортировка - более медленно, но более качественно
*
* @return код ошибки
************************************************************************/
ESldError CSldSearchList::SortListRelevant(const UInt16* aText, const CSldVector<TSldMorphologyWordStruct>& aMorphologyForms, ESldFTSSortingTypeEnum aMode)
{
	if (!aText)
		return eMemoryNullPointer;

	ESldError error = eOK;
	Int32 NumberOfWords = WordCount();

	if (!NumberOfWords)
		return eOK;

	auto words = sld2::make_span(m_WordVector.data(), NumberOfWords);
	auto lists = sld2::make_span(m_ListVector.data(), m_ListCount);

	switch (aMode)
	{
	case eFTSSortingType_Simple:
		if (m_EnableSimpleRelevantSort)
			error = sort::doSimpleRelevantSort(words, lists, aText, aMorphologyForms);
		else
			error = sort::doFullRelevantSort(words, lists, aText, aMorphologyForms);
		break;

	case eFTSSortingType_AlphabeticalSortedGroups:
		error = sort::doRelevantSortByGroups(words, lists, aText, aMorphologyForms, m_ListInfo.get());
		break;

	case eFTSSortingType_Full:
		error = sort::doFullRelevantSort(words, lists, aText, aMorphologyForms);
		break;

	case eFTSSortingType_Smart:
		if (m_EnableSimpleRelevantSort && NumberOfWords > SLD_FTS_NUMBER_OF_WORDS_FOR_FULL_MODE)
			error = sort::doSimpleRelevantSort(words, lists, aText, aMorphologyForms);
		else
			error = sort::doFullRelevantSort(words, lists, aText, aMorphologyForms);
		break;

	case eFTSSortingType_Alphabetical:
		error = sort::doAlphabeticalSort(words, lists);
		break;

	default:
		break;
	}

	return error;
}

/** *********************************************************************
* Производит полнотекстовый поиск одного слова
*
* @param[in]	aText			- шаблон поиска
* @param[in]	aMaximumWords	- максимальное количество слов, которые могут быть найдены
* @param[in]	aList			- указатель на массив списков слов словаря
* @param[in]	aListCount		- количество списков слов в массиве
* @param[in]	aRealListIndex	- реальный индекс списка слов, в котором производим поиск
* @param[out]	aResultData		- указатель на класс, куда нужно сохранять результаты поиска
*
* @return код ошибки
************************************************************************/
ESldError CSldSearchList::DoWordFullTextSearch(const UInt16* aText, Int32 aMaximumWords, ISldList** aList, Int32 aListCount, Int32 aRealListIndex, CSldSearchWordResult* aResultData)
{
	if (!aText || !aList || !aResultData)
		return eMemoryNullPointer;
	if (!isInit())
		return eCommonListNotInitialized;
	if (aRealListIndex >= aListCount)
		return eCommonWrongIndex;

	ESldError error = eOK;
	Int32 i = 0;
	const CSldListInfo* pListInfo = NULL;
	
	// Обычный список, в котором производится поиск
	CSldList* pList = (CSldList*)aList[aRealListIndex];
	if (!pList)
		return eCommonListNotInitialized;
	
	error = pList->GetWordListInfo(&pListInfo);
	if (error != eOK)
		return error;

	// Свойства списка
	if (!pListInfo->IsFullTextSearchList())
		return eCommonWrongList;

	const bool isSuffixFullTextSearchList = pListInfo->IsSuffixFullTextSearchList();

	// Количество вариантов написания
	const UInt32 NumberOfVariants = pListInfo->GetNumberOfVariants();

	// Длина переданного текста
	Int32 TextLen = pList->GetCMP()->StrLen(aText);
	
	// Нечего искать
	if (!TextLen)
		return eOK;
	
	// Состоит ли слово полностью из разделителей
	UInt16 emptyStr[2] = {0};
	UInt8 isFullSpacedWord = 0;
	if (pList->GetCMP()->StrICmp(aText, emptyStr) == 0)
		isFullSpacedWord = 1;

	// Границы поиска
	Int32 LowIndex = 0;
	Int32 HighIndex = 0;
	ESldSearchRange searchRange;
	if(pListInfo->GetUsage() == eWordListType_Dictionary && pListInfo->IsHierarchy())
	{
		searchRange = eSearchRangeRoot;
	}
	else
	{
		searchRange = eSearchRangeCurrentLevelRecursive;
	}

	error = pList->GetSearchBounds(searchRange, &LowIndex, &HighIndex);
	if (error != eOK)
		return error;
	
	error = pList->GetWordByText(aText);
	if (error != eOK)
		return error;
	
	Int32 StartPos = 0;
	error = pList->GetCurrentGlobalIndex(&StartPos);
	if (error != eOK)
		return error;

	UInt16 *pWordCopyBuf = NULL;
	sld2::DynArray<UInt16> _wordCopyBuf;
	if (isSuffixFullTextSearchList)
	{
		if (!_wordCopyBuf.resize(sld2::default_init, pListInfo->GetMaximumWordSize()))
			return eMemoryNotEnoughMemory;
		pWordCopyBuf = _wordCopyBuf.data();
	}
	
	// Поиск
	UInt8 suffix_search_mode = 0;
	Int32 globalWordIdx = 0;
	for (i=StartPos;i<HighIndex;i++)
	{
		error = pList->GetWordByGlobalIndex(i);
		if (error != eOK)
			return error;
		
		error = pList->GetCurrentGlobalIndex(&globalWordIdx);
		if (error != eOK)
			return error;
		
		UInt8 full_word_found = 0;
		UInt8 suffix_word_found = 0;
		for (UInt32 v=0;v<NumberOfVariants;v++)
		{
			EListVariantTypeEnum VariantType = pListInfo->GetVariantType(v);

			// Вспомогательные варианты написания не учитываем
			if (!(VariantType==eVariantShow || VariantType==eVariantShowSecondary))
				continue;
			
			if (!suffix_search_mode)
			{
				if (pList->GetCMP()->StrICmp(aText, pList->GetWord(v)) == 0)
					full_word_found = 1;
				else if (isSuffixFullTextSearchList)
					suffix_search_mode = 1;
			}
			
			if (!full_word_found && suffix_search_mode)
			{
				// Суффиксный поиск
				// TODO: оптимизировать
				// ------------------------------------------------
				pList->GetCMP()->StrCopy(pWordCopyBuf, pList->GetWord(v));
				Int32 WordLen = pList->GetCMP()->StrLen(pWordCopyBuf);

				for (Int32 n=WordLen-1;n>=0;n--)
				{
					if (pList->GetCMP()->StrICmp(aText, pWordCopyBuf) == 0)
					{
						suffix_word_found = 1;
						break;
					}
					pWordCopyBuf[n] = 0;
				}
			}
			
			if (!full_word_found && !suffix_word_found)
				continue;
			
			if (isFullSpacedWord)
			{
				if (pList->GetCMP()->StrCmp(aText, pList->GetWord(v)) != 0)
					continue;
			}
			
			// Добавляем слова
			Int32 TranslationCount = 0;
			error = pList->GetReferenceCount(globalWordIdx, &TranslationCount);
			if (error != eOK)
				return error;
			
			Int32 ListIdx = 0;
			Int32 ListEntryIdx = 0;
			Int32 TransIdx = 0;
			Int32 ShiftIdx = 0;
			for (Int32 j=0;j<TranslationCount;j++)
			{
				error = pList->GetFullTextTranslationData(globalWordIdx, j, &ListIdx, &ListEntryIdx, &TransIdx, &ShiftIdx);
				if (error != eOK)
					return error;
				
				error = aResultData->AddWord(ListIdx, ListEntryIdx, TransIdx, ShiftIdx);
				if (error != eOK)
					return error;
				
				Int32 ResultCount = aResultData->GetWordCount();
				// Запрашиваем у оболочки, может уже нужно остановить поиск
				if ((ResultCount%SLD_SEARCH_CALLBACK_INTERLEAVE) == 0)
				{
					error = m_LayerAccess->WordFound(eWordFoundCallbackInterleave, ResultCount);
					if (error == eExceptionSearchStop)
						return error;
				}
			}
			// Один из вариантов написания слова подходит - другие проверять не будем
			break;
		}
		
		if (full_word_found || suffix_word_found)
			continue;
		
		break;
	}

	return eOK;
}

/** *********************************************************************
* Производит добавление результатов полнотекстового поиска одного слова
*
* @param[out]	aResultData		- указатель на класс, содержащий результаты поиска
* @param[in]	aList			- указатель на массив списков слов словаря
* @param[in]	aListCount		- количество списков слов в массиве
* @param[in]	aMaximumWords	- максимальное количество слов, которое мы хотим видеть в массиве
*
* @return код ошибки
************************************************************************/
ESldError CSldSearchList::AddWordFullTextSearchResult(CSldSearchWordResult* aResultData, ISldList** aList, Int32 aListCount, Int32 aMaximumWords)
{
	if (!aResultData || !aList)
		return eMemoryNullPointer;
	
	ESldError error;

	Int32 ListCount = aResultData->GetListCount();
	if (aListCount != ListCount)
		return eCommonWrongInitSearchList;
	
	Int32 WordCount = aResultData->GetWordCount();
	
	error = SetMaximumLists(ListCount);
	if (error != eOK)
		return error;
	
	error = SetMaximumWords(WordCount);
	if (error != eOK)
		return error;
	
	TSldSearchListStruct* addList = NULL;
	for (Int32 i=0;i<ListCount;i++)
	{
		error = MakeList((CSldList*)aList[i], i, &addList);
		if (error != eOK)
			return error;
	}

	// Добавляем слова тогда, когда они найдены
	if (WordCount)
		m_WordCount = aResultData->FillWordVector(m_WordVector, aMaximumWords);

	if (!m_WordCount)
		return eOK;

	// Проверка, можем ли мы корректно (в рамках текущей реализации) отображать результаты поиска.
	// Для этого нужно, чтобы списки слов, на которые ссылаются слова из результатов поиска, имели одинаковые
	// количество вариантов написания, их порядок следования и их типы.
	
	// Найдем свойства первого списка, на который ссылаемся
	Int32 ListIndex = 0;
	for (;ListIndex<ListCount;ListIndex++)
	{
		if (aResultData->IsListPresent(ListIndex))
			break;
	}
	
	// Количество найденных слов != 0, а список не найден
	if (ListIndex >= ListCount)
		return eCommonFullTextSearchError;

	// Свойства первого найденного списка (списка с номером ListIndex)
	const CSldListInfo* pListInfo = NULL;
	error = aList[ListIndex]->GetWordListInfo(&pListInfo);
	if (error != eOK)
		return error;

	const UInt32 NumberOfVariants = pListInfo->GetNumberOfVariants();

	// Общее количество списков слов, в которых мы что-то нашли при поиске
	Int32 TotalListsWithResults = 1;

	// Свойства вариантов написания остальных списков должны совпадать со свойствами первого найденного
	UInt32 tmpNumberOfVariants = 0;
	const CSldListInfo* tmpListInfo = NULL;
	for (Int32 i=ListIndex+1;i<ListCount;i++)
	{
		if (!aResultData->IsListPresent(i))
			continue;
		
		TotalListsWithResults++;
		
		error = aList[i]->GetNumberOfVariants(&tmpNumberOfVariants);
		if (error != eOK)
			return error;

		if (tmpNumberOfVariants != NumberOfVariants)
			return eCommonFullTextSearchWrongVariants;

		error = aList[i]->GetWordListInfo(&tmpListInfo);
		if (error != eOK)
			return error;

		for (UInt32 j=0;j<NumberOfVariants;j++)
		{
			EListVariantTypeEnum VariantType1 = pListInfo->GetVariantType(j);
			EListVariantTypeEnum VariantType2 = tmpListInfo->GetVariantType(j);
			if (VariantType1 != VariantType2)
				return eCommonFullTextSearchWrongVariants;
		}
	}
	
	// Результы полнотекстового поиска ссылаются только на один реальный список
	// И он отсортирован - устанавливаем флаг для возможности упрощенной сортировки результатов поиска
	if (TotalListsWithResults == 1 && pListInfo->IsSortedList())
		m_EnableSimpleRelevantSort = 1;

	// Изменяем заголовок нашего поискового списка
	error = UpdateListHeader(pListInfo);
	if (error != eOK)
		return error;

	// TODO: остается реализовать случай, когда свойства вариантов написания в списках, на которые ссылаются результаты
	// полнотекстового поиска, не совпадают друг с другом.
	// То же самое справедливо в общем случае для любого поискового списка.
	
	return eOK;
}

/** *********************************************************************
* Производит добавление результатов поиска по шаблону
*
* @param[out]	aResultData		- указатель на класс, содержащий результаты поиска
* @param[in]	aList			- указатель на список слов
* @param[in]	aRealListIndex	- реальный номер списка слов
* @param[in]	aMaximumWords	- максимальное количество слов, которое мы хотим видеть в массиве
*
* @return код ошибки
************************************************************************/
ESldError CSldSearchList::AddWildCardSearchSearchResults(CSldSimpleSearchWordResult* aResultData, ISldList* aList, Int32 aRealListIndex, Int32 aMaximumWords)
{
	if (!aResultData || !aList)
		return eMemoryNullPointer;

	ESldError error;

	TSldSearchListStruct* addList = NULL;
	error = MakeList((CSldList*)aList, aRealListIndex, &addList);
	if (error != eOK)
		return error;

	m_CurrentListIndex = addList->ListIndex;
	m_List = addList;

	// Добавляем слова тогда, когда они найдены
	if (aResultData->GetWordCount())
		m_WordCount = aResultData->FillWordVector(m_WordVector, aMaximumWords, m_CurrentListIndex);

	return eOK;
}

/** *********************************************************************
* Устанавливает максимальное количество списков
*
* @param[in]	aMaxLists	- максимальное количество списков
*
* @return код ошибки
************************************************************************/
ESldError CSldSearchList::SetMaximumLists(Int32 aMaxLists)
{
	m_ListCount = 0;
	m_CurrentListIndex = 0;
	m_List = NULL;

	if (!aMaxLists)
		return eOK;

	if (!m_ListVector.resize(aMaxLists))
		return eMemoryNotEnoughMemory;

	return eOK;
}

/** *********************************************************************
* Устанавливает максимальное количество слов
*
* @param[in]	aMaxWords	- максимальное количество слов
*
* @return код ошибки
************************************************************************/
ESldError CSldSearchList::SetMaximumWords(Int32 aMaxWords)
{
	m_WordCount = 0;
	m_CurrentWordIndex = 0;
	m_Word = NULL;

	if (!aMaxWords)
		return eOK;

	if (!m_WordVector.resize(aMaxWords))
		return eMemoryNotEnoughMemory;

	return eOK;
}

/** *********************************************************************
* Добавляет список слов, в котором производился поиск
*
* @param[in]	aList	- указатель на структуру, описывающую добавляемый список
*
* @return код ошибки
************************************************************************/
ESldError CSldSearchList::AddList(const TSldSearchListStruct &aList)
{
	if (m_ListCount >= m_ListVector.size())
		return eCommonWrongIndex;

	m_ListVector[m_ListCount++] = aList;
	return eOK;
}

/** *********************************************************************
* Создает или находит уже имеющуюся внутреннюю запись о реальном списке слов
*
* @param[in]	aRealList		- указатель на реальный список слов, в котором производим поиск
* @param[in]	aRealListIndex	- реальный индекс реального списка слов
* @param[out]	aList			- указатель, по которому будет записан указатель на нужную структуру
*
* @return код ошибки
************************************************************************/
ESldError CSldSearchList::MakeList(CSldList* aRealList, Int32 aRealListIndex, TSldSearchListStruct** aList)
{
	ESldError error;

	if (!aList)
		return eMemoryNullPointer;

	*aList = NULL;

	// Может мы уже искали ранее в этом списке
	for (Int32 i = 0; i < m_ListCount; i++)
	{
		if (m_ListVector[i].RealListIndex == aRealListIndex)
		{
			*aList = &m_ListVector[i];
			break;
		}
	}

	// Нашли
	if (*aList)
		return eOK;

	// Создаем новую запись
	TSldSearchListStruct addList;
	// Локальный номер списка
	addList.ListIndex = m_ListCount;
	// Указатель на реальный список
	addList.pList = aRealList;
	// Реальный индекс списка слов
	addList.RealListIndex = aRealListIndex;

	// Добавляем запись
	error = AddList(addList);
	if (error != eOK)
		return error;

	*aList = &m_ListVector.back();

	return eOK;
}

/** *********************************************************************
* Добавляет слово
*
* @param[in]	aWord	- указатель на структуру, описывающую добавляемое слово
*
* @return код ошибки
************************************************************************/
ESldError CSldSearchList::AddWord(const TSldSearchWordStruct &aWord)
{
	if (m_WordCount >= m_WordVector.size())
		return eCommonWrongIndex;

	m_WordVector[m_WordCount++] = aWord;

	return eOK;
}

/** *********************************************************************
* Добавляет слово, сортируя по расстоянию редактирования
*
* @param[in]	aWord	- указатель на структуру, описывающую добавляемое слово
*
* @return код ошибки
************************************************************************/
ESldError CSldSearchList::AddFuzzyWord(const TSldSearchWordStruct &aWord)
{
	if (m_WordCount >= m_WordVector.size())
		return eCommonWrongIndex;
	
	Int32 i;
	for (i=0;i<m_WordCount;i++)
	{
		if (m_WordVector[i].WordDistance > aWord.WordDistance)
			break;
	}
	
	if (i == m_WordCount)
	{
		m_WordVector[m_WordCount] = aWord;
	}
	else
	{
		for (Int32 j=m_WordCount;j>i;j--)
		{
			m_WordVector[j] = m_WordVector[j-1];
		}
		m_WordVector[i] = aWord;
	}
	
	m_WordCount++;

	return eOK;
}

/** *********************************************************************
* Добавляет слово, не допуская дублирования
*
* @param[in]	aWord		- указатель на структуру, описывающую добавляемое слово
* @param[in]	aIsDouble	- указатель, куда будет записан результат
*							  0 - слово еще не добавлялось
*							  1 - слово уже есть в результатах поиска
*
* @return код ошибки
************************************************************************/
ESldError CSldSearchList::AddSpellingWord(const TSldSearchWordStruct &aWord, Int8* aIsDouble)
{
	if (m_WordCount >= m_WordVector.size())
		return eCommonWrongIndex;
	
	for (Int32 i=0;i<m_WordCount;i++)
	{
		if (m_WordVector[i].ListIndex == aWord.ListIndex && m_WordVector[i].WordIndex == aWord.WordIndex)
		{
			*aIsDouble = 1;
			return eOK;
		}
	}

	m_WordVector[m_WordCount++] = aWord;
	*aIsDouble = 0;

	return eOK;
}

/** *********************************************************************
* Удаляет последнее слово в списке
************************************************************************/
void CSldSearchList::RemoveLastWord(void)
{
	if (m_WordCount)
		m_WordCount--;
}

/** *********************************************************************
* Получает список по индексу
*
* @param[in]	aIndex	- индекс запрашиваемого списка
*
* @return указатель на список или NULL, если такой список не найден
************************************************************************/
TSldSearchListStruct* CSldSearchList::GetList(Int32 aIndex) const
{
	if (aIndex >= m_ListCount)
		return NULL;

	return (TSldSearchListStruct*) &m_ListVector[aIndex];
}

/** *********************************************************************
* Получает слово по индексу
*
* @param[in]	aIndex	- индекс запрашиваемого слова
*
* @return указатель на слово или NULL, если такое слово не найдено
************************************************************************/
TSldSearchWordStruct* CSldSearchList::GetWord(Int32 aIndex) const
{
	if (aIndex >= m_WordCount)
		return NULL;

	return (TSldSearchWordStruct*) &m_WordVector[aIndex];
}

/** *********************************************************************
* Устанавливаем HASH для декодирования данного списка слов.
*
* @param[in]	aHASH	- HASH, используемый для декодирования
*
* @return код ошибки
************************************************************************/
ESldError CSldSearchList::SetHASH(UInt32 aHASH)
{
	return eOK;
}

/** *********************************************************************
* Сохраняет текущее состояние списка слов
*
* @return код ошибки
************************************************************************/
ESldError CSldSearchList::SaveCurrentState(void)
{
	ESldError error;

	m_CurrentState.m_Path.Clear();
	
	if (!m_WordCount)
		return eOK;

	Int32 GlobalIndex = 0;
	error = GetCurrentGlobalIndex(&GlobalIndex);
	if (error != eOK)
		return error;

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
ESldError CSldSearchList::RestoreState(void)
{
	if (!m_CurrentState.m_Path.BaseList)
		return eOK;

	return GoToByPath(&m_CurrentState.m_Path, eGoToWord);
}

/** *********************************************************************
* Производит переход по таблице быстрого поиска
*
* @param[in]	aSearchIndex	- номер записи в таблице быстрого поиска
*
* @return код ошибки
************************************************************************/
ESldError CSldSearchList::GoToBySearchIndex(Int32 aSearchIndex)
{
	return eCommonWrongList;
}

/** ********************************************************************
* Получает следующее слово
*
* @return код ошибки
************************************************************************/
ESldError CSldSearchList::GetNextWord()
{
	return eCommonWrongList;
}

/** ********************************************************************
* Возвращает таблицу сравнения которая используется в данном списке слов
*
* @param[out]	aCompare	- указатель на переменную, в которую будет помещен указатель на объект сравнения строк
*
* @return код ошибки
************************************************************************/
ESldError CSldSearchList::GetCompare(CSldCompare **aCompare)
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
ESldError CSldSearchList::QAGetNumberOfQAItems(Int32 *aNumberOfQAItems)
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
ESldError CSldSearchList::QAGetItemText(Int32 aIndex, const UInt16 **aQAText)
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
ESldError CSldSearchList::QAGetItemIndex(Int32 aIndex, Int32 *aQAIndex)
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
ESldError CSldSearchList::JumpByQAIndex(Int32 aIndex)
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
ESldError CSldSearchList::GetHierarchyLevelBounds(Int32* aLowGlobalIndex, Int32* aHighGlobalIndex)
{
	if (!aLowGlobalIndex || !aHighGlobalIndex)
		return eMemoryNullPointer;
	
	*aLowGlobalIndex = 0;
	*aHighGlobalIndex = WordCount();
	
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
ESldError CSldSearchList::GetSoundIndexByText(const UInt16 *aText, CSldVector<Int32> & aSoundIndexes)
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
ESldError CSldSearchList::SetLocalization(ESldLanguage aLocalizationLangCode)
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
ESldError CSldSearchList::SetLocalization(ESldLanguage aLocalizationLangCode, const UInt16* aLocalizationDetails)
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
ESldError CSldSearchList::InitSortedList(ISldList* aSortedListPtr)
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
ESldError CSldSearchList::GetWordBySortedText(const UInt16* aText, UInt32* aResultFlag)
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
ESldError CSldSearchList::GetWordByTextExtended(const UInt16* aText, UInt32* aResultFlag, UInt32 aActionsOnFailFlag)
{
	if (!aResultFlag)
		return eMemoryNullPointer;

	*aResultFlag = 0;
	return GetWordByText(aText);
}

/** *********************************************************************
* Синхронизирует состояние вспомогательного сортированного списка с базовым
*
* @return код ошибки
************************************************************************/
ESldError CSldSearchList::SynchronizeWithASortedList()
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
ESldError CSldSearchList::GetWordBySortedIndex(Int32 aIndex)
{
	return GetWordByIndex(aIndex);
}

/** *********************************************************************
* Получает следующее сортированное слово
*
* @return код ошибки
************************************************************************/
ESldError CSldSearchList::GetNextSortedWord()
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
ESldError CSldSearchList::GetNextRealSortedWord(Int8* aResult)
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
ESldError CSldSearchList::GetCurrentSortedIndex(Int32* aIndex) const
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
ESldError CSldSearchList::QAGetNumberOfSortedQAItems(Int32 *aNumberOfQAItems)
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
ESldError CSldSearchList::QAGetSortedItemText(Int32 aIndex, const UInt16 **aQAText)
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
ESldError CSldSearchList::QAGetSortedItemIndex(Int32 aIndex, Int32 *aQAIndex)
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
ESldError CSldSearchList::JumpByQASortedIndex(Int32 aIndex)
{
	return eCommonWrongList;
}

/** *********************************************************************
* Проверяет наличие вспомогательного сортированного списка
*
* @return 1, если вспомогательный список имеется, иначе 0
************************************************************************/
Int8 CSldSearchList::HasSimpleSortedList()
{
	return false;
}

/** *********************************************************************
* Возвращает флаг, проверяющий на реальном ли слове установлен список, или на альтернативном заголовке
*
* @return SLD_SIMPLE_SORTED_NORMAL_WORD, если вспомогательный список имеется, иначе SLD_SIMPLE_SORTED_VIRTUAL_WORD
************************************************************************/
Int8 CSldSearchList::IsNormalWord()
{
	return SLD_SIMPLE_SORTED_NORMAL_WORD;
}

/** *********************************************************************
* Проверяет наличие иерархии у списка
*
* @return 1, если есть иерархия, иначе 0
************************************************************************/
Int8 CSldSearchList::HasHierarchy()
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
ESldError CSldSearchList::GoToCachedPoint(UInt32 aQAPointIndex, Int32 aNeedGlobalIndex)
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
ESldError CSldSearchList::IsCurrentLevelSorted(UInt32* aIsSorted)
{
	*aIsSorted = false;
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
ESldError CSldSearchList::GetWordByHistoryElement(CSldHistoryElement* aHistoryElement, ESldHistoryResult* aResultFlag, Int32* aListIndex)
{
	return eCommonWrongList;
}

/** *********************************************************************
* Возвращает количество слов из вспомогательного сортированного списка
*
* @return количество слов
************************************************************************/
UInt32 CSldSearchList::GetSortedWordsCount()
{
	return m_WordCount;
}

/** *********************************************************************
* Устанавливает значение флага синхронизации каталога с текущим индексом
*
* @param[in]	aFlag	- значение флага (0 или 1)
*
* @return eOK
************************************************************************/
ESldError CSldSearchList::SetCatalogSynchronizedFlag(UInt8 aFlag)
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
ESldError CSldSearchList::GetExactWordBySetOfVariants(TWordVariantsSet* aWordVariantsSet, UInt32* aResultFlag)
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
ESldError CSldSearchList::GetWordBySetOfVariants(TWordVariantsSet* aWordVariantsSet, UInt32 aWordSearchType, UInt32* aResultFlag, UInt32 aActionsOnFailFlag)
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
ESldError CSldSearchList::GetFullTextResultCount(const UInt16* aRequest, Int32* aCount)
{
	return eCommonWrongList;
}

/** *********************************************************************
* Возвращает индекс статьи, содержащей стилизованный текст варианта написания для текущего слова
*
* @param[in]	aVariantIndex	- Индекс варианта написания
*
* @return индекс статьи 
************************************************************************/
Int32 CSldSearchList::GetStylizedVariantArticleIndex(UInt32 aVariantIndex) const
{
	return m_List->pList->GetStylizedVariantArticleIndex(aVariantIndex);
}

/** *********************************************************************
* Возвращает индекс стиля для заданного варианта написания из eVariantLocalizationPreferences
*
* @param[in]	aVariantIndex	- номер варианта написания, для которого мы хотим получить стиль
*
* @return индекс стиля варианта написания
************************************************************************/
Int32 CSldSearchList::GetStyleVariantIndex(Int32 aVariantIndex)
{
	return m_List->pList->GetStyleVariantIndex(aVariantIndex);
}

/** *********************************************************************
* Переходит по пути, соответсвующему заданному глобальному индексу
*
* @param[in]	aGlobalIndex	- Глобальный индекс, к которому мы хотим перейти
*
* @return индекс статьи 
************************************************************************/
ESldError CSldSearchList::GoToByGlobalIndex(const Int32 aGlobalIndex)
{
	return GetWordByGlobalIndex(aGlobalIndex);
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
ESldError CSldSearchList::SortListByVariant(const Int32 aVariantIndex, const Int8 aCreateWordsBuffer)
{
	return eCommonWrongList;
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
ESldError CSldSearchList::GetRealIndexes(Int32 aWordIndex, Int32 aTrnslationIndex, Int32* aRealListIndex, Int32* aGlobalWordIndex)
{
	if (!aRealListIndex || !aGlobalWordIndex)
		return eMemoryNullPointer;

	ESldError error;
	Int32 GlobalIndex;

	error = LocalIndex2GlobalIndex(aWordIndex, &GlobalIndex);
	if (error != eOK)
		return error;

	TSldSearchWordStruct* pWord = GetWord(aWordIndex);
	if (!pWord)
		return eMemoryNullPointer;

	TSldSearchListStruct* pList = GetList(pWord->ListIndex);
	if (!pList)
		return eMemoryNullPointer;

	*aRealListIndex = pList->RealListIndex;
	*aGlobalWordIndex = pWord->WordIndex;

	return eOK;
}

/** *********************************************************************
* Производит сортировку поискового списка по шаблону поиска, содержащему Wildcard-символы
*
* @param[in]	aText - шаблон поиска
*
* @return код ошибки
************************************************************************/
ESldError CSldSearchList::SortWildCardResultList(const UInt16* aText)
{
	if (m_WordCount < 2)
		return eOK;

	ESldError error;

	TSldSearchListStruct* sourceList = GetList(m_WordVector[0].ListIndex);
	if (!sourceList)
		return eMemoryNullPointer;

	CSldCompare *CMP;
	error = sourceList->pList->GetCompare(&CMP);
	if (error != eOK)
		return error;

	SldU16String textMassStr;
	error = CMP->GetSearchPatternOfMass(aText, textMassStr, 1);
	if (error != eOK)
	{
		return error;
	}
	SldU16String textMassString(textMassStr);

	Int32 showVariantIndex = m_ListInfo->GetVariantIndexByType(eVariantShow);
	if (showVariantIndex == SLD_DEFAULT_VARIANT_INDEX)
		showVariantIndex = 0;

	CSldVector<UInt8> weights(m_WordCount);

	UInt16* currentWord = NULL;
	SldU16String currentMassStr;
	for (UInt32 wordIndex = 0; wordIndex < m_WordCount; wordIndex++)
	{
		error = GetWordByGlobalIndex(wordIndex);
		if (error != eOK)
			return error;
	
		error = GetCurrentWord(showVariantIndex, &currentWord);
		if (error != eOK)
			return error;

		error = CMP->GetStrOfMassWithDelimiters(currentWord, currentMassStr, 0, 1);
		if (error != eOK)
			return error;

		weights[wordIndex] = CMP->GetCompareLen(textMassString.c_str(), currentMassStr.c_str());
	}

	return eOK;
}

/** *********************************************************************
* Сортирует список, согласно заданному массиву индексов
*
* @param[in]	aWordIndexes	- соритрованный вектор с индексами
*
* @return код ошибки
************************************************************************/
ESldError CSldSearchList::SortByVector(const CSldVector<Int32> & aWordIndexes)
{
	if (aWordIndexes.size() != m_WordCount)
		return eCommonWrongNumberOfWords;

	for (UInt32 wordIndex = 0; wordIndex < m_WordCount; wordIndex++)
	{
		m_WordVector[wordIndex].WordIndex = aWordIndexes[wordIndex];
	}

	return eOK;
}

/** *********************************************************************
* Получает количество локализаций записей в списке слов
*
* @param[out]	aNumberOfLocalization	- количество локализаций
*
* @return код ошибки
************************************************************************/
ESldError CSldSearchList::GetNumberOfLocalization(Int32 & aNumberOfLocalization)
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
ESldError CSldSearchList::SetLocalizationByIndex(const Int32 aIndex)
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
ESldError CSldSearchList::GetLocalizationInfoByIndex(const Int32 aIndex, UInt32 & aLocalizationCode, SldU16String & aLocalizationName)
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
ESldError CSldSearchList::GetCurrentLocalizationIndex(Int32 & aIndex)
{
	return eOK;
}

/** *********************************************************************
* Обновляет заголовок поискового списка до состояния словарного списка, в котором находятся результаты
*
* @param[in] aSourceListInfo	- информация об исходном поисковом списке
*
* @return код ошибки
************************************************************************/
ESldError CSldSearchList::UpdateListHeader(const CSldListInfo* aSourceListInfo)
{
	m_ListInfo->m_Header.NumberOfVariants = aSourceListInfo->GetNumberOfVariants();
	m_ListInfo->m_Header.VariantsPropertyResourceIndex = aSourceListInfo->m_Header.VariantsPropertyResourceIndex;

	m_ListInfo->m_VariantPropertyTable = aSourceListInfo->m_VariantPropertyTable;
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
ESldError CSldSearchList::GetDictionaryIndexesByGlobalIndex(const Int32 aGlobalIndex, CSldVector<Int32> & aDictIndexes)
{
	TSldSearchWordStruct* tmpWord = GetWord(m_CurrentWordIndex);
	if (!tmpWord)
		return eCommonWrongIndex;

	TSldSearchListStruct* tmpList = GetList(tmpWord->ListIndex);
	if (!tmpList)
		return eCommonWrongIndex;

	ESldError error;
	Int32 realWordIndex = tmpWord->WordIndex;
	if (tmpList->pList->GetListInfo()->GetUsage() == eWordListType_FullTextAuxiliary)
	{
		Int32 realListIndex = SLD_DEFAULT_LIST_INDEX;
		Int32 realIndexesCount = 0;

		error = tmpList->pList->GetReferenceCount(tmpWord->WordIndex, &realIndexesCount);
		if (error != eOK)
			return error;

		if (realIndexesCount)
		{
			error = tmpList->pList->GetRealIndexes(tmpWord->WordIndex, 0, &realListIndex, &realWordIndex);
			if (error != eOK)
				return error;

			tmpList = GetList(realListIndex);
		}
	}

	return tmpList->pList->GetDictionaryIndexesByGlobalIndex(aGlobalIndex, aDictIndexes);
}
