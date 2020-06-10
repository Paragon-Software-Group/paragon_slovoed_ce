#include "Articles.h"

#include <algorithm>

#include "Log.h"
#include "sld2_Error.h"
#include "sld_Platform.h"
#include "SlovoEdProject.h"
#include "Tools.h"

static const CArticle emptyArticle;

// Конструктор
CArticles::CArticles(void)
	: m_compression(eCompressionTypeNoCompression, eCompressionOptimal),
	  blockAllocator((1 << 22) / sizeof(BlockType)), // 4MB
	  metadataAllocator((1 << 21) / sizeof(BlockType::Metadata)), // 2MB
	  articleAllocator((1 << 20) / sizeof(CArticle)) // 1MB
{}

// Возвращает количество загруженных статей
UInt32 CArticles::GetNumberOfArticles() const
{
	return (UInt32)m_Articles.size();
}

/**
 * Устанавливает способ сжатия данных для статей (для всех стилей)
 *
 * @param[in] aConfig - способ сжатия
 */
void CArticles::SetCompressionMethod(CompressionConfig aConfig)
{
	m_compression = aConfig;
}

/// Возвращает настройки сжатия статей
CompressionConfig CArticles::GetCompressionMethod(void) const
{
	return m_compression;
}

int CArticles::SetPositionByID(UInt32 pos, const wstring &aID)
{
	if (pos >= GetNumberOfArticles())
	{
		sldILog("Error! CArticles::SetPositionById : wrong index position!\n");
		return ERROR_WRONG_INDEX;
	}

	// Ищем статью с указанным идентификатором
	auto fit = m_ArticlesSortedMap.find(aID);

	// Если статья не найдена, выходим с сообщением об ошибке
	if (fit == m_ArticlesSortedMap.end())
	{
		sldILog("Error! Article not found! ArticleID = %s\n", sld::as_ref(aID));
		return ERROR_ARTICLE_NOT_FOUND;
	}

	if (isEmpty(fit->second))
		return ERROR_NO_ARTICLE;

	// Позиция статьи с указанным идентификатором
	UInt32 oldPos = fit->second.articleIndex;

	// Предполагается, что сортируя, мы последовательно перемещаем статьи и
	// назад не возвращаемся. Если такое случилось, значит имеются проблемы
	// и мы должны о них сообщить!
	if (oldPos < pos)
	{
		sldILog("Warning! Article moved twice! ArticleID = %s\n", sld::as_ref(aID));
	}

	// Проверяем id статьи, которую нужно поменять местами со статьей в oldPos
	const wstring &pId = m_Articles[pos]->GetID();
	auto fit2 = m_ArticlesSortedMap.find(pId);
	if (fit2 == m_ArticlesSortedMap.end())
	{
		sldILog("Error! Article not found! ArticleID = %s\n", sld::as_ref(pId));
		return ERROR_ARTICLE_NOT_FOUND;
	}

	if (fit2->second.articleIndex != pos)
	{
		sldILog("Error! Can't sort article with ArticleID = %s\n", sld::as_ref(pId));
		return ERROR_NO_ARTICLE;
	}

	// Производим обмен статьями, находящимися в pos и oldPos
	std::swap(m_Articles[pos], m_Articles[oldPos]);

	// И обновляем данные в поисковом массиве
	fit2->second.articleIndex = oldPos;
	fit->second.articleIndex = pos;

	return ERROR_NO;
}

/**
 * Возвращает индекс статьи по строковому id
 *
 * @param[in]  aID          - строковый id статьи
 * @param[out] articleIndex - числовой индекс статьи
 *
 * @return код ошибки:
 *             ERROR_NO - success
 *             ERROR_ARTICLE_NOT_FOUND - статья с запрашиваемый id не найдена
 *             ERROR_NO_ARTICLE - статья с запрашиваем id пустая
 */
int CArticles::GetPositionByID(const wstring &aID, UInt32 &articleIndex) const
{
	const auto fit = m_ArticlesSortedMap.find(aID);
	if (fit == m_ArticlesSortedMap.end())
	{
		sldILog("Error! Article not found! ArticleID = %s\n", sld::as_ref(aID));
		return ERROR_ARTICLE_NOT_FOUND;
	}

	if (isEmpty(fit->second))
		return ERROR_NO_ARTICLE;

	articleIndex = fit->second.articleIndex;
	return ERROR_NO;
}

/**
 * Возвращает статью по ее индексу.
 *
 * @param[in] aArticleIndex - индекс статьи
 *
 * @return указатель на объект со статьей или nullptr если статья с указанным индексом не найдена
 */
const CArticle* CArticles::GetArticleByIndex(UInt32 aArticleIndex) const
{
	return aArticleIndex < m_Articles.size() ? m_Articles[aArticleIndex] : nullptr;
}

/// Возвращает статью по ее идентификатору
const CArticle* CArticles::GetArticleByID(const std::wstring &aId) const
{
	const auto fit = m_ArticlesSortedMap.find(aId);
	return fit == m_ArticlesSortedMap.end() ? nullptr : fit->second.articlePtr;
}

int CArticles::addArticle(CArticle *aArticle)
{
	const wstring &articleId = aArticle->GetID();
	// Проверяем id статьи на уникальность, если статья с таким id уже есть - это недопустимо
	if (m_ArticlesSortedMap.find(articleId) != m_ArticlesSortedMap.end())
	{
		sldILog("Error! Duplicated article id: '%s'\n", sld::as_ref(articleId));
		return ERROR_DUPLICATED_ARTICLE_ID;
	}

	// Пустые статьи просто не добавляем
	if (aArticle->GetBlockCount() == 0)
	{
		sldILog("Warning! Empty article, id: '%s'\n", sld::as_ref(articleId));
		SortedItem item;
		item.articlePtr = &emptyArticle;
		item.articleIndex = ~0u;
		m_ArticlesSortedMap.emplace(articleId, item);
		return ERROR_NO_ARTICLE;
	}

	// Добавляем статью в массив для поиска
	SortedItem item;
	item.articlePtr = aArticle;
	item.articleIndex = static_cast<UInt32>(m_Articles.size());
	m_ArticlesSortedMap.emplace(articleId, item);

	// Добавляем статью в основной массив
	m_Articles.push_back(aArticle);
	return ERROR_NO;
}

/// Возвращает есть ли в базе пустые статьи
bool CArticles::HasEmptyArticles() const
{
	for (auto&& article : m_ArticlesSortedMap)
	{
		if (isEmpty(article.second))
			return true;
	}
	return false;
}

bool CArticles::isEmpty(const SortedItem &item) const
{
	return item.articlePtr == &emptyArticle && item.articleIndex == ~0u;
}
