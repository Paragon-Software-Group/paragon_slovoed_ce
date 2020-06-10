#pragma once

#include <map>
#include <set>
#include <unordered_set>
#include <vector>

#include "sld_Types.h"
#include "sld2_Error.h"
#include "string_ref.h"
#include "util.h"

/// ������� ����, ��� ������ ������� �������� �������� � ��� ������ �� ��������� �������
#define LIST_END	(0xFFFFFFFFUL)

/// ������� ����, ��� ��������������� ��������� ��������� ���
#define STYLIZED_VARIANT_NO	(0xFFFFFFFFUL)

/// ������� ����, ��� ����� ���
#define MESH_NO	(0xFFFFFFFFUL)

/// ������� ����, ��� ���������� ���
#define MATERIAL_NO	(0xFFFFFFFFUL)


/// ���������, ����������� ������ �� ������� � ������ ����
struct ArticleRefStruct
{
	/// �����������
	ArticleRefStruct()
	{
		ID.clear();
		index = -1;
		shift = 0;
		initial_list_idx = -1;
		initial_list_entry_idx = -1;
	}

	/// ������������� �������� (������)
	std::wstring ID;

	/// ����� ��������
	UInt32 index;

	/// �������� � ������ (������������ ��� ������� ��������������� ������)
	UInt32 shift;

	/// ����� �������������� ������ ���� (����������), ��� ���� ������, ������� ��������� �� �� �� ��������, ��� � (list_idx, entry_idx)
	UInt32 initial_list_idx;

	/// ����� ������ � ������������� ������ ����
	UInt32 initial_list_entry_idx;
};


/// ���������, ����������� ������ �� ������ � ������ ����
struct TListEntryRef
{
	/// �����������
	TListEntryRef(void)
	{
		list_idx = -1;
		entry_idx = -1;
		translation_idx = -1;
		shift = -1;
	}

	/// ����� ������ ����
	UInt32 list_idx;

	/// ����� ������ � ������ ���� (����� �����)
	UInt32 entry_idx;

	/// ����� �������� � ������ ������ ����
	UInt32 translation_idx;

	/// �������� � ������
	UInt32 shift;
};


/// ��������� ��������� ��������� ������ ���� ��������������� ������
struct TFullTextSearchOptions
{
	UInt32 IsOptionsPresent;
	EFullTextSearchLinkType LinkType;
	UInt32 ShiftType;
	UInt32 Priority;
	bool isSuffixSearch;
	bool isFullMorphologyWordForms;
	UInt32 MorphologyId;
	/// ������ id ������� ����, �� ������� ����� ��������� ������
	std::set<std::wstring> mAllowedSourceListId;

	TFullTextSearchOptions()
	{
		IsOptionsPresent = 0;
		LinkType = eLinkType_ArticleId;
		ShiftType = 0;
		Priority = 0;
		isSuffixSearch = false;
		isFullMorphologyWordForms = false;
		MorphologyId = 0;
	}
};

/// ���������, ����������� ���� ������� ��������/������ ����.
struct ListElement
{
	/// ���������� �������������
	std::wstring Id;

	/// �������� ��������
	std::vector<std::wstring> Name;

	/// �������������� ���������
	std::set<std::wstring> AltName;

	/// �������� ����� �� �������
	std::map<UInt16, std::wstring> Styles;

	/// �������������� ������ (����� �� �������� � ���������� ������ ��� ������� ��������)
	std::vector<ArticleRefStruct> ArticleRefs;

	/// ������ �� ������ � ������� ����, ������������ ��� ������ ArticleRefs ��� ������� ��������������� ������
	std::vector<TListEntryRef> ListEntryRefs;

	/// ����� �������� ��� ������� �����. ���� �������� ���, ����� #PICTURE_NO
	UInt32 PictureIndex;

	/// ����� ����� ��� ������� �����. ���� ����� ���, ����� #VIDEO_NO
	UInt32 VideoIndex;

	/// ����� ������� ��� ������� �����. ���� ������� ���, ����� #SOUND_NO
	UInt32 SoundIndex;

	/// ����� ����� ��� ������� �����. ���� ����� ���, ����� #SCENE_NO
	UInt32 SceneIndex;

	/// ������� ������� ��������
	UInt32 Level;

	/// ���������� ��������� �� ���������� �������� ������
	UInt32 Shift2Next;

	/// ����, ����� �� ����������� ����� 
	bool childrenIsSorted;

	/// ��� ������ ��������
	EHierarchyLevelType HierarchyLevelType;

	/// ���� ����� - �������������� ��������� - ����� ����������� ID ���������, �� ������� ��������� ������ �����
	std::set<std::wstring> IDForAlternativeWords;

	/// ���� ����� - �������������� ��������� - ����� ����������� ������� ���������, �� ������� ��������� ������ �����
	std::set<UInt32> IndexesForAlternativeWords;

	std::map<UInt32, std::wstring> m_StylizedVariantsArticleIndexes;

	/// �����������
	ListElement(void)
	{
		PictureIndex = SLD_INDEX_PICTURE_NO;
		VideoIndex = SLD_INDEX_VIDEO_NO;
		SceneIndex = SLD_INDEX_SCENE_NO;
		SoundIndex = SLD_INDEX_SOUND_NO;
		Level = 0;
		Shift2Next = 0;
		childrenIsSorted = false;
		HierarchyLevelType = eLevelTypeNormal;
	}

	/// ���������� ������� � �������� ��������� ���������
	sld::wstring_ref GetName(UInt32 aVariant) const
	{
		return aVariant < Name.size() ? Name[aVariant] : sld::wstring_ref();
	}
};

struct ListVariantProperty
{
	/// ��� �������� ���������
	EListVariantTypeEnum Type;
	/// ��� �����
	UInt32 LangCode;

	ListVariantProperty(EListVariantTypeEnum aType, UInt32 aLangCode)
		: Type(aType), LangCode(aLangCode) {}
};

class CSortTablesManager;
class CStringCompare;

/// ����� �������� ������ � ����������� ��������� ������
class CWordList
{
	friend class CListLoader;
public:
	/// �����������
	CWordList(void);

	/// ������������� ListId
	void SetListId(std::wstring aId);
	/// ���������� ListId
	const std::wstring& GetListId(void) const;

	/// ���������� ���������� ������ �������������� �����
	UInt32 GetLocalizedListStringsCount(void) const;

	/// ��������� �������� �������� �� ��� ������������ �������� ���������
	int AddVariantProperty(UInt32 aNumber, const ListVariantProperty &aVariantProperty);

	/// ���������� ��������� �� ���������, ���������� �������� �������� ���������, �� ������� ��������
	const ListVariantProperty* GetVariantProperty(UInt32 aIndex) const;

	/// ���������� ���� �������������� ����� �� �������
	const TListLocalizedNames* GetLocalizedListStringsByIndex(UInt32 aIndex) const;

	/// ���������� ���������� ��������� � ������
	UInt32 GetNumberOfElements() const;

	/// ���������� ��������� �� ������� ������ ���� (const ������)
	const ListElement* GetListElement(UInt32 aIndex) const;

	/// ������� ������ �� �������
	bool RemoveArticleRef(UInt32 aElementIndex, UInt32 aArticleIndex);

	/// ���������� ��������� ��������������� ������
	const TFullTextSearchOptions* GetFullTextSearchOptions(void) const;

	/// ������������� ��������� ��������������� ������
	void SetFullTextSearchOptions(const TFullTextSearchOptions& aOptions);

	/// ������������� ��� ���������� ������.
	void SetType(UInt32 aType);
	/// ���������� ��� ���������� ������.
	UInt32 GetType(void) const;
	
	/// ������������� ���� ����, ��������� �� ������ ���� ����������
	void SetSorted(bool aIsSorted);
	/// ���������� ���� ����, ��������� �� ������ ���� ����������
	bool GetSorted() const;

	/// ������������� ���� ����, �������� �� ������ ����������� ���� �� ������ �����
	void SetLocalizedEntries(bool aIsLocalizedEntries);
	/// ���������� ���� ����, �������� �� ������ ����������� ���� �� ������ �����
	bool GetLocalizedEntries() const;

	/// ������������� ���� ����, ��� ������ ���� �������������
	void SetHierarchy(bool aIsHierarchy);
	/// ���������� ���� ����, ��� ������ ���� �������������
	bool GetHierarchy() const;

	/// ������������� ���� ����, ��� ������ ������ � ������ ������
	void SetLinkedList(bool aIsLinkedList);
	/// ���������� ���� ����, ��� ������ ������ � ������ ������
	bool GetLinkedList() const;

	/// ���������� ���� ����, ��� �� ���� ����� ������ ���� ����� ���������� ��������� ���������
	bool GetIsNumberOfArticles() const;

	/// ���������� ���������� �������� ������ ���� (���� ��������)
	int DoSort(const CStringCompare *aCompare);

	/// ������������� ���� ����, ��� ������ ������ ���� �������� ������������ � ������ � �� �� 
	/// ��������� � ��������, �.�. ��������� ������ � ���� �� ������ ���������(� �������� ����������
	/// ��������� � ������ ����).
	void SetDirectList(bool aIsDirectList);
	/// ���������� ���� ����, ��� ������ ������ ���� �������� ������������� ���������.
	bool GetDirectList() const;
	/// ���������� ���� ����, ��� ������ ������ ���� �������� ������� ���� ��������������� ������
	bool IsFullTextSearchList(void) const;
	/// ���������� ����, �������� �� ��� ������� ������ ���� ����� ������ ���� ��������������� ������
	bool IsFullTextSearchListType(void) const;
	/// ���������� ���� ����, ��� ������ ������ ���� ��������������� ������ �������� ����������
	UInt32 IsSuffixFullTextSearchList(void) const;
	/// ���������� ���� ����, �������� �� ������ ���� ��������������� ������ ��� ������ (����� ��������� �� ���� ����, ���� ���� ��� "�����"->������)
	bool IsFullTextSearchAuxillaryList(void) const;
	/// ���������� ����, �������� ��� ��� ������ ������ ��� ������� ����� ��� ����������
	UInt32 IsFullMorphologyWordFormsList(void) const;
	/// ���������� ID ��������������� ����, � ������ ������� ��������� ������ ������
	/// � ������ ������ ���� ������ ������� �����
	UInt32 GetMorphologyId(void) const;
	
	/// ���������� ����, ��������� ��� ��� �� ���� ������� (������) ��������� ��������� ������ ����
	/// ���� ��, �� � ����� ������ �� ����� ������ ���� ������ ����������� ������
	bool IsOneArticleReferencedByManyListEntries() const;

	/// ���������� ���������� ��������� ��������� ������� ���������� ��� ������ ����� �� ������ ����.
	UInt32 GetNumberOfVariants() const;

	/// ���������� ���������� ��� ��� ������������� ���������� ��������� � �������
	UInt32 GetNumberOfArticlesSize(void) const;
	/// ������������� ���������� ��� ��� ������������� ���������� ��������� � �������
	void SetNumberOfArticlesSize(UInt32 aCount);

	/// ������������� ���� ����, ��� � ������ ������ ���� ������������ ��������
	void SetPicture(bool aIsListHasIcons);
	/// ���������� ���� ����, ��� � ������ ������ ���� ������������ ��������
	bool GetPicture() const;

	/// ������������� ���� ����, ��� � ������ ������ ���� ������������ �����
	void SetVideo(bool aIsListHasVideo);
	/// ���������� ���� ����, ��� � ������ ������ ���� ������������ �����
	bool GetVideo() const;

	/// ������������� ���� ����, ��� � ������ ������ ���� ������������ �����
	void SetScene(bool aIsListHasScene);
	/// ���������� ���� ����, ��� � ������ ������ ���� ������������ �����
	bool GetScene() const;

	/// ���������� ���������� ��� ��� ������������� ������� ��������
	UInt32 GetPictureIndexSize() const;
	/// ������������� ���������� ��� ��� ������������� ������� ��������
	void SetPictureIndexSize(UInt32 aIndexSize);

	/// ���������� ���������� ��� ��� ������������� ������� �����
	UInt32 GetVideoIndexSize() const;
	/// ������������� ���������� ��� ��� ������������� ������� �����
	void SetVideoIndexSize(UInt32 aIndexSize);

	/// ���������� ���������� ��� ��� ������������� ������� �����
	UInt32 GetSceneIndexSize() const;
	/// ������������� ���������� ��� ��� ������������� ������� �����
	void SetSceneIndexSize(UInt32 aIndexSize);
	
	/// ������������� ���� ����, ��� � ������ ������ ���� ������������ �������
	void SetSound(bool aIsListHasSounds);
	/// ���������� ���� ����, ��� � ������ ������ ���� ������������ �������
	bool GetSound() const;
	/// ���������� ���������� ��� ��� ������������� ������� �������
	UInt32 GetSoundIndexSize() const;
	/// ������������� ���������� ��� ��� ������������� ������� �������
	void SetSoundIndexSize(UInt32 aIndexSize);

	/// ���������� �������� �������������� ��� ��������
	MemoryRef GetHierarchyData() const;

	/// ���������� ������ ������ �� �������(� �����).
	UInt32 GetAticleIndexSize() const;
	/// ������������� ������ ������ �� �������(� �����).
	void SetArticleIndexSize(UInt32 aIndexSize);

	/// ������������� ��������� ������ ������
	void SetCompressionMethod(CompressionConfig aConfig);
	/// ���������� ��������� ������ ������
	CompressionConfig GetCompressionMethod(void) const;
	
	/// ���������� ��� ����� � �������� ������������ ������� � ������ ������ ����.
	UInt32 GetLanguageCodeFrom() const;
	/// ������������� ��� ���� � �������� ������������ ������� � ������ ������ ����.
	void SetLanguageCodeFrom(UInt32 aLanguageCodeFrom);

	/// ���������� ��� ����� �� ������� ������������ ������� � ������ ������ ����.
	UInt32 GetLanguageCodeTo() const;
	/// ������������� ��� ���� �� ������� ������������ ������� � ������ ������ ����.
	void SetLanguageCodeTo(UInt32 aLanguageCodeTo);

	/// ��������� ����������� ������.
	int PostProcess();
	
	/// ������������� ���������� ���, ����������� ��� ������������� ������ ������, �� ������� ��������� ������ � ��������� ������
	void SetFullTextSearchLinkListIndexSize(UInt32 aValue) { m_FullTextSearch_LinkListIndexSize = aValue; };
	/// ���������� ���������� ���, ����������� ��� ������������� ������ ������, �� ������� ��������� ������ � ��������� ������
	UInt32 GetFullTextSearchLinkListIndexSize(void) const { return m_FullTextSearch_LinkListIndexSize; };
	
	/// ������������� ����� ������, �� ������� ��������� ��� ������ � ��������� ������ � ������, ���� ��� ���������� �� ����������� ��������������� � ������
	void SetFullTextSearchDefaultLinkListIndex(UInt32 aValue) { m_FullTextSearch_DefaultLinkListIndex = aValue; };
	/// ���������� ����� ������, �� ������� ��������� ��� ������ � ��������� ������ � ������, ���� ��� ���������� �� ����������� ��������������� � ������
	UInt32 GetFullTextSearchDefaultLinkListIndex(void) const { return m_FullTextSearch_DefaultLinkListIndex; };
	
	/// ������������� ���������� ���, ����������� ��� ������������� ������ ����� � ������ ����, �� ������� ��������� ������ � ��������� ������
	void SetFullTextSearchLinkListEntryIndexSize(UInt32 aValue) { m_FullTextSearch_LinkListEntryIndexSize = aValue; };
	/// ���������� ���������� ���, ����������� ��� ������������� ������ ����� � ������ ����, �� ������� ��������� ������ � ��������� ������
	UInt32 GetFullTextSearchLinkListEntryIndexSize(void) const { return m_FullTextSearch_LinkListEntryIndexSize; };
	
	/// ������������� ���������� ���, ����������� ��� ������������� ������ �������� � ������ ������ ����, �� ������� ��������� ������ � ��������� ������
	void SetFullTextSearchLinkTranslationIndexSize(UInt32 aValue) { m_FullTextSearch_LinkTranslationIndexSize = aValue; };
	/// ���������� ���������� ���, ����������� ��� ������������� ������ �������� � ������ ������ ����, �� ������� ��������� ������ � ��������� ������
	UInt32 GetFullTextSearchLinkTranslationIndexSize(void) const { return m_FullTextSearch_LinkTranslationIndexSize; };
	
	/// ������������� ����� ��������, �� ������� ��������� ��� ������ � ��������� ������ � ������, ���� ��� ���������� �� ����������� ��������������� � ������
	void SetFullTextSearchDefaultLinkTranslationIndex(UInt32 aValue) { m_FullTextSearch_DefaultLinkTranslationIndex = aValue; };
	/// ���������� ����� ��������, �� ������� ��������� ��� ������ � ��������� ������ � ������, ���� ��� ���������� �� ����������� ��������������� � ������
	UInt32 GetFullTextSearchDefaultLinkTranslationIndex(void) const { return m_FullTextSearch_DefaultLinkTranslationIndex; };
	
	/// ���������� ��� ������ � ��������� ������
	UInt32 GetFullTextSearchShiftType(void) const { return m_FullTextSearchOptions.ShiftType; };
	
	/// ������������� ���������� ���, ����������� ��� ������������� �������� �������� � ������, �� ������� ��������� ������ � ��������� ������
	void SetFullTextSearchShiftIndexSize(UInt32 aValue) { m_FullTextSearch_ShiftIndexSize = aValue; };
	/// ���������� ���������� ���, ����������� ��� ������������� �������� �������� � ������, �� ������� ��������� ������ � ��������� ������
	UInt32 GetFullTextSearchShiftIndexSize(void) const { return m_FullTextSearch_ShiftIndexSize; };

	/// �������� ����� ���������� �������� � ������ ����
	UInt32 GetTotalPictureCount(void) const;
	/// ������������� ����� ���������� �������� � ������ ����
	void SetTotalPictureCount(UInt32 aCount);

	/// �������� ����� ���������� ����� � ������ ����
	UInt32 GetTotalVideoCount(void) const;
	/// ������������� ����� ���������� ����� � ������ ����
	void SetTotalVideoCount(UInt32 aCount);

	/// �������� ����� ���������� ���� � ������ ����
	UInt32 GetTotalSceneCount(void) const;
	/// ������������� ����� ���������� ���� � ������ ����
	void SetTotalSceneCount(UInt32 aCount);

	/// �������� ����� ���������� ������� � ������ ����
	UInt32 GetTotalSoundCount(void) const;
	/// ������������� ����� ���������� ������� � ������ ����
	void SetTotalSoundCount(UInt32 aCount);

	/// �������� ��� �������� ������
	UInt32 GetAlphabetType() const { return m_AlphabetType; };
	/// ������������� ��� �������� � ������
	void SetAlphabetType(EAlphabetType aAlphabetType);

	/// �������� ������ ����� � ������ ���� �� id �����
	UInt32 GetIndexByName(sld::wstring_ref aName) const;

	/// �������� ������ ������, ��� �������� ������ ������ �������� ���������������
	Int32 GetSimpleSortedListIndex() const { return m_SimpleSortedListIndex; };

	/// ������������� ������ ������, ��� �������� ������ ������ �������� ���������������
	void SetSimpleSortedListIndex(Int32 aListIndex);

	/// ���������� ��������������� ������� ���� � ������������� ������ ������ ��������� ���������
	int DoSimpleSortedList(const CWordList *aNotSortedList);

	/// ����������� ������ �� ��������������� ������������� ������
	int ConvertListToSimpleSortedList();

	/// ���������� ���������� ��������� ���������
	size_t GetVariantPropertyCount() const { return m_VariantsProperty.size(); };

		/// ������������� ���� ����, ��� ������ ����� �������������� ���������
	void SetAlternativeHeadwordsFlag(bool aFlag) { m_HasAlternativeHeadwords = aFlag; }
	/// ���������� ���� ����, ��� ������ ����� �������������� ���������
	bool GetAlternativeHeadwordsFlag() const { return m_HasAlternativeHeadwords; }

	/// ��������� �������������� ��������� � ������
	int AddAlternativeHeadwords();

	/// ��������� �������������� ����������
	int PostProccessAlternativeHeadwords();

	/// ���������� ���������� ����, ��� ����� ��������������� ����������
	UInt32 GetRealWordCount() const { return m_RealNumberOfWords; }
	/// ������������� ���������� ����, ��� ����� ��������������� ����������
	void SetRealWordCount(UInt32 aWordsCount) { m_RealNumberOfWords = aWordsCount; }

	/// ��������� ������� ��������������� ���������� � ������ ����
	bool HasStylizedVariants() const { return m_StylizedVariantsArticlesCount != 0; }

	/// ���������� ������ ������ � ��������������� ��������� ���������
	UInt32 GetStylizedVariantArticleIndex(UInt32 aWordIndex, UInt32 aVariantIndex) const;
	/// ������������� ������ ������ � ��������������� ��������� ���������
	void SetStylizedVariantArticleIndex(UInt32 aWordIndex, UInt32 aVariantIndex, UInt32 aArticleIndex) { m_StylizedVariantsMap[aWordIndex][aVariantIndex] = aArticleIndex; }

	/// ��������� ������� �������� ������ � ������ ����
	bool HasListEntryId(const std::wstring &aListEntryID) const { return m_mListEntryId.find(aListEntryID) != m_mListEntryId.end(); }

	/// ��������� ������ � ������ ������-������
	int AddAtomicArticle(sld::wstring_ref aAtomicID, const std::vector<std::wstring>& aExternID);

	/// ������������� ��� ����� � ��������������� ���������
	void SetResourceFolderName(const std::wstring& aFolderName) { m_ResourceFolderName = aFolderName; }
	/// ���������� ��� ����� � ��������������� ���������
	const std::wstring& GetResourceFolderName() const { return m_ResourceFolderName; }

	/// ������������� ������ ��������, �� ������� ����� ������������� �����
	void SetSearchRange(ESldSearchRange aSearchRange) { m_SearchRange = aSearchRange; }
	/// ���������� ������ ��������, �� ������� ����� ������������� �����
	ESldSearchRange GetSearchRange() const { return m_SearchRange; }

private:

	/// ���������� ���������� ������ ���� � ������������ ������.
	int PrepareHierarchy();

	/// ����������� ����� ������������ ���, ��� ���������� ����� ������ ��������.
	int CreateHierarchy(std::vector<ListElement> &newList,
						std::vector<ListElement> &oldList, UInt32 startPos, UInt32 endPos,
						UInt8 *hierarchy, UInt32 *hierarchyStart, UInt8 aSortedFlag, EHierarchyLevelType aType);

	/// ���������� ����� ���������� ����� ��������� �����
	int AddListElement(ListElement&& aElem);

private:

	/// ������������� ������ ����, ������������ ��� ���������� ���� ����������� (��� ���������� ������� ���� ��������������� ������)
	std::wstring m_ListId;

	/// ������ ���������, �� ������� ������� ������
	std::vector<ListElement>	m_List;
	/// ������ id ��������� ������ ����, ��� �������� ������������ id
	std::unordered_set<std::wstring> m_mListEntryId;

	/// ������� �������������� �������� ������ ����
	std::vector<TListLocalizedNames> m_LocalizedNames;

	/// ������� ������� ��������� ���������
	std::vector<ListVariantProperty> m_VariantsProperty;

	/// ����� ����� �������� ������� ��������(� �������� ����)
	std::vector<uint8_t> m_hierarchy;

	//////////////////////////////////////////////////////////////////////////
	// �������� ������ ���� - ����� ������� ��������/��������� ��������� �����������
	//////////////////////////////////////////////////////////////////////////

	/// ��� ���������� ������ (EWordListTypeEnum)
	UInt32 m_Usage;
	/// ��������� �� ������ ���� ����������
	bool m_isSorted;
	/// ���� ����, ��� ������ ���� �������������
	bool m_isHierarchy;
	/// ���� ����, ��� ������ ������ ���� �������� ����������� � ����������.
	/**
		������ ������ ���� �������� ������������ � ������ � �� �� ��������� � ��������, 
		�.�. ��������� ������ � ���� �� ������ ���������(� �������� ���������� ��������� 
		� ������ ����).
	*/
	bool m_isDirectList;

	/// ���� ����, ��� ��������� ������ �� ������ ���� ������������ ��������
	bool m_isPicture;

	/// ���� ����, ��� ��������� ������ �� ������ ���� ������������ �����
	bool m_isVideo;

	/// ���� ����, ��� ��������� ������ �� ������ ���� ������������ �����
	bool m_isScene;

	/// ���� ����, ��� ��������� ������ �� ������ ���� ������������ �������
	bool m_isSound;

	/// ���� ����, ��� ������ ����� ��������� ����������� ��� ����
	bool m_isLocalizedEntriesList;

	/// ���� ����, ��� ������ ������ � ������ ������
	bool m_isLinkedList;

	/// ���������� ��� ����������� ��� ������������� ����� ��������� �� ������� ��������� ���� ������.
	UInt32 m_NumberOfArticlesSize;

	/// ���������� ��� ����������� ��� ������������� ������� ��������
	UInt32 m_PictureIndexSize;

	/// ���������� ��� ����������� ��� ������������� ������� �����
	UInt32 m_VideoIndexSize;

	/// ���������� ��� ����������� ��� ������������� ������� �����
	UInt32 m_SceneIndexSize;

	/// ���������� ��� ����������� ��� ������������� ������� �������
	UInt32 m_SoundIndexSize;

	/// ������ ������ �� ������� � �����.
	UInt32 m_AticleIndexSize;

	/// ��� �����, � �������� ������������ ������� � ������ ������ ����.
	UInt32 m_LanguageCodeFrom;

	/// ��� �����, �� ������� ������������ ������� � ������ ������ ����.
	UInt32 m_LanguageCodeTo;

	/// ��� �������� ������
	UInt32 m_AlphabetType;

	/// ������ ������ ������
	CompressionConfig m_compressionConfig;

	/// ��������� ������, ���� ������ �������� ������� ���� ��������������� ������
	TFullTextSearchOptions m_FullTextSearchOptions;
	
	/// ���������� ���, ����������� ��� ������������� ������ ������, �� ������� ��������� ������ � ��������� ������
	UInt32 m_FullTextSearch_LinkListIndexSize;
	/// ����� ������, �� ������� ��������� ��� ������ � ��������� ������ � ������, ���� ��� ���������� �� ����������� ��������������� � ������
	UInt32 m_FullTextSearch_DefaultLinkListIndex;
	/// ���������� ���, ����������� ��� ������������� ������ ����� � ������ ����, �� ������� ��������� ������ � ��������� ������
	UInt32 m_FullTextSearch_LinkListEntryIndexSize;
	/// ���������� ���, ����������� ��� ������������� ������ �������� � ������ ������ ����, �� ������� ��������� ������ � ��������� ������
	UInt32 m_FullTextSearch_LinkTranslationIndexSize;
	/// ����� ��������, �� ������� ��������� ��� ������ � ��������� ������ � ������, ���� ��� ���������� �� ����������� ��������������� � ������
	UInt32 m_FullTextSearch_DefaultLinkTranslationIndex;
	/// ���������� ���, ����������� ��� ������������� �������� �������� � ������, �� ������� ��������� ������ � ��������� ������
	UInt32 m_FullTextSearch_ShiftIndexSize;

	/// ����� ���������� �������� � ������ ����
	/// ����������� �������� ����� ���� ������ (������) � �������� � ���������, �� ������� ��������� ����� ������
	UInt32 m_TotalPictureCount;

	/// ����� ���������� ����� � ������ ����
	/// ����������� ����� � ���������, �� ������� ��������� ����� ������
	UInt32 m_TotalVideoCount;

	/// ����� ���������� ���� � ������ ����
	/// ����������� ����� � ��������� (3d ����� ���� ������ � ��������), �� ������� ��������� ����� ������
	UInt32 m_TotalSceneCount;

	/// ����� ���������� ����� ����������� � ������ ����
	/// ��������� ����� ���������� � ��������� (����� ��������� ����� ���� ������ � ��������)
	UInt32 m_TotalMediaContainerCount;

	/// ����� ���������� ������� � ������ ����
	/// ����������� ������� ����� ���� ������ � ������� � ���������, �� ������� ��������� ����� ������
	UInt32 m_TotalSoundCount;
	
	/// ���������� �� ������� ������� ��������
	bool m_ZeroLevelSorted;

	/// ������ ������, ��� �������� ������ ������ �������� ���������������
	Int32 m_SimpleSortedListIndex;

	/// ���� ����, ��� ������ ����� �������������� ���������
	bool m_HasAlternativeHeadwords;

	/// ���������� ����, ��� ����� ��������������� ����������
	UInt32 m_RealNumberOfWords;

	/// ���������� ��������������� ��������� ���������
	UInt32 m_StylizedVariantsArticlesCount;

	/// ������� ������������ �������� ������� � ������ ���� � �������� ������ � ���������������� ���������� ��������� map<wordIndex, map<variantIndex, articleIndex>
	std::map<UInt32, std::map<UInt32, UInt32>> m_StylizedVariantsMap;

	/// ��� ����� � ��������������� ���������
	std::wstring m_ResourceFolderName;

	/// ���������� ��������� ������ ��������, �� ������� ����� ������������� ����� �� #ESldSearchRange
	ESldSearchRange m_SearchRange;

public:

	auto elements() -> RangeImpl<decltype(m_List.begin())>
	{
		return{ m_List.begin(), m_List.end() };
	}

	auto elements() const -> RangeImpl<decltype(m_List.begin())>
	{
		return{ m_List.begin(), m_List.end() };
	}
};
