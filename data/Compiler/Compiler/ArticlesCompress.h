#pragma once
#include "Out.h"
#include "ICompressBy.h"

class CArticles;
class ThreadPool;
class TSlovoedStyleManager;

/// Максимальный размер ресурса (в байтах) с таблицей быстрого доступа для статей (включая заголовок)
#define SLD_MAX_ARTICLE_QA_TABLE_RESOURCE_SIZE		(0xFFFF)


/// Класс занимающийся сжатием переводов.
class CArticlesCompress
{
public:
	/// Конструктор
	CArticlesCompress(void) :
		m_Compress(NULL),
		m_maxWordTypeSize(0),
		m_maxTextSize(0)
	{};
	/// Деструктор
	~CArticlesCompress(void)
	{
		if (m_Compress)
			delete m_Compress;
	};

	/// Производим сжатие статей
	int DoCompress(const CArticles *aArticles, const TSlovoedStyleManager *aStyles, UInt32 aHASH, ThreadPool &aThreadPool);

	/// Возвращает количество деревьев(стилей) использованных при сжатии. 
	UInt32 GetCompressedTreeCount() const;
	
	/// Возвращает данные указанного дерева. 
	int GetCompressedTree(UInt32 aIndex, const UInt8 **aDataPtr, UInt32 *aDataSize) const;

	/// Возвращает данные с описанием стиля
	int GetStyleData(UInt32 aIndex, const UInt8 **aDataPtr, UInt32 *aDataSize) const;

	/// Возвращает сжатые данные
	MemoryRef GetCompressedData() const;

	/// Возвращает данные таблицы быстрого доступа.
	MemoryRef GetQA() const;

	/// Возвращает максимальный размер типа перевода в символах
	UInt32 GetMaxWordtypeSize() const;

	/// Возвращает максимальный размер буфера перевода в символах
	UInt32 GetMaxBufferSize() const;

	/// Возвращает количество стилей
	UInt16 GetStylesCount() const;

private:
	/// Производим заключительные действия после сжатия
	int FinalizeData(const TSlovoedStyleManager *aStyles);

private:
	/// Класс, занимающийся хранением сжатых данных
	COut m_Out;

	/// Класс, хранящий таблицу быстрого доступа
	COut m_OutQA;

	/// Класс, занимающийся сжатием данных
	ICompressBy *m_Compress;

	/// Массив указателей на готовые бинарные данные стилей
	std::vector<MemoryBuffer> m_StylesBinaryData;

	/// Максимальный размер буфера для типа перевода (в символах)
	UInt32 m_maxWordTypeSize;

	/// Максимальный размер буфера для текста перевода (в символах)
	UInt32 m_maxTextSize;
};
