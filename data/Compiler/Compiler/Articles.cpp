#include "Articles.h"

#include <algorithm>

#include "Log.h"
#include "sld2_Error.h"
#include "sld_Platform.h"
#include "SlovoEdProject.h"
#include "Tools.h"

static const CArticle emptyArticle;

// �����������
CArticles::CArticles(void)
	: m_compression(eCompressionTypeNoCompression, eCompressionOptimal),
	  blockAllocator((1 << 22) / sizeof(BlockType)), // 4MB
	  metadataAllocator((1 << 21) / sizeof(BlockType::Metadata)), // 2MB
	  articleAllocator((1 << 20) / sizeof(CArticle)) // 1MB
{}

// ���������� ���������� ����������� ������
UInt32 CArticles::GetNumberOfArticles() const
{
	return (UInt32)m_Articles.size();
}

/**
 * ������������� ������ ������ ������ ��� ������ (��� ���� ������)
 *
 * @param[in] aConfig - ������ ������
 */
void CArticles::SetCompressionMethod(CompressionConfig aConfig)
{
	m_compression = aConfig;
}

/// ���������� ��������� ������ ������
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

	// ���� ������ � ��������� ���������������
	auto fit = m_ArticlesSortedMap.find(aID);

	// ���� ������ �� �������, ������� � ���������� �� ������
	if (fit == m_ArticlesSortedMap.end())
	{
		sldILog("Error! Article not found! ArticleID = %s\n", sld::as_ref(aID));
		return ERROR_ARTICLE_NOT_FOUND;
	}

	if (isEmpty(fit->second))
		return ERROR_NO_ARTICLE;

	// ������� ������ � ��������� ���������������
	UInt32 oldPos = fit->second.articleIndex;

	// ��������������, ��� ��������, �� ��������������� ���������� ������ �
	// ����� �� ������������. ���� ����� ���������, ������ ������� ��������
	// � �� ������ � ��� ��������!
	if (oldPos < pos)
	{
		sldILog("Warning! Article moved twice! ArticleID = %s\n", sld::as_ref(aID));
	}

	// ��������� id ������, ������� ����� �������� ������� �� ������� � oldPos
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

	// ���������� ����� ��������, ������������ � pos � oldPos
	std::swap(m_Articles[pos], m_Articles[oldPos]);

	// � ��������� ������ � ��������� �������
	fit2->second.articleIndex = oldPos;
	fit->second.articleIndex = pos;

	return ERROR_NO;
}

/**
 * ���������� ������ ������ �� ���������� id
 *
 * @param[in]  aID          - ��������� id ������
 * @param[out] articleIndex - �������� ������ ������
 *
 * @return ��� ������:
 *             ERROR_NO - success
 *             ERROR_ARTICLE_NOT_FOUND - ������ � ������������� id �� �������
 *             ERROR_NO_ARTICLE - ������ � ����������� id ������
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
 * ���������� ������ �� �� �������.
 *
 * @param[in] aArticleIndex - ������ ������
 *
 * @return ��������� �� ������ �� ������� ��� nullptr ���� ������ � ��������� �������� �� �������
 */
const CArticle* CArticles::GetArticleByIndex(UInt32 aArticleIndex) const
{
	return aArticleIndex < m_Articles.size() ? m_Articles[aArticleIndex] : nullptr;
}

/// ���������� ������ �� �� ��������������
const CArticle* CArticles::GetArticleByID(const std::wstring &aId) const
{
	const auto fit = m_ArticlesSortedMap.find(aId);
	return fit == m_ArticlesSortedMap.end() ? nullptr : fit->second.articlePtr;
}

int CArticles::addArticle(CArticle *aArticle)
{
	const wstring &articleId = aArticle->GetID();
	// ��������� id ������ �� ������������, ���� ������ � ����� id ��� ���� - ��� �����������
	if (m_ArticlesSortedMap.find(articleId) != m_ArticlesSortedMap.end())
	{
		sldILog("Error! Duplicated article id: '%s'\n", sld::as_ref(articleId));
		return ERROR_DUPLICATED_ARTICLE_ID;
	}

	// ������ ������ ������ �� ���������
	if (aArticle->GetBlockCount() == 0)
	{
		sldILog("Warning! Empty article, id: '%s'\n", sld::as_ref(articleId));
		SortedItem item;
		item.articlePtr = &emptyArticle;
		item.articleIndex = ~0u;
		m_ArticlesSortedMap.emplace(articleId, item);
		return ERROR_NO_ARTICLE;
	}

	// ��������� ������ � ������ ��� ������
	SortedItem item;
	item.articlePtr = aArticle;
	item.articleIndex = static_cast<UInt32>(m_Articles.size());
	m_ArticlesSortedMap.emplace(articleId, item);

	// ��������� ������ � �������� ������
	m_Articles.push_back(aArticle);
	return ERROR_NO;
}

/// ���������� ���� �� � ���� ������ ������
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
