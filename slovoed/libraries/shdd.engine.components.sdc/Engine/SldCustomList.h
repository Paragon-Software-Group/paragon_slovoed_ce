#ifndef _SLD_CUSTOM_LIST_H_
#define _SLD_CUSTOM_LIST_H_

#include "SldCustomListControl.h"

/// Структура для хранения информации о слове
typedef struct TSldCustomListWordInfo
{	
	/// Реальный индекс списка слов, в котором производился поиск
	Int32						RealListIndex;

	/// Индекс слова в базовом списке
	Int32						RealWordIndex;

	/// Количество вариантов написания
	Int32						NumberOfVariants;

	/// Указатель на реальный список слов
	ISldList*					RealListPtr;
	
	/// Массив указателей на слова
	UInt16**					CustomWords;

	/// Массив индексов слов, связанных с данной записью
	CSldVector<TSldWordIndexes>	SubIndexes;

	/// Флаг, описывающий состояние связанных слов (показаны слова в списке или нет)
	bool						UncoverFlag;

	/// Очистка
	void Clear()
	{
		RealListIndex = 0;
		RealWordIndex = SLD_DEFAULT_WORD_INDEX;
		NumberOfVariants = 0;
		RealListPtr = NULL;
		CustomWords = NULL;
		SubIndexes.clear();
		UncoverFlag = false;
	}

	/// Освобождение памяти
	void Close();

	/// Конструктор
	TSldCustomListWordInfo(void) 
	{
		Clear();
	};

	/// Деструктор
	~TSldCustomListWordInfo(void)
	{
		Close();
		Clear();
	};

	/// Возвращает указатель на запрашиваемый вариант написания
	ESldError GetCurrentWordPtr(Int32 aVariantIndex, UInt16** aWord, UInt16* aDefaultWord);

}TSldCustomListWordInfo;

struct SortTreeLeaf
{
	Int32 WordIndex;
	Int32 LeftLeafIndex;
	Int32 RightLeafIndex;

	SortTreeLeaf() :
		WordIndex(SLD_DEFAULT_WORD_INDEX),
		LeftLeafIndex(SLD_DEFAULT_WORD_INDEX),
		RightLeafIndex(SLD_DEFAULT_WORD_INDEX)
	{}
};

/// Структура, содержащая указатели на словарные списки слов
struct TSldRealLists
{
	/// Списки слов
	ISldList**					WordLists;

	/// Количество списков слов
	Int32						NumberOfLists;

	/// Обнуление
	void Clear() { WordLists = NULL; NumberOfLists = 0; }

	/// Очистка памяти
	void Close()
	{
		if (WordLists)
		{
			sldMemFree(WordLists);
		}
	}

	/// Конструктор
	TSldRealLists() { Clear(); }

	/// Деструктор
	~TSldRealLists() { Close(); Clear(); }

	/// Инициализация
	ESldError Init(ISldList** aWordLists, const UInt32 aNumberOfLists);

	/// Возвращает указатель на словарный список слов
	ESldError GetListPtr(const Int32 aListIndex, ISldList** aListPtr);
};

class CSldCustomList : public ISldList
{

	friend class CSldList;
	friend class CSldSearchList;

public:

	//Стандартный конструктор
	CSldCustomList(void) :
	m_ListInfo(NULL),
	m_LayerAccess(NULL),
	m_WordVector(NULL),
	m_CurrentIndex(SLD_DEFAULT_WORD_INDEX),
	m_NumbersOfWords(0),
	m_MaxNumbersOfWords(0),
	m_DefaultChar(0),
	m_SortedWordIndexes(NULL),
	m_SortedVariantIndex(0),
	m_CMP(NULL),
	m_TmpWordBuff(NULL),
	m_TmpWordSize(0),
	m_SearchSourceListIndex(SLD_DEFAULT_LIST_INDEX)
	{
	};

	/// Деструктор
	~CSldCustomList();

	/// Конструктор копирования
	CSldCustomList(const CSldCustomList& aRef) = delete;
	CSldCustomList& operator=(const CSldCustomList& aRef) = delete;

public:

	//*** Реализация функций из интерфейсного класса ***//
	
	/// Инициализация списка слов
	ESldError Init(CSDCReadMy &aData, ISldLayerAccess* aLayerAccess, const CSldListInfo* aListInfo, CSldCompare* aCMP, UInt32 aHASH)
	{
		return Init(&aData, aLayerAccess, aListInfo, aCMP, aHASH);
	}

	ESldError Init(CSDCReadMy* aData, ISldLayerAccess* aLayerAccess, const CSldListInfo* aListInfo, CSldCompare* aCMP, UInt32 aHASH);

	/// Проверяем, проинициализирован ли список слов
	bool isInit(void);

	/// Устанавливаем HASH для декодирования данного списка слов.
	ESldError SetHASH(UInt32 aHASH);
	
	/// Возвращает полное количество слов в списке слов
	ESldError GetTotalWordCount(Int32* aCount);

	/// Возвращает количество слов у списка слов на текущем уровне вложенности
	ESldError GetNumberOfWords(Int32* aNumberOfWords) const;
	
	/// Возвращает количество вариантов написания для списка слов
	ESldError GetNumberOfVariants(UInt32* aNumberOfVariants);

	/// Возвращает указатель на последнее найденное слово
	ESldError GetCurrentWord(Int32 aVariantIndex, UInt16** aWord);

	/// Возвращает указатель на eVariantLabel последнего найденного слова
	ESldError GetCurrentWordLabel(UInt16** aLabel);

	/// Получает локальный номер текущего слова (номер слова в пределах текущего списка слов и текущего уровня вложенности)
	ESldError GetCurrentIndex(Int32* aIndex) const;
	
	/// Получает глобальный номер текущего слова (номер слова в пределах всего списка слов)
	ESldError GetCurrentGlobalIndex(Int32* aIndex) const;
	
	/// Конвертирует локальный номер слова в пределах текущего списка слов и уровня вложенности
	/// в глобальный номер слова в пределах текущего списка слов
	ESldError LocalIndex2GlobalIndex(Int32 aLocalIndex, Int32* aGlobalIndex);
	
	/// Получает номер списка слов по локальному номеру слова
	ESldError GetRealListIndex(Int32 aLocalWordIndex, Int32* aListIndex);

	/// Получает глобальный номер слова по локальному номеру слова
	ESldError GetRealGlobalIndex(Int32 aLocalWordIndex, Int32* aGlobalWordIndex);

	/// Получает слово по его глобальному номеру
	ESldError GetWordByGlobalIndex(Int32 aGlobalIndex, const bool aInLocalization = true);
	/// Получаем слово по локальному номеру (номеру слова в пределах текущего списка слов и текущего уровня вложенности)
	ESldError GetWordByIndex(Int32 aWordIndex);
	
	/// Устанавливаем базу для списка слов
	ESldError SetBase(Int32 aBase);
	
	/// Возвращает указатель на класс, хранящий информацию о свойствах списка слов
	ESldError GetWordListInfo(const CSldListInfo** aListInfo, Int32 aDictIndex = SLD_DEFAULT_DICTIONARY_INDEX) const;
	
	/// Возвращает количество кодов использования списка слов
	ESldError GetUsageCount(Int32* aCount) const;
	
	/// Возвращает код использования списка слов по индексу кода
	ESldError GetListUsage(Int32 aIndex, UInt32* aUsage) const;
	
	/// Возвращает флаг того, сортированный или нет данный список слов
	ESldError IsListSorted(UInt32* aIsSorted);
	
	/// Возвращает флаг того, сопоставлены или нет некоторым словам из списка слов картинки
	ESldError IsListHasPicture(UInt32* aIsPicture);
	
	/// Возвращает флаг того, сопоставлено или нет некоторым словам из списка слов видео
	ESldError IsListHasVideo(UInt32* aIsVideo);

	/// Возвращает флаг того, сопоставлена или нет некоторым словам из списка слов озвучка
	ESldError IsListHasSound(UInt32* aIsSound);

	/// Возвращает флаг того, сопоставлена или нет некоторым словам из списка слов 3d сцена
	ESldError IsListHasScene(UInt32* aIsScene);
	
	/// Возвращает флаг того, имеет или нет указанное слово поддерево иерархии
	ESldError isWordHasHierarchy(Int32 aIndex, UInt32* aIsHierarchy, EHierarchyLevelType* aLevelType);
	
	/// Возвращает вектор индексов изображений для текущего слова
	ESldError GetPictureIndex(CSldVector<Int32> & aPictureIndexes);

	/// Возвращает номер видео текущего слова
	ESldError GetVideoIndex(Int32* aVideoIndex);

	/// Возвращает вектор индексов озвучек текущего слова
	ESldError GetSoundIndex(CSldVector<Int32> & aSoundIndexes);

	/// Возвращает номер 3d сцены текущего слова
	ESldError GetSceneIndex(Int32* aSceneIndex);

	/// Возвращает количество переводов у указанного слова из списка слов
	ESldError GetTranslationCount(Int32 aGlobalIndex, Int32* aTranslationCount);

	/// Возвращает количество ссылок у указанного слова из списка слов
	ESldError GetReferenceCount(Int32 aGlobalIndex, Int32* aTranslationCount);

	/// Возвращает номер статьи по номеру слова и номеру перевода в обычном списке слов
	ESldError GetTranslationIndex(Int32 aGlobalIndex, Int32 aTranslationIndex, Int32* aArticleIndex);
	
	/// Возвращает данные перевода по номеру слова и номеру перевода в списке слов полнотекстового поиска
	ESldError GetFullTextTranslationData(Int32 aGlobalIndex, Int32 aWordTranslationIndex, Int32* aListIndex, Int32* aListEntryIndex, Int32* aTranslationIndex, Int32* aShiftIndex);

	/// Возвращает путь к текущему положению в каталоге
	ESldError GetCurrentPath(TCatalogPath* aPath);

	/// Возвращает путь в каталоге к элементу с глобальным номером (т.е. "прямой" номер слова без учета иерархии)
	ESldError GetPathByGlobalIndex(Int32 aIndex, TCatalogPath* aPath);
	
	/// Переходит по указанному пути
	ESldError GoToByPath(const TCatalogPath* aPath, ESldNavigationTypeEnum aNavigationType);
	
	/// Поднимаемся в каталоге на уровень выше текущего или остаемся на текущем, если выше некуда
	ESldError GoToLevelUp(void);
	
	/// Возвращает список названий родительских категорий, разделенных строкой-разделителем, по глобальному номеру слова
	ESldError GetParentWordNamesByGlobalIndex(Int32 aGlobalWordIndex, UInt16** aText, const UInt16* aSeparatorText, Int32 aMaxLevelCount = -1);
	

	/// Ищем ближайшее слово, которое больше или равно заданному
	ESldError GetWordByText(const UInt16* aText);

	/// Ищет ближайшее слово, которое больше или равно заданному
	/// Работает в несортированных списках, для сортированных по сорткею списках ищет по Show-варианту(начиная с баз версии 112+)
	ESldError GetWordBySortedText(const UInt16* aText, UInt32* aResultFlag);
	
	/// Ищет максимально похожее слово с помощью GetWordBySortedText()
	ESldError GetWordByTextExtended(const UInt16* aText, UInt32* aResultFlag, UInt32 aActionsOnFailFlag);

	/// Ищет наиболее подходящее слово по переданным вариантам написания
	ESldError GetWordBySetOfVariants(TWordVariantsSet* aWordVariantsSet, UInt32 aWordSearchType, UInt32* aResultFlag, UInt32 aActionsOnFailFlag = 1);

	/// Ищет слово, бинарно совпадающее по переданным вариантам написания
	ESldError GetExactWordBySetOfVariants(TWordVariantsSet* aWordVariantsSet, UInt32* aResultFlag);

	/// Ищем наиболее похожее слово, которое равно заданному
	ESldError GetMostSimilarWordByText(const UInt16* aText, UInt32* aResultFlag);
	
	/// Производит поиск слов по шаблону, в пределах текущего уровня вложенности
	ESldError DoWildCardSearch(TExpressionBox* aExpressionBox, Int32 aMaximumWords, ISldList* aList, Int32 aRealListIndex, ISldList* aRealList = NULL);

	/// Производит поиск похожих слов, в пределах текущего уровня вложенности
	ESldError DoFuzzySearch(const UInt16* aText, Int32 aMaximumWords, Int32 aMaximumDifference, ISldList* aList, Int32 aRealListIndex, EFuzzySearchMode aSearchMode);
	
	/// Производит поиск анаграмм, в пределах текущего уровня вложенности
	ESldError DoAnagramSearch(const UInt16* aText, Int32 aTextLen, ISldList* aList, Int32 aRealListIndex);
	
	/// Производит поиск слов с учетом возможных опечаток в пределах текущего уровня вложенности
	ESldError DoSpellingSearch(const UInt16* aText, ISldList* aList, Int32 aRealListIndex);
	
	/// Производит полнотекстовый поиск
	ESldError DoFullTextSearch(const UInt16* aText, Int32 aMaximumWords, ISldList** aList, Int32 aListCount, Int32 aRealListIndex);
	
	/// Производит сортировку поискового списка слов по релевантности шаблону поиска
	ESldError SortListRelevant(const UInt16* aText, const CSldVector<TSldMorphologyWordStruct>& aMorphologyForms, ESldFTSSortingTypeEnum aMode);
	
	/// Сохраняет текущее состояние списка слов
	ESldError SaveCurrentState(void);
	
	/// Восстанавливает ранее сохраненное состояние списка слов
	ESldError RestoreState(void);

	/// Метод декодирует следующее слово в списке слов
	ESldError GetNextWord();

	/// Возвращает таблицу сравнения которая используется в данном списке слов
	ESldError GetCompare(CSldCompare **aCompare);

	/// Возвращает количество записей в таблице быстрого доступа
	ESldError QAGetNumberOfQAItems(Int32 *aNumberOfQAItems);

	/// Возвращает текст для записи в таблице быстрого доступа.
	ESldError QAGetItemText(Int32 aIndex, const UInt16 **aQAText);

	/// Возвращает номер для записи в таблице быстрого доступа.
	ESldError QAGetItemIndex(Int32 aIndex, Int32 *aQAIndex);

	/// Производит переход по таблице быстрого доступа по номеру записи.
	ESldError JumpByQAIndex(Int32 aIndex);
	
	/// Определяет глобальные номера границ текущего уровня вложенности (включая все подуровни)
	/// Получается диапазон глобальных индексов слов вида [aLowGlobalIndex, aHighGlobalIndex) - левая граница включается, правая - нет
	ESldError GetHierarchyLevelBounds(Int32* aLowGlobalIndex, Int32* aHighGlobalIndex);

	/// Получает вектор индексов озвучек по введенному тексту
	ESldError GetSoundIndexByText(const UInt16 *aText, CSldVector<Int32> & aSoundIndexes);

	/// Устанавливает нужную локализацию записей по переданному языковому коду
	ESldError SetLocalization(ESldLanguage aLocalizationLangCode);
	/// Устанавливает нужную локализацию записей по переданному языковому коду и набору настроек
	ESldError SetLocalization(ESldLanguage aLocalizationLangCode, const UInt16* aLocalizationDetails);

	/// Инициализирует вспомогательный сортированный список
	ESldError InitSortedList(ISldList* aSortedListPtr);

	/// Синхронизирует состояние вспомогательного сортированного списка с базовым
	ESldError SynchronizeWithASortedList();

	/// Получает слово по его номеру в сортированном списке
	ESldError GetWordBySortedIndex(Int32 aIndex);

	/// Получает следующее сортированное слово
	ESldError GetNextSortedWord();

	/// Получает следующее реальное сортированное слово
	ESldError GetNextRealSortedWord(Int8* aResult);

	/// Возвращает текущий индекс сортированного списка
	ESldError GetCurrentSortedIndex(Int32* aIndex) const;

	/// Возвращает количество записей в таблице быстрого доступа для сортированного списка
	ESldError QAGetNumberOfSortedQAItems(Int32 *aNumberOfQAItems);

	/// Возвращает текст для записи в таблице быстрого доступа для сортированного списка
	ESldError QAGetSortedItemText(Int32 aIndex, const UInt16 **aQAText);

	/// По номеру точки входа в сортированном списке возвращает номер слова, соответствующий этой точке
	ESldError QAGetSortedItemIndex(Int32 aIndex, Int32 *aQAIndex);

	/// Производит переход по таблице быстрого доступа по номеру записи в сортированном списке
	ESldError JumpByQASortedIndex(Int32 aIndex);

	/// Возвращает флаг, проверяющий на реальном ли слове установлен список, или на альтернативном заголовке
	Int8 IsNormalWord();

	/// Проверяет наличие вспомогательного сортированного списка
	Int8 HasSimpleSortedList();

	/// Проверяет наличие иерархии у списка
	Int8 HasHierarchy();

	/// Переходит к ближайшей закешированной точке
	ESldError GoToCachedPoint(UInt32 aQAPointIndex, Int32 aNeedGlobalIndex);

	/// Проверяет сортированность текущего уровня иерархии
	ESldError IsCurrentLevelSorted(UInt32* aIsSorted);

	/// Получает слово по элементу истории
	ESldError GetWordByHistoryElement(CSldHistoryElement* aHistoryElement, ESldHistoryResult* aResultFlag, Int32* aGlobalIndex);

	/// Возвращает количество слов из вспомогательного сортированного списка
	UInt32 GetSortedWordsCount();

	/// Устанавливает значение флага синхронизации каталога с текущим индексом
	ESldError SetCatalogSynchronizedFlag(UInt8 aFlag);

	/// Возвращает количество результатов полнотекстового поиска
	ESldError GetFullTextResultCount(const UInt16* aRequest, Int32* aCount);

	/// Возвращает индекс статьи, содержащей стилизованный текст варианта написания для текущего слова
	Int32 GetStylizedVariantArticleIndex(UInt32 aVariantIndex) const;

	/// Возвращает индекс стиля для заданного варианта написания из eVariantLocalizationPreferences
	Int32 GetStyleVariantIndex(Int32 aVariantIndex);

	/// Переходит по пути, соответсвующему заданному глобальному индексу
	ESldError GoToByGlobalIndex(const Int32 aGlobalIndex);

	/// Индексирует все слова, содержащиеся в списке в соответсвии с заданным вариантом написания
	ESldError SortListByVariant(const Int32 aVariantIndex, const Int8 aCreateWordsBuffer = 0);

	/// Получает реальные индексы списка и записи в реальном списке в по глобальному номеру слова
	ESldError GetRealIndexes(Int32 aWordIndex, Int32 aTrnslationIndex, Int32* aRealListIndex, Int32* aGlobalWordIndex);

	/// Сортирует список, согласно заданному массиву индексов
	ESldError SortByVector(const CSldVector<Int32> & aWordIndexes);

	/// Получает количество локализаций записей в списке слов
	ESldError GetNumberOfLocalization(Int32 & aNumberOfLocalization);

	/// Устанавливает нужную локализацию записей в списке слов по номеру локализации
	ESldError SetLocalizationByIndex(const Int32 aIndex);

	/// Получает информацию о локализации записей в списке слов по номеру локализации
	ESldError GetLocalizationInfoByIndex(const Int32 aIndex, UInt32 & aLocalizationCode, SldU16String & aLocalizationName);

	/// Возвращает индекс текущей локализации
	ESldError GetCurrentLocalizationIndex(Int32 & aIndex);

	/// Возвращает тип списка в иерархии наследования
	EHereditaryListType GetHereditaryListType() const { return eHereditaryListTypeCustom; }

	/// Возвращает вектор слитых индексов словарей для заданного слова
	ESldError GetDictionaryIndexesByGlobalIndex(const Int32 aGlobalIndex, CSldVector<Int32> & aDictIndexes);

protected:
	/// Производит переход по таблице быстрого поиска
	ESldError GoToBySearchIndex(Int32 aSearchIndex);

public:

	/// Заполняет массив словарных списков слов
	ESldError SetRealLists(ISldList** aWordLists, const UInt32 aNumberOfLists);

	/// Добавляет слово в список на основе уже существующей записи в словаре
	ESldError AddWord(const Int32 aRealListIndex, const Int32 aRealWordIndex, const UInt32 aUncoverHierarchy);
	/// Добавляет слово в список
	ESldError AddWord(const Int32 aRealListIndex = SLD_DEFAULT_LIST_INDEX);

	/// Привязывает слово к записи из списка
	ESldError AddSubWord(const Int32 aWordIndex, const Int32 aSubWordIndex, const Int32 aSubWordListIndex);

	/// Добавляет диапазон слов в список
	ESldError InsertWords(const Int32 aRealListIndex, const Int32 aBeginRealWordIndex, const Int32 aEndRealWordIndex, Int32 aInsertAfterIndex = SLD_DEFAULT_WORD_INDEX);

	/// Удаляет записи из списка
	ESldError EraseWords(const Int32 aBeginIndex, const Int32 aEndIndex);

	/// Устанавливает вариант написания для уже добавленного слова
	ESldError SetCurrentWord(const Int32 aWordIndex,const  UInt16* aText, const Int32 aVariantIndex, const UInt32 aResetSearchVector = 1);

	/// Устанавливает индекс базового списка, в котором нужно проводить поиск
	void SetSearchSourceListIndex(const Int32 aListIndex) { m_SearchSourceListIndex = aListIndex; }

	/// Возвращает количество связанных со словом записей
	ESldError GetSubWordsCount(const Int32 aWordIndex, UInt32* aSubWordsCount);

	/// Возвращает массив связанных со словом записей
	ESldError GetSubWords(const Int32 aWordIndex, const CSldVector<TSldWordIndexes> ** aSubWordsIndexes, ESubwordsState* aUncoverFlag);

	/// Проверяет состояние слова и связанных с ним слов
	ESldError CheckSubwordsState(const Int32 aWordIndex, ESubwordsState* aWordState);

	/// Меняет состояние слова и связанных с ним слов
	ESldError ChengeSubwordsState(const Int32 aWordIndex, ESubwordsState* aWordState);

	/// Объединяет все записи для вспомогательных листов, относящиеся к одному заголовку
	ESldError GroupWordsByHeadwords();

    ESldError GetUncoveredWordsCount(const Int32 aWordIndex, UInt32* aUncoveredWordsCount);

private:

	/// Выделяет память, если это необходимо
	ESldError CheckAllocateMemory();

	/// Сравнивает два слова
	ESldError CompareWords(const UInt16* aFirstWord, const UInt16* aSecondWord, Int32* aResult);

	/// Сортирует записи в поисковом индексе
	ESldError QuickSortSearchVector(const Int32 aFirstIndex, const Int32 aLastIndex, const Int8 aSortByIndex);

	/// Устанавливает вариант написания для уже добавленного слова
	ESldError SetCurrentWord(TSldCustomListWordInfo* aWordInfoPtr, const  UInt16* aText, const Int32 aVariantIndex, const UInt32 aResetSearchVector);

	/// Ищет максимально похожее слово с помощью GetWordBySortedText() в базовом списке слов
	ESldError GetWordByTextInRealList(const UInt16* aText, UInt32* aResultFlag, UInt32 aActionsOnFailFlag);

	/// Получает индекс варианта написания в базовом списке слов
	Int32 GetRealVariantIndex(Int32 aVariantIndex) const;
	
private:

	/// Заголовок данного списка слов
	CSldListInfo*				m_ListInfo;

	/// Класс, отвечающий за общение с оболочкой
	ISldLayerAccess*			m_LayerAccess;

	/// Массив со словами
	TSldCustomListWordInfo*		m_WordVector;

	/// Индекс текущего слова
	Int32						m_CurrentIndex;

	/// Количество слов в списке
	Int32						m_NumbersOfWords;

	/// Количество слов в списке
	Int32						m_MaxNumbersOfWords;

	/// Массив всех возможных словарных списков слов (CSldList)
	TSldRealLists				m_RealListVector;

	/// Слово по умолчанию
	UInt16						m_DefaultChar;

	/// Массив с сортированными индексами слов
	Int32*						m_SortedWordIndexes;

	/// Вариант, по которому отсортирован массив поиска
	Int32						m_SortedVariantIndex;

	/// Таблица сортировки
	const CSldCompare*			m_CMP;

	/// Временный буфер, для сортировки слов
	UInt16**					m_TmpWordBuff;

	/// Размер временного буфера
	UInt32						m_TmpWordSize;

	/// Индекс базового списка, в котором нужно проводить поиск
	Int32						m_SearchSourceListIndex;
};

#endif
