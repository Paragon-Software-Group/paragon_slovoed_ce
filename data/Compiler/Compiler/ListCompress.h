#pragma once

#include "sld2_Error.h"
#include "sld_Types.h"
#include "WordList.h"
#include "Out.h"
#include "ICompressBy.h"

class ThreadPool;

/// Максимальный размер ресурса (в байтах) с таблицей быстрого доступа для списка слов (включая заголовок)
#define SLD_MAX_LIST_QA_TABLE_RESOURCE_SIZE				(0xFFFF)

/// Максимальный размер ресурса (в байтах) с таблицей быстрого доступа для индексов списка слов (без заголовка)
#define SLD_MAX_LIST_INDEXES_QA_TABLE_RESOURCE_SIZE		(0xFFFF)

/// Максимальная глубина дерева поиска
#define SLD_MAX_SEARCH_TREE_DEPTH						(7)

struct TBinaryTreeElement
{
	TBinaryTreeElement(void) :
	WordIndex(-1),
	LeftLeafIndex(0),
	RightLeafIndex(0)
	{
	}

	Int32			WordIndex;
	std::wstring	CurrentWord;
	UInt16			LeftLeafIndex;
	UInt16			RightLeafIndex;
};

/// Класс занимается сжатием данных списков слов.
class CListCompress
{
public:
	/// Деструктор
	~CListCompress(void);

	/// Производит сжатие списка слов
	static std::pair<int, CListCompress*> Compress(CWordList &aList, ThreadPool &aThreadPool);

	/// Возвращаем сжатые данные.
	MemoryRef GetCompressedData() const;

	/// Возвращаем иерархию.
	MemoryRef GetCompressedHierarchy() const;

	/// Возвращаем ссылки на статьи
	MemoryRef GetIndexesData() const;

	/// Возвращаем ссылки на статьи
	MemoryRef GetIndexesCount() const;

	/// Возвращаем таблицу быстрого доступа ссылок на статьи
	MemoryRef GetIndexesQA() const;

	/// Возвращаем массив смещений каждого слова в списке
	MemoryRef GetDirectWordsShifts() const;

	/// Возвращаем массив точек для бинарного поиска
	MemoryRef GetSearchTreePoints() const;

	MemoryRef GetAlternativeHeadwordInfo() const;

	/// Возвращает указатель на заголовок хранилища индексов 
	const TIndexesHeader* GetIndexesHeader() const;

	/// Возвращаем количество деревьев.
	UInt32 GetCompressedTreeCount() const;
	/// Возвращаем дерево.
	int GetCompressedTree(UInt32 aTreeIndex, const UInt8 **aPtr, UInt32 *aSize) const;

	/// Возвращает максимальный размер слова в символах.
	UInt32 GetMaxWordSize() const;

private:
	/// Конструктор
	CListCompress(ICompressBy *aCompressor);

	/// Производит сжатие списка слов.
	int DoCompress(CWordList *aList);

	/// Добавляем индексы (фиксированое количество или динамическое не важно) переводов.
	int AddIndexes(const CWordList *aList, const ListElement &aListElem, UInt32 *aPrevQAPosition);
	/// Добавляем индексы (фиксированое количество или динамическое не важно) переводов, если список является списком слов полнотекстового поиска
	int AddFullTextSearchIndexes(const CWordList* aList, const ListElement &aListElem, UInt32* aPrevQAPosition);

	/// Добавляем медиа индексы (картинка, звук, видео, сцена) соответствующие данному элементу
	int AddMediaIndexes(const CWordList* aList, const ListElement &aListElem);

	/// Добавляет элемент дерева быстрого поиска по тексту
	int AddSearchLeaf(const CWordList *aList, Int32 aMinIndex, Int32 aMaxIndex, UInt32 aDepth);

	/// Добавляет кастомизированный вариант отбражения
	int AddStylizedVariant(const CWordList *aList, UInt32 aElementIndex, UInt32 aVariantIndex);

	/// Создает еще один элемент таблицы быстрого доступа для индексов если нужно
	void AddQAIndexesEntry(UInt32 *aPrevQAPosition);

	/// готовые данные.
	COut m_Out;

	/// иерархия списка слов
	COut m_OutHierarchy;

	/// индексы статей на которые ссылаются слова из списка слов - здесь хранится количество статей
	COut m_OutIndexesCount;

	/// индексы статей на которые ссылаются слова из списка слов - здесь хранятся данные индексов (номера статей, номера списков слов и т.д.)
	COut m_OutIndexesData;

	/// таблица быстрого доступа для индексов
	COut m_OutIndexesQA;

	/// массив смещений каждого слова в списке
	COut m_OutDirectWordsShifts;

	/// массив точек для бинарного поиска
	COut m_OutSearchTreePoints;

	/// массив индексов для альтернативных заголовков
	COut m_OutALternativeHeadwords;

	/// Количество бит которые закодированы в индексе.
	UInt32 m_IndexesQAInterleave;

	/// Количество символов в самом большом слове списка слов.
	UInt32 m_MaxWordSize;

	/// Указатель на конкретный способ сжатия данных.
	ICompressBy	*m_Compress;

	/// Таблица быстрого доступа для индексов переводов
	std::vector<TQAIndexesEntry>	m_IndexesTable;

	/// Заголовок для индексов
	TIndexesHeader	m_IndexesHeader;

	/// Вектор точек для бинарного поиска
	std::vector<TBinaryTreeElement> m_SearchTreePoints;

	/// Реальный текущий индекс слова (альтернативные заголовки не учитываются)
	Int32 m_RealWordIndex;

	/// Флаг того, что на одно слово списка слов может приходится несколько переводов
	bool m_isNumberOfArticlesPerItem;
};
