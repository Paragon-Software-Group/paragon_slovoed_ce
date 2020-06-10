#pragma once
#include "Out.h"
#include "ICompressBy.h"

class CArticles;
class ThreadPool;
class TSlovoedStyleManager;

/// ������������ ������ ������� (� ������) � �������� �������� ������� ��� ������ (������� ���������)
#define SLD_MAX_ARTICLE_QA_TABLE_RESOURCE_SIZE		(0xFFFF)


/// ����� ������������ ������� ���������.
class CArticlesCompress
{
public:
	/// �����������
	CArticlesCompress(void) :
		m_Compress(NULL),
		m_maxWordTypeSize(0),
		m_maxTextSize(0)
	{};
	/// ����������
	~CArticlesCompress(void)
	{
		if (m_Compress)
			delete m_Compress;
	};

	/// ���������� ������ ������
	int DoCompress(const CArticles *aArticles, const TSlovoedStyleManager *aStyles, UInt32 aHASH, ThreadPool &aThreadPool);

	/// ���������� ���������� ��������(������) �������������� ��� ������. 
	UInt32 GetCompressedTreeCount() const;
	
	/// ���������� ������ ���������� ������. 
	int GetCompressedTree(UInt32 aIndex, const UInt8 **aDataPtr, UInt32 *aDataSize) const;

	/// ���������� ������ � ��������� �����
	int GetStyleData(UInt32 aIndex, const UInt8 **aDataPtr, UInt32 *aDataSize) const;

	/// ���������� ������ ������
	MemoryRef GetCompressedData() const;

	/// ���������� ������ ������� �������� �������.
	MemoryRef GetQA() const;

	/// ���������� ������������ ������ ���� �������� � ��������
	UInt32 GetMaxWordtypeSize() const;

	/// ���������� ������������ ������ ������ �������� � ��������
	UInt32 GetMaxBufferSize() const;

	/// ���������� ���������� ������
	UInt16 GetStylesCount() const;

private:
	/// ���������� �������������� �������� ����� ������
	int FinalizeData(const TSlovoedStyleManager *aStyles);

private:
	/// �����, ������������ ��������� ������ ������
	COut m_Out;

	/// �����, �������� ������� �������� �������
	COut m_OutQA;

	/// �����, ������������ ������� ������
	ICompressBy *m_Compress;

	/// ������ ���������� �� ������� �������� ������ ������
	std::vector<MemoryBuffer> m_StylesBinaryData;

	/// ������������ ������ ������ ��� ���� �������� (� ��������)
	UInt32 m_maxWordTypeSize;

	/// ������������ ������ ������ ��� ������ �������� (� ��������)
	UInt32 m_maxTextSize;
};
