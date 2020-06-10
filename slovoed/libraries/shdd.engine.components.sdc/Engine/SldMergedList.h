#ifndef _SLD_MERGED_LIST_H_
#define _SLD_MERGED_LIST_H_

#include "SldList.h"

/// Класс для хранения одиночных индексов
class CSldIndexesStore
{
public:

	/// Максимальный размер блока данных
	enum : UInt32 { MemBlockSize = 512 };

	/// Конструктор
	CSldIndexesStore() : m_ListCount(0), m_IndexCount(0) { }

	/// Инициализация
	ESldError Init(CSldVector<ISldList*> & aLists);

	/// Добавляет новый индекс
	void AddIndex(const Int32 * aSingleIndexes);

	/// Получает набор одиночных индексов для индекса в ядре слияния
	ESldError GetIndex(const Int32 aIndex, const Int32 ** aSingleIndexes) const;

	/// Возвращает количество списков участвующих с лиянии
	const UInt32 GetListCount() const { return m_ListCount; }

	/// Возвращает общее количество слов
	const UInt32 GetTotalIndexesCount() const { return m_IndexCount; }

private:

	/// Вектор одиночных индексов словарей
	CSldVector<CSldVector<Int32>>	m_Indexes;

	/// Количество списков участвующих с лиянии
	UInt32							m_ListCount;

	/// Общее количество слов
	UInt32							m_IndexCount;
};

class CSldMergedList : public CSldList
{

public:

	/// Конструктор
	CSldMergedList(CSldVector<TSldPair<ISldList*, Int32>> & aLists, const TMergedMetaInfo& aMergedMetaInfo);

	/// Деструктор
	~CSldMergedList(void) { }

	/// Инициализация
	ESldError InitMergedList(CSldVector<CSldVector<Int32>> & aWordIndexes, CSldMergedList * aMergedList = NULL);

	/// Получает слово по его глобальному номеру
	ESldError GetWordByGlobalIndex(Int32 aGlobalIndex, const bool aInLocalization = true) override;

	/// Получаем слово по локальному номеру (номеру слова в пределах текущего списка слов и текущего уровня вложенности)
	ESldError GetWordByIndex(Int32 aWordIndex) override { return GetWordByGlobalIndex(aWordIndex); }

	/// Ищем ближайшее слово, которое больше или равно заданному
	ESldError GetWordByText(const UInt16* aText) override;

	/// Ищет ближайшее слово, которое больше или равно заданному
	/// Работает в несортированных списках, для сортированных по сорткею списках ищет по Show-варианту(начиная с баз версии 112+)
	ESldError GetWordBySortedText(const UInt16* aText, UInt32* aResultFlag) override;

	/// Возвращает указатель на класс, хранящий информацию о свойствах списка слов
	ESldError GetWordListInfo(const CSldListInfo** aListInfo, Int32 aDictIndex = SLD_DEFAULT_DICTIONARY_INDEX) const override;

	/// Возвращает указатель на последнее найденное слово
	ESldError GetCurrentWord(Int32 aVariantIndex, UInt16** aWord) override;

	/// Возвращает количество переводов у указанного слова из списка слов
	ESldError GetTranslationCount(Int32 aGlobalIndex, Int32* aTranslationCount) override;

	/// Возвращает номер статьи по номеру слова и номеру перевода в обычном списке слов
	ESldError GetTranslationIndex(Int32 aGlobalIndex, Int32 aTranslationIndex, Int32* aArticleIndex) override;

	/// Ищет точное вхождение слова в Show-вариантах и альтернативных заголовках
	ESldError FindBinaryMatch(const UInt16* aText, UInt32* aResult) override;

	/// Возвращает тип списка в иерархии наследования
	EHereditaryListType GetHereditaryListType() const override { return eHereditaryListTypeMerged; }

	/// Возвращает вектор слитых индексов словарей для заданного слова
	ESldError GetDictionaryIndexesByGlobalIndex(const Int32 aGlobalIndex, CSldVector<Int32> & aDictIndexes) override;

	/// Возвращает вектор индексов озвучек текущего слова
	ESldError GetSoundIndex(CSldVector<Int32> & aSoundIndexes) override;

	/// Возвращает вектор индексов изображений для текущего слова
	ESldError GetPictureIndex(CSldVector<Int32> & aPictureIndexes) override;

	/// Получает следующее сортированное слово
	ESldError GetNextSortedWord() override;

	/// Возвращает индекс слова из одиночного словаря по смерженому индексу
	ESldError GetSingleIndex(Int32 aMergedIndex, Int32 aTranslationIndex, Int32 & aSingleIndex);

	/// Получает следующее реальное сортированное слово
	ESldError GetNextRealSortedWord(Int8* aResult) override;

private:

	/// Определяет в каких одиночных списках текущее слово активно
	ESldError GetMinWordIndexes(const CSldVector<Int32> & aShowVariantIndexes, const CSldVector<Int8> & aEndList, CSldVector<bool> & aIndexes);


	/// Возвращает указатель на заголовок данного списка слов
	const CSldListInfo* GetListInfo() const override { return &m_ListInfo; }

	/// Возвращает указатель на класс, отвечающий за сравнение строк
	CSldCompare* GetCMP() override { return ((CSldList*)m_Lists.front())->GetCMP(); }

	/// Возвращает индекс текущего слова
	Int32 GetCurrentWordIndex() const override { return m_CurrentGlobalIndex; };

	/// Возвращает указатель на заданный вариант написаничя слова
	UInt16* GetWord(const Int32 aIndex) override { UInt16* result = NULL; GetCurrentWord(aIndex, &result); return result; }

private:

	/// Вектор списков, участвующих в слиянии
	CSldVector<ISldList*>							m_Lists;

	/// Вектор соответсвия номера списка и номера словаря
	CSldVector<Int32>								m_DictionaryIndexes;

	/// Массив одиночных индексов
	CSldIndexesStore								m_SingleIndexes;

	/// Общее количество слов
	Int32											m_NumberOfWords;

	/// Номер текущего слова
	Int32											m_CurrentGlobalIndex;

	/// Заголовок данного списка слов
	CSldListInfo									m_ListInfo;

	/// Вектор типов вариантов написания в одиночных списках
	CSldVector<CSldVector<Int32>>					m_LocalVariantIndexes;

	/// Буфер под пустую строку
	SldU16String									m_StringBuf;
};

#endif //_SLD_MERGED_LIST_H_
