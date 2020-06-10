#ifndef _I_SLD_LIST_H_
#define _I_SLD_LIST_H_

#include "SldError.h"
#include "SldCatalog.h"
#include "ISldInput.h"
#include "ISldLayerAccess.h"
#include "SldIndexes.h"
#include "SldCompare.h"
#include "SldListInfo.h"
#include "SldHistory.h"


/// Структура, которая описывает состояние списка слов
/// Используется для сохранения и восстановления состояния при переключении списков
struct TListState
{
	/// Путь к текущему месту в каталоге
	TCatalogPath m_Path;

	TListState() = default;
	~TListState() = default;
};

/// Структура для хранения набора логических выражений
typedef struct TExpressionBox
{
	/// Массив операндов
	UInt16**	Operands;
	/// Массив операторов
	UInt8*		Operators;
	/// Количество операций
	UInt8		Count;
	/// Флаг того, был ли преобразован запрос в строку масс
	UInt8		IsQueryWasPrepared;

	/// Конструктор
	TExpressionBox(void) :
		Operands(NULL),
		Operators(NULL),
		Count(0),
		IsQueryWasPrepared(0)
	{
	};
	
	/// Деструктор
	~TExpressionBox(void)
	{
		Clear();
	};

	void Clear()
	{
		if(Operands)
		{
			for(UInt32 currentOperandIndex = 0; currentOperandIndex < Count ; currentOperandIndex++)
			{
				if(Operands[currentOperandIndex])
				{
					sldMemFree(Operands[currentOperandIndex]);
					Operands[currentOperandIndex] = NULL;
				}
			}
			sldMemFree(Operands);
			Operands = NULL;
		}

		if (Operators)
		{
			sldMemFree(Operators);
			Operators = NULL;
		}

		Count = 0;
	}

	UInt8 AddExpression(const UInt16* aOperand, UInt8 aOperation)
	{
		if (IsQueryWasPrepared || !aOperand)
			return 0;

		Count++;
		UInt16** tmpOperands = NULL;
		tmpOperands = (UInt16**)sldMemNewZero(Count * sizeof(tmpOperands[0]));
		if(!tmpOperands)
			return 0;

		UInt8* tmpOperators = NULL;
		tmpOperators = (UInt8*)sldMemNewZero(Count * sizeof(tmpOperators[0]));
		if(!tmpOperators)
			return 0;

		for(UInt8 currentOperandIndex = 0; currentOperandIndex < Count-1 ; currentOperandIndex++)
		{
			tmpOperands[currentOperandIndex] = Operands[currentOperandIndex];
			tmpOperators[currentOperandIndex] = Operators[currentOperandIndex];
		}

		if (Operands)
		{
			sldMemFree(Operands);
		}
		Operands = tmpOperands;

		Operands[Count-1] = NULL;
		Operands[Count-1] = (UInt16*)sldMemNew((CSldCompare::StrLen(aOperand) + 1) * sizeof(UInt16));
		if(!Operands[Count-1])
			return 0;

		UInt16* operandPtr = Operands[Count-1];
		while(*aOperand)
		{
			*operandPtr++ = *aOperand++;
		}
		*operandPtr = 0;

		if (Operators)
		{
			sldMemFree(Operators);
		}
		Operators = tmpOperators;
		Operators[Count-1] = aOperation;

		return 1;
	}

} TExpressionBox;


/// Структура для хранения набора различных вариантов написания слова
struct TWordVariantsSet
{
private:
	struct Variant
	{
		SldU16String Word;
		UInt32 Type;

		Variant(SldU16StringRef aWord, UInt32 aType) : Word(aWord), Type(aType) {}
	};
	/// Массив вариантов написания
	CSldVector<Variant> WordVariants;

public:
	/// Конструктор
	TWordVariantsSet() = default;
	/// Деструктор
	~TWordVariantsSet() = default;

	void Clear()
	{
		WordVariants.clear();
	}

	ESldError AddWordVariant(SldU16StringRef aWordVariant, UInt32 aVariantType)
	{
		/// Варианты написания в наборе должны быть разными. Если пытаемся добавить уже существующий тип, то игнорируем его
		for (auto&& variant : WordVariants)
		{
			if (variant.Type == aVariantType)
				return eOK;
		}

		WordVariants.emplace_back(aWordVariant, aVariantType);
		return eOK;
	}

	const UInt16* GetWordVariant(UInt32 aIndex) const
	{
		return WordVariants[aIndex].Word.c_str();
	}

	UInt32 GetVariantType(UInt32 aIndex) const
	{
		return WordVariants[aIndex].Type;
	}

	UInt32 GetCount() const
	{
		return WordVariants.size();
	}
};


/// Интерфейсный класс для списка слов
class ISldList
{
public:

	virtual ~ISldList(void) {};

public:

	/// Инициализация списка слов
	virtual ESldError Init(CSDCReadMy &aData, ISldLayerAccess* aLayerAccess,
							const CSldListInfo* aListInfo, CSldCompare* aCMP, UInt32 aHASH) = 0;

	/// Проверяем, проинициализирован ли список слов
	virtual bool isInit(void) = 0;

	/// Устанавливаем HASH для декодирования данного списка слов.
	virtual ESldError SetHASH(UInt32 aHASH) = 0;
	
	/// Возвращает полное количество слов в списке слов
	virtual ESldError GetTotalWordCount(Int32* aCount) = 0;

	/// Возвращает количество слов у списка слов на текущем уровне вложенности
	virtual ESldError GetNumberOfWords(Int32* aNumberOfWords) const = 0;
	
	/// Возвращает количество вариантов написания для списка слов
	virtual ESldError GetNumberOfVariants(UInt32* aNumberOfVariants) = 0;

	/// Возвращает указатель на последнее найденное слово
	virtual ESldError GetCurrentWord(Int32 aVariantIndex, UInt16** aWord) = 0;

	/// Возвращает указатель на eVariantLabel последнего найденного слова
	virtual ESldError GetCurrentWordLabel(UInt16** aLabel) = 0;

	/// Получает локальный номер текущего слова (номер слова в пределах текущего списка слов и текущего уровня вложенности)
	virtual ESldError GetCurrentIndex(Int32* aIndex) const = 0;
	
	/// Получает глобальный номер текущего слова (номер слова в пределах всего списка слов)
	virtual ESldError GetCurrentGlobalIndex(Int32* aIndex) const = 0;
	
	/// Конвертирует локальный номер слова в пределах текущего списка слов и уровня вложенности
	/// в глобальный номер слова в пределах текущего списка слов
	virtual ESldError LocalIndex2GlobalIndex(Int32 aLocalIndex, Int32* aGlobalIndex) = 0;
	
	/// Получает номер списка слов по локальному номеру слова
	virtual ESldError GetRealListIndex(Int32 aLocalWordIndex, Int32* aListIndex) = 0;
	
	/// Получает глобальный номер слова по локальному номеру слова
	virtual ESldError GetRealGlobalIndex(Int32 aLocalWordIndex, Int32* aGlobalWordIndex) = 0;

	/// Получает слово по его глобальному номеру
	virtual ESldError GetWordByGlobalIndex(Int32 aGlobalIndex, const bool aInLocalization = true) = 0;
	/// Получаем слово по локальному номеру (номеру слова в пределах текущего списка слов и текущего уровня вложенности)
	virtual ESldError GetWordByIndex(Int32 aWordIndex) = 0;
	
	/// Устанавливаем базу для списка слов
	virtual ESldError SetBase(Int32 aBase) = 0;
	
	/// Возвращает указатель на класс, хранящий информацию о свойствах списка слов
	virtual ESldError GetWordListInfo(const CSldListInfo** aListInfo, Int32 aDictIndex = SLD_DEFAULT_DICTIONARY_INDEX) const = 0;
	
	/// Возвращает количество кодов использования списка слов
	virtual ESldError GetUsageCount(Int32* aCount) const = 0;
	
	/// Возвращает код использования списка слов по индексу кода
	virtual ESldError GetListUsage(Int32 aIndex, UInt32* aUsage) const = 0;
	
	/// Возвращает флаг того, сортированный или нет данный список слов
	virtual ESldError IsListSorted(UInt32* aIsSorted) = 0;
	
	/// Возвращает флаг того, сопоставлены или нет некоторым словам из списка слов картинки
	virtual ESldError IsListHasPicture(UInt32* aIsPicture) = 0;
	
	/// Возвращает флаг того, сопоставлена или нет некоторым словам из списка слов видео
 	virtual ESldError IsListHasVideo(UInt32* aIsVideo) = 0;

	/// Возвращает флаг того, сопоставлена или нет некоторым словам из списка слов озвучка
	virtual ESldError IsListHasSound(UInt32* aIsSound) = 0;

	/// Возвращает флаг того, сопоставлена или нет некоторым словам из списка слов 3d сцена
	virtual ESldError IsListHasScene(UInt32* aIsScene) = 0;
	
	/// Возвращает флаг того, имеет или нет указанное слово поддерево иерархии
	virtual ESldError isWordHasHierarchy(Int32 aIndex, UInt32* aIsHierarchy, EHierarchyLevelType* aLevelType) = 0;
	
	/// Возвращает вектор индексов изображений для текущего слова
	virtual ESldError GetPictureIndex(CSldVector<Int32> & aPictureIndexes) = 0;

	/// Возвращает номер видео текущего слова
	virtual ESldError GetVideoIndex(Int32* aVideoIndex) = 0;

	/// Возвращает вектор индексов озвучек текущего слова
	virtual ESldError GetSoundIndex(CSldVector<Int32> & aSoundIndexes) = 0;

	/// Возвращает номер 3d сцены текущего слова
	virtual ESldError GetSceneIndex(Int32* aSceneIndex) = 0;

	/// Возвращает количество переводов у указанного слова из списка слов
	virtual ESldError GetTranslationCount(Int32 aGlobalIndex, Int32* aTranslationCount) = 0;

	/// Возвращает количество ссылок у указанного слова из списка слов
	virtual ESldError GetReferenceCount(Int32 aGlobalIndex, Int32* aTranslationCount) = 0;

	/// Возвращает номер статьи с переводом, по номеру перевода
	virtual ESldError GetTranslationIndex(Int32 aGlobalIndex, Int32 aTranslationIndex, Int32* aArticleIndex) = 0;

	/// Возвращает путь к текущему положению в каталоге
	virtual ESldError GetCurrentPath(TCatalogPath* aPath) = 0;

	/// Возвращает путь в каталоге к элементу с глобальным номером (т.е. "прямой" номер слова без учета иерархии)
	virtual ESldError GetPathByGlobalIndex(Int32 aIndex, TCatalogPath* aPath) = 0;

	/// Переходит по указанному пути
	virtual ESldError GoToByPath(const TCatalogPath* aPath, ESldNavigationTypeEnum aNavigationType) = 0;

	/// Поднимаемся в каталоге на уровень выше текущего или остаемся на текущем, если выше некуда
	virtual ESldError GoToLevelUp(void) = 0;
	
	/// Возвращает список названий родительских категорий, разделенных строкой-разделителем, по глобальному номеру слова
	virtual ESldError GetParentWordNamesByGlobalIndex(Int32 aGlobalWordIndex, UInt16** aText, const UInt16* aSeparatorText, Int32 aMaxLevelCount = -1) = 0;
	
	
	/// Ищем ближайшее слово, которое больше или равно заданному
	virtual ESldError GetWordByText(const UInt16* aText) = 0;

	/// Ищет ближайшее слово, которое больше или равно заданному
	/// Работает в несортированных списках, для сортированных по сорткею списках ищет по Show-варианту(начиная с баз версии 112+)
	virtual ESldError GetWordBySortedText(const UInt16* aText, UInt32* aResultFlag) = 0;
	
	/// Ищет максимально похожее слово с помощью GetWordBySortedText()
	virtual ESldError GetWordByTextExtended(const UInt16* aText, UInt32* aResultFlag, UInt32 aActionsOnFailFlag) = 0;

	/// Ищет наиболее подходящее слово по переданным вариантам написания
	virtual ESldError GetWordBySetOfVariants(TWordVariantsSet* aWordVariantsSet, UInt32 aWordSearchType, UInt32* aResultFlag, UInt32 aActionsOnFailFlag = 1) = 0;

	/// Ищет слово, бинарно совпадающее по переданным вариантам написания
	virtual ESldError GetExactWordBySetOfVariants(TWordVariantsSet* aWordVariantsSet, UInt32* aResultFlag) = 0;

	/// Ищем наиболее похожее слово, которое равно заданному
	virtual ESldError GetMostSimilarWordByText(const UInt16* aText, UInt32* aResultFlag) = 0;

	/// Производит поиск слов по шаблону, в пределах текущего уровня вложенности
	virtual ESldError DoWildCardSearch(TExpressionBox* aExpressionBox, Int32 aMaximumWords, ISldList* aList, Int32 aRealListIndex, ISldList* aRealList) = 0;

	/// Производит поиск похожих слов, в пределах текущего уровня вложенности
	virtual ESldError DoFuzzySearch(const UInt16* aText, Int32 aMaximumWords, Int32 aMaximumDifference, ISldList* aList, Int32 aRealListIndex, EFuzzySearchMode aSearchMode) = 0;
	
	/// Производит поиск анаграмм, в пределах текущего уровня вложенности
	virtual ESldError DoAnagramSearch(const UInt16* aText, Int32 aTextLen, ISldList* aList, Int32 aRealListIndex) = 0;
	
	/// Производит поиск слов с учетом возможных опечаток в пределах текущего уровня вложенности
	virtual ESldError DoSpellingSearch(const UInt16* aText, ISldList* aList, Int32 aRealListIndex) = 0;
	
	/// Производит полнотекстовый поиск
	virtual ESldError DoFullTextSearch(const UInt16* aText, Int32 aMaximumWords, ISldList** aList, Int32 aListCount, Int32 aRealListIndex) = 0;
	
	/// Производит сортировку поискового списка слов по релевантности шаблону поиска
	virtual ESldError SortListRelevant(const UInt16* aText, const CSldVector<TSldMorphologyWordStruct>& aMorphologyForms, ESldFTSSortingTypeEnum aMode) = 0;
	
	/// Сохраняет текущее состояние списка слов
	virtual ESldError SaveCurrentState(void) = 0;

	/// Восстанавливает ранее сохраненное состояние списка слов
	virtual ESldError RestoreState(void) = 0;

	/// Получает следующее слово
	virtual ESldError GetNextWord() = 0;

	/// Возвращает таблицу сравнения которая используется в данном списке слов
	virtual ESldError GetCompare(CSldCompare **aCompare) = 0;

	/// Возвращает количество записей в таблице быстрого доступа
	virtual ESldError QAGetNumberOfQAItems(Int32 *aNumberOfQAItems) = 0;

	/// Возвращает текст для записи в таблице быстрого доступа.
	virtual ESldError QAGetItemText(Int32 aIndex, const UInt16 **aQAText) = 0;

	/// Возвращает номер для записи в таблице быстрого доступа.
	virtual ESldError QAGetItemIndex(Int32 aIndex, Int32 *aQAIndex) = 0;

	/// Производит переход по таблице быстрого доступа по номеру записи.
	virtual ESldError JumpByQAIndex(Int32 aIndex) = 0;
	
	/// Определяет глобальные номера границ текущего уровня вложенности (включая все подуровни)
	/// Получается диапазон глобальных индексов слов вида [aLowGlobalIndex, aHighGlobalIndex) - левая граница включается, правая - нет
	virtual ESldError GetHierarchyLevelBounds(Int32* aLowGlobalIndex, Int32* aHighGlobalIndex) = 0;

	/// Получает вектор индексов озвучек по введенному тексту
	virtual ESldError GetSoundIndexByText(const UInt16 *aText, CSldVector<Int32> & aSoundIndexes) = 0;

	/// Устанавливает нужную локализацию записей по переданному языковому коду
	virtual ESldError SetLocalization(ESldLanguage aLocalizationLangCode) = 0;
	/// Устанавливает нужную локализацию записей по переданному языковому коду и набору настроек
	virtual ESldError SetLocalization(ESldLanguage aLocalizationLangCode, const UInt16* aLocalizationDetails) = 0;

	/// Инициализирует вспомогательный сортированный список
	virtual ESldError InitSortedList(ISldList* aSortedListPtr) = 0;

	/// Синхронизирует состояние вспомогательного сортированного списка с базовым
	virtual ESldError SynchronizeWithASortedList() = 0;

	/// Получает слово по его номеру в сортированном списке
	virtual ESldError GetWordBySortedIndex(Int32 aIndex) = 0;

	/// Получает следующее сортированное слово
	virtual ESldError GetNextSortedWord() = 0;

	/// Получает следующее реальное сортированное слово
	virtual ESldError GetNextRealSortedWord(Int8* aResult) = 0;

	/// Возвращает текущий индекс сортированного списка
	virtual ESldError GetCurrentSortedIndex(Int32* aIndex) const = 0;

	/// Возвращает количество записей в таблице быстрого доступа для сортированного списка
	virtual ESldError QAGetNumberOfSortedQAItems(Int32 *aNumberOfQAItems) = 0;

	/// Возвращает текст для записи в таблице быстрого доступа для сортированного списка
	virtual ESldError QAGetSortedItemText(Int32 aIndex, const UInt16 **aQAText) = 0;

	/// По номеру точки входа в сортированном списке возвращает номер слова, соответствующий этой точке
	virtual ESldError QAGetSortedItemIndex(Int32 aIndex, Int32 *aQAIndex) = 0;

	/// Производит переход по таблице быстрого доступа по номеру записи в сортированном списке
	virtual ESldError JumpByQASortedIndex(Int32 aIndex) = 0;

	/// Возвращает флаг, проверяющий на реальном ли слове установлен список, или на альтернативном заголовке
	virtual Int8 IsNormalWord() = 0;

	/// Проверяет наличие вспомогательного сортированного списка
	virtual Int8 HasSimpleSortedList() = 0;

	/// Проверяет наличие иерархии у списка
	virtual Int8 HasHierarchy() = 0;

	/// Переходит к ближайшей закешированной точке
	virtual ESldError GoToCachedPoint(UInt32 aQAPointIndex, Int32 aNeedGlobalIndex) = 0;

	/// Проверяет сортированность текущего уровня иерархии
	virtual ESldError IsCurrentLevelSorted(UInt32* aIsSorted) = 0;

	/// Получает слово по элементу истории
	virtual ESldError GetWordByHistoryElement(CSldHistoryElement* aHistoryElement, ESldHistoryResult* aResultFlag, Int32* aGlobalIndex) = 0;

	/// Возвращает количество слов из вспомогательного сортированного списка
	virtual UInt32 GetSortedWordsCount() = 0;

	/// Устанавливает значение флага синхронизации каталога с текущим индексом
	virtual ESldError SetCatalogSynchronizedFlag(UInt8 aFlag) = 0;

	/// Возвращает количество результатов полнотекстового поиска
	virtual ESldError GetFullTextResultCount(const UInt16* aRequest, Int32* aCount) = 0;

	/// Возвращает индекс статьи, содержащей стилизованный текст варианта написания для текущего слова
	virtual Int32 GetStylizedVariantArticleIndex(UInt32 aVariantIndex) const = 0;

	/// Возвращает индекс стиля для заданного варианта написания из eVariantLocalizationPreferences
	virtual Int32 GetStyleVariantIndex(Int32 aVariantIndex) = 0;

	/// Переходит по пути, соответсвующему заданному глобальному индексу
	virtual ESldError GoToByGlobalIndex(const Int32 aGlobalIndex) = 0;

	/// Индексирует все слова, содержащиеся в списке в соответсвии с заданным вариантом написания
	virtual ESldError SortListByVariant(const Int32 aVariantIndex, const Int8 aCreateWordsBuffer = 0) = 0;

	/// Получает реальные индексы списка и записи в реальном списке в по локальному номеру слова
	virtual ESldError GetRealIndexes(Int32 aWordIndex, Int32 aTrnslationIndex, Int32* aRealListIndex, Int32* aGlobalWordIndex) = 0;

	/// Сортирует список, согласно заданному массиву индексов
	virtual ESldError SortByVector(const CSldVector<Int32> & aWordIndexes) = 0;

	/// Получает количество локализаций записей в списке слов
	virtual ESldError GetNumberOfLocalization(Int32 & aNumberOfLocalization) = 0;

	/// Устанавливает нужную локализацию записей в списке слов по номеру локализации
	virtual ESldError SetLocalizationByIndex(const Int32 aIndex) = 0;

	/// Получает информацию о локализации записей в списке слов по номеру локализации
	virtual ESldError GetLocalizationInfoByIndex(const Int32 aIndex, UInt32 & aLocalizationCode, SldU16String & aLocalizationName) = 0;

	/// Возвращает индекс текущей локализации
	virtual ESldError GetCurrentLocalizationIndex(Int32 & aIndex) = 0;

	/// Возвращает тип списка в иерархии наследования
	virtual EHereditaryListType GetHereditaryListType() const = 0;

	/// Возвращает вектор слитых индексов словарей для заданного слова
	virtual ESldError GetDictionaryIndexesByGlobalIndex(const Int32 aGlobalIndex, CSldVector<Int32> & aDictIndexes) = 0;

protected:
	/// Производит переход по таблице быстрого поиска
	virtual ESldError GoToBySearchIndex(Int32 aSearchIndex) = 0;

};

#endif
