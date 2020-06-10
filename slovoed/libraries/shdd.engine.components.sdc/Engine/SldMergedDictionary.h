#ifndef _SLD_MERGED_DICTIONARY_H_
#define _SLD_MERGED_DICTIONARY_H_

#include "SldDictionary.h"
#include "SldMergedList.h"

/// Класс словаря.
class CSldMergedDictionary : public CSldDictionary
{
public:

	/// Стандартный конструктор
	CSldMergedDictionary(void) { Clear();}

	/// Деструктор
	virtual ~CSldMergedDictionary(void) { Close(); }

public:

	/// Открывает словарь
	ESldError Open(ISDCFile *aFile, ISldLayerAccess *aLayerAccess) override;

	/// Закрывает словарь
	ESldError Close() override;

	/// Добавляет новый словарь
	ESldError AddDictionary(ISDCFile & aFile);

	/// Производит слияние словарей
	ESldError Merge(const CSldMergedDictionary * aMergedDictionary = NULL);

	/// Устанавливает список слов в качестве текущего.
	ESldError SetCurrentWordlist(Int32 aIndex) override;

	/// Получает индекс текущего списка слов
	ESldError GetCurrentWordList(Int32* aIndex) const override;

	/// Возвращает количество списков слов
	ESldError GetNumberOfLists(Int32 *aNumberOfLists) const override;

	/// Возвращает класс, хранящий информацию о свойствах списка слов
	ESldError GetWordListInfo(Int32 aListIndex, const CSldListInfo **aListInfo) const override;

	/// Возвращает количество стилей
	UInt32 GetNumberOfStyles() const override;
	/// Возвращает класс, хранящий информацию об указанном стиле
	ESldError GetStyleInfo(Int32 aIndex, const CSldStyleInfo **aStyleInfo) const override;

	/// Производит перевод указанной статьи по глобальному номеру статьи
	ESldError TranslateByArticleIndex(Int32 aIndex, ESldTranslationFullnesType aFullness, UInt32 aStartBlock = 0, UInt32 aEndBlock = -1) override;
	/// Производит получение остатков перевода 
	ESldError TranslateContinue(ESldTranslationFullnesType aFullness) override;
	/// Производит завершение перевода и подготовку к получению нового перевода
	ESldError TranslateClear() override;

	/// Инициализируем систему защиты
	ESldError InitRegistration(void) override;

	/// Возвращает форматированный вариант написания для текущего слова
	ESldError GetCurrentWordStylizedVariant(Int32 aVariantIndex, SplittedArticle & aArticle) override;

	/// Возвращает общее количество картинок в словаре
	ESldError GetTotalPictureCount(Int32* aCount) const override;

	/// Возвращает общее количество озвучек в словаре
	ESldError GetTotalSoundCount(Int32* aCount) const override;

	/// Получает картинку по номеру
	ESldError GetWordPicture(Int32 aPictureIndex, Int32 aMaxSideSize, TImageElement* aImageElement) override;

	/// Получает готовую для проигрывания озвучку по индексу озвучки
	ESldError PlaySoundByIndex(Int32 aSoundIndex, UInt8 aIsLast, UInt32* aStartPos, UInt32 aExternFlag, UInt8 aBeginPlay) override;

	/// Возвращает класс, хранящий информацию о строчках для всего словаря
	ESldError GetLocalizedStrings(const CSldLocalizedString** aStrings, const Int32 aDictionaryIndex = 0) const override;

	/// Возвращает номер старшей версии словарной базы
	ESldError GetDictionaryMajorVersion(UInt32* aVersion, const Int32 aDictionaryIndex = 0) const override;

	/// Возвращает номер младшей версии словарной базы
	ESldError GetDictionaryMinorVersion(UInt32* aVersion, const Int32 aDictionaryIndex = 0) const override;

	/// Возвращает константу, которая обозначает бренд словарной базы (см. EDictionaryBrandName)
	ESldError GetDictionaryBrand(UInt32* aBrand, const Int32 aDictionaryIndex = 0) const override;

	/// Возвращает хэш словарной базы
	ESldError GetDictionaryHash(UInt32* aHash, const Int32 aDictionaryIndex = 0) const override;

	/// Возвращает ID словарной базы
	ESldError GetDictionaryID(UInt32* aDictID, const Int32 aDictionaryIndex = 0) const override;

	/// Производит полнотекстовый поиск в указанном списке слов
	ESldError DoFullTextSearch(Int32 aListIndex, const UInt16* aText, Int32 aMaximumWords) override;

	/// Получает реальные индексы списка и записи в реальном списке в по номеру слова в указанном списке слов
	ESldError GetRealIndexes(Int32 aListIndex, Int32 aWordIndex, Int32 aTrnslationIndex, Int32* aRealListIndex, Int32* aGlobalWordIndex) override;

	/// Получает слово по элементу истории
	ESldError GetWordByHistoryElement(CSldHistoryElement* aHistoryElement, ESldHistoryResult* aResultFlag, Int32* aListIndex, Int32* aGlobalIndex) override;

	/// Обновляет элемент истории до текущего состояния базы
	ESldError UpdateHistoryElement(CSldHistoryElement* aHistoryElement, ESldHistoryResult* aResultFlag) override;

private:

	/// Инициализация членов по умолчанию
	void Clear() override;

	/// Подготавливает списки для слияния
	ESldError CheckWordLists();

	/// Ищет для заданного списка одиночного словаря подходящий список слияния
	ESldError CheckList(const Int32 aDictIndex, ISldList * aListPtr, UInt32 & resultListIndex);

	/// Для полнотекстовых списков возвращает индекс базового списка
	ESldError GetBaseListIndex(const Int32 aDictIndex, ISldList * aListPtr, Int32 & aBaseListIndex);

	/// Возвращает указатель на экземпляр объекта списка слов по номеру списка
	ESldError GetWordList(Int32 aIndex, ISldList** aListPtr) override;

	/// Добавляет список слов
	ESldError AddList(sld2::UniquePtr<ISldList> aList, Int32 aListIndex) override;

	/// Удаляет список слов
	ESldError RemoveList(Int32 aListIndex) override;

	/// Получает элемент истории для заданного слова в текущем списке слов
	ESldError GetHistoryElement(Int32 aIndex, CSldVector<CSldHistoryElement> & aHistoryElements);

	/// Возвращает данные о слиянии
	const TMergedMetaInfo* GetMergeInfo() const override { return &m_MergedMetaInfo; }


	/// Возвращает указатель на заголовок словаря
	const TDictionaryHeader * GetDictionaryHeader() const override { return m_Dictionaries.front()->GetDictionaryHeader(); }

	/// Возвращает указатель на данные, необходимые для работы прослойки
	ISldLayerAccess * GetLayerAccess() override { return m_LayerAccess; }

	/// Возвращает индекс текущего списка слов
	Int32 GetCurrentListIndex() const override { return m_CurrentListIndex; }

	/// Возвращает индекс списка, который был текущим до операции поиска
	Int32 GetPrevListIndex() const override { return m_PrevListIndex; };

	/// Возвращает класс чтения данных
	CSDCReadMy* GetReadData() override { return m_Dictionaries.front()->GetReadData(); }

	/// Возвращает указатель на списки слов
	ISldList ** GetLists() override { return m_MergedWordlists.data(); }

	/// Возвращает класс, занимающийся сравнением строк
	ESldError GetCompare(const CSldCompare** aCMP) const override { return m_Dictionaries.front()->GetCompare(aCMP); }

	/// Возвращает класс сравнения строк
	CSldCompare	& GetCMP() override { return m_Dictionaries.front()->GetCMP(); }


private:

	/// Вектор экземпляров CSldDictionary, участвующих в слиянии
	CSldVector<sld2::UniquePtr<CSldDictionary>>			m_Dictionaries;

	/// Вектор всех списков, отображаемых в CSldMergedDictionary
	CSldVector<ISldList*>								m_MergedWordlists;

	/// Указатель на данные, необходимые для работы прослойки
	ISldLayerAccess										*m_LayerAccess;

	/// Индекс текущего списка слов
	Int32												m_CurrentListIndex;

	/// Индекс списка, который был текущим до операции поиска
	Int32												m_PrevListIndex;

	/// Номер словаря, из которого мы запросили перевод последним
	Int32												m_TranslateDictionaryIndex;

	/// Информация о количестве статей, стилей, метаданных
	TMergedMetaInfo										m_MergedMetaInfo;

	/// Временный буфер для списков
	CSldVector<CSldVector<TSldPair<ISldList*, Int32>>>	m_ListsBuffer;
};

#endif //_SLD_MERGED_DICTIONARY_H_