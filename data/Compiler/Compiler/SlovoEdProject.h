#pragma once

#include "VersionInfo.h"
#include "sld_Types.h"
#include "sld2_Error.h"
#include "Articles.h"
#include "Styles.h"
#include "Log.h"
#include "WordLists.h"
#include "ImageManager.h"
#include "VideoManager.h"
#include "SoundManager.h"
#include "SceneManager.h"
#include "MeshManager.h"
#include "AbstractItemManager.h"
#include "MaterialManager.h"
#include "XmlTagContent.h"
#include "SortTablesManager.h"
#include "MetadataManager.h"
#include "MorphoDataManager.h"
#include "CSSDataManager.h"

#include <vector>
#include <wchar.h>
#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>

#include <stdio.h>
#include <list>
#include <sstream>
#include <set>
#include <map>
#include <iterator>

using namespace std;

class ArticlesLoader;
class CListLoader;

/**
	����� �������� ���������� �� ������� �������.
	������ ������� ����������� �� �����.
*/
class SlovoEdProject
{
public:

	/// �����������
	SlovoEdProject(void);
	/// ����������
	~SlovoEdProject(void);

	/// ��������� ���� �������
	int Load(wstring aFileName);
	
	/// ���������� ��� ����� ��� ������/���������� ���������� � ������ ��������� ����
	wstring GetVersionInfoFilename(void);
	/// ��������� ���������� � ������ ��������� ���� �� ���������� �����
	int LoadVersionInfo(const wstring &aFileName);
	/// ��������� ���������� � ������ ��������� ���� � ��������� ����
	int SaveVersionInfo(const wstring &aFileName);
	/// ���������� ��������� �� ��������� � ����������� � ������ ��������� ����
	const TDictionaryVersionInfo* GetDictionaryVersionInfo(void);

	/// ���������� ���������� ���������� ������� ����.
	UInt32 GetNumberOfLists() const;

	///	���������� ������ ������ ���� �� ��� ������ ����� ������ ���������� ������� ����
	CWordList* GetWordList(UInt32 index);
	const CWordList* GetWordList(UInt32 index) const;

	/// ��������� ����� ���� � �������
	int AddWordList(CWordList&& aList) { return m_WordLists.AddList(std::move(aList)); }

	/// ���������� ��������� �� ������ � ��������
	CArticles *GetArticles(void);

	/// ���������� ��������� �� ������ ��� ������ � ��������� ����������
	CSortTablesManager* GetSortTablesManager(void);

	/// ���������� ��������� �� ������ ��� ������ � ������ ����������
	CMorphoDataManager* GetMorphoDataManager(void);

	/// ���������� ��������� �� �������� ����������
	MetadataManager* GetMetadataManager();

	/// ���������� ��������� �� �������� css ����������
	CSSDataManager* GetCSSDataManager();

	/// ���������� ��������� ��������� �������.
	const TDictionaryHeader *GetDictionaryHeader();

	/// ���������� ��������� � �������������� ����������� ��������
	const TAdditionalInfo *GetAdditionalInfo();

	/// ���������� ��������� � ��������
	wstring GetAnnotation() const;
	
	/// ���������� ���������� ������� �������������� ����� (��������)
	UInt32 LocalizedNamesCount(void) const;
	/// ���������� ��������� �� ����� �������������� ����� (��������) �� ������
	TLocalizedNames* GetLocalizedNames(UInt32 aIndex);

	/// ���������� ���������� ������� � ��������������� ���������
	UInt32 GetStringsCount();

	/// ���������� ��������� �� �������������� ������� �� ������
	const TLocalizedNames * GetStrings(UInt32 aIndex);

	/// ���������� �������� ������
	TSlovoedStyleManager * GetStyles(void);

	/// ���������� �������� ��������
	CImageManager* GetImageManager(void);

	/// ���������� �������� �����
	CVideoManager* GetVideoManager(void);
	
	/// ���������� �������� �������
	CSoundManager* GetSoundManager(void);

	/// ���������� �������� �����
	CSceneManager* GetSceneManager(void);

	/// ���������� �������� �������
	CMeshManager* GetMeshManager(void);

	/// ���������� �������� ����������
	CMaterialManager* GetMaterialManager(void);

	/// ���������� �������� ����������
	CAbstractItemManager* GetAbstractItemManager(void);

	/// ������������� ������������ ������ ���� ��������
	int SetMaxWordtypeSize(UInt32 aMaxWordtypeSize);

	/// ������������� ������������ ������ ������� � ��������
	int SetMaxBufferSize(UInt32 aMaxBufferSize);
	
	/// ������������� ��� ����������� �������
	void SetDatabaseType(ESlovoEdContainerDatabaseTypeEnum aDatabaseType);
	/// �������� ��� ����������� �������
	ESlovoEdContainerDatabaseTypeEnum GetDatabaseType(void);

	/// ���������, ��������� �� ���� � ��� (�������, �����, ...)
	bool IsETBDatabase();
	
	/// ������������� ��� ��������� �������
	void SetSoundSourceType(EMediaSourceTypeEnum aType);
	/// �������� ��� ��������� �������
	EMediaSourceTypeEnum GetSoundSourceType(void);
	
	/// ������������� ��� ��������� ��������
	void SetImageSourceType(EMediaSourceTypeEnum aType);
	/// �������� ��� ��������� ��������
	EMediaSourceTypeEnum GetImageSourceType(void);

	/// ������������� ��� ��������� �����
	void SetVideoSourceType(EMediaSourceTypeEnum aType);
	/// �������� ��� ��������� �����
	EMediaSourceTypeEnum GetVideoSourceType(void);

	/// ������������� ��� ��������� ����
	void SetSceneSourceType(EMediaSourceTypeEnum aType);
	/// �������� ��� ��������� ����
	EMediaSourceTypeEnum GetSceneSourceType(void);

	/// ������������� ��� ��������� ����������
	void SetMaterialSourceType(EMediaSourceTypeEnum aType);
	/// �������� ��� ��������� ����������
	EMediaSourceTypeEnum GetMaterialSourceType(void);

	/// ������������� ��� ��������� ����������
	void SetMeshSourceType(EMediaSourceTypeEnum aType);
	/// �������� ��� ��������� ����������
	EMediaSourceTypeEnum GetMeshSourceType(void);
	
	/// ������������� ����� ������, ���������� ��� ������ � ���� ������� ����
	int SetCompressionMethod(wstring aMethod);
	CompressionConfig GetCompressionMethod() const { return m_compressionConfig; }

	/// ������������� ���� ����, ��� ��� ����-����
	void SetInApp(UInt32 aInAppFlag) { m_IsInApp = aInAppFlag; }
	/// ��������� ������ ��� ���� ��� ���
	UInt32 GetInApp(void);

	/// ���������� ������ ���� ������� html-������
	map<wstring, wstring>& GetHtmlSourceNames() { return m_ListEntryId_HtmlSourceName; }

	/// ���������� java script ���� ��� ������
	const std::wstring& GetJavaScriptFileContents() const { return m_javaScriptFileContents; }

private:
	
	/// ������ ������������� ������������ �������
	int PostProcessProject(ArticlesLoader &aArticlesLoader);

	// Posprocesses a list before adding it
	int PostProcessList(CListLoader &aList, std::vector<CWordList> &aSimpleSortedLists, ArticlesLoader &aArticlesLoader);

	/// ������������� ����������
	int PostProcessMetadata(ArticlesLoader &aArticlesLoader);

	/// ������������� ���������� � ������
	int PostProcessListsResources();

	/// ������������� ������ ����������
	int PostProcessSortTables(void);

	/// �������������� � �������� ������
	int PostProcessStyles();

	/// ������� � ���������� ��������� �������������� ����������
	TAdditionalInfo *AdditionalInfo();

	/// �������������� ���������� � ���������
	int DoPrepagePages();

public:

	/// ���� - ��� ����
	static map<wstring, UInt32> m_ExternContent[eExternContentTypeMax];
private:


	/// ��������� �������������� ������
	int AddStrings(pugi::xml_node aNode);
	/// ��������� ������
	int AddArticles(pugi::xml_node aNode, const wstring& aRoot, ArticlesLoader &aArticlesLoader);
	/// ��������� ������ ����
	int AddList(pugi::xml_node aNode, std::vector<CWordList> &aSimpleSortedLists, ArticlesLoader &aArticlesLoader);
	/// ��������� �������� �����
	int AddStyle(pugi::xml_node aNode, CImageContainer &aImages);
	/// ������������� ������ �������� �����
	int SetStyleVariantData(TSlovoedStyleVariant* aStyleVariant, pugi::xml_node aNode, CImageContainer &aImages);
	/// ��������� ���� � �������� ����������
	int AddSortFile(pugi::xml_node aNode, wstring aRoot);
	/// ��������� ���� � ��������������� �������� ����������
	int AddSecondarySortFile(pugi::xml_node aNode, const wstring& aRoot);
	/// ��������� ���� � ����� ����������
	int AddMorphoBase(pugi::xml_node aNode);

	/// ��������� �������� �������
	int AddDictionaryFeatures(pugi::xml_node aNode);
	/// ������������� ������������ �������� �������
	int SetFeature(ESldFeatures aFeature);
	/// ������������� ������������ ��� �����������
	int SetContentType(const ESldContentType aType);
	/// ������������� �������� ������� SkeletalMode
	int SetSkeletalMode(UInt32 aValue);
	/// ������������� �������� ������� Taboo
	int SetTaboo(UInt32 aValue);
	/// ������������� �������� ������� Stress
	int SetStress(UInt32 aValue);
	/// ������������� �������� ������� DarkTheme
	int SetDarkTheme(UInt32 aValue);

	/// ���������� ����������� ������ ����, ������� ����� ��� ��������� ���� ��� �������
	UInt32 GetDictionaryVersion();

	/// ��������� ������, ������������ � Atomic-���
	int ProcessAtomicReferenceLists(CWordList &aAtomicList);
	int DoPreparePages();

	static vector<wstring> initKeyWords();
	
private:

	/// ������ �������� ����
	static const vector<wstring> KeyWords;

	/// ������� �������������� ��������
	vector<TLocalizedNames> m_names;

	/// ��������� ������� � ��������� �������
	TDictionaryHeader m_header;

	/// ��������� � �������������� ����������� �� ����
	TAdditionalInfo *m_pAdditionalInfo;

	/// ��������� � ����
	wstring m_Annotation;
	
	/// ���������� � ������ ��������� ����
	TDictionaryVersionInfo m_VersionInfo;

	/// ������ ������ ������� ������ ���� � �������.
	CArticles m_Articles;
	
	/// ������ ��� ���������� � ������� �� ���� ������ �������
	TSlovoedStyleManager m_Styles;
	
	/// ������ ��� ���������� ����� ���������� �������
	CImageManager m_ImageManager;

	/// ������ ��� ���������� ����� ����� �������
	CVideoManager m_VideoManager;
	
	/// ������ ��� ���������� ����� ��������� �������
	CSoundManager m_SoundManager;

	/// ������ ��� ���������� ����� ������� �������
	CSceneManager m_SceneManager;

	/// ������ ��� ���������� ����� �������� �������
	CMeshManager m_MeshManager;

	/// ������ ��� ���������� ����� ������������ ��������� �������
	CAbstractItemManager m_AbstractItemManager;

	/// ������ ��� ���������� ����� 3d ����������� �������
	CMaterialManager m_MaterialManager;

	/// ������ ����
	CWordListManager m_WordLists;

	/// ������ ��� ������ � ��������� ����������
	CSortTablesManager m_SortTablesManager;

	/// ��� ����������� �������
	ESlovoEdContainerDatabaseTypeEnum m_DatabaseType;
	
	/// ��� ��������� �������
	EMediaSourceTypeEnum m_SoundSourceType;
	
	/// ��� ��������� ��������
	EMediaSourceTypeEnum m_ImageSourceType;

	/// ��� ��������� �����
	EMediaSourceTypeEnum m_VideoSourceType;

	/// ��� ��������� ���������� ����
	EMediaSourceTypeEnum m_SceneSourceType;

	/// ��� ��������� �������
	EMediaSourceTypeEnum m_MeshSourceType;

	/// ��� ��������� ����������
	EMediaSourceTypeEnum m_MaterialSourceType;

	/// ������ ��� ������ � ������ ����������
	CMorphoDataManager m_MorphoDataManager;

	// ���� ����, ��� � ���� ���� switch-����� � ����������
	UInt32 m_IsHasSwitchThematics;

	/// ���� ����, ��� ��� ����-����
	UInt32				m_IsInApp;

	/// ������ ���� ������� html-������
	map<wstring, wstring> m_ListEntryId_HtmlSourceName;

	/// �������� ����������
	MetadataManager m_MetadataManager;

	/// ���������������� ����� ������
	// ��� �� ������������ ��� ��������� ����� ������ ��� "��������" ��� ���� �������� ��������
	CompressionConfig m_compressionConfig;

	/// �������� css ����������
	CSSDataManager m_CSSDataManager;

	/// ���� ����, ��� ���� ������������� (��� ����� ������ �� ������ ��������� � ������ ����� ����)
	bool			m_IsMultiLanguageBase;

	/// ���������� java scrupt �����
	std::wstring	m_javaScriptFileContents;

public:

	auto lists()       -> decltype(m_WordLists.lists()) { return m_WordLists.lists(); }
	auto lists() const -> decltype(m_WordLists.lists()) { return m_WordLists.lists(); }

	auto articles()       -> decltype(m_Articles.articles()) { return m_Articles.articles(); }
	auto articles() const -> decltype(m_Articles.articles()) { return m_Articles.articles(); }

	auto styles()       -> decltype(m_Styles.styles()) { return m_Styles.styles(); }
	auto styles() const -> decltype(m_Styles.styles()) { return m_Styles.styles(); }
};
