#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "allocators.h"
#include "Article.h"
#include "util.h" // for RangeImpl

// Класс хранящий все статьи словаря
class CArticles
{
	friend class ArticlesLoader;
public:

	/// Конструктор
	CArticles(void);

	/// Перемещает статью с заданным идентификатором в указанную позицию.
	int SetPositionByID(UInt32 pos, const std::wstring &id);

	/// Возвращает номер перевода по его идентификатору.
	int GetPositionByID(const std::wstring &id, UInt32 &articleIndex) const;

	/// Возвращает статью по ее индексу
	const CArticle* GetArticleByIndex(UInt32 aArticleIndex) const;

	/// Возвращает статью по ее идентификатору
	const CArticle* GetArticleByID(const std::wstring &aId) const;

	/// Возвращает количество загруженых статей.
	UInt32 GetNumberOfArticles() const;

	/// Устанавливает способ сжатия данных для статей (для всех стилей)
	void SetCompressionMethod(CompressionConfig aConfig);

	/// Возвращает настройки сжатия статей
	CompressionConfig GetCompressionMethod(void) const;

	/// Возвращает количество бит закодированных данных через которые необходимо ставить точку быстрого доступа.
	UInt32 GetQAInterleave() const { return 4000; };

	/// Возвращает есть ли в базе пустые статьи
	bool HasEmptyArticles() const;

private:

	/// Добавляет распарсенную статью
	int addArticle(CArticle *aArticle);

	/// Массив статей
	std::vector<CArticle*> m_Articles;

	struct SortedItem
	{
		/// Указатель на статью
		const CArticle *articlePtr;
		/// Номер статьи до сортировки
		UInt32		articleIndex;
	};
	bool isEmpty(const SortedItem&) const;

	/// Мапка для быстрого поиска статьи по id
	std::unordered_map<std::wstring, SortedItem> m_ArticlesSortedMap;

	/// Описывает способ и качество сжатия данных
	CompressionConfig m_compression;

	/// Менеджеры памяти для статей и блоков
	sld::ArenaAllocator<BlockType> blockAllocator;
	sld::ArenaAllocator<BlockType::Metadata> metadataAllocator;
	sld::ArenaAllocator<CArticle> articleAllocator;

public:

	auto articles() -> sld::DereferencingRange<decltype(m_Articles.begin())>
	{
		return{ m_Articles.begin(), m_Articles.end() };
	}

	auto articles() const -> sld::DereferencingRange<decltype(m_Articles.begin())>
	{
		return{ m_Articles.begin(), m_Articles.end() };
	}
};
