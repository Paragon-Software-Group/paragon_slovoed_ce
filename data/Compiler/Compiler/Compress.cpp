#include "Compress.h"

#include <memory>
#include <unordered_map>
#include <unordered_set>

#include "ArticlesCompress.h"
#include "ListCompress.h"
#include "MyProcessTimer.h"
#include "sld_Platform.h"
#include "SlovoEdProject.h"
#include "SpeexPack.h"
#include "StringCompare.h"
#include "ThreadPool.h"
#include "Tools.h"

namespace {

/// Класс занимающийся упаковкой словарей.
class CCompress
{
public:

	CCompress(ThreadPool &threadPool, SlovoEdProject *project, const CompressConfig &aConfig);
	~CCompress(void);

	/// Производит сжатие и упаковку словаря в контейнер.
	int DoCompress();

	/// Производит сортировку списков
	int DoSortLists();

	/// Производит сортировку статей основываясь на том, как отсортированы списки слов.
	int DoSortArticles();

	/// Производит связывание спиков слов и переводы.
	int AssociateListWithArticles();

	/// Производит связывание элементарых объектов с переводами
	int AssociateAtomicObjectsWithArticles();

	/// Производит обработку внутренних ссылок словаря
	int DoProcessLinks(void);

	/// Производим упаковку списков слов.
	int DoCompressLists();

	/// Сжимаем переводы
	int DoCompressArticles();

	/// Добавляем все ресурсы словаря в SDC.
	int DoAddResourcesToSDC();

	/// Добавляем ресурсы со статьями
	int DoAddArticlesData();

	/// Добавляет таблицы сортировки в контейнер
	int DoAddSortTables();

	/// Добавляет встроенные базы морфологии в контейнер
	int DoAddMophology();

	/// Добавляем ресурсы листа в SDC
	int DoAddListResourcesToSDC(UInt32 aListIndex);

	/// Добавляет лист с бинарными данными
	int DoAddBinaryDataList();

	/// Добавляем в словарь озвучку
	int DoAddSounds(void);

	/// Добавляем в словарь картинки
	int DoAddPictures(void);

	/// Добавляем в словарь видео
	int DoAddVideos(void);

	/// Добавляем в словарь 3d модели
	int DoAddModels(void);

	/// Добавляем в словарь 3d материалы
	int DoAddMaterial(void);

	/// Добавляем в словарь 3d сцены
	int DoAddScene(void);

	/// Добавляем абстрактные объекты
	int DoAddAbstractResources();

	/// Добавляем в словарь индекс элементарных объектов
	int DoAddAtomicObjectIndex(void);

	/// Добавляем в словарь информацию о связях с другими базами
	int DoAddExternContentInfo(void);

	/// Добавляем в словарь таблицу информации о ресурсах
	int DoAddResourceInfo(void);

	/// Добавляем в словарь имена ресурсов
	int DoAddResourceNames(void);

	void FillResourceInfo(UInt32 aResType, const wstring& aResName);
	/// Добавляем в словарь тематики блоков switch
	int DoAddThematics(void);

	/// Добавляем в словарь структурированные метаданные
	int DoAddStructuredMetadata();

	/// Добавляем в словарь css метаданные
	int DoAddCSSData();

	/// Обновляем заголовок словаря
	int DoUpdateHeader();

	/// Проект словаря заранее считанный.
	SlovoEdProject *m_Project;

	/// Класс занимающийся сохранением данных в специальном контейнере
	/// SlovoEd Data Container (SDC)
	CSDCWrite	m_SDC;

	/// Сжатые списки слов. Нужно освободить при окончании работы память.
	vector<CListCompress*> m_CompressedLists;

	/// Массив заголовков списков слов. Количество заголовков хранится в заголовке словаря.
	TListHeader *m_ListHeader;

	/// Класс занимающийся сжатием словаря
	CArticlesCompress m_ArticlesCompress;

	/// Заголовок словаря.
	TDictionaryHeader m_DictHeader;

	/// Дополнительная информация по учебнику
	TAdditionalInfo m_additionalInfo;

	/// Аннотация к учебнику
	wstring m_Annotation;

	/// Массив соответствия индекса объектов
	vector<TAtomicObjectInfo> m_AtomicObjects;

	/// Массив соответствия КЭСов и внутренних id ресурсов с ЭО
	map<UInt32, vector<UInt32> > m_KESmap;
	/// Массив соответствия ФП и внутренних id ресурсов с ЭО
	map<UInt32, vector<UInt32> > m_FCmap;

	/// Имена ресурсов
	map<UInt32, vector<TResourceInfo> > m_ResourceInfo;

	vector<wstring> m_ResourceNames;

	ThreadPool &m_threadPool;

	CompressConfig m_resCompCfg;
};

CCompress::CCompress(ThreadPool &threadPool, SlovoEdProject *project, const CompressConfig &aConfig) :
	m_Project(project),
	m_ListHeader(nullptr),
	m_Annotation(project->GetAnnotation()),
	m_threadPool(threadPool),
	m_resCompCfg(aConfig)
{
	memcpy(&m_DictHeader, m_Project->GetDictionaryHeader(), sizeof(m_DictHeader));

	memset(&m_additionalInfo, 0, sizeof(m_additionalInfo));
	const TAdditionalInfo *const pAdditionalInfo(m_Project->GetAdditionalInfo());
	if (pAdditionalInfo)
		memcpy(&m_additionalInfo, pAdditionalInfo, sizeof(m_additionalInfo));

	const auto compression = aConfig.resourceCompression();
	if (compression.first)
		m_SDC.OverrideCompressionConfig(compression.second);
}

CCompress::~CCompress(void)
{
	for (CListCompress *list : m_CompressedLists)
	{
		if (list)
			delete list;
	}

	if (m_ListHeader)
		delete [] m_ListHeader;
}

template <typename Callback>
static int doCompressStep(const char *msg, CMyProcessTimer& timer, const char *timeString, Callback cb)
{
	sld::printf("\n%s", msg);
	timer.Start();
	int error = cb();
	if (error != ERROR_NO)
	{
		CLogW::PrintToConsole("[FAIL]", eLogStatus_Error);
		return error;
	}
	timer.Stop();
	timer.PrintTimeString(timeString);
	CLogW::PrintToConsole("[OK]");
	return ERROR_NO;
}

static const struct {
	using CompressFuncPtr = int (CCompress::*)();

	const char *msg;
	const CompressFuncPtr func;
	const char *funcName;
} g_CompressSteps[] = {
#define STEP(_msg, _func) { _msg, &CCompress::_func, #_func },
	STEP("Sorting lists...", DoSortLists)
	STEP("Adding css metadata...", DoAddCSSData)
	STEP("Sorting articles...", DoSortArticles)
	STEP("Associate list with articles...", AssociateListWithArticles)
	STEP("Associate atomic objects with articles...", AssociateAtomicObjectsWithArticles)
	STEP("Process links...", DoProcessLinks)
	STEP("Adding structured metadata...", DoAddStructuredMetadata)
	STEP("Compress lists...", DoCompressLists)
	STEP("Compress articles...", DoCompressArticles)
	STEP("Update header...", DoUpdateHeader)
	STEP("Build SDC in memory...", DoAddResourcesToSDC)
#undef STEP
};

/**
	Производит сжатие словаря и упаковку его в контейнер.

	@return код ошибки
*/
int CCompress::DoCompress()
{
	CMyProcessTimer timer("CompressTime.time");

	for (auto&& step : g_CompressSteps)
	{
		int error = doCompressStep(step.msg, timer, step.funcName,
								   [this, &step]() { return (this->*step.func)(); });
		if (error != ERROR_NO)
			return error;
	}

	m_SDC.SetDatabaseType((UInt32)m_Project->GetDatabaseType());
	m_SDC.SetDictID(m_DictHeader.DictID);
	m_SDC.SetInApp(m_Project->GetInApp());

	//////////////////////////////////////////////////////////////////////////
	// Сжатие закончено - сохраняем все в файл.
	//////////////////////////////////////////////////////////////////////////

	UIntUnion DictID;
	DictID.ui_32 = m_DictHeader.DictID;

	wchar_t fileName[9] = {};
	if (!m_Project->IsETBDatabase())
	{
		fileName[0] = DictID.ui_8[0];
		fileName[1] = DictID.ui_8[1];
		fileName[2] = DictID.ui_8[2];
		fileName[3] = DictID.ui_8[3];
	}
	else
		swprintf(fileName, sld::array_size(fileName), L"%08X", DictID.ui_32);

	std::wstring suffix(
		m_Project->GetInApp() ? L".InApp.sdc" :
		m_Project->IsETBDatabase() ? L".datf" : L".sdc"
		);
	const std::wstring outFileName = fileName + suffix;

	return doCompressStep("Save SDC to disk...", timer, "SDC Write", [&]() {
		return m_SDC.Write(outFileName.c_str());
	});
}

// helper structs/functions for sorting lists
namespace {

struct SortListData
{
	CWordList *list;
	const CWordList *baseList;
	const CStringCompare *compare;
	int error;
};

static int sortList(const SortListData *data)
{
	int error = ERROR_NO;
	if (data->baseList)
		error = data->list->DoSimpleSortedList(data->baseList);
	else if (data->list->GetAlternativeHeadwordsFlag())
		error = data->list->AddAlternativeHeadwords();
	if (error != ERROR_NO)
		return error;

	error = data->list->DoSort(data->compare);
	if (error != ERROR_NO)
		return error;

	if (data->list->GetAlternativeHeadwordsFlag())
		return data->list->PostProccessAlternativeHeadwords();
	return ERROR_NO;
};

static int sortLists(ThreadPool &threadPool, std::vector<SortListData> &lists)
{
	if (lists.size() <= 1 || !threadPool.active())
	{
		for (SortListData &data : lists)
			data.error = sortList(&data);
	}
	else
	{
		// sort the lists by element count, descending
		std::sort(lists.begin(), lists.end(),
				  [](const SortListData &lhs, const SortListData &rhs) {
			return lhs.list->GetNumberOfElements() > rhs.list->GetNumberOfElements();
		});

		std::vector<ThreadPool::Task> tasks(lists.size());
		for (size_t i : indices(lists))
		{
			SortListData *data = &lists[i];
			tasks[i] = threadPool.queue([data]() { data->error = sortList(data); });
		}
		threadPool.complete(tasks.data(), tasks.size());
	}

	// XXX: Realistically... no function called by sortList can currently trigger an error...
	for (const SortListData &data : lists)
	{
		if (data.error)
		{
			sldILog("Error! Can't sort list id: %s\n", sld::as_ref(data.list->GetListId()));
			return data.error;
		}
	}
	return ERROR_NO;
}

} // anon namespace

// Производит сортировку списков
int CCompress::DoSortLists()
{
	int error = ERROR_NO;

	const CSortTablesManager* pSortTablesManager = m_Project->GetSortTablesManager();
	assert(pSortTablesManager);

	// build the string compare mapping
	map<UInt32, std::unique_ptr<CStringCompare>> stringCompareMap;
	for (const CWordList &list : m_Project->lists())
	{
		const UInt32 langCodeFrom = list.GetLanguageCodeFrom();
		const TLanguageSortTable* pSortTable = pSortTablesManager->GetLanguageSortTable(langCodeFrom);
		if (!pSortTable)
		{
			sldILog("Error! CCompress::DoSortLists : Sort table doesn't exist!\n");
			return ERROR_NULL_SORT_TABLE;
		}

		auto &stringCompare = stringCompareMap[langCodeFrom];
		if (stringCompare == nullptr)
		{
			stringCompare.reset(new CStringCompare());
			error = stringCompare->Init(pSortTable->MainTable.data(), pSortTable->MainTable.size(),
										pSortTable->SecondTable.data(), pSortTable->SecondTable.size());
			if (error != ERROR_NO)
				return error;
		}
	}

	// sort simple sorted lists first
	std::vector<SortListData> listsToSort;
	map<size_t, CWordList*> simpleSortedLists;
	for (CWordList &list : m_Project->lists())
	{
		if (list.GetSimpleSortedListIndex() == SLD_DEFAULT_LIST_INDEX)
			continue;

		simpleSortedLists[list.GetSimpleSortedListIndex()] = &list;

		SortListData data = {};
		data.list = &list;
		data.compare = stringCompareMap[list.GetLanguageCodeFrom()].get();

		listsToSort.push_back(data);
	}

	error = sortLists(m_threadPool, listsToSort);
	if (error != ERROR_NO)
		return error;

	// then sort all of the other lists
	listsToSort.clear();
	for (auto&& list : enumerate(m_Project->lists()))
	{
		if (list->GetSimpleSortedListIndex() != SLD_DEFAULT_LIST_INDEX)
			continue;

		SortListData data = {};
		data.list = &*list;
		data.compare = stringCompareMap[list->GetLanguageCodeFrom()].get();
		data.baseList = simpleSortedLists.count(list.index) ? simpleSortedLists[list.index] : nullptr;

		listsToSort.push_back(data);
	}

	return sortLists(m_threadPool, listsToSort);
}

/**
	Производит сортировку статей основываясь на том, как отсортированы списки слов.

	@return код ошибки.
*/
int CCompress::DoSortArticles()
{
	assert(m_Project);

	CArticles* pArticles = m_Project->GetArticles();
	assert(pArticles);

	// Если нечего сортировать - выходим
	if (pArticles->GetNumberOfArticles() == 0)
		return ERROR_NO;

	const bool hasEmptyArticles = pArticles->HasEmptyArticles();
	const auto hasEmptyArticleRefs = [pArticles](const CWordList &list) {
		for (auto&& elem : list.elements())
		{
			if (elem.IndexesForAlternativeWords.size())
				continue;

			UInt32 id;
			if (pArticles->GetPositionByID(elem.ArticleRefs[0].ID, id) == ERROR_NO_ARTICLE)
				return true;
		}
		return false;
	};

	// Максимальное количество слов в найденном списке слов
	UInt32 maxElements = 0;
	// Указатель на список слов с максимальным количество элементов.
	CWordList *maxList = nullptr;

	// Ищем сортированный список с наибольшим количеством слов.
	for (auto&& list : m_Project->lists())
	{
		// Если список слов не сортирован, тогда по нему сортировать статьи не обязательно.
		if (!list.GetSorted())
			continue;

		// Если список слов иерархический, то сортировать по нему так же не получится.
		if (list.GetHierarchy())
			continue;

		// Если слово из списка слов ссылается на несколько статей, тогда по
		// этому списку слов нельзя ориентироваться при сортировке статей.
		if (list.GetIsNumberOfArticles())
			continue;

		// Если список слов - список полнотекстового поиска, то он тоже неприменим для сортировки
		if (list.IsFullTextSearchList() || list.GetLinkedList())
			continue;

		// Если у элементов списка слов вообще нет переводов - такой список также не подходит
		if (!list.GetNumberOfArticlesSize())
			continue;

		// Если в списке слов есть несколько элементов, которые ссылаются на один перевод - такой список также не подходит
		if (list.IsOneArticleReferencedByManyListEntries())
			continue;

		// В SimpleSortedList-ах не нужно получать переводы
		if (list.GetType() == eWordListType_SimpleSearch)
			continue;

		// Если у нас есть пустые статьи и список на них ссылается - мы не можем его использовать
		if (hasEmptyArticles && hasEmptyArticleRefs(list))
			continue;

		const UInt32 NumberOfElements = list.GetNumberOfElements();
		if (NumberOfElements > maxElements)
		{
			maxElements = NumberOfElements;
			maxList = &list;
		}
	}

	// Если сортированный список не найден, тогда выходим, сортировать статьи нет необходимости.
	if (maxList == nullptr)
		return ERROR_NO;

	sld::printf("\nSorted 0 articles");
	for (auto&& elem : enumerate(maxList->elements()))
	{
		if (elem->IndexesForAlternativeWords.size())
			continue;
		if (elem->ArticleRefs.empty())
			continue;

		int error = pArticles->SetPositionByID(static_cast<UInt32>(elem.index), elem->ArticleRefs[0].ID);
		if (error != ERROR_NO)
			return error;

		if (!(elem.index % 1000))
			sld::printf("\rSorted %u articles", static_cast<UInt32>(elem.index));
	}
	sld::printf("\rSorted %u articles", maxList->GetNumberOfElements());

	maxList->SetDirectList(true);

	return ERROR_NO;
}


namespace {
namespace fts {

/// Тип массива соответствия id-статей (переводов) элементам списков слов, в которых они используются
/// [article_id][list_index][list_entry_index][translation_index](count)
typedef std::unordered_map<sld::wstring_ref, std::unordered_map<UInt32, std::unordered_map<UInt32, std::unordered_set<UInt32>>>> ArticleIdMap;

struct ListEntry {
	UInt32 listId;
	UInt32 listEntryId;
};
/// Тип массива соответствия id элементов списков слов элементам списков слов
/// [list_entry_id](ListEntry)
typedef std::unordered_map<sld::wstring_ref, ListEntry> ListEntryIdMap;

static void logWarning(const std::wstring &str)
{
	const std::string u8str = sld::as_utf8(str);
	sldILog("%s\n", u8str);
	sld::printf(eLogStatus_Warning, "\n%s", u8str);
}

struct ListData {
	bool NeedToSaveListNumber = false;
	bool NeedToSaveTranslationNumber = false;
	bool NeedToSaveShift = false;

	UInt32 DefaultListIdx = ~0u;
	UInt32 DefaultTranslationIdx = ~0u;

	UInt32 MaxListIdx = 0;
	UInt32 MaxListEntryIdx = 0;
	UInt32 MaxTranslationIdx = 0;
	UInt32 MaxShift = 0;
	UInt32 MaxTranslationCount = 0;
};

// returns a string list id from its integer index
static const std::wstring& listIdByIndex(const SlovoEdProject *aProj, UInt32 aListId)
{
	const CWordList *list = aProj->GetWordList(aListId);

	// considering how we get aListId here that's impossible to hit
	if (!list)
		sldILog("Error! [fts] invalid list index: %u!\n", aListId);
	assert(list);

	return list->GetListId();
}

// проверяет можно ли поисковому списку ссылаться на список под индексом aListId
static bool isListAllowed(const TFullTextSearchOptions *aOptions, const SlovoEdProject *aProj, UInt32 aListId)
{
	if (aOptions->mAllowedSourceListId.empty())
		return true;

	const std::wstring& listId = listIdByIndex(aProj, aListId);
	return aOptions->mAllowedSourceListId.find(listId) != aOptions->mAllowedSourceListId.end();
}

static void updateTranslationCount(ListData &aData, UInt32 aRefsCount, UInt32 aListIndex, const std::wstring &aListElementId)
{
	if (aRefsCount == 0)
	{
		STString<1024> str(L"Warning! Full text search list entry linking to nothing: list_idx=%d; entry_id=%s;",
						   aListIndex, aListElementId.c_str());
		logWarning(str);
	}

	if (aRefsCount > aData.MaxTranslationCount)
		aData.MaxTranslationCount = aRefsCount;
}

// производит обработку фтс листов с линковкой по article id
static int processArticleIdList(CWordList &aList, UInt32 aListIndex, ListData &aData,
								const SlovoEdProject *aProj, const ArticleIdMap &aArtIdMap)
{
	const TFullTextSearchOptions* pFTSOptions = aList.GetFullTextSearchOptions();
	assert(pFTSOptions);
	assert(pFTSOptions->LinkType == eLinkType_ArticleId);

	for (auto listElement : enumerate(aList.elements()))
	{
		for (auto ref : enumerate(listElement->ArticleRefs))
		{
			UInt32 Shift = ref->shift;
			if (Shift && !aData.NeedToSaveShift)
			{
				Shift = 0;
				logWarning(L"Warning! FullTextSearchOptions.ShiftType set to \"None\" but shift found. Zero value will be used.");
			}

			if (aData.NeedToSaveShift && Shift > aData.MaxShift)
				aData.MaxShift = Shift;

			auto fit = aArtIdMap.find(ref->ID);
			if (fit == aArtIdMap.end())
			{
				const STString<1024> str(L"Warning! Unresolved full text search link by article_id: list_idx=%u; entry_idx=%u; art_id=\"%s\";",
										 aListIndex, listElement.index, ref->ID.c_str());
				logWarning(str);
				continue;
			}

			// если ссылка на статью найдена более чем в одном списке - устанавливаем флаг, что номера списков нужно будет сохранять
			if (fit->second.size() > 1)
				aData.NeedToSaveListNumber = true;

			// перебираем номера списков слов
			for (auto &it0 : fit->second)
			{
				const UInt32 ListIdx = it0.first;

				// Проверим, можно ли поисковому списку слов ссылаться на этот список
				if (!isListAllowed(pFTSOptions, aProj, ListIdx))
					continue;

				if (aData.DefaultListIdx != ~0u && ListIdx != aData.DefaultListIdx)
					aData.NeedToSaveListNumber = true;

				aData.DefaultListIdx = ListIdx;
				if (ListIdx > aData.MaxListIdx)
					aData.MaxListIdx = ListIdx;

				// перебираем номера записей в списке слов
				for (auto &it1 : it0.second)
				{
					const UInt32 ListEntryIdx = it1.first;
					if (ListEntryIdx > aData.MaxListEntryIdx)
						aData.MaxListEntryIdx = ListEntryIdx;

					// если номеров переводов более чем 1 - устанавливаем флаг, что номера переводов нужно будет сохранять
					if (it1.second.size() > 1)
						aData.NeedToSaveTranslationNumber = true;

					// перебираем номера переводов
					for (const UInt32 TranslationIdx : it1.second)
					{
						if (aData.DefaultTranslationIdx != ~0u && TranslationIdx != aData.DefaultTranslationIdx)
							aData.NeedToSaveTranslationNumber = true;

						aData.DefaultTranslationIdx = TranslationIdx;
						if (TranslationIdx > aData.MaxTranslationIdx)
							aData.MaxTranslationIdx = TranslationIdx;

						// Добавляем ссылку
						TListEntryRef leRef;
						leRef.list_idx = ListIdx;
						leRef.entry_idx = ListEntryIdx;
						leRef.translation_idx = TranslationIdx;
						leRef.shift = Shift;

						listElement->ListEntryRefs.push_back(leRef);
					}
				}
			}
		}

		updateTranslationCount(aData, listElement->ListEntryRefs.size(), aListIndex, listElement->Id);
	}

	return ERROR_NO;
}

// производит обработку фтс листов с линковкой по list entry id
static int processListEntryIdList(CWordList &aList, UInt32 aListIndex, ListData &aData,
								  const SlovoEdProject *aProj, const ListEntryIdMap &aListEntryIdMap)
{
	const TFullTextSearchOptions* pFTSOptions = aList.GetFullTextSearchOptions();
	assert(pFTSOptions);
	assert(pFTSOptions->LinkType == eLinkType_ListEntryId);

	std::vector<UInt32> refsToRemove;

	for (auto listElement : enumerate(aList.elements()))
	{
		refsToRemove.clear();

		for (auto ref : enumerate(listElement->ArticleRefs))
		{
			UInt32 Shift = ref->shift;
			if (Shift)
			{
				Shift = 0;
				logWarning(L"Warning! Shift in article specified but LinkType==ListEntryId. Shift value will be ignored.");
			}

			// ищем запись по id записи списка слов
			const auto &fit = aListEntryIdMap.find(ref->ID);
			if (fit == aListEntryIdMap.end() || aList.HasListEntryId(ref->ID))
			{
				refsToRemove.push_back(ref.index);
				STString<1024> str(L"Warning! Unresolved full text search link by list_entry_id: list_idx=%d; entry_idx=%d; list_entry_id=\"%s\";",
								   aListIndex, listElement.index, ref->ID.c_str());
				logWarning(str);
				continue;
			}

			aData.DefaultTranslationIdx = 0;

			const ListEntry &entry = fit->second;

			// Проверим, можно ли поисковому списку слов ссылаться на этот список
			if (!isListAllowed(pFTSOptions, aProj, entry.listId))
				continue;

			if (aData.DefaultListIdx != ~0u && entry.listId != aData.DefaultListIdx)
				aData.NeedToSaveListNumber = true;

			aData.DefaultListIdx = entry.listId;
			if (entry.listId > aData.MaxListIdx)
				aData.MaxListIdx = entry.listId;

			if (entry.listEntryId > aData.MaxListEntryIdx)
				aData.MaxListEntryIdx = entry.listEntryId;

			// Добавляем ссылку
			TListEntryRef leRef;
			leRef.list_idx = entry.listId;
			leRef.entry_idx = entry.listEntryId;
			leRef.translation_idx = aData.DefaultTranslationIdx;
			leRef.shift = Shift;

			listElement->ListEntryRefs.push_back(leRef);
		}

		for (auto it = refsToRemove.crbegin(); it != refsToRemove.crend(); ++it)
			aList.RemoveArticleRef(listElement.index, *it);

		updateTranslationCount(aData, listElement->ListEntryRefs.size(), aListIndex, listElement->Id);
	}

	return ERROR_NO;
}

} // namespace fts

static int ProcessFullTextSearchLists(SlovoEdProject *aProj, const fts::ArticleIdMap &aArtIdMap, const fts::ListEntryIdMap &aListEntryIdMap)
{
	for (auto list : enumerate(aProj->lists()))
	{
		if (!list->IsFullTextSearchList() && !list->GetLinkedList())
			continue;

		const TFullTextSearchOptions* pFullTextSearchOptions = list->GetFullTextSearchOptions();
		assert(pFullTextSearchOptions);

		if (pFullTextSearchOptions->LinkType == eLinkType_Unknown)
		{
			sldILog("Error! Unknown link type in full text search list #%lu ('%s')\n",
					list.index, sld::as_ref(list->GetListId()));
			return ERROR_WRONG_LIST_TYPE;
		}

		fts::ListData data;

		if (pFullTextSearchOptions->ShiftType != eShiftType_None)
			data.NeedToSaveShift = true;

		int error = ERROR_NO;
		if (pFullTextSearchOptions->LinkType == eLinkType_ArticleId)
		{
			error = fts::processArticleIdList(*list, list.index, data, aProj, aArtIdMap);
		}
		else if (pFullTextSearchOptions->LinkType == eLinkType_ListEntryId)
		{
			error = fts::processListEntryIdList(*list, list.index, data, aProj, aListEntryIdMap);
		}
		if (error != ERROR_NO)
			return error;

		// После того, как мы нашли максимальное число переводов на 1 статью,
		// можно рассчитать количество бит, необходимых для хранения
		list->SetNumberOfArticlesSize(GetCodeLen(data.MaxTranslationCount));

		// Сохраняем информацию для заголовка списка слов
		// ----------------------------------------------

		UInt32 BitCount = 1 + GetCodeLen(data.MaxListIdx >> 1);
		list->SetFullTextSearchLinkListIndexSize(BitCount);
		if (!data.NeedToSaveListNumber)
			list->SetFullTextSearchDefaultLinkListIndex(data.DefaultListIdx);

		BitCount = 1 + GetCodeLen(data.MaxListEntryIdx >> 1);
		list->SetFullTextSearchLinkListEntryIndexSize(BitCount);

		BitCount = 1 + GetCodeLen(data.MaxTranslationIdx >> 1);
		list->SetFullTextSearchLinkTranslationIndexSize(BitCount);
		if (!data.NeedToSaveTranslationNumber)
			list->SetFullTextSearchDefaultLinkTranslationIndex(data.DefaultTranslationIdx);

		BitCount = 1 + GetCodeLen(data.MaxShift >> 1);
		if (data.NeedToSaveShift)
			list->SetFullTextSearchShiftIndexSize(BitCount);
	}

	return ERROR_NO;
}

} // anon namespace

/**
	Производит связывание списков слов и переводы.

	Это нужно для того, чтобы в дальнейшем списки слов содержали все 
	необходимые данные для сжатия. Также на этом этапе можно определить
	какие переводы остались не учтенными(по прямым ссылкам).

	@return код ошибки.
*/
int CCompress::AssociateListWithArticles()
{
	int error;
	
	const CArticles* pArticles = m_Project->GetArticles();
	assert(pArticles);

	//////////////////////////////////////////////////////////////////////////
	// Подсчитываем количество бит необходимых для того, чтобы представить 
	// номер перевода на который ссылается элемент списка слов.
	//////////////////////////////////////////////////////////////////////////
	UInt32 NumberOfArticles = pArticles->GetNumberOfArticles();
	if (NumberOfArticles > 1)
		NumberOfArticles--;
	const UInt32 IndexSize = GetCodeLen(NumberOfArticles);

	// данные для фтс процессинга
	fts::ArticleIdMap articleIdMap;
	fts::ListEntryIdMap listEntryIdMap;

	for (auto list : enumerate(m_Project->lists()))
	{
		if (!list->IsFullTextSearchList())
			list->SetArticleIndexSize(IndexSize);

		UInt32 alternativeHeadwordsCount = 0;
		for (auto listElement : enumerate(list->elements()))
		{
			if (listElement->IDForAlternativeWords.size())
			{
				alternativeHeadwordsCount++;
				continue;
			}

			const UInt32 listEntryId = static_cast<UInt32>(listElement.index - alternativeHeadwordsCount);

			// Учитываем ссылки на записи списков слов
			auto fit = listEntryIdMap.find(listElement->Id);
			if (fit != listEntryIdMap.end())
			{
				// list entry id (strings) must be unique throughout the whole dictionary
				sldILog("Error! Duplicated list_entry_id '%s'! First encountered in list id: '%s', duplicate in list id: '%s'\n",
						sld::as_ref(listElement->Id),
						sld::as_ref(fts::listIdByIndex(m_Project, fit->second.listId)),
						sld::as_ref(list->GetListId()));
				return ERROR_DUPLICATED_LIST_ENTRY_ID;
			}
			const fts::ListEntry entry = { static_cast<UInt32>(list.index), listEntryId };
			listEntryIdMap.emplace(listElement->Id, entry);

			if (list->HasStylizedVariants())
			{
				auto &stylizedIndexes = listElement->m_StylizedVariantsArticleIndexes;
				for (auto stylizedVariant = stylizedIndexes.begin(); stylizedVariant != stylizedIndexes.cend(); )
				{
					UInt32 articleIndex;
					error = pArticles->GetPositionByID(stylizedVariant->second, articleIndex);

					// если статья пустая - просто убираем вариант и продолжаем как ни в чем не бывало
					if (error == ERROR_NO_ARTICLE)
					{
						stylizedVariant = stylizedIndexes.erase(stylizedVariant);
						continue;
					}

					if (error != ERROR_NO)
						return error;

					list->SetStylizedVariantArticleIndex(listElement.index, stylizedVariant->first, articleIndex);
					++stylizedVariant;
				}
			}

			if (list->IsFullTextSearchList() || list->GetLinkedList())
				continue;

			uint32_t index = 0;
			for (auto ref = listElement->ArticleRefs.begin(); ref != listElement->ArticleRefs.cend(); )
			{
				UInt32 articleIndex;
				error = pArticles->GetPositionByID(ref->ID, articleIndex);

				// если статья пустая - просто убираем ссылку и продолжаем как ни в чем не бывало
				if (error == ERROR_NO_ARTICLE)
				{
					ref = listElement->ArticleRefs.erase(ref);
					continue;
				}

				if (error != ERROR_NO)
					return error;

				ref->index = articleIndex;

				// Учитываем ссылки на переводы
				articleIdMap[ref->ID][list.index][listEntryId].insert(index);
				++ref;
				index++;
			}
		}
	}

	/// Обрабатываем списки слов полнотекстового поиска
	return ProcessFullTextSearchLists(m_Project, articleIdMap, listEntryIdMap);
}

/**
	Производит связывание элементарых объектов с переводами

	@return код ошибки.
*/
int CCompress::AssociateAtomicObjectsWithArticles()
{
	const CWordList* pList = NULL;
	for (const CWordList &list : m_Project->lists())
	{
		if (list.GetType() == eWordListType_Atomic)
		{
			pList = &list;
			break;
		}
	}

	if (!pList)
		return ERROR_NO;

	TAtomicObjectInfo tatomic;
	memset(&tatomic, 0, sizeof(TAtomicObjectInfo));

	map<wstring, UInt32> atomicID_atomicIndex;
	for (const auto element : enumerate(pList->elements()))
		atomicID_atomicIndex[element->Name[0]] = static_cast<UInt32>(element.index);

	for (const auto article : enumerate(m_Project->articles()))
	{
		for (const auto block : enumerate(article->blocks()))
		{
			if (block->metaType == eMetaAtomicObject)
			{
				if (block->isClosing)
				{
					m_AtomicObjects.push_back(tatomic);
				}
				else
				{
					tatomic.BlockCount = 1;
					tatomic.FirstBlockIndex = static_cast<UInt32>(block.index);
					tatomic.TranslateIndex = static_cast<UInt32>(article.index);

					auto metaData = m_Project->GetMetadataManager()->findMetadata<eMetaAtomicObject>(block->metadata());
					if (metaData && block->TextLength(BlockType::AtomicObjectId))
						metaData->Index = atomicID_atomicIndex[block->GetText(BlockType::AtomicObjectId)];
				}
			}
			else
				tatomic.BlockCount++;
		}
	}
	

	return ERROR_NO;
}

// struct types for the links processing step
namespace {

struct TListLinkEntry
{
	UInt32			ListNumber;
	UInt32			ListElementNumber;
	UInt32			NameNumber;
	ListElement*	pListElement;
	bool			self;

	TListLinkEntry(UInt32 listNumber, ListElement *element)
		: ListNumber(listNumber), ListElementNumber(0), NameNumber(0), pListElement(element), self(false)
	{}
};

struct TArticleLink
{
	CArticle* Article;
	BlockType* Block;
	sld::wstring_ref list_entry_id;
	UInt32 article_id;
	TListLinkEntry *ListEntry;

	TArticleLink(CArticle *article, BlockType *block) : Article(article), Block(block), article_id(~0u), ListEntry(nullptr) {}
};

}

/***********************************************************************
* Производит обработку внутренних ссылок словаря
*
* @return код ошибки
************************************************************************/
int CCompress::DoProcessLinks(void)
{
	CArticles* Articles = m_Project->GetArticles();
	if (!Articles)
	{
		sldILog("Error! CCompress::DoProcessLinks : pArticles has not been created!\n");
		return ERROR_NULL_POINTER;
	}

	struct TSimpleLink {
		CArticle *article;
		BlockType *block;
	};

	std::vector<TArticleLink> ArtLinks;
	std::vector<TSimpleLink> flashCardsLinks;
	std::unordered_map<sld::wstring_ref, std::pair<UInt32, UInt32>> listEntryIdToIndexesMap;
	std::unordered_map<sld::wstring_ref, UInt32> articleIdToIndexesMap;

	// Заполняем мапу соответствия id статьи -> числового индекса
	for (auto article : enumerate(Articles->articles()))
		articleIdToIndexesMap.emplace(article->GetID(), static_cast<uint32_t>(article.index));

	// Пересчитываем все ссылки которые имеются в статьях и заполняем массив ссылок
	for (CArticle &article : Articles->articles())
	{
		for (BlockType &block : article.blocks())
		{
			// don't bother handling closing tags
			if (block.isClosing)
				continue;

			const ESldStyleMetaTypeEnum type = block.metaType;
			if (type == eMetaLink || type == eMetaImageArea || type == eMetaPopupArticle || type == eMetaExternArticle)
			{
				TArticleLink alink(&article, &block);
				if (block.TextLength(BlockType::ArticleId))
					alink.article_id = articleIdToIndexesMap[block.GetText(BlockType::ArticleId)];
				if (block.TextLength(BlockType::ListEntryId))
					alink.list_entry_id = block.GetText(BlockType::ListEntryId);

				ArtLinks.push_back(alink);
			}
			else if (type == eMetaFlashCardsLink)
			{
				flashCardsLinks.push_back({ &article, &block });
			}
		}
	}

	const auto &htmlSourceName = m_Project->GetHtmlSourceNames();
	if (ArtLinks.empty() && htmlSourceName.empty())
		return ERROR_NO;

	std::sort(ArtLinks.begin(), ArtLinks.end(),
			  [](const TArticleLink &a1, const TArticleLink &a2) {
			      return a1.list_entry_id < a2.list_entry_id;
			  });

	sld::ArenaAllocator<TListLinkEntry> linkEntryAlloc(ArtLinks.size());
	for (auto&& list : enumerate(m_Project->lists()))
	{
		// TODO: этот код использует знания о структуре каталога - его нужно перенести на уровень ниже, в CWordList

		// считаем количество элементов на 0 уровне в локализованном списке
		const bool isLocalizedList = list->GetLocalizedEntries();
		UInt32 numberOfLocalizations = 0;
		if (isLocalizedList)
		{
			for (auto&& element : list->elements())
			{
				if (element.Level == 0)
					numberOfLocalizations++;
			}
		}

		// количество записей в каждой из папок с локализацией
		// (фактически это некий аналог глобального индекса в списке без локализации)
		UInt32 localizedEntriesNumber = 0;
		if (numberOfLocalizations)
			localizedEntriesNumber = list->GetNumberOfElements() / numberOfLocalizations - 1;

		UInt32 alternativeHeadwordsCount = 0;
		for (auto&& element : enumerate(list->elements()))
		{
			if (element->IDForAlternativeWords.size())
			{
				alternativeHeadwordsCount++;
				continue;
			}

			listEntryIdToIndexesMap.emplace(element->Id, make_pair(static_cast<UInt32>(list.index), static_cast<UInt32>(element.index - alternativeHeadwordsCount)));

			auto link = std::lower_bound(ArtLinks.begin(), ArtLinks.end(), element->Id,
										 [](const TArticleLink &link, const std::wstring &id) { return link.list_entry_id < id; });
			if (link == ArtLinks.cend() || link->list_entry_id != element->Id)
				continue;

			TListLinkEntry listLink(static_cast<UInt32>(list.index), &*element);

			if (isLocalizedList && localizedEntriesNumber)
				listLink.ListElementNumber = (element.index - numberOfLocalizations - alternativeHeadwordsCount) % localizedEntriesNumber;
			else
				listLink.ListElementNumber = static_cast<UInt32>(element.index - alternativeHeadwordsCount);

			for (; link != ArtLinks.cend() && link->list_entry_id == element->Id; ++link)
			{
				listLink.self = element->ArticleRefs.size() && element->ArticleRefs[0].ID == link->Article->GetID();
				link->ListEntry = linkEntryAlloc.create(listLink);
			}
		}
	}

	std::vector<TSimpleLink> unresolvedLinks;
	for (TArticleLink &link : ArtLinks)
	{
		BlockType *const pBlock = link.Block;
		const TListLinkEntry* linkEntry = link.ListEntry;

		MetadataManager *mgr = m_Project->GetMetadataManager();
		switch (pBlock->metaType)
		{
		case eMetaLink:
		{
			auto metaData = mgr->findMetadata<eMetaLink>(pBlock->metadata());
			if (!metaData)
				return ERROR_NULL_POINTER;

			// this tries to follow the previous logic
			if (linkEntry)
			{
				metaData->ListIndex = linkEntry->ListNumber;
				metaData->EntryIndex = linkEntry->ListElementNumber;
				metaData->Self = linkEntry->self;

				// clear external link when we have an internal one
				metaData->ExtKey = TMetadataExtKey();
			}
			else
			{
				metaData->Title = TMetadataString();
				metaData->Label = TMetadataString();

				if (!checkExtKey(metaData->ExtKey, mgr))
				{
					metaData->ExtKey = TMetadataExtKey();
					unresolvedLinks.push_back({ link.Article, pBlock });
				}
			}
			break;
		}
		case eMetaImageArea:
		{
			auto metaData = mgr->findMetadata<eMetaImageArea>(pBlock->metadata());
			if (!metaData)
				return ERROR_NULL_POINTER;

			if (!linkEntry && (metaData->Type == eImageAreaType_Link || metaData->Type == eImageAreaType_Slide))
			{
				unresolvedLinks.push_back({ link.Article, pBlock });
			}
			else
			{
				switch (metaData->Type)
				{
				case eImageAreaType_Link:
				case eImageAreaType_Slide:
				{
					const STString<128> action(L"%u.%u", linkEntry->ListNumber, linkEntry->ListElementNumber);
					metaData->ActionScript = mgr->addString(action);
					break;
				}
				default:
					break;
				}
			}
			break;
		}
		case eMetaPopupArticle:
		{
			auto metaData = mgr->findMetadata<eMetaPopupArticle>(pBlock->metadata());
			if (!metaData)
				return ERROR_NULL_POINTER;

			if (linkEntry)
			{
				metaData->ListIndex = linkEntry->ListNumber;
				metaData->EntryIndex = linkEntry->ListElementNumber;

				// clear external link when we have an internal one
				metaData->ExtKey = TMetadataExtKey();
			}
			else
			{
				metaData->Title = TMetadataString();
				metaData->Label = TMetadataString();

				if (!checkExtKey(metaData->ExtKey, mgr))
				{
					metaData->ExtKey = TMetadataExtKey();
					unresolvedLinks.push_back({ link.Article, pBlock });
				}
			}
			break;
		}
		case eMetaExternArticle:
		{
			auto metaData = mgr->findMetadata<eMetaExternArticle>(pBlock->metadata());
			if (!metaData)
				return ERROR_NULL_POINTER;

			if (link.article_id != -1)
			{
				metaData->ArticleIndex = link.article_id;

				// clear external link when we have an internal one
				metaData->ExtKey = TMetadataExtKey();
			}
			else if (linkEntry)
			{
				metaData->ListIndex = linkEntry->ListNumber;
				metaData->EntryIndex = linkEntry->ListElementNumber;

				// clear external link when we have an internal one
				metaData->ExtKey = TMetadataExtKey();
			}
			else
			{
				if (!checkExtKey(metaData->ExtKey, mgr))
				{
					metaData->ExtKey = TMetadataExtKey();
					unresolvedLinks.push_back({ link.Article, pBlock });
				}
			}
			break;
		}
		default:
			assert(!"Should not be here!");
			break;
		}
	}

	// print out warnings for all unresolved links
	for (const TSimpleLink &link : unresolvedLinks)
	{
		sldILog("Warning! Unresolved link: tag=`%s` article_id=`%s` list_entry_id=`%s` text=`%s`\n",
				sld::as_ref(GetTagNameByTextType(link.block->metaType)),
				sld::as_ref(link.article->GetID()),
				sld::as_ref(link.block->GetText(BlockType::ListEntryId)),
				sld::as_ref(link.block->text));
	}

	for (TSimpleLink& link : flashCardsLinks)
	{
		auto metaData = m_Project->GetMetadataManager()->findMetadata<eMetaFlashCardsLink>(link.block->metadata());
		if (!metaData)
			continue;

		bool valid = false;
		const std::wstring frontId = link.block->GetText(BlockType::FlashCardListEntryIdFront);
		const std::wstring backId = link.block->GetText(BlockType::FlashCardListEntryIdBack);
		auto frontIt = listEntryIdToIndexesMap.find(frontId);
		auto backIt = listEntryIdToIndexesMap.find(backId);
		if (!frontId.empty() && frontIt != listEntryIdToIndexesMap.end())
		{
			metaData->FrontListId = frontIt->second.first;
			metaData->FrontListEntryId = frontIt->second.second;
			valid = true;
		}

		if (!backId.empty() && backIt != listEntryIdToIndexesMap.end())
		{
			metaData->BackListId = backIt->second.first;
			metaData->BackListEntryId = backIt->second.second;
			valid = true;
		}

		if (!valid)
		{
			sldILog("Warning! Unresolved link in flash cards: article_id=`%s` front=`%s` back=`%s`\n",
					sld::as_ref(link.article->GetID()),
					sld::as_ref(frontId),
					sld::as_ref(backId));
			unresolvedLinks.push_back(link);
		}
	}

	if (unresolvedLinks.size())
	{
		sld::printf(eLogStatus_Warning, "\nWarning! The dictionary has %lu unresolved links! Check the logs please!",
										unresolvedLinks.size());

		// remove the unresolved link blocks
		for (TSimpleLink &link : unresolvedLinks)
		{
			MetadataRef ref = link.block->metadata();
			m_Project->GetMetadataManager()->removeMetadata(ref);
			link.article->removeBlock(link.block);
		}
	}
	unresolvedLinks.clear();

	for (auto&& It : htmlSourceName)
	{
		pugi::xml_document htmlTree;
		htmlTree.load_file(It.second.c_str(), pugi::parse_full);
		if (!htmlTree)
		{
			sldILog("Error! Can't open xhtml file `%s`.\n", sld::as_ref(It.second));
			continue;
		}

		pugi::xpath_node_set pathSet = htmlTree.select_nodes(L".//a[@list-entry-id]");
		for (auto&& xnode : pathSet)
		{
			pugi::xml_node node = xnode.node();
			const sld::wstring_ref listEntryId = node.attribute(L"list-entry-id").value();

			auto entryIt = listEntryIdToIndexesMap.find(listEntryId);
			if (entryIt == listEntryIdToIndexesMap.cend())
			{
				sldILog("Error! Can't find listEntryId: `%s`.\n", sld::as_ref(listEntryId));
				continue;
			}

			wstringstream ss;
			ss << L"link:list_idx=(" << entryIt->second.first << L");entry_idx=(" << entryIt->second.second
				<< L");link_type=(0);label=(" << L");";

			pugi::xml_attribute hrefAttr = node.attribute(L"href");
			if (hrefAttr.empty())
				hrefAttr = node.append_attribute(L"href");
			hrefAttr.set_value(ss.str().c_str());
		}

		htmlTree.save_file(It.second.c_str(), L"");
	}

	return ERROR_NO;
}

/**
	Производим упаковку списков слов.

	@return код ошибки.
*/
int CCompress::DoCompressLists()
{
	// Сжимаем все списки слов
	if (m_threadPool.active())
	{
		const UInt32 listsCount = m_Project->GetNumberOfLists();
		m_CompressedLists.resize(listsCount);

		std::vector<ThreadPool::Task> tasks(listsCount);
		std::vector<int> errors(listsCount, ERROR_NO);
		for (auto&& list : enumerate(m_Project->lists()))
		{
			tasks[list.index] = m_threadPool.queue([this, list, &errors]() {
					std::tie(errors[list.index], m_CompressedLists[list.index]) =
						CListCompress::Compress(*list, m_threadPool);
				});
		}
		m_threadPool.complete(tasks.data(), tasks.size());

		for (int error : errors)
		{
			if (error != ERROR_NO)
				return error;
		}
	}
	else
	{
		for (auto&& list : m_Project->lists())
		{
			auto compList = CListCompress::Compress(list, m_threadPool);
			if (compList.first != ERROR_NO)
				return compList.first;
			m_CompressedLists.push_back(compList.second);
		}
	}

	// Создаем заголовки для списков слов
	m_ListHeader = new TListHeader[m_CompressedLists.size()];
	memset(m_ListHeader, 0, m_CompressedLists.size() * sizeof(m_ListHeader[0]));

	for (auto&& list : enumerate(m_Project->lists()))
	{
		CListCompress *compressedList = m_CompressedLists[list.index];

		TListHeader &listHeader = m_ListHeader[list.index];
		listHeader.CompressionMethod = list->GetCompressionMethod().type;
		listHeader.HeaderSize = sizeof(TListHeader);
		listHeader.NumberOfVariants = list->GetNumberOfVariants();
		listHeader.NumberOfWords = list->GetRealWordCount();
		listHeader.SizeOfNumberOfArticles = list->GetNumberOfArticlesSize();
		listHeader.SizeOfArticleIndex = list->GetAticleIndexSize();
		listHeader.IsNumberOfArticles = list->GetIsNumberOfArticles();
		listHeader.IsDirectList = list->GetDirectList();
		listHeader.IsHierarchy = list->GetHierarchy();
		listHeader.IsSortedList = list->GetSorted();
		listHeader.IsLocalizedList = list->GetLocalizedEntries();
		listHeader.LanguageCodeFrom = list->GetLanguageCodeFrom();
		listHeader.LanguageCodeTo = list->GetLanguageCodeTo();
		listHeader.WordListUsage = list->GetType();
		listHeader.IsPicture = list->GetPicture();
		listHeader.IsVideo = list->GetVideo();
		listHeader.IsSound = list->GetSound();
		listHeader.IsScene = list->GetScene();
		listHeader.PictureIndexSize = list->GetPictureIndexSize();
		listHeader.SoundIndexSize = list->GetSoundIndexSize();
		listHeader.VideoIndexSize = list->GetVideoIndexSize();
		listHeader.SceneIndexSize = list->GetSceneIndexSize();

		listHeader.Version = VERSION_LIST_CURRENT;
		listHeader.Type_CompressedData = SLD_RESOURCE_COMPRESSED + list.index;
		listHeader.Type_Hierarchy = SLD_RESOURCE_HIERARCHY + list.index;
		listHeader.Type_Tree = SLD_RESOURCE_TREE + list.index;
		listHeader.Type_IndexesCount = RESOURCE_TYPE_INDEXES_COUNT + list.index;
		listHeader.Type_IndexesData = RESOURCE_TYPE_INDEXES_DATA + list.index;
		listHeader.Type_IndexesQA = RESOURCE_TYPE_INDEXES_QA + list.index;
		listHeader.Type_IndexesHeader = RESOURCE_TYPE_INDEXES_HEADER + list.index;
		listHeader.Type_DirectWordsShifts = 0;
		listHeader.Type_SearchTreePoints = 0;
		listHeader.Type_AlternativeHeadwordsInfo = 0;
		listHeader.MaximumWordSize = compressedList->GetMaxWordSize();

		listHeader.IsFullTextSearchList = (list->IsFullTextSearchList() || list->GetLinkedList()) ? 1 : 0;
		listHeader.IsSuffixFullTextSearchList = list->IsSuffixFullTextSearchList();
		listHeader.IsFullMorphologyWordFormsList = list->IsFullMorphologyWordFormsList();
		listHeader.MorphologyId = list->GetMorphologyId();

		listHeader.TotalPictureCount = list->GetTotalPictureCount();
		listHeader.TotalVideoCount = list->GetTotalVideoCount();
		listHeader.TotalSceneCount = list->GetTotalSceneCount();
		listHeader.TotalSoundCount = list->GetTotalSoundCount();
		listHeader.AlphabetType = list->GetAlphabetType();
		listHeader.SimpleSortedListIndex = list->GetSimpleSortedListIndex();
		listHeader.IsStylizedVariant = list->HasStylizedVariants() ? 1 : 0;
		listHeader.SearchRange = list->GetSearchRange();

		if (compressedList->GetDirectWordsShifts().size())
			listHeader.Type_DirectWordsShifts = RESOURCE_TYPE_DIRECT_SHIFT + list.index;

		if (compressedList->GetSearchTreePoints().size())
			listHeader.Type_SearchTreePoints = RESOURCE_TYPE_SEARCH_TREE_POINT + list.index;

		if (compressedList->GetAlternativeHeadwordInfo().size())
			listHeader.Type_AlternativeHeadwordsInfo = RESOURCE_TYPE_ALTERNATIVE_HEADWORD_INFO + list.index;
	}

	return ERROR_NO;
}

/// Сжимаем переводы
int CCompress::DoCompressArticles()
{
	// Производим сжатие
	return m_ArticlesCompress.DoCompress(m_Project->GetArticles(), m_Project->GetStyles(), m_DictHeader.HASH, m_threadPool);
}

/// Добавляем в словарь озвучку
int CCompress::DoAddSounds(void)
{
	assert(m_Project);

	if (m_Project->GetSoundSourceType() != eMediaSourceType_Database)
		return ERROR_NO;

	std::map<std::wstring, std::unique_ptr<CSpeexPackRead>> speexPacks;

	int globalError = ERROR_NO;
	const CSoundManager* soundManager = m_Project->GetSoundManager();
	for (UInt32 i = 1; i <= soundManager->GetSoundCount(); i++)
	{
		const TSoundElement *pSoundElement = soundManager->GetSoundBySoundIndex(i);
		if (!pSoundElement)
		{
			sldILog("Error! Can't process sound with index idx=%u\n", i);
			return eMemoryNULLPointer;
		}
		
		if (pSoundElement->SpeexPackMode)
		{
			const std::wstring &packFile = pSoundElement->SpeexPackFilename;
			auto &speexPack = speexPacks[packFile];
			if (speexPack == nullptr)
			{
				speexPack.reset(new CSpeexPackRead());

				if (speexPack->Open(packFile.c_str()) != spOK)
				{
					sldILog("Error! Can't open SpeexPack archive: '%s'\n", sld::as_ref(packFile));
					return ERROR_CANT_OPEN_FILE;
				}
			}

			const UInt8* data = NULL;
			UInt32 dataSize = 0;
			const std::wstring &soundFile = pSoundElement->SoundName;

			speexPack->GetFileData(soundFile.c_str(), &data, &dataSize);
			if (dataSize == 0)
			{
				sldILog("Error! Can't extract file '%s' from SpeexPack archive '%s'\n",
						sld::as_ref(soundFile), sld::as_ref(packFile));
				return SDC_READ_CANT_READ;
			}

			int error = m_SDC.AddResource(data, dataSize, RESOURCE_TYPE_SOUND, i);
			if (error != SDC_OK)
			{
				sldILog("Error! Can't add sound data for file '%s' from SpeexPack archive '%s'\n",
						sld::as_ref(soundFile), sld::as_ref(packFile));
				globalError = error;
			}
		}
		else
		{
			const std::wstring &soundFile = pSoundElement->FullSoundName;
			int error = m_SDC.AddResource(soundFile, RESOURCE_TYPE_SOUND, i);
			if (error != SDC_OK)
			{
				sldILog("Error! Can't add sound data for file: '%s'\n", sld::as_ref(soundFile));
				globalError = error;
			}
		}
	}

	return globalError;
}

template <typename FileNameGetter>
static int addFileResources(CCompress &aCompress, UInt32 aCount, UInt32 aType,
							FileNameGetter aGetFilename, bool aFillResInfo)
{
	int globalError = ERROR_NO;
	for (UInt32 i = 1; i <= aCount; i++)
	{
		const std::wstring fileName = aGetFilename(i);
		int error = aCompress.m_SDC.AddResource(fileName, aType, i);
		if (error != SDC_OK)
		{
			sldILog("Error! Can't open file: '%s'\n", sld::as_ref(fileName));
			globalError = error;
		}
		if (aFillResInfo)
			aCompress.FillResourceInfo(aType, fileName);
	}
	return globalError;
}

/// Добавляем в словарь абстрактные ресурсы
int CCompress::DoAddAbstractResources(void)
{
	const CAbstractItemManager* manager = m_Project->GetAbstractItemManager();
	return addFileResources(*this, manager->GetItemsCount(), RESOURCE_TYPE_ABSTRACT,
							[manager](UInt32 i) {return manager->GetFullItemNameByItemIndex(i); }, true);
}

/// Добавляем в словарь картинки
int CCompress::DoAddPictures(void)
{
	if (m_Project->GetImageSourceType() != eMediaSourceType_Database)
		return ERROR_NO;

	const CImageManager* manager = m_Project->GetImageManager();
	// Нумерация картинок начинается с 1, 0 используется как признак отсутствия картинки
	FillResourceInfo(RESOURCE_TYPE_IMAGE, L"");
	return addFileResources(*this, manager->GetImageCount(), RESOURCE_TYPE_IMAGE,
							[manager](UInt32 i) {return manager->GetFullImageNameByImageIndex(i); }, true);
}

/// Добавляем в словарь видео
int CCompress::DoAddVideos(void)
{
	if (m_Project->GetVideoSourceType() != eMediaSourceType_Database)
		return ERROR_NO;

	const CVideoManager* manager = m_Project->GetVideoManager();
	return addFileResources(*this, manager->GetVideoCount(), RESOURCE_TYPE_VIDEO,
							[manager](UInt32 i) {return manager->GetFullVideoNameByVideoIndex(i); }, false);
}

/// Добавляем в словарь 3d модели
int CCompress::DoAddModels(void)
{
	if (m_Project->GetMeshSourceType() != eMediaSourceType_Database)
		return ERROR_NO;

	const CMeshManager* manager = m_Project->GetMeshManager();
	return addFileResources(*this, manager->GetMeshCount(), RESOURCE_TYPE_MESH,
							[manager](UInt32 i) {return manager->GetFullMeshNameByMeshIndex(i); }, false);
}

/// Добавляем в словарь 3d материалы
int CCompress::DoAddMaterial(void)
{
	assert(m_Project);

	if (m_Project->GetMaterialSourceType() != eMediaSourceType_Database)
		return ERROR_NO;

	int errorGlobal = ERROR_NO;
	const CMaterialManager* materialManager = m_Project->GetMaterialManager();
	for (UInt32 i = 1; i <= materialManager->GetMaterialCount(); i++)
	{
		const CMaterial *mat = materialManager->GetMaterialByIndex(i);
		if (!mat)
			return ERROR_WRONG_INDEX;

		int error = m_SDC.AddResource(mat->Serialize(), RESOURCE_TYPE_MATERIAL, i);
		if (error != SDC_OK)
		{
			const std::wstring fileName = materialManager->GetFullMaterialNameByMaterialIndex(i);
			sldILog("Error! Can't add material: '%s'\n", sld::as_ref(fileName));
			errorGlobal = error;
		}
	}
	return errorGlobal;
}


/// Добавляем в словарь 3d сцены
int CCompress::DoAddScene(void)
{
	assert(m_Project);

	if (m_Project->GetSceneSourceType() != eMediaSourceType_Database)
		return ERROR_NO;

	int errorGlobal = ERROR_NO;
	const CSceneManager* sceneManager = m_Project->GetSceneManager();
	for (UInt32 i = 1; i <= sceneManager->GetSceneCount(); i++)
	{
		const CScene *scene = sceneManager->GetSceneByIndex(i);
		if (!scene)
			return ERROR_WRONG_INDEX;

		int error = m_SDC.AddResource(scene->Serialize(), RESOURCE_TYPE_SCENE, i);
		if (error != SDC_OK)
		{
			const std::wstring fileName = sceneManager->GetFullSceneNameBySceneIndex(i);
			sldILog("Error! Can't add scene: '%s'\n", sld::as_ref(fileName));
			errorGlobal = error;
		}
	}
	return errorGlobal;
}

/// Добавляем в словарь индекс элементарных объектов
int CCompress::DoAddAtomicObjectIndex()
{
	if (m_AtomicObjects.empty())
		return ERROR_NO;

	SDCError error = m_SDC.AddResource(m_AtomicObjects.data(),
									   m_AtomicObjects.size() * sizeof(m_AtomicObjects[0]),
									   SLD_AO_ARTICLE_TABLE, 0);
	if (error != SDC_OK)
		sldILog("Error! Can't add atomic objects\n");
	return error == SDC_OK ? ERROR_NO : error;
}

/// Добавляем в словарь информацию о связи с другими базами
int CCompress::DoAddExternContentInfo()
{
	SDCError error;
	
	// Количество
	UInt8 count = 0;
	for (int i = 0; i < eExternContentTypeMax; i++)
	{
		if (!SlovoEdProject::m_ExternContent[i].empty())
			count++;
	}

	// Пытаемся максимально сохранить бинарную совместимость баз.
	// Если ссылок на другие базы нет, то не записываем вообще ничего.
	if (count == 0)
		return ERROR_NO;

	error = m_SDC.AddResource((void*)&count, sizeof(UInt8), SLD_EXTERN_CONTENT_INFO, 0);
	if (error != SDC_OK)
	{
		sldILog("Error! Can't write extern content info count.\n");
		return error;
	}

	int currentResNum = 1;
	// Сама информация
	for (UInt32 i = 0; i < eExternContentTypeMax; i++)
	{
		if (!SlovoEdProject::m_ExternContent[i].empty())
		{
			char buffer[2048] = {0};
			void* pb = buffer;
			pb = memmoveshift(pb, &i, sizeof(i));

			UInt32 size = SlovoEdProject::m_ExternContent[i].size();
			pb = memmoveshift(pb, &size, sizeof(size));
			for (auto it = SlovoEdProject::m_ExternContent[i].begin(); it != SlovoEdProject::m_ExternContent[i].end(); it++)
			{
				UInt32 length = it->first.length();
				const wchar_t* str = it->first.c_str();
				pb = memmoveshift(pb, &length, sizeof(length));
				const std::u16string s = sld::as_utf16(str);
				pb = memmoveshift(pb, s.c_str(), sizeof(UInt16) * length);
				pb = memmoveshift(pb, &it->second, sizeof(it->second));
			}
			
			error = m_SDC.AddResource(buffer, static_cast<char*>(pb) - buffer, SLD_EXTERN_CONTENT_INFO, currentResNum);
			if (error != SDC_OK)
			{
				sldILog("Error! Can't write extern content info!\n");
				return error;
			}
			currentResNum++;
		}
	}

	return ERROR_NO;
}
/// Добавляем в словарь таблицу информации о ресурсах
int CCompress::DoAddResourceInfo()
{
	SDCError error;

	// Количество типов ресурсов
	UInt16 count = m_ResourceInfo.size();

	// Пытаемся максимально сохранить бинарную совместимость баз.
	// Если ссылок на другие базы нет, то не записываем вообще ничего.
	if (count == 0)
		return ERROR_NO;

	// Записываем число типов ресурсов
	error = m_SDC.AddResource((void*)&count, sizeof(UInt16), RESOURCE_TYPE_RESOURCE_INFO, 0);
	if (error != SDC_OK)
	{
		sldILog("Error! CCompress::DoAddResourceInfo - Can't write resource types count.\n");
		return error;
	}

	// Записываем таблицу смещений на каждый тип ресурсов
	UInt32 shift = count + 1;
	UInt32 index = 1;
	for (auto info = m_ResourceInfo.begin(); info != m_ResourceInfo.end(); info++)
	{
		UInt32 tmp[2];
		tmp[0] = info->first;
		tmp[1] = shift;
		error = m_SDC.AddResource(tmp, sizeof(UInt32) * 2, RESOURCE_TYPE_RESOURCE_INFO, index++);
		if (error != SDC_OK)
		{
			sldILog("Error! CCompress::DoAddResourceInfo - Can't write resource shift table.\n");
			return error;
		}
		shift += info->second.size();
	}

	// Сама информация
	for (auto info = m_ResourceInfo.begin(); info != m_ResourceInfo.end(); info++)
	{
		for (auto data = info->second.begin(); data != info->second.end(); data++)
		{
			error = m_SDC.AddResource(&(*data), sizeof(TResourceInfo), RESOURCE_TYPE_RESOURCE_INFO, index++);
			if (error != SDC_OK)
			{
				sldILog("Error! CCompress::DoAddResourceInfo - Can't write resource info data.\n");
				return error;
			}
		}
	}

	return ERROR_NO;
}



/// Добавляем в словарь имена ресурсов
int CCompress::DoAddResourceNames()
{
	SDCError error;

	// Количество
	UInt16 count = m_ResourceNames.size();

	// Пытаемся максимально сохранить бинарную совместимость баз.
	// Если ссылок на другие базы нет, то не записываем вообще ничего.
	if (count == 0)
		return ERROR_NO;

	error = m_SDC.AddResource((void*)&count, sizeof(UInt16), RESOURCE_TYPE_RESOURCE_NAMES, 0);
	if (error != SDC_OK)
	{
		sldILog("Error! Can't write resource names count.\n");
		return error;
	}

	// Сама информация
	for (int i = 0; i < count; i++)
	{
		const sld::wstring_ref name = m_ResourceNames[i];
		const size_t slash = name.find_last_of(L"\\/");
		const std::u16string s = sld::as_utf16(name.substr(slash == name.npos ? 0 : slash + 1));
		const std::u16string path = sld::as_utf16(name.substr(0, slash == name.npos ? 0 : slash));
		error = m_SDC.AddResource((void*)s.c_str(), sizeof(UInt16) * (s.length() + 1), RESOURCE_TYPE_RESOURCE_NAMES, i + 1);
		if (error != SDC_OK)
		{
			sldILog("Error! Can't write resource names count.\n");
			return error;
		}
		error = m_SDC.AddResource((void*)path.c_str(), sizeof(UInt16) * (path.length() + 1), RESOURCE_TYPE_RESOURCE_PATHS, i + 1);
		if (error != SDC_OK)
		{
			sldILog("Error! Can't write resource paths count.\n");
			return error;
		}
	}

	return ERROR_NO;
}

/// Обновляем заголовок словаря
int CCompress::DoUpdateHeader()
{
	m_DictHeader.WordtypeSize = m_ArticlesCompress.GetMaxWordtypeSize();
	m_DictHeader.ArticlesBufferSize = m_ArticlesCompress.GetMaxBufferSize();
	
	// TODO: при вычислении HASH сделать в старшем слове хранение контрольной суммы
	UInt32 newHASH = m_DictHeader.HASH;
	newHASH ^= m_DictHeader.DictID;
	newHASH ^= m_DictHeader.NumberOfArticles;
	m_DictHeader.HASH = newHASH;

	// Если поле m_DictHeader.MarketingTotalWordsCount==0, заполним его автоматически
	if (!m_DictHeader.MarketingTotalWordsCount)
	{
		CArticles* pArticles = m_Project->GetArticles();
		m_DictHeader.MarketingTotalWordsCount = pArticles->GetNumberOfArticles();
	}

	return ERROR_NO;
}

/// Добавляем все ресурсы словаря в SDC.
int CCompress::DoAddResourcesToSDC()
{
	int error;

	// Дополнительная информация
	if (m_additionalInfo.StructSize)
	{
		error = m_SDC.AddResource(&m_additionalInfo, m_additionalInfo.StructSize, SLD_RESOURCE_INFO, eInfoIndex_CommonInfo);
		if (error != SDC_OK)
			return error;
	}

	// Аннотация к базе
	if (!m_Annotation.empty())
	{
		const std::u16string s = sld::as_utf16(m_Annotation);
		error = m_SDC.AddResource((void*)s.c_str(), (s.length() + 1) * sizeof(s[0]), SLD_RESOURCE_INFO, eInfoIndex_Annotation);
		if (error != SDC_OK)
			return error;
	}
	
	// Информация о версии словарной базы
	const TDictionaryVersionInfo* pDictVersionInfo = m_Project->GetDictionaryVersionInfo();
	error = m_SDC.AddResource(pDictVersionInfo, pDictVersionInfo->structSize, SLD_RESOURCE_DICTIONARY_VERSION_INFO, 0);
	if (error != SDC_OK)
		return error;

	// Локализованные строки словаря в целом
	for (UInt32 i : xrange(m_Project->GetStringsCount()))
	{
		const TLocalizedNames* ptr = m_Project->GetStrings(i);
		error = m_SDC.AddResource(ptr, ptr->structSize, SLD_RESOURCE_STRINGS, i);
		if (error != SDC_OK)
			return error;
	}
	
	//  Локализованные строки списков слов
	for (auto&& list : enumerate(m_Project->lists()))
	{
		const UInt32 recCount = list->GetLocalizedListStringsCount();
		if (recCount == 0)
			continue;
		
		const UInt32 recSize = sizeof(TListLocalizedNames);
		MemoryBuffer resMem(recCount * recSize);
		
		for (UInt32 j : xrange(recCount))
		{
			const TListLocalizedNames* ptr = list->GetLocalizedListStringsByIndex(j);
			if (!ptr)
				return SDC_MEM_NULL_POINTER;

			memcpy(resMem.data() + j * recSize, ptr, recSize);
		}

		error = m_SDC.AddResource(std::move(resMem), SLD_RESOURCE_LIST_STRINGS, list.index);
		if (error != SDC_OK)
			return error;

		TListHeader &listHeader = m_ListHeader[list.index];
		listHeader.LocalizedStringsResourceIndex = static_cast<UInt32>(list.index);
		listHeader.LocalizedStringsRecordsCount = recCount;
	}

	//  Таблицы свойств вариантов написания списков слов
	for (auto&& list : enumerate(m_Project->lists()))
	{
		TListVariantProperty prop = {};

		const UInt32 recCount = list->GetNumberOfVariants();
		const UInt32 recSize = sizeof(prop);
		MemoryBuffer resMem(recCount * recSize);

		for (UInt32 j : xrange(recCount))
		{
			const ListVariantProperty* ptr = list->GetVariantProperty(j);
			if (!ptr)
				return SDC_MEM_NULL_POINTER;

			prop.Type = ptr->Type;
			prop.LangCode = ptr->LangCode;
			prop.Number = j;
			memcpy(resMem.data() + j * recSize, &prop, recSize);
		}

		error = m_SDC.AddResource(std::move(resMem), SLD_RESOURCE_VARIANTS_PROPERTY_TABLE, list.index);
		if (error != SDC_OK)
			return error;

		m_ListHeader[list.index].VariantsPropertyResourceIndex = static_cast<UInt32>(list.index);
	}

	error = DoAddBinaryDataList();
	if (error != ERROR_NO)
		return error;

	// Заголовки списков слов
	error = m_SDC.AddResource(m_ListHeader, m_Project->GetNumberOfLists()*sizeof(m_ListHeader[0]),
								 SLD_RESOURCE_HEADER, 1);
	if (error != SDC_OK)
		return error;

	// Добавляем списки слов
	for (UInt32 i : xrange(m_Project->GetNumberOfLists()))
	{
		error = DoAddListResourcesToSDC(i);
		if (error != ERROR_NO)
			return error;
	}

	// файл с Java Script'ом
	if (m_Project->GetJavaScriptFileContents().size())
	{
		MemoryBuffer buf;
		try {
			CompressionConfig config = m_Project->GetCompressionMethod();
			buf = StringStore::compress(m_Project->GetJavaScriptFileContents(), config, m_threadPool);
		}
		catch (sld::exception &e) {
			return e.error();
		}

		error = m_SDC.AddResource(std::move(buf), SLD_RESOURCE_ARTICLES_JAVA_SCRIPT, 0);
		if (error != SDC_OK)
			return error;
	}

	using F = int (CCompress::*)();
	static const F funcs[] = {
		&CCompress::DoAddSortTables,
		&CCompress::DoAddMophology,
		&CCompress::DoAddArticlesData,
		&CCompress::DoAddPictures,
		&CCompress::DoAddVideos,
		&CCompress::DoAddModels,
		&CCompress::DoAddMaterial,
		&CCompress::DoAddScene,
		&CCompress::DoAddSounds,
		&CCompress::DoAddThematics,
		&CCompress::DoAddAbstractResources,
		&CCompress::DoAddAtomicObjectIndex,
		&CCompress::DoAddExternContentInfo,
		&CCompress::DoAddResourceNames,
		&CCompress::DoAddResourceInfo
	};

	for (auto&& func : funcs)
	{
		error = (this->*func)();
		if (error != ERROR_NO)
			return error;
	}

	// Заголовок словаря.
	return m_SDC.AddResource(&m_DictHeader, m_DictHeader.HeaderSize, SLD_RESOURCE_HEADER, 0);
}

/// Добавляем ресурсы со статьями
int CCompress::DoAddArticlesData()
{
	int error;

	// Деревья
	const UInt8* ptr = NULL;
	UInt32 ptrSize = 0;
	for (UInt32 t : xrange(m_ArticlesCompress.GetCompressedTreeCount()))
	{
		error = m_ArticlesCompress.GetCompressedTree(t, &ptr, &ptrSize);
		if (error != ERROR_NO)
			return error;
		if (ptrSize)
		{
			error = m_SDC.AddResource(ptr, ptrSize, m_DictHeader.articlesTreeType, t);
			if (error != SDC_OK)
				return error;
		}
	}

	// Стили
	{
		// stuff styles into several resources, 256kb large at most (as allocating more is
		// kinda hostile on mobile devices)
		const uint32_t maxResourceSize = 1u << 18;
		std::vector<UInt8> data;
		const uint32_t styleCount = m_ArticlesCompress.GetStylesCount();
		for (uint32_t resourceIndex = 0, styleIndex = 0; styleIndex < styleCount; resourceIndex++)
		{
			for (; data.size() < maxResourceSize && styleIndex < styleCount; styleIndex++)
			{
				error = m_ArticlesCompress.GetStyleData(styleIndex, &ptr, &ptrSize);
				if (error != ERROR_NO)
					return error;

				data.insert(data.end(), ptr, ptr + ptrSize);
			}

			error = m_SDC.AddResource(data.data(), data.size(), m_DictHeader.articlesStyleType, resourceIndex);
			if (error != SDC_OK)
				return error;

			data.clear();
		}

		m_DictHeader.HasPackedStyles = 1;
	}

	// Сжатые данные
	auto resData = m_ArticlesCompress.GetCompressedData();
	if (resData.size())
	{
		error = m_SDC.AddResource(resData, m_DictHeader.articlesDataType, 0, MAX_COMPRESSED_DATA_RESOURCE_SIZE);
		if (error != SDC_OK)
			return error;
	}

	// Таблица быстрого доступа
	resData = m_ArticlesCompress.GetQA();
	if (resData.size())
	{
		if (resData.size() > MAX_QA_DATA_RESOURCE_SIZE)
		{
			STString<128> logbuf(L"Warning! May be a too big resource size of \"QA Table for Articles\" resource for some platforms: %u bytes", resData.size());
			sldILog("%s\n", logbuf);
			sld::printf(eLogStatus_Warning, "\n%s", logbuf);
		}
		error = m_SDC.AddResource(resData, m_DictHeader.articlesQAType, 0);
		if (error != SDC_OK)
			return error;
	}

	return ERROR_NO;
}

/// Добавляет таблицы сортировки в контейнер
int CCompress::DoAddSortTables()
{
	CSortTablesManager* pSortManager = m_Project->GetSortTablesManager();
	assert(pSortManager);

	const UInt32 Count = pSortManager->GetTotalSortTablesCount();

	TCMPTablesHeader tableHeader = {};
	tableHeader.structSize = sizeof(tableHeader);
	tableHeader.HeaderVersion = 1;
	tableHeader.NumberOfCMPtables = Count;

	MemoryBuffer resMem(sizeof(TCMPTableElement) * Count + sizeof(tableHeader));
	memcpy(resMem.data(), &tableHeader, sizeof(tableHeader));
	uint8_t *resData = resMem.data() + sizeof(tableHeader);

	UInt32 ResourceIdx = 0;
	auto addTable = [&](UInt32 langCode, const MemoryBuffer &data, UInt32 priority, const std::wstring &file) {
		if (data.empty())
			return ERROR_NO;

		TCMPTableElement elem = {};
		elem.LanguageCode = langCode;
		elem.ResourceType = SLD_RESOURCE_COMPARE_TABLE;
		elem.ResourceIndex = ResourceIdx;
		elem.Priority = priority;

		memcpy(resData + sizeof(elem) * ResourceIdx, &elem, sizeof(elem));

		int error = m_SDC.AddResource(data, SLD_RESOURCE_COMPARE_TABLE, ResourceIdx);
		if (error != SDC_OK)
			return error;

		FillResourceInfo(SLD_RESOURCE_COMPARE_TABLE, file);

		ResourceIdx++;
		return ERROR_NO;
	};

	for (const TLanguageSortTable &table : pSortManager->tables())
	{
		int error = addTable(table.LangCode, table.MainTable, 0, table.MainFileName);
		if (error != ERROR_NO)
			return error;

		error = addTable(table.LangCode, table.SecondTable, 1, table.SecondFileName);
		if (error != ERROR_NO)
			return error;
	}

	return m_SDC.AddResource(std::move(resMem), SLD_RESOURCE_COMPARE_TABLE_HEADER, 0);
}

/// Добавляет встроенные базы морфологии в контейнер
int CCompress::DoAddMophology()
{
	// Базы морфологии
	const CMorphoDataManager* pMorphoManager = m_Project->GetMorphoDataManager();
	assert(pMorphoManager);

	const UInt32 Count = pMorphoManager->GetMorphoBasesCount();

	MemoryBuffer resMem(sizeof(TMorphoBaseElement) * Count + sizeof(Count));
	memcpy(resMem.data(), &Count, sizeof(Count));
	uint8_t *resData = resMem.data() + sizeof(Count);

	UInt32 ResourceIdx = 0;
	for (const TMorphoData &base : pMorphoManager->bases())
	{
		if (base.Data.empty())
			continue;

		TMorphoBaseElement elem = {};
		elem.LanguageCode = base.LangCode;
		elem.DictId = base.DictId;

		memcpy(resData + sizeof(elem) * ResourceIdx, &elem, sizeof(elem));

		int error = m_SDC.AddResource(base.Data, RESOURCE_TYPE_MORPHOLOGY_DATA, ResourceIdx, CSDCWrite::NoCompression);
		if (error != SDC_OK)
			return error;
		ResourceIdx++;
	}

	return m_SDC.AddResource(std::move(resMem), RESOURCE_TYPE_MORPHOLOGY_DATA_HEADER, 0);
}

/// Добавляет ресурсы листа под заданным индексом в базу
int CCompress::DoAddListResourcesToSDC(UInt32 aListIndex)
{
	int error;

	const TListHeader &listHeader = m_ListHeader[aListIndex];
	const CListCompress *compList = m_CompressedLists[aListIndex];

	// always honor disabled list resource compression and also disable it for CharChain encoded lists
	const bool canCompress = m_resCompCfg.canCompressListResources() &&
	                         listHeader.CompressionMethod == eCompressionTypeNoCompression;

	auto addChunkedResource = [this, canCompress](MemoryRef aData, UInt32 aType) {
		if (canCompress || aData.size() <= MAX_COMPRESSED_DATA_RESOURCE_SIZE)
			return m_SDC.AddResource(aData, aType, 0, MAX_COMPRESSED_DATA_RESOURCE_SIZE);

		return m_SDC.AddResource(aData, aType, 0, MAX_COMPRESSED_DATA_RESOURCE_SIZE, CSDCWrite::NoCompression);
	};

	// Деревья
	for (UInt32 t = 0; t < compList->GetCompressedTreeCount(); t++)
	{
		const UInt8* ptr = NULL;
		UInt32 ptrSize = 0;
		error = compList->GetCompressedTree(t, &ptr, &ptrSize);
		if (error != ERROR_NO)
			return error;
		if (ptrSize)
		{
			error = m_SDC.AddResource(ptr, ptrSize, listHeader.Type_Tree, t);
			if (error != SDC_OK)
				return error;
		}
	}

	// Иерархия
	auto resData = compList->GetCompressedHierarchy();
	if (resData.size())
	{
		error = addChunkedResource(resData, listHeader.Type_Hierarchy);
		if (error != SDC_OK)
			return error;
	}

	// Сжатые данные
	resData = compList->GetCompressedData();
	error = addChunkedResource(resData, listHeader.Type_CompressedData);
	if (error != SDC_OK)
	{
		sldILog("Error! Can't add resource (type=0x%06X, index=%u, size=%u, ptr=0x%p) at list %u\n",
				listHeader.Type_CompressedData, 0, resData.size(), resData.data(), aListIndex);
		return error;
	}

	if (listHeader.Type_DirectWordsShifts)
	{
		error = addChunkedResource(compList->GetDirectWordsShifts(), listHeader.Type_DirectWordsShifts);
		if (error != SDC_OK)
			return error;

		resData = compList->GetSearchTreePoints();
		if (resData.size())
		{
			error = addChunkedResource(resData, listHeader.Type_SearchTreePoints);
			if (error != SDC_OK)
				return error;
		}
	}

	if (listHeader.Type_AlternativeHeadwordsInfo)
	{
		error = m_SDC.AddResource(compList->GetAlternativeHeadwordInfo(), listHeader.Type_AlternativeHeadwordsInfo, 0);
		if (error != SDC_OK)
			return error;
	}

	// Индексы: количество
	resData = compList->GetIndexesCount();
	if (resData.size())
	{
		error = addChunkedResource(resData, listHeader.Type_IndexesCount);
		if (error != SDC_OK)
			return error;
	}

	// Индексы: данные
	resData = compList->GetIndexesData();
	if (resData.size())
	{
		error = addChunkedResource(resData, listHeader.Type_IndexesData);
		if (error != SDC_OK)
			return error;
	}

	// Индексы: таблица быстрого доступа
	resData = compList->GetIndexesQA();
	if (resData.size())
	{
		if (resData.size() > MAX_QA_DATA_RESOURCE_SIZE)
		{
			STString<512> logmsg(L"Warning! May be a too big resource size of \"QA Table for List Indexes\" resource for some platforms: %u bytes", resData.size());
			sldILog("%s\n", logmsg);
			sld::printf(eLogStatus_Warning, "\n%s", logmsg);
		}
		error = m_SDC.AddResource(resData, listHeader.Type_IndexesQA, 0);
		if (error != SDC_OK)
			return error;
	}

	// Индексы: заголовок
	const TIndexesHeader *indexesHeader = compList->GetIndexesHeader();
	if (resData.size())
	{
		error = m_SDC.AddResource(indexesHeader, indexesHeader->structSize, listHeader.Type_IndexesHeader, 0);
		if (error != SDC_OK)
			return error;
	}

	return ERROR_NO;
}

/// Добавляет лист с бинарными данными
int CCompress::DoAddBinaryDataList()
{
	for (const CWordList &list : m_Project->lists())
	{
		if (list.GetType() != eWordListType_BinaryResource)
			continue;

		for (auto&& element : enumerate(list.elements()))
		{
			wstring fullFileName = list.GetResourceFolderName() + element->Name[0];
			str_all_string_replace(fullFileName, L"/", L"\\"); // XXX: Why???

			auto fileData = sld::read_file(fullFileName);
			if (fileData.empty())
			{
				STString<1024> logmsg(L"Warning! Can't read file: '%s'", fullFileName.c_str());
				sldILog("%s\n", logmsg);
				sld::printf(eLogStatus_Warning, "\n%s", logmsg);
				continue;
			}

			const size_t size = fileData.size();
			int error = m_SDC.AddResource(std::move(fileData), RESOURCE_TYPE_BINARY,
										  static_cast<UInt32>(element.index));
			if (error != SDC_OK)
			{
				STString<1024> logmsg(L"Error! Can't add file '%s' (size: %lu)", fullFileName.c_str(), size);
				sldILog("%s\n", logmsg);
				sld::printf(eLogStatus_Warning, "\n%s", logmsg);
				return error;
			}
		}
		break;
	}
	return ERROR_NO;
}

/// Добавляем в словарь тематики блоков switch
int CCompress::DoAddThematics( void )
{
	// если тематик нет, то ресурс не добавляем
	if (!m_DictHeader.HasSwitchThematics)
		return ERROR_NO;

	UInt32 articlesCount = m_Project->GetArticles()->GetNumberOfArticles();
	if (articlesCount == 0)
		return ERROR_NO;

	UInt32 thematicsBitMap = 0;
	for (const auto &thematicMapIt : CArticle::m_ThematicToStatesNumberMap)
	{
		thematicsBitMap += (1 << thematicMapIt.first);
	}

	return m_SDC.AddResource(&thematicsBitMap, sizeof(thematicsBitMap), RESOURCE_TYPE_THEMATIC, 0);
}

template <typename ResourceGenCallback>
static int addResources(CSDCWrite &sdc, uint32_t count, uint32_t type, ResourceGenCallback generate)
{
	for (uint32_t index : xrange(count))
	{
		int err = sdc.AddResource(generate(index), type, index);
		if (err != ERROR_NO)
			return err;
	}
	return ERROR_NO;
}

/// Обрабатываем структурированные метаданные
int CCompress::DoAddStructuredMetadata()
{
	std::unique_ptr<MetadataManager::Optimized> metaData;
	// Check if we actually have any metadata parsed
	const MetadataManager *metadataManager = m_Project->GetMetadataManager();
	if (metadataManager->metadataCount() != 0)
	{
		// get "optimized" metadata
		metaData.reset(new MetadataManager::Optimized(metadataManager->optimized()));
		if (metaData->metadataCount() != 0)
		{
			// Actually adding metadata resources
			int err = m_SDC.AddResource(metaData->generateHeader(), SLD_RESOURCE_STRUCTURED_METADATA_HEADER, 0);
			if (err != ERROR_NO)
				return err;

			err = addResources(m_SDC, metaData->resourceCount(), SLD_RESOURCE_STRUCTURED_METADATA_DATA,
							   [&metaData](uint32_t index) { return metaData->generateResource(index); });
			if (err != ERROR_NO)
				return err;
		}
	}

	// bail out when we don't have both structured metadata and css blocks
	if (metaData == nullptr && m_Project->GetCSSDataManager()->blocksResourceCount() == 0)
		return ERROR_NO;

	// rewrite metadata indexes
	for (CArticle &article : m_Project->articles())
	{
		for (BlockType &block : article.blocks())
		{
			// don't bother handling closing tags
			if (block.isClosing)
				continue;

			const auto metadataRef = block.metadata();
			if (metadataRef && metaData)
			{
				const uint32_t newIndex = metaData->getIndex(metadataRef);
				if (newIndex == InvalidMetadataIndex)
				{
					sldILog("Internal Error! CCompress::DoAddStructuredMetadata : invalid metadata index!\n");
					return ERROR_NULL_POINTER; // XXX
				}
				else if (newIndex != RemovedMetadataIndex)
				{
					// the format for structured metadata "text" is `@<id>`
					// we also don't expect to have any other text, so clear it
					block.text = STString<32>(L"@%x", newIndex).c_str();
				}
			}

			// Add css block index, the format is `#<id>`
			if (block.cssBlockIndex() != InvalidCSSBlockIndex)
				block.text += STString<32>(L"#%x", block.cssBlockIndex()).c_str();

			// Add embedded string
			if (metadataRef && metaData)
				metaData->appendBlockString(metadataRef, block.text);
		}
	}

	return ERROR_NO;
}

/// Обрабатываем css метаданные
int CCompress::DoAddCSSData()
{
	// NOTE: CSS Block indexes are added to the blocks inside DoAddStructuredMetadata

	// Check if we actually have any css blocks parsed
	const CSSDataManager *cssDataManager = m_Project->GetCSSDataManager();

	// log if we got some unused css classes
	const auto unusedClasses = cssDataManager->unusedCSSClasses();
	if (unusedClasses.size())
	{
		// XXX: We can actually log the full list here.
		sldXLog("Warning! The db has %lu unused css classes.\n", unusedClasses.size());
	}

	// bail out if don't have any css stylization
	if (cssDataManager->blocksResourceCount() == 0)
		return ERROR_NO;

	// Add a string store of css strings...
	int err;
	try {
		StringStore stringStore = cssDataManager->generateStringStore(m_Project->GetCompressionMethod(), m_threadPool);
		err = addResources(m_SDC, stringStore.resourceCount(), cssDataManager->stringsResourceType(),
		                   [&stringStore](uint32_t index) { return stringStore.getResourceBlob(index); });
	}
	catch (sld::exception &e) {
		err = e.error();
	}
	if (err != ERROR_NO)
		return err;

	// Actually adding css resources
	err = m_SDC.AddResource(cssDataManager->generateHeader(), SLD_RESOURCE_CSS_DATA_HEADER, 0);
	if (err != ERROR_NO)
		return err;

	// css props
	err = addResources(m_SDC, cssDataManager->propsResourceCount(), SLD_RESOURCE_CSS_DATA_PROPERTIES,
	                   [&cssDataManager](uint32_t index) {
	                       return cssDataManager->generatePropsResource(index);
	                   });
	if (err != ERROR_NO)
		return err;

	// css blocks
	return addResources(m_SDC, cssDataManager->blocksResourceCount(), SLD_RESOURCE_CSS_DATA_BLOCKS,
	                    [&cssDataManager](uint32_t index) {
	                        return cssDataManager->generateBlocksResource(index);
	                    });
}

/// Добавляет новое имя ресурса и определяет его индекс
void CCompress::FillResourceInfo(UInt32 aResType, const wstring& aResName)
{
	TResourceInfo info;
	auto it = std::find(m_ResourceNames.begin(), m_ResourceNames.end(), aResName);
	if (it != m_ResourceNames.end())
	{
		info.NameIndex = std::distance(m_ResourceNames.begin(), it) + 1;
	}
	else
	{
		m_ResourceNames.push_back(aResName);
		info.NameIndex = m_ResourceNames.size();
	}
	m_ResourceInfo[aResType].push_back(info);

}

} // anon namespace

/**
 * Производит сжатие и упаковку словаря в контейнер
 *
 * @param[in] aThreadPool - ссылка на thread pool используемый при сжатии
 * @param[in] aProj       - указатель на объект словаря для сжатия
 * @param[in] aConfig     - настройки для сжатия
 *
 * @return код ошибки
 */
int Compress(ThreadPool &aThreadPool, SlovoEdProject *aProj, const CompressConfig &aConfig)
{
	assert(aProj);

	CCompress compressor(aThreadPool, aProj, aConfig);
	return compressor.DoCompress();
}
