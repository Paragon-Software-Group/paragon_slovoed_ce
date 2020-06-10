#ifndef _SLD_DICTIONARY_HELPER_H_
#define _SLD_DICTIONARY_HELPER_H_

#include "SldAuxiliary.h"

/// Структура, описывающая внешний ресурс
struct TExternResource
{
	/// Имя ресурса
	SldU16String						ResourceName;

	/// Имя базы с ресурсом
	SldU16String						BaseName;

	/// Индекс имени ресурса (техническое поле, необходимо при загрузке информации о ресурсах)
	Int32								WordIndex;

	/// стандартный конуструктор
	TExternResource()					{}
};

/// Структура, содержащая список ресурсов для определенного приоритета
struct TExternResourcePriority
{
	/// Вектор с именами ресурсов
	CSldVector<TExternResource>			ExternResources;

	/// Приоритет данных ресурсов
	UInt32								Priority;

	/// стандартный конуструктор
	TExternResourcePriority()			{}
};

/// Вектор структур TExternResource, не обязательно сортирован по приоритету
typedef CSldVector<TExternResourcePriority> TExternReference;

class CSldDictionaryHelper
{
public:
	/// Конструктор
	CSldDictionaryHelper(CSldDictionary & aDict) : m_Dictionary(aDict){}
	/// Деструктор
	~CSldDictionaryHelper() {}

	/// Получает информацию о внешних ресурсах по локальному индексу в текущем списке слов
	ESldError GetExternReference(Int32 aLocalIndex, TExternReference* aRefs);

	/// Производит поиск по вспомогательным поисковым спискам
	ESldError SearchByAuxiliarySearchList(const UInt16* aWord, const UInt32 aLangCode, const UInt32 aLocalizationCode, const UInt32 aUnlockedCategoriesCount, Int32 & aResultListIndex);

	/// Производит поиск по списку типа eWordListType_DictionaryForSearch
	ESldError SearchByDictionaryForSearchList(const UInt16* aWord, MorphoDataType* aMorpho, CSldCustomListControl & aListControl, EDictionaryForSearchResultType & aResult);

	/// Производит расширенный поиск по спискам полнотекстового поиска с заголовками
	ESldError AdvancedHeadwordSearch(const UInt16* aWord, const UInt32 aLangCode, MorphoDataType* aMorpho, Int32 & aResultListIndex);

	/// Производит поиск Collocations в базах Oxford Collocations
	ESldError CollocationsSearch(const UInt16* aRequest, const UInt32 aLangCode, MorphoDataType* aMorpho, CSldCustomListControl & aListControl, Int32 & aResultListIndex);

	/// Получает алфавит для заданного сортированного списка
	ESldError GetAlphabet(const Int32 aListIndex, CSldVector<TSldPair<UInt16, Int32>> & aAlphabet);

	/// Получает морфо-подсказки для заданного списка
	ESldError GetMorphoTips(const Int32 aListIndex, const UInt16* aWord, MorphoDataType* aMorpho, CSldVector<Int32> & aMorphoTipsIndexes, const bool aOnlyRoot = true);

	/// Ищет записи, соответсвующие запросу в словарных/каталожных списках
	ESldError GetCrossReference(const ESldLanguage aLangCode, const SldU16String & aWord, MorphoDataType* aMorpho, SldWordsCollection& aRefs, const bool aOnlyRoot = true);

	/// Выполняет свайп в любом списке, в направлении aDirection/abs(aDirection), в режиме aSwipingMode
	ESldError Swipe(Int32, ESwipingMode, CSldCustomListControl *);

	/// Ищет записи, соответсвующие словам во фразе
	ESldError GetWordReference(SldU16StringRef aPhrase, const CSldVector<MorphoDataType*>& aMorphos, CSldVector<TWordRefInfo>& aRefs, UInt8 aUseBuiltInMorpho = 0);

	/// Ищет записи, соответсвующие словам во фразе в указанном списке
	ESldError GetWordReferenceInList(SldU16StringRef aPhrase, const CSldVector<MorphoDataType*>& aMorphos, CSldVector<TWordRefInfo>& aRefs, Int32 aListIndex = SLD_DEFAULT_LIST_INDEX, UInt8 aUseBuiltInMorpho = 0);

	/// Ищет слова из запроса во фразе
	ESldError GetWordReferenceInQuery(SldU16StringRef aPhrase, const CSldVector<MorphoDataType*>& aMorphos, CSldVector<UInt32>& aRefs, SldU16StringRef aQuery, UInt8 aUseBuiltInMorpho = 0) const;

	/// Ищет диапазоны фразы и слов запроса в тексте
	ESldError GetQueryHighlightData(SldU16StringRef aText, SldU16StringRef aPhrase, SldU16StringRef aQuery, SldU16StringRef aHeadword, const CSldVector<MorphoDataType*>& aMorphos, CSldVector<UInt32>& aHighlightData, UInt8 aUseBuiltInMorpho = 0) const;

	/// Возвращает диапазоны во фразе, соответсвующие указанным словам с обрамляющими пробелами
	ESldError GetWordsPositionsInPhraseIncludingWhitespaces(SldU16StringRef aPhrase, const CSldVector<UInt32>& aWords, CSldVector<TWordRefInfo>& aRefs);

private:

	ESldError GetExternalBaseName(Int32 aListIndex, SldU16WordsArray& aBaseNames);

	ESldError CloseSearch(Int32 aListIndex);

	ESldError AddCustomList(const Int32 aListIndex, CSldCustomListControl & aListControl);

	Int32 GetIndexFromSimpleSortedList(const Int32 aListIndex, const Int32 aWordIndex);

	ESldError GetMaxSearchBound(const Int32 aListIndex, const bool aOnlyRoot, Int32& aMaxBound);

	/// Выполняет свайп в некастомном списке, в направлении aDirection/abs(aDirection), в режиме aSwipingMode
	ESldError Swipe(Int32, ESwipingMode);

	CSldVector<Int32> GetListIndexes(const EWordListTypeEnum aType, const ESldLanguage aLang = SldLanguage::Unknown);

	const CSldCompare* GetCompare() const { const CSldCompare* cmp = nullptr; m_Dictionary.GetCompare(&cmp); return cmp; }  

private:

	/// Указатель на класс CSldDictionary
	CSldDictionary		&m_Dictionary;
};
#endif