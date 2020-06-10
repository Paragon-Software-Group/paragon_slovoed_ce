#pragma once

#include <string>
#include <vector>
#include <map>

#include "sld2_Error.h"
#include "sld_Types.h"
#include "MetadataManager.h" // for MetadataRef
#include "CSSDataManager.h" // mostly for InvalidCSSBlockIndex
#include "util.h" // for RangeImpl

using namespace std;

/// ��������� ����������� ���� ���� � ������
struct BlockType
{
	enum {
		/// list_entry_id ��� ������
		ListEntryId = 0,
		/// ������ ��� ���� ��������
		FlashCardListEntryIdFront,
		FlashCardListEntryIdBack,
		/// ������ ���� ��� ��������
		SlideShowList,
		/// ��� ������������� �������
		AtomicObjectId,
		/// ������������� ������ ��� ������ �������� �� article
		ArticleId,
		/// ������� �������
		ListMarkers,

		TextType_COUNT
	};

	struct Metadata
	{
		friend struct BlockType;
	public:
		/// "������" �� ����������
		MetadataRef ref;

		/// ������ css block'a
		uint32_t cssBlockIndex;

	private:
		/// ��������� ������ �����
		std::vector<std::wstring> stringData;

		/// ������� ��������� ������ �����
		UInt8 stringIndexes[TextType_COUNT];

		UInt8 textIndex(int aTextType) const
		{
			assert(aTextType < TextType_COUNT);
			return stringIndexes[aTextType];
		}

	public:
		Metadata() : cssBlockIndex(InvalidCSSBlockIndex) {
			memset(stringIndexes, 0xff, sizeof(stringIndexes));
		}
	};

public:

	// ��� ���������� �����
	ESldStyleMetaTypeEnum metaType;

	/// ����� �����
	UInt16 styleNumber;

	/// ���� ����, ��� ���� �������� ����������� ��������� (table, tr, td ...)
	bool isClosing;

	/// �����
	std::wstring text;

	/// ��������� �� ���������� ������������ �����
	// XXX: make private?
	Metadata *metadata_;

	/// ��������� ����� ������������� ����
	void AddText(int aTextType, sld::wstring_ref aText, bool aClear = false);

	/// ���������� ����� ������ ������������� ����
	size_t TextLength(int aTextType) const
	{
		if (sld2_unlikely(!metadata_)) return 0;
		const UInt8 index = metadata_->textIndex(aTextType);
		return index != 0xFF ? metadata_->stringData[index].length() : 0;
	}

	/// ���������� ���������� ������ ������������� ����
	const std::wstring& GetText(int aTextType) const
	{
		static const std::wstring dummy;
		if (sld2_unlikely(!metadata_)) return dummy;
		const UInt8 index = metadata_->textIndex(aTextType);
		return index != 0xFF ? metadata_->stringData[index] : dummy;
	}

	/// ���������� "������" �� ����������
	MetadataRef metadata() const { return metadata_ ? metadata_->ref : MetadataRef(); }

	/// ���������� ������ CSS �����
	UInt32 cssBlockIndex() const { return metadata_ ? metadata_->cssBlockIndex : InvalidCSSBlockIndex; }

	/// �����������
	BlockType(ESldStyleMetaTypeEnum aType, UInt16 aStyleNum, std::wstring&& aText, bool aIsClosing) :
		metaType(aType), styleNumber(aStyleNum), isClosing(aIsClosing), text(std::move(aText)), metadata_(nullptr)
	{}
};

/// ����� �������� ���������� � ������
class CArticle
{
	friend class ArticlesLoader;
public:

	/// �����������
	CArticle() {}

	// non-copyable
	CArticle(const CArticle&) = delete;
	CArticle& operator=(const CArticle&) = delete;

	/// ������������� ������������� ������.
	void setID(const std::wstring &text);
	/// ���������� ������������� ������
	const wstring& GetID() const { return m_id; };

	/// ���������� ���������� ������ � ������
	UInt32 GetBlockCount(void) const;
	
	/// ���������� ���� ������ �� ���������� ������
	BlockType* GetBlock(UInt32 aNumber);

	struct PageIndex
	{
		uint32_t pageNum;
		wstring labelId;
	};
	const vector<PageIndex>& GetPages() const { return m_Pages; }

	bool removeBlock(const BlockType *aBlock);

public:

	/// Map ������������ ������� ������ switch � ���������� ��������� � ��� (��� ��������)
	static std::map<UInt32, UInt32> m_ThematicToStatesNumberMap;

private:

	/// ������������� ������ �� �������� ����� ����������� �������� �� ������ ����.
	wstring				m_id;

	/// ����� � ������������� �������
	vector<BlockType*>	m_blocks;

	/// ������ �������, ����������� � ������
	vector<PageIndex>		m_Pages;

public:

	auto blocks() -> sld::DereferencingRange<decltype(m_blocks.begin())>
	{
		return{ m_blocks.begin(), m_blocks.end() };
	}

	auto blocks() const -> sld::DereferencingRange<decltype(m_blocks.begin())>
	{
		return{ m_blocks.begin(), m_blocks.end() };
	}
};
