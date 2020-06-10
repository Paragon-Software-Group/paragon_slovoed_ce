#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "allocators.h"
#include "Article.h"
#include "util.h" // for RangeImpl

// ����� �������� ��� ������ �������
class CArticles
{
	friend class ArticlesLoader;
public:

	/// �����������
	CArticles(void);

	/// ���������� ������ � �������� ��������������� � ��������� �������.
	int SetPositionByID(UInt32 pos, const std::wstring &id);

	/// ���������� ����� �������� �� ��� ��������������.
	int GetPositionByID(const std::wstring &id, UInt32 &articleIndex) const;

	/// ���������� ������ �� �� �������
	const CArticle* GetArticleByIndex(UInt32 aArticleIndex) const;

	/// ���������� ������ �� �� ��������������
	const CArticle* GetArticleByID(const std::wstring &aId) const;

	/// ���������� ���������� ���������� ������.
	UInt32 GetNumberOfArticles() const;

	/// ������������� ������ ������ ������ ��� ������ (��� ���� ������)
	void SetCompressionMethod(CompressionConfig aConfig);

	/// ���������� ��������� ������ ������
	CompressionConfig GetCompressionMethod(void) const;

	/// ���������� ���������� ��� �������������� ������ ����� ������� ���������� ������� ����� �������� �������.
	UInt32 GetQAInterleave() const { return 4000; };

	/// ���������� ���� �� � ���� ������ ������
	bool HasEmptyArticles() const;

private:

	/// ��������� ������������ ������
	int addArticle(CArticle *aArticle);

	/// ������ ������
	std::vector<CArticle*> m_Articles;

	struct SortedItem
	{
		/// ��������� �� ������
		const CArticle *articlePtr;
		/// ����� ������ �� ����������
		UInt32		articleIndex;
	};
	bool isEmpty(const SortedItem&) const;

	/// ����� ��� �������� ������ ������ �� id
	std::unordered_map<std::wstring, SortedItem> m_ArticlesSortedMap;

	/// ��������� ������ � �������� ������ ������
	CompressionConfig m_compression;

	/// ��������� ������ ��� ������ � ������
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
