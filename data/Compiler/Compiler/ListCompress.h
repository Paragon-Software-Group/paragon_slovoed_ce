#pragma once

#include "sld2_Error.h"
#include "sld_Types.h"
#include "WordList.h"
#include "Out.h"
#include "ICompressBy.h"

class ThreadPool;

/// ������������ ������ ������� (� ������) � �������� �������� ������� ��� ������ ���� (������� ���������)
#define SLD_MAX_LIST_QA_TABLE_RESOURCE_SIZE				(0xFFFF)

/// ������������ ������ ������� (� ������) � �������� �������� ������� ��� �������� ������ ���� (��� ���������)
#define SLD_MAX_LIST_INDEXES_QA_TABLE_RESOURCE_SIZE		(0xFFFF)

/// ������������ ������� ������ ������
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

/// ����� ���������� ������� ������ ������� ����.
class CListCompress
{
public:
	/// ����������
	~CListCompress(void);

	/// ���������� ������ ������ ����
	static std::pair<int, CListCompress*> Compress(CWordList &aList, ThreadPool &aThreadPool);

	/// ���������� ������ ������.
	MemoryRef GetCompressedData() const;

	/// ���������� ��������.
	MemoryRef GetCompressedHierarchy() const;

	/// ���������� ������ �� ������
	MemoryRef GetIndexesData() const;

	/// ���������� ������ �� ������
	MemoryRef GetIndexesCount() const;

	/// ���������� ������� �������� ������� ������ �� ������
	MemoryRef GetIndexesQA() const;

	/// ���������� ������ �������� ������� ����� � ������
	MemoryRef GetDirectWordsShifts() const;

	/// ���������� ������ ����� ��� ��������� ������
	MemoryRef GetSearchTreePoints() const;

	MemoryRef GetAlternativeHeadwordInfo() const;

	/// ���������� ��������� �� ��������� ��������� �������� 
	const TIndexesHeader* GetIndexesHeader() const;

	/// ���������� ���������� ��������.
	UInt32 GetCompressedTreeCount() const;
	/// ���������� ������.
	int GetCompressedTree(UInt32 aTreeIndex, const UInt8 **aPtr, UInt32 *aSize) const;

	/// ���������� ������������ ������ ����� � ��������.
	UInt32 GetMaxWordSize() const;

private:
	/// �����������
	CListCompress(ICompressBy *aCompressor);

	/// ���������� ������ ������ ����.
	int DoCompress(CWordList *aList);

	/// ��������� ������� (������������ ���������� ��� ������������ �� �����) ���������.
	int AddIndexes(const CWordList *aList, const ListElement &aListElem, UInt32 *aPrevQAPosition);
	/// ��������� ������� (������������ ���������� ��� ������������ �� �����) ���������, ���� ������ �������� ������� ���� ��������������� ������
	int AddFullTextSearchIndexes(const CWordList* aList, const ListElement &aListElem, UInt32* aPrevQAPosition);

	/// ��������� ����� ������� (��������, ����, �����, �����) ��������������� ������� ��������
	int AddMediaIndexes(const CWordList* aList, const ListElement &aListElem);

	/// ��������� ������� ������ �������� ������ �� ������
	int AddSearchLeaf(const CWordList *aList, Int32 aMinIndex, Int32 aMaxIndex, UInt32 aDepth);

	/// ��������� ����������������� ������� ����������
	int AddStylizedVariant(const CWordList *aList, UInt32 aElementIndex, UInt32 aVariantIndex);

	/// ������� ��� ���� ������� ������� �������� ������� ��� �������� ���� �����
	void AddQAIndexesEntry(UInt32 *aPrevQAPosition);

	/// ������� ������.
	COut m_Out;

	/// �������� ������ ����
	COut m_OutHierarchy;

	/// ������� ������ �� ������� ��������� ����� �� ������ ���� - ����� �������� ���������� ������
	COut m_OutIndexesCount;

	/// ������� ������ �� ������� ��������� ����� �� ������ ���� - ����� �������� ������ �������� (������ ������, ������ ������� ���� � �.�.)
	COut m_OutIndexesData;

	/// ������� �������� ������� ��� ��������
	COut m_OutIndexesQA;

	/// ������ �������� ������� ����� � ������
	COut m_OutDirectWordsShifts;

	/// ������ ����� ��� ��������� ������
	COut m_OutSearchTreePoints;

	/// ������ �������� ��� �������������� ����������
	COut m_OutALternativeHeadwords;

	/// ���������� ��� ������� ������������ � �������.
	UInt32 m_IndexesQAInterleave;

	/// ���������� �������� � ����� ������� ����� ������ ����.
	UInt32 m_MaxWordSize;

	/// ��������� �� ���������� ������ ������ ������.
	ICompressBy	*m_Compress;

	/// ������� �������� ������� ��� �������� ���������
	std::vector<TQAIndexesEntry>	m_IndexesTable;

	/// ��������� ��� ��������
	TIndexesHeader	m_IndexesHeader;

	/// ������ ����� ��� ��������� ������
	std::vector<TBinaryTreeElement> m_SearchTreePoints;

	/// �������� ������� ������ ����� (�������������� ��������� �� �����������)
	Int32 m_RealWordIndex;

	/// ���� ����, ��� �� ���� ����� ������ ���� ����� ���������� ��������� ���������
	bool m_isNumberOfArticlesPerItem;
};
