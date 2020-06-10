#ifndef _SLD_LIST_H_
#define _SLD_LIST_H_

#include "ISldList.h"
#include "SldSearchList.h"
#include "SldSimpleSortedList.h"
/** 
	Реализует функциональность обычного списка слов (сортированный список, каталог).

	Т.е. поиск слов, различные способы навигации, получение структуры и прочее.
*/

class CSldSimpleSortedList;

struct TCurrentQAWordsShift
{
	TCurrentQAWordsShift(void) :
	IsInit(0),
	QAPointIndex(0),
	BeginGlobalIndex(SLD_DEFAULT_WORD_INDEX),
	LastGlobalIndex(SLD_DEFAULT_WORD_INDEX)
	{ }

	/// Инициализация
	ESldError Init(const Int32 aQAInterleaveSize, const UInt32 aMaxPrefixSize);
	/// Добавляет текущую позицию в кэш
	ESldError AddCashedPoint(const UInt16 aSize, const UInt16 * aCurrentWord, const UInt32 aCurrentShift, const Int32 aNextGlobalIndex);
	/// Устанавливает индекс текущей QA-точки
	ESldError SetNewQAPoint(const Int32 aQAPointIndex, const UInt16 * aCurrentWord, const UInt32 aCurrentShift, const Int32 aCurrentGlobalIndex);

	Int32 LastIndex() const { return LastGlobalIndex - BeginGlobalIndex; }

	bool IsCahed(const Int32 aGlobalIndex) const { return aGlobalIndex >= BeginGlobalIndex && aGlobalIndex < LastGlobalIndex; }

	UInt32		IsInit;

	Int32		QAPointIndex;

	Int32		BeginGlobalIndex;
	Int32		LastGlobalIndex;

	sld2::DynArray<UInt32>		Shift;
	sld2::DynArray<UInt32>		PrefixPos;
	sld2::DynArray<UInt16>		Prefix;
};

/// Элемент структуры бинарного поиска
struct TBinaryTreeElement
{
	TBinaryTreeElement(void) :
		WordIndex(SLD_DEFAULT_WORD_INDEX),
		CurrentWord(NULL),
		LeftLeafIndex(0),
		RightLeafIndex(0)
	{
	}

	/// Очистка
	void Clear()
	{
		if(CurrentWord)
		{
			sldMemFree(CurrentWord);
			CurrentWord = NULL;
		}
	}

	/// Индекс кэшированного слова
	Int32			WordIndex;
	/// Кэшированное слово
	UInt16*			CurrentWord;

	/// Индекс элемента, соответствующего низшей границе поискового диапазона
	UInt16			LeftLeafIndex;
	/// Индекс элемента, соответствующего  высшей границе поискового диапазона
	UInt16			RightLeafIndex;
};

/// Структура, содержащая кэшированные значения самых востребованных точек бинарного поиска
struct TQuickSearchPoints
{
	TQuickSearchPoints(void) :
	TextSearchTree(NULL),
	MaxSearchTreeDepth(0),
	SearchTreeElementCount(0)
	{
	}

	~TQuickSearchPoints();

	/// Инициализация
	ESldError Init(UInt32 aWordsCount, UInt32* aSearchTreePointsPtr);

	/// Получает границы бинарного поиска
	ESldError GetSearchBounds(const UInt16* aText, CSldCompare* aCompare, Int32* aHi, Int32* aLow, UInt32* aDepth, TBinaryTreeElement** aLastElement);

	/// Добавляет элемент быстрого поиска
	ESldError AddElement(UInt16* aCurrentWord, Int32 aCurrentIndex, CSldCompare* aCompare, UInt32* aDepth, TBinaryTreeElement** aLastElement);


	/// Массив, содержащий кэшированные значения самых востребованных точек бинарного поиска
	/// Нулевой элемент массива соответсвует корню дерева бинарного поиска (середина словарного списка)
	TBinaryTreeElement*		TextSearchTree;

	/// Максимальная глубина кэширования m_TextSearchTree
	UInt32					MaxSearchTreeDepth;

	/// Количество кэшированных элементов m_TextSearchTree
	UInt32					SearchTreeElementCount;

};

/// Класс, отвечающий за работу с альтернативными заголовками
struct AltNameManager
{
	AltNameManager();

	/// Инициализация
	ESldError	Init(Int8* aData, UInt32 aDataSize, UInt32 aMaxHeadWordSize);
	/// Проверяет, проинициализирован ли AltNameManager (в случае отсутсвия альтернативных заголовков так же возвращает 0)
	Int8		IsInit() const { return m_AltInfo.empty() ? 0 : 1; }
	/// Получает информацию об альтернативном заголовке по смещению
	ESldError	GetAltnameInfo(UInt32 aShift, UInt32* aRealWordCount, const UInt32** aRealWordIndexes) const;
	/// Возвращает смещение последнего закодированного слова
	UInt32		GetShiftForLastWord() const;

	/// Массив информации об альтернативных заголовках
	sld2::DynArray<TAlternativeHeadwordInfo>	m_AltInfo;
	/// Массив индексов в списке слов, на которые ссылаются альтернативные заголовки
	sld2::DynArray<UInt32>						m_Indexes;
	/// Последний найденный альтернативный заголовок
	sld2::DynArray<UInt16>						m_AlternativeWord;
	/// Смещение альтернативного заголовка
	/// Значение может использоваться как ключ. При переходе на реальное слово сюда устанавливается значение -1
	UInt32						m_LastShift;
};

class CSldList : public ISldList
{
	friend class CSldSearchList;
	friend class CSldMergedList;
	
public:

	/// Стандартный конструктор
	CSldList(void) :
		m_data(NULL),
		m_LayerAccess(NULL),
		m_CMP(NULL),
		m_ListInfo(NULL),
		m_CurrentIndex(SLD_DEFAULT_WORD_INDEX),
		m_PictureIndex(SLD_INDEX_PICTURE_NO),
		m_VideoIndex(SLD_INDEX_VIDEO_NO),
		m_SoundIndex(SLD_INDEX_SOUND_NO),
		m_SceneIndex(SLD_INDEX_SCENE_NO),
		m_BaseIndex(0),
		m_LocalizedBaseIndex(SLD_DEFAULT_LIST_INDEX),
		m_LocalizedBaseGlobalIndex(SLD_DEFAULT_LIST_INDEX),
		m_CurrentWord(NULL),
		m_SortedList(),
		m_CurrentDirectAccessResourceIndex(0),
		m_IsCatalogSynchronized(1),
		m_LastShift(-1),
		m_SortedWordIndexes(NULL),
		m_SortedVariantIndex(0),
		m_WordsBuff(NULL),
		m_MergedDictInfo(NULL)
	{
		sldMemZero(&m_Path, sizeof(m_Path));
	};

	/// Деструктор
	virtual ~CSldList(void);

	/// Конструктор копирования
	CSldList(const CSldList& aRef) = delete;
	CSldList& operator=(const CSldList& aRef) = delete;

public:

	//*** Реализация функций из интерфейсного класса ***//
	
	/// Инициализация списка слов
	ESldError Init(CSDCReadMy &aData, ISldLayerAccess* aLayerAccess, const CSldListInfo* aListInfo, CSldCompare* aCMP, UInt32 aHASH);

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
	virtual ESldError GetCurrentWord(Int32 aVariantIndex, UInt16** aWord);

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
	virtual ESldError GetWordByGlobalIndex(Int32 aGlobalIndex, const bool aInLocalization = true);
	/// Получаем слово по локальному номеру (номеру слова в пределах текущего списка слов и текущего уровня вложенности)
	virtual ESldError GetWordByIndex(Int32 aWordIndex);
	
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
	virtual ESldError GetPictureIndex(CSldVector<Int32> & aPictureIndexes);

	/// Возвращает номер видео текущего слова
	ESldError GetVideoIndex(Int32* aVideoIndex);

	/// Возвращает вектор озвучек текущего слова
	virtual ESldError GetSoundIndex(CSldVector<Int32> & aSoundIndexes);

	/// Возвращает номер 3d сцены текущего слова
	ESldError GetSceneIndex(Int32* aSceneIndex);

	/// Возвращает количество переводов у указанного слова из списка слов
	virtual ESldError GetTranslationCount(Int32 aGlobalIndex, Int32* aTranslationCount);

	/// Возвращает количество ссылок у указанного слова из списка слов
	ESldError GetReferenceCount(Int32 aGlobalIndex, Int32* aTranslationCount);

	/// Возвращает номер статьи по номеру слова и номеру перевода в обычном списке слов
	virtual ESldError GetTranslationIndex(Int32 aGlobalIndex, Int32 aTranslationIndex, Int32* aArticleIndex);
	
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
	virtual ESldError GetWordByText(const UInt16* aText);

	/// Ищет ближайшее слово, которое больше или равно заданному
	/// Работает в несортированных списках, для сортированных по сорткею списках ищет по Show-варианту(начиная с баз версии 112+)
	virtual ESldError GetWordBySortedText(const UInt16* aText, UInt32* aResultFlag);
	
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
	virtual ESldError GetNextSortedWord();

	/// Получает следующее реальное сортированное слово
	virtual ESldError GetNextRealSortedWord(Int8* aResult);

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

	/// Возвращает тип списка в иерархии наследования
	virtual EHereditaryListType GetHereditaryListType() const { return eHereditaryListTypeNormal; }

	/// Возвращает вектор слитых индексов словарей для заданного слова
	virtual ESldError GetDictionaryIndexesByGlobalIndex(const Int32 aGlobalIndex, CSldVector<Int32> & aDictIndexes);

	/// Устанавливает данные о слиянии словарей
	void SetMergeInfo(const TMergedDictInfo *aInfo) { m_MergedDictInfo = aInfo; }

protected:
	/// Производит переход словаря по указанной записи в таблице быстрого поиска
	ESldError GoToBySearchIndex(Int32 aSearchIndex);

private:

	/// Определяет границы, в которых будет производиться поиск (границы текущего уровня вложенности)
	ESldError GetSearchBounds(ESldSearchRange aSearchRange, Int32* aLowIndex, Int32* aHighIndex);

	/// Получает ближайшую точку быстрого доступа для заданного слова на текущем уровне
	ESldError GetClosestQAPointByText(const UInt16* aText, Int32* aQAPoint);

	/// Получает слово напрямую при наличии массива Индекс - Смещение
	ESldError GetWordByShift(UInt32 aWordIndex);

	/// Получает смещение для текущего индекса
	ESldError GetShiftByIndex(UInt32 aWordIndex, UInt32* aShift);

	/// Получает вес текущего слова при сравнении с переданными вариантами написания
	ESldError GetCurrentWordVariantsWeight(TWordVariantsSet* aWordVariantsSet, UInt32* aResultWeight, UInt32* aIsFullBinaryMatch, UInt32* aIsFullMassMatch);

	/// Ищет точное вхождение слова в Show-вариантах и альтернативных заголовках
	virtual ESldError FindBinaryMatch(const UInt16* aText, UInt32* aResult);

	/// Добавить слово в буффер
	ESldError InsetWordToBuffer(const Int32 aIndex);

	/// Сортирует записи в поисковом индексе
	ESldError QuickSortSearchVector(const Int32 aFirstIndex, const Int32 aLastIndex);

	/// Возвращает количество слов на текущем уровне включай подкаталоги
	ESldError GetNumberOfWordsAtCurrentLevel(const Int32 aGlobalIndex, UInt32 * aWordsCount);

	/// Получает количество локализаций записей в списке слов
	ESldError GetNumberOfLocalization(Int32 & aNumberOfLocalization);

	/// Устанавливает нужную локализацию записей в списке слов по номеру локализации
	ESldError SetLocalizationByIndex(const Int32 aIndex);

	/// Получает информацию о локализации записей в списке слов по номеру локализации
	ESldError GetLocalizationInfoByIndex(const Int32 aIndex, UInt32 & aLocalizationCode, SldU16String & aLocalizationName);

	/// Возвращает индекс текущей локализации
	ESldError GetCurrentLocalizationIndex(Int32 & aIndex);

	/// Возвращает базовый индекс без учета локализации
	Int32 GetBaseIndex() const { return m_LocalizedBaseGlobalIndex == SLD_DEFAULT_WORD_INDEX ? m_BaseIndex : m_BaseIndex - m_LocalizedBaseGlobalIndex; }

	/// Сравнивает все варианты элемента истории и текущего слова
	UInt32 CompareHistoryElementWithCurrentWord(CSldHistoryElement & aElement, bool aByMass = false);



	/// Возвращает указатель на заголовок данного списка слов
	virtual const CSldListInfo* GetListInfo() const { return m_ListInfo; }

	/// Возвращает указатель на класс, отвечающий за сравнение строк
	virtual CSldCompare* GetCMP() { return m_CMP; }

	/// Возвращает индекс текущего слова
	virtual Int32 GetCurrentWordIndex() const { return m_CurrentIndex; }

	/// Возвращает указатель на заданный вариант написания слова
	virtual UInt16* GetWord(const Int32 aIndex) { return m_CurrentWord[aIndex]; }

private:

	/// Класс, отвечающий за получение данных из контейнера
	CSDCReadMy*					m_data;

	/// Класс, отвечающий за получение данных
	sld2::UniquePtr<ISldInput>	m_Input;

	/// Класс, отвечающий за общение с оболочкой
	ISldLayerAccess*			m_LayerAccess;

	/// Класс, отвечающий за работу с иерархией
	sld2::UniquePtr<CSldCatalog> m_Catalog;

	/// Класс, отвечающий за хранение индексов быстрого поиска
	sld2::UniquePtr<CSldIndexes> m_Indexes;

	/// Класс, отвечающий за сравнение строк
	CSldCompare*				m_CMP;
	
	/// Путь к текущему положению в каталоге
	TCatalogPath				m_Path;
	

	/// Заголовок данного списка слов
	const CSldListInfo*			m_ListInfo;
	
	/// Номер текущего слова для таблицы быстрого поиска
	Int32						m_CurrentIndex;

	///	Номер картинки для текущего слова. Если картинки нет, тогда #SLD_INDEX_PICTURE_NO
	Int32						m_PictureIndex;

	///	Номер видео для текущего слова. Если видео нет, тогда #SLD_INDEX_VIDEO_NO
	Int32						m_VideoIndex;

	/// Номер озвучки для текущего слова. Если озвучки нет, тогда #SLD_INDEX_SOUND_NO
	Int32						m_SoundIndex;

	/// Номер сцены для текущего слова. Если озвучки нет, тогда #SLD_INDEX_SCENE_NO
	Int32						m_SceneIndex;

	/// Номер меша для текущего слова. Если озвучки нет, тогда #SLD_INDEX_SCENE_NO
	Int32						m_MeshIndex;

	/// Номер сцены для текущего слова. Если озвучки нет, тогда #SLD_INDEX_SCENE_NO
	Int32						m_MaterialIndex;

	/// Смещение начала для текущего уровня в иерархии
	Int32						m_BaseIndex;

	/// Смещение начала для начала локализованной части текущего списка
	Int32						m_LocalizedBaseIndex;


	/// Глобальный индекс папки с локализацией
	Int32						m_LocalizedBaseGlobalIndex;

	/// Массив строчек текущего слова (со всеми вариантами)
	UInt16**					m_CurrentWord;
	
	/// Текущее состояние списка
	TListState					m_CurrentState;

	/// Структура с сортированным по Show-варианту списком слов
	CSldSimpleSortedList		m_SortedList;

	/// @deprecated Структура, хранящая смещения для индексов слов в пределах текущей точки быстрого доступа
	TCurrentQAWordsShift		m_QAShift;

	/// Индекс текущего загруженного ресурса с массивом смещений для прямого доступа к слову
	UInt32						m_CurrentDirectAccessResourceIndex;

	/// Буфер, хранящий массив смещений для прямого доступа к слову
	sld2::DynArray<UInt32>		m_DirectAccessShifts;

	/// Структура с точками быстрого поиска по тексту
	TQuickSearchPoints			m_QuickSearchPoints;

	/// Флаг: определяет синхронизирован ли каталог с текущим индексом списка 
	/// (если слово было получено с помощью GetWordByGlobalIndex, то каталог не синхронизирован)
	UInt8						m_IsCatalogSynchronized;

	/// Класс, отвечающий за работу с альтернативными заголовками
	AltNameManager				m_AltNameManager;

	/// Аналог NumberOfWords, для проверки граничных условий в списках со скрытыми словами
	UInt32						m_LastShift;

	/// Массив индексов статей с стилизованными вариантами написания
	sld2::DynArray<Int32>		m_StylizedVariant;

	/// Массив с сортированными индексами слов
	UInt32*						m_SortedWordIndexes;

	/// Вариант, по которому отсортирован массив поиска
	Int32						m_SortedVariantIndex;

	/// Буфер, для сортировки слов
	UInt16**					m_WordsBuff;

	/// Данные о смреженых словарях
	const TMergedDictInfo*		m_MergedDictInfo;
};

#endif
