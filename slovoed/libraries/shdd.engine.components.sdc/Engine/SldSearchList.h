#ifndef _SLD_SEARCH_LIST_H_
#define _SLD_SEARCH_LIST_H_

#include "ISldList.h"
#include "SldList.h"
#include "SldSearchWordResult.h"

#define WORD_WEIGHT_IS_UNDEFINED	-2
#define MAX_NUMBER_OF_WORDS_IS_UNDEFINED	-1

class CSldList;
class CSldSimpleSortedList;

/// Структура для хранения списка слов, в котором производился поиск
struct TSldSearchListStruct
{
	/// Локальный индекс списка слов (TSldSearchWordStruct::ListIndex)
	UInt32		ListIndex;

	/// Реальный индекс списка слов, в котором производился поиск
	UInt32		RealListIndex;

	/// Указатель на реальный список слов
	CSldList*	pList;

	/// Конструктор
	TSldSearchListStruct()
		: ListIndex(0), RealListIndex(0), pList(NULL)
	{}
};

/*
	Класс реализует функциональность поискового списка слов,
	в котором хранятся результаты поиска (по шаблону/похожих слов)
*/
class CSldSearchList : public ISldList
{
	friend class CWildCardSearchImplementation;
	friend class CFullTextSearchImplementation;
	
public:
	
	/// Стандартный конструктор
	CSldSearchList(void) :
		m_LayerAccess(NULL),
		m_ListCount(0),
		m_WordCount(0),
		m_CurrentListIndex(0),
		m_CurrentWordIndex(0),
		m_List(NULL),
		m_Word(NULL),
		m_EnableSimpleRelevantSort(0)
	{};

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
	ESldError IsListHasScene(UInt32* aIsSound);
	
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

	/// Возвращает номер статьи с переводом, по номеру перевода
	ESldError GetTranslationIndex(Int32 aGlobalIndex, Int32 aTranslationIndex, Int32* aArticleIndex);

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

	/// Производит сортировку поискового списка по шаблону поиска, содержащему Wildcard-символы
	ESldError SortWildCardResultList(const UInt16* aText);
	
	
	/// Сохраняет текущее состояние списка слов
	ESldError SaveCurrentState(void);

	/// Восстанавливает ранее сохраненное состояние списка слов
	ESldError RestoreState(void);

	/// Получает следующее слово
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
	EHereditaryListType GetHereditaryListType() const { return eHereditaryListTypeSearch; }

	/// Возвращает вектор слитых индексов словарей для заданного слова
	ESldError GetDictionaryIndexesByGlobalIndex(const Int32 aGlobalIndex, CSldVector<Int32> & aDictIndexes);

public:

	/// Устанавливает максимальное количество списков
	ESldError SetMaximumLists(Int32 aMaxLists);

	/// Устанавливает максимальное количество слов
	ESldError SetMaximumWords(Int32 aMaxWords);

	/// Создает или находит уже имеющуюся внутреннюю запись о реальном списке слов
	ESldError MakeList(CSldList* aRealList, Int32 aRealListIndex, TSldSearchListStruct** aList);

	/// Добавляет слово
	ESldError AddWord(const TSldSearchWordStruct &aWord);

	/// Обновляет заголовок поискового списка до состояния словарного списка, в котором находятся результаты
	ESldError UpdateListHeader(const CSldListInfo* aSourceListInfo);

	struct WordGetter;

protected:
	/// Производит переход по таблице быстрого поиска
	ESldError GoToBySearchIndex(Int32 aSearchIndex);

private:

	/// Добавляет список слов, в котором производился поиск
	ESldError AddList(const TSldSearchListStruct &aList);
	
	/// Добавляет слово, сортируя по расстоянию редактирования
	ESldError AddFuzzyWord(const TSldSearchWordStruct &aWord);
	
	/// Добавляет слово, не допуская дублирования
	ESldError AddSpellingWord(const TSldSearchWordStruct &aWord, Int8* aIsDouble);
	
	/// Удаляет последнее слово в списке
	void RemoveLastWord(void);

	/// Получает количество списков, в которых производился поиск
	Int32 ListCount(void) const { return m_ListCount; };

	/// Получает количество слов
	Int32 WordCount(void) const { return m_WordCount; };
	
	/// Получает список по индексу
	TSldSearchListStruct* GetList(Int32 aIndex) const;
	
	/// Получает слово по индексу
	TSldSearchWordStruct* GetWord(Int32 aIndex) const;
	
	/// Возвращает массив уникальных кодов использования списка слов
	ESldError GetAllUsages(UInt32** aUsages, Int32* aCount) const;
	
	/// Производит полнотекстовый поиск одного слова
	ESldError DoWordFullTextSearch(const UInt16* aText, Int32 aMaximumWords, ISldList** aList, Int32 aListCount, Int32 aRealListIndex, CSldSearchWordResult* aResultData);
	
	/// Производит поиск по шаблону одного слова/фразы, в пределах текущего уровня вложенности
	ESldError DoWordWildCardSearch(const UInt16* aText, ISldList* aList, Int32 aRealListIndex, CSldSimpleSearchWordResult* aResultData);
	ESldError DoExpressionWildCardSearch(TExpressionBox* aExpressionBox, ISldList* aList, Int32 aMaximumWords, CSldSimpleSearchWordResult* aResultData);
	
	/// Производит добавление результатов полнотекстового поиска одного слова
	ESldError AddWordFullTextSearchResult(CSldSearchWordResult* aResultData, ISldList** aList, Int32 aListCount, Int32 aMaximumWords);
	
	/// Производит добавление результатов поиска по шаблону
	ESldError AddWildCardSearchSearchResults(CSldSimpleSearchWordResult* aResultData, ISldList* aList, Int32 aRealListIndex, Int32 aMaximumWords);

	/***********************************************************************************/
	/// Функции сравнения слов для различных видов поиска FuzzySearch
	
	/// См. #EFuzzySearchMode::eFuzzy_WithoutOptimization
	ESldError FuzzyCompareWithoutOptimization(CSldList* aList, const UInt16* aWord, const UInt16* aText, Int32 aWordLen, Int32 aPatternLen, sld2::Span<UInt32> aSymbolsCheckTable, Int32 (*aFuzzyBuffer)[ARRAY_DIM], Int32 aMaxEditDistance, UInt32* aFlag, Int32* aEditDistance);
	
	/// См. #EFuzzySearchMode::eFuzzy_CompareDirect
	ESldError FuzzyCompareDirect(CSldList* aList, const UInt16* aWord, const UInt16* aText, Int32 aWordLen, Int32 aPatternLen, sld2::Span<UInt32> aSymbolsCheckTable, Int32 (*aFuzzyBuffer)[ARRAY_DIM], Int32 aMaxEditDistance, UInt32* aFlag, Int32* aEditDistance);
	
	/// См. #EFuzzySearchMode::eFuzzy_CompareSortTable
	ESldError FuzzyCompareSortTable(CSldList* aList, const UInt16* aWord, const UInt16* aText, Int32 aWordLen, Int32 aPatternLen, sld2::Span<UInt32> aSymbolsCheckTable, Int32 (*aFuzzyBuffer)[ARRAY_DIM], Int32 aMaxEditDistance, UInt32* aFlag, Int32* aEditDistance);
	/***********************************************************************************/
	
private:

	/// Заголовок данного списка слов
	sld2::UniquePtr<CSldListInfo>	m_ListInfo;

	/// Класс, отвечающий за общение с оболочкой
	ISldLayerAccess*			m_LayerAccess;

	/// Массив списков слов, на который ссылается данный поисковый список
	sld2::DynArray<TSldSearchListStruct>	m_ListVector;

	/// Массив с найденными словами
	sld2::DynArray<TSldSearchWordStruct>	m_WordVector;

	/// Количество списков
	UInt32						m_ListCount;

	/// Количество слов
	UInt32						m_WordCount;

	/// Индекс текущего списка
	Int32						m_CurrentListIndex;

	/// Индекс текущего слова
	Int32						m_CurrentWordIndex;

	/// Текущий список
	TSldSearchListStruct*		m_List;

	/// Текущее слово
	TSldSearchWordStruct*		m_Word;

	/// Текущее состояние списка
	TListState					m_CurrentState;

	/// Флаг, указывающий, можно ли к данному поисковому списку применять упрощенный ускоренный
	/// метод сортировки по релевантности шаблону поиска
	UInt32						m_EnableSimpleRelevantSort;
};

/// Тип указателя на функцию сравнения слов при поиске FuzzySearch
typedef ESldError (CSldSearchList::* TFuncFuzzyCmp)(CSldList*, const UInt16*, const UInt16*, Int32, Int32, sld2::Span<UInt32>, Int32 (*)[ARRAY_DIM], Int32, UInt32*, Int32*);

#endif
