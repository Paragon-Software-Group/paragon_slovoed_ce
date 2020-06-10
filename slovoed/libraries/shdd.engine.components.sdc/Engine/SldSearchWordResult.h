#ifndef _SLD_SEARCH_WORD_RESULT_H_
#define _SLD_SEARCH_WORD_RESULT_H_

#include "SldTypes.h"
#include "ISldList.h"

/// Структура для хранения найденного слова
struct TSldSearchWordStruct
{
	/// Локальный индекс списка слов (индекс списка слов, в котором найдено слово - внутренний в классе CSldSearchList)
	UInt32 ListIndex;

	/// Глобальный индекс найденного слова в списке слов
	UInt32 WordIndex;

	/// Расстояние редактирования между данным словом и искомым
	Int32 WordDistance;

	/// Конструктор
	TSldSearchWordStruct()
		: ListIndex(0), WordIndex(0), WordDistance(0)
	{}
};

/// Класс, который содержит результаты поиска одного слова при полнотекстовом поиске
class CSldSearchWordResult
{
public:
	
	/// Конструктор
	CSldSearchWordResult(void);

public:

	/// Инициализация
	ESldError Init(ISldList** aList, Int32 aListCount);
	
	/// Проверяет, инициализирован класс или нет
	bool IsInit() const;

	/// Очищает все результаты поиска
	void Clear();

	/// Добавляет к результатам поиска ссылку на слово
	ESldError AddWord(Int32 aListIndex, Int32 aWordIndex, Int32 aTranslationIndex, Int32 aShiftIndex);
	
	/// Проверяет, содержится ли в результатах поиска список
	bool IsListPresent(Int32 aListIndex) const;
	
	/// Проверяет, содержится ли в результатах поиска слово
	bool IsWordPresent(Int32 aListIndex, Int32 aWordIndex) const;
	
	/// Возвращает количество слов в наборе
	Int32 GetWordCount();
	
	/// Возвращает количество списков слов
	Int32 GetListCount() const { return static_cast<Int32>(m_Lists.size()); }
	
	/// Заполняет итоговый массив найденными словами
	Int32 FillWordVector(sld2::Span<TSldSearchWordStruct> aWordVector, Int32 aMaximumWords) const;

	/// Объединяет результаты 2 поисков через оператор AND (т.е. получается пересечение двух поисков)
	ESldError ResultsAND(const CSldSearchWordResult *aSearchResults);

	/// Объединяет результаты 2 поисков через оператор OR (т.е. получается сложение двух поисков)
	ESldError ResultsOR(const CSldSearchWordResult *aSearchResults);

	/// Применяет к результатам поиска операцию отрицания
	void ResultsNOT();

private:

	/// Пересчитывает количество уникальных слов.
	void ReCountWords();

	/// Общее количество уникальных слов в наборе
	Int32 m_WordCount;

	/// Битовая таблица
	struct BitTable
	{
		UInt32 *data;
		UInt32 size;

		operator sld2::Span<UInt32>()             { return{ data, size }; }
		operator sld2::Span<const UInt32>() const { return{ data, size }; }
	};

	/// Массив битовых таблиц для всех листов
	sld2::DynArray<BitTable> m_Lists;

	/// Хранилище данных битовых таблиц
	sld2::DynArray<UInt32> m_DataBuf;

	/// Флаг актуальности хранимого количества слов (m_WordCount)
	bool m_staleWordCount;
};

/// Класс, который содержит результаты поиска одного слова при поиске по шаблону
/// (или вообще результаты любого поиска только в одном списке слов, в отличие от результатов полнотекстового поиска)
class CSldSimpleSearchWordResult
{
public:
	
	/// Конструктор
	CSldSimpleSearchWordResult(void);

public:

	/// Инициализация
	ESldError Init(Int32 aMaximumNumberOfWords, Int32 aRealListIndex);
	
	/// Проверяет, инициализирован класс или нет
	bool IsInit() const;

	/// Очищает все результаты поиска
	void Clear();

	/// Добавляет к результатам поиска ссылку на слово
	ESldError AddWord(Int32 aWordIndex);
	
	/// Проверяет, содержится ли в результатах поиска слово
	bool IsWordPresent(Int32 aWordIndex) const;
	
	/// Возвращает количество слов в наборе
	Int32 GetWordCount();
	
	/// Заполняет итоговый массив найденными словами
	Int32 FillWordVector(sld2::Span<TSldSearchWordStruct> aWordVector, Int32 aMaximumWords, Int32 aLocalListIndex) const;

	/// Объединяет результаты 2 поисков через оператор AND (т.е. получается пересечение двух поисков)
	ESldError ResultsAND(const CSldSimpleSearchWordResult* aSearchResult);

	/// Объединяет результаты 2 поисков через оператор OR (т.е. получается сложение двух поисков)
	ESldError ResultsOR(const CSldSimpleSearchWordResult* aSearchResult);
	
	/// Применяет к результатам поиска операцию отрицания
	void ResultsNOT();

private:

	/// Настоящий номер списка слов в словаре
	Int32 m_RealListIndex;

	/// Битовая таблица
	sld2::DynArray<UInt32> m_Data;

	/// Общее количество уникальных слов в наборе
	Int32 m_WordCount;

	/// Флаг актуальности хранимого количества слов (m_WordCount)
	bool m_staleWordCount;
};

#endif // _SLD_SEARCH_WORD_RESULT_H_
