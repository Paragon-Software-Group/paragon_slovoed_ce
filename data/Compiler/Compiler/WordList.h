#pragma once

#include <map>
#include <set>
#include <unordered_set>
#include <vector>

#include "sld_Types.h"
#include "sld2_Error.h"
#include "string_ref.h"
#include "util.h"

/// Признак того, что данный элемент является конечным и нет ссылок на следующий уровень
#define LIST_END	(0xFFFFFFFFUL)

/// Признак того, что форматированных вариантов написания нет
#define STYLIZED_VARIANT_NO	(0xFFFFFFFFUL)

/// Признак того, что мешей нет
#define MESH_NO	(0xFFFFFFFFUL)

/// Признак того, что материалов нет
#define MATERIAL_NO	(0xFFFFFFFFUL)


/// Структура, описывающая ссылку на перевод в списке слов
struct ArticleRefStruct
{
	/// Конструктор
	ArticleRefStruct()
	{
		ID.clear();
		index = -1;
		shift = 0;
		initial_list_idx = -1;
		initial_list_entry_idx = -1;
	}

	/// Идентификатор перевода (статьи)
	std::wstring ID;

	/// Номер перевода
	UInt32 index;

	/// Смещение в статье (используется для списков полнотекстового поиска)
	UInt32 shift;

	/// Номер иерархического списка слов (оглавления), где есть запись, которая ссылается на то же значение, что и (list_idx, entry_idx)
	UInt32 initial_list_idx;

	/// Номер записи в иерархическом списке слов
	UInt32 initial_list_entry_idx;
};


/// Структура, описывающая ссылку на запись в списке слов
struct TListEntryRef
{
	/// Конструктор
	TListEntryRef(void)
	{
		list_idx = -1;
		entry_idx = -1;
		translation_idx = -1;
		shift = -1;
	}

	/// Номер списка слов
	UInt32 list_idx;

	/// Номер записи в списке слов (номер слова)
	UInt32 entry_idx;

	/// Номер перевода в записи списка слов
	UInt32 translation_idx;

	/// Смещение в статье
	UInt32 shift;
};


/// Структура описывает настройки списка слов полнотекстового поиска
struct TFullTextSearchOptions
{
	UInt32 IsOptionsPresent;
	EFullTextSearchLinkType LinkType;
	UInt32 ShiftType;
	UInt32 Priority;
	bool isSuffixSearch;
	bool isFullMorphologyWordForms;
	UInt32 MorphologyId;
	/// Массив id списков слов, на которые может ссылаться список
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

/// Структура, описывающая один элемент каталога/списка слов.
struct ListElement
{
	/// Уникальный идентификатор
	std::wstring Id;

	/// Название элемента
	std::vector<std::wstring> Name;

	/// Альтернативные заголовки
	std::set<std::wstring> AltName;

	/// Названия тегов со стилями
	std::map<UInt16, std::wstring> Styles;

	/// Идентификаторы статей (здесь же хранится и количество статей для данного элемента)
	std::vector<ArticleRefStruct> ArticleRefs;

	/// Ссылки на записи в списках слов, используются как аналог ArticleRefs для списков полнотекстового поиска
	std::vector<TListEntryRef> ListEntryRefs;

	/// Номер картинки для данного слова. Если картинки нет, тогда #PICTURE_NO
	UInt32 PictureIndex;

	/// Номер видео для данного слова. Если видео нет, тогда #VIDEO_NO
	UInt32 VideoIndex;

	/// Номер озвучки для данного слова. Если озвучки нет, тогда #SOUND_NO
	UInt32 SoundIndex;

	/// Номер сцены для данного слова. Если видео нет, тогда #SCENE_NO
	UInt32 SceneIndex;

	/// Глубина данного элемента
	UInt32 Level;

	/// Количество элементов до следующего элемента дерева
	UInt32 Shift2Next;

	/// Флаг, нужно ли сортировать детей 
	bool childrenIsSorted;

	/// Тип уровня иерархии
	EHierarchyLevelType HierarchyLevelType;

	/// Если слово - альтернативный заголовок - здесь содержаться ID элементов, на которые ссылается данное слово
	std::set<std::wstring> IDForAlternativeWords;

	/// Если слово - альтернативный заголовок - здесь содержаться индексы элементов, на которые ссылается данное слово
	std::set<UInt32> IndexesForAlternativeWords;

	std::map<UInt32, std::wstring> m_StylizedVariantsArticleIndexes;

	/// Конструктор
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

	/// Возвращает строчку с заданным вариантом написания
	sld::wstring_ref GetName(UInt32 aVariant) const
	{
		return aVariant < Name.size() ? Name[aVariant] : sld::wstring_ref();
	}
};

struct ListVariantProperty
{
	/// Тип варианта написания
	EListVariantTypeEnum Type;
	/// Код языка
	UInt32 LangCode;

	ListVariantProperty(EListVariantTypeEnum aType, UInt32 aLangCode)
		: Type(aType), LangCode(aLangCode) {}
};

class CSortTablesManager;
class CStringCompare;

/// Класс хранящий данные о загруженном словарном списке
class CWordList
{
	friend class CListLoader;
public:
	/// Конструктор
	CWordList(void);

	/// Устанавливает ListId
	void SetListId(std::wstring aId);
	/// Возвращает ListId
	const std::wstring& GetListId(void) const;

	/// Возвращает количество блоков локализованных строк
	UInt32 GetLocalizedListStringsCount(void) const;

	/// Добавляем свойства варианта из уже настроенного варианта написания
	int AddVariantProperty(UInt32 aNumber, const ListVariantProperty &aVariantProperty);

	/// Возвращает указатель на структуру, содержащую свойства варианта написания, по индексу варианта
	const ListVariantProperty* GetVariantProperty(UInt32 aIndex) const;

	/// Возвращает блок локализованных строк по индексу
	const TListLocalizedNames* GetLocalizedListStringsByIndex(UInt32 aIndex) const;

	/// Возвращает количество элементов в списке
	UInt32 GetNumberOfElements() const;

	/// Возвращает указатель на элемент списка слов (const версия)
	const ListElement* GetListElement(UInt32 aIndex) const;

	/// Удаляет ссылку на перевод
	bool RemoveArticleRef(UInt32 aElementIndex, UInt32 aArticleIndex);

	/// Возвращает настройки полнотекстового поиска
	const TFullTextSearchOptions* GetFullTextSearchOptions(void) const;

	/// Устанавливает настройки полнотекстового поиска
	void SetFullTextSearchOptions(const TFullTextSearchOptions& aOptions);

	/// Устанавливает тип назначения списка.
	void SetType(UInt32 aType);
	/// Возвращает тип назначения списка.
	UInt32 GetType(void) const;
	
	/// Устанавливает флаг того, требуется ли списку слов сортировка
	void SetSorted(bool aIsSorted);
	/// Возвращает флаг того, требуется ли списку слов сортировка
	bool GetSorted() const;

	/// Устанавливает флаг того, содержит ли список локализацию слов на разные языки
	void SetLocalizedEntries(bool aIsLocalizedEntries);
	/// Возвращает флаг того, содержит ли список локализацию слов на разные языки
	bool GetLocalizedEntries() const;

	/// Устанавливает флаг того, что список слов иерархический
	void SetHierarchy(bool aIsHierarchy);
	/// Возвращает флаг того, что список слов иерархический
	bool GetHierarchy() const;

	/// Устанавливает флаг того, что список связан с другим листом
	void SetLinkedList(bool aIsLinkedList);
	/// Возвращает флаг того, что список связан с другим листом
	bool GetLinkedList() const;

	/// Возвращает флаг того, что на одно слово списка слов может приходится несколько переводов
	bool GetIsNumberOfArticles() const;

	/// Производит сортировку обычного списка слов (если возможно)
	int DoSort(const CStringCompare *aCompare);

	/// Устанавливает флаг того, что данный список слов напрямую отображается в статьи и он не 
	/// нуждается в индексах, т.к. нумерация статей и слов из списка совпадают(в пределах количества
	/// элементов в списке слов).
	void SetDirectList(bool aIsDirectList);
	/// Возвращает флаг того, что данный список слов напрямую соответствует переводам.
	bool GetDirectList() const;
	/// Возвращает флаг того, что данный список слов является списком слов полнотекстового поиска
	bool IsFullTextSearchList(void) const;
	/// Возвращает флаг, является ли тип данного списка слов типом списка слов полнотекстового поиска
	bool IsFullTextSearchListType(void) const;
	/// Возвращает флаг того, что данный список слов полнотекстового поиска является суффиксным
	UInt32 IsSuffixFullTextSearchList(void) const;
	/// Возвращает флаг того, является ла данный лист вспомогательным листом для поиска (поиск ссылается на этот лист, этот лист это "фразы"->статьи)
	bool IsFullTextSearchAuxillaryList(void) const;
	/// Возвращает флаг, содержит или нет данный список для каждого слова все словоформы
	UInt32 IsFullMorphologyWordFormsList(void) const;
	/// Возвращает ID морфологической базы, с учетом которой собирался данный список
	/// В списке должны быть только базовые формы
	UInt32 GetMorphologyId(void) const;
	
	/// Возвращает флаг, ссылается или нет на один перевод (статью) несколько элементов списка слов
	/// Если да, то в таком случае по этому списку слов нельзя сортировать статьи
	bool IsOneArticleReferencedByManyListEntries() const;

	/// Возвращает количество вариантов написания которое существует для одного слова из списка слов.
	UInt32 GetNumberOfVariants() const;

	/// Возвращает количество бит для представления количества переводов в статьях
	UInt32 GetNumberOfArticlesSize(void) const;
	/// Устанавливает количество бит для представления количества переводов в статьях
	void SetNumberOfArticlesSize(UInt32 aCount);

	/// Устанавливает флаг того, что в данном списке слов используются картинки
	void SetPicture(bool aIsListHasIcons);
	/// Возвращает флаг того, что в данном списке слов используются картинки
	bool GetPicture() const;

	/// Устанавливает флаг того, что в данном списке слов используется видео
	void SetVideo(bool aIsListHasVideo);
	/// Возвращает флаг того, что в данном списке слов используется видео
	bool GetVideo() const;

	/// Устанавливает флаг того, что в данном списке слов используется сцена
	void SetScene(bool aIsListHasScene);
	/// Возвращает флаг того, что в данном списке слов используется сцена
	bool GetScene() const;

	/// Возвращает количество бит для представления индекса картинки
	UInt32 GetPictureIndexSize() const;
	/// Устанавливает количество бит для представления индекса картинки
	void SetPictureIndexSize(UInt32 aIndexSize);

	/// Возвращает количество бит для представления индекса видео
	UInt32 GetVideoIndexSize() const;
	/// Устанавливает количество бит для представления индекса видео
	void SetVideoIndexSize(UInt32 aIndexSize);

	/// Возвращает количество бит для представления индекса сцены
	UInt32 GetSceneIndexSize() const;
	/// Устанавливает количество бит для представления индекса сцены
	void SetSceneIndexSize(UInt32 aIndexSize);
	
	/// Устанавливает флаг того, что в данном списке слов используется озвучка
	void SetSound(bool aIsListHasSounds);
	/// Возвращает флаг того, что в данном списке слов используется озвучка
	bool GetSound() const;
	/// Возвращает количество бит для представления индекса озвучки
	UInt32 GetSoundIndexSize() const;
	/// Устанавливает количество бит для представления индекса озвучки
	void SetSoundIndexSize(UInt32 aIndexSize);

	/// Возвращает иерархию подготовленную для хранения
	MemoryRef GetHierarchyData() const;

	/// Возвращает размер ссылки на перевод(в битах).
	UInt32 GetAticleIndexSize() const;
	/// Устанавливаем размер ссылки на перевод(в битах).
	void SetArticleIndexSize(UInt32 aIndexSize);

	/// Устанавливает настройки сжатия данных
	void SetCompressionMethod(CompressionConfig aConfig);
	/// Возвращает настройки сжатия данных
	CompressionConfig GetCompressionMethod(void) const;
	
	/// Возвращает код языка с которого производится перевод в данном списке слов.
	UInt32 GetLanguageCodeFrom() const;
	/// Устанавливаем код язык с которого производится перевод в данном списке слов.
	void SetLanguageCodeFrom(UInt32 aLanguageCodeFrom);

	/// Возвращает код языка на который производится перевод в данном списке слов.
	UInt32 GetLanguageCodeTo() const;
	/// Устанавливаем код язык на который производится перевод в данном списке слов.
	void SetLanguageCodeTo(UInt32 aLanguageCodeTo);

	/// Обработка загруженных данных.
	int PostProcess();
	
	/// Устанавливает количество бит, необходимых для представления номера списка, на который ссылается запись в поисковом списке
	void SetFullTextSearchLinkListIndexSize(UInt32 aValue) { m_FullTextSearch_LinkListIndexSize = aValue; };
	/// Возвращает количество бит, необходимых для представления номера списка, на который ссылается запись в поисковом списке
	UInt32 GetFullTextSearchLinkListIndexSize(void) const { return m_FullTextSearch_LinkListIndexSize; };
	
	/// Устанавливает номер списка, на который ссылаются все записи в поисковом списке в случае, если эта информация не сохраняется непосредственно в данных
	void SetFullTextSearchDefaultLinkListIndex(UInt32 aValue) { m_FullTextSearch_DefaultLinkListIndex = aValue; };
	/// Возвращает номер списка, на который ссылаются все записи в поисковом списке в случае, если эта информация не сохраняется непосредственно в данных
	UInt32 GetFullTextSearchDefaultLinkListIndex(void) const { return m_FullTextSearch_DefaultLinkListIndex; };
	
	/// Устанавливает количество бит, необходимых для представления номера слова в списке слов, на которое ссылается запись в поисковом списке
	void SetFullTextSearchLinkListEntryIndexSize(UInt32 aValue) { m_FullTextSearch_LinkListEntryIndexSize = aValue; };
	/// Возвращает количество бит, необходимых для представления номера слова в списке слов, на которое ссылается запись в поисковом списке
	UInt32 GetFullTextSearchLinkListEntryIndexSize(void) const { return m_FullTextSearch_LinkListEntryIndexSize; };
	
	/// Устанавливает количество бит, необходимых для представления номера перевода в записи списка слов, на который ссылается запись в поисковом списке
	void SetFullTextSearchLinkTranslationIndexSize(UInt32 aValue) { m_FullTextSearch_LinkTranslationIndexSize = aValue; };
	/// Возвращает количество бит, необходимых для представления номера перевода в записи списка слов, на который ссылается запись в поисковом списке
	UInt32 GetFullTextSearchLinkTranslationIndexSize(void) const { return m_FullTextSearch_LinkTranslationIndexSize; };
	
	/// Устанавливает номер перевода, на который ссылаются все записи в поисковом списке в случае, если эта информация не сохраняется непосредственно в данных
	void SetFullTextSearchDefaultLinkTranslationIndex(UInt32 aValue) { m_FullTextSearch_DefaultLinkTranslationIndex = aValue; };
	/// Возвращает номер перевода, на который ссылаются все записи в поисковом списке в случае, если эта информация не сохраняется непосредственно в данных
	UInt32 GetFullTextSearchDefaultLinkTranslationIndex(void) const { return m_FullTextSearch_DefaultLinkTranslationIndex; };
	
	/// Возвращает тип ссылок в поисковом списке
	UInt32 GetFullTextSearchShiftType(void) const { return m_FullTextSearchOptions.ShiftType; };
	
	/// Устанавливает количество бит, необходимых для представления величины смещения в статье, на которую ссылается запись в поисковом списке
	void SetFullTextSearchShiftIndexSize(UInt32 aValue) { m_FullTextSearch_ShiftIndexSize = aValue; };
	/// Возвращает количество бит, необходимых для представления величины смещения в статье, на которую ссылается запись в поисковом списке
	UInt32 GetFullTextSearchShiftIndexSize(void) const { return m_FullTextSearch_ShiftIndexSize; };

	/// Получает общее количество картинок в списке слов
	UInt32 GetTotalPictureCount(void) const;
	/// Устанавливает общее количество картинок в списке слов
	void SetTotalPictureCount(UInt32 aCount);

	/// Получает общее количество видео в списке слов
	UInt32 GetTotalVideoCount(void) const;
	/// Устанавливает общее количество видео в списке слов
	void SetTotalVideoCount(UInt32 aCount);

	/// Получает общее количество сцен в списке слов
	UInt32 GetTotalSceneCount(void) const;
	/// Устанавливает общее количество сцен в списке слов
	void SetTotalSceneCount(UInt32 aCount);

	/// Получает общее количество озвучек в списке слов
	UInt32 GetTotalSoundCount(void) const;
	/// Устанавливает общее количество озвучек в списке слов
	void SetTotalSoundCount(UInt32 aCount);

	/// Получает тип алфавита списке
	UInt32 GetAlphabetType() const { return m_AlphabetType; };
	/// Устанавливает тип алфавита в списке
	void SetAlphabetType(EAlphabetType aAlphabetType);

	/// Получает индекс слова в списке слов по id слова
	UInt32 GetIndexByName(sld::wstring_ref aName) const;

	/// Получает индекс списка, для которого данный список является вспомогательным
	Int32 GetSimpleSortedListIndex() const { return m_SimpleSortedListIndex; };

	/// Устанавливает индекс списка, для которого данный список является вспомогательным
	void SetSimpleSortedListIndex(Int32 aListIndex);

	/// Записывает несортированные индексы слов в сортированный список вторым вариантом написания
	int DoSimpleSortedList(const CWordList *aNotSortedList);

	/// Преобразует список во вспомогательный сортированный список
	int ConvertListToSimpleSortedList();

	/// Возвращает количество вариантов написания
	size_t GetVariantPropertyCount() const { return m_VariantsProperty.size(); };

		/// Устанавливает флаг того, что список имеет альтернативные заголовки
	void SetAlternativeHeadwordsFlag(bool aFlag) { m_HasAlternativeHeadwords = aFlag; }
	/// Возвращает флаг того, что список имеет альтернативные заголовки
	bool GetAlternativeHeadwordsFlag() const { return m_HasAlternativeHeadwords; }

	/// Добавляет альтернативные заголовки в список
	int AddAlternativeHeadwords();

	/// Обработка альтернативных заголовков
	int PostProccessAlternativeHeadwords();

	/// Возвращает количество слов, без учета альтернативыных заголовков
	UInt32 GetRealWordCount() const { return m_RealNumberOfWords; }
	/// Устанавливает количество слов, без учета альтернативыных заголовков
	void SetRealWordCount(UInt32 aWordsCount) { m_RealNumberOfWords = aWordsCount; }

	/// Проверяет наличие форматированных заголовков в списке слов
	bool HasStylizedVariants() const { return m_StylizedVariantsArticlesCount != 0; }

	/// Возвращает индекс статьи с форматированным вариантом написания
	UInt32 GetStylizedVariantArticleIndex(UInt32 aWordIndex, UInt32 aVariantIndex) const;
	/// Устанавливает индекс статьи с форматированным вариантом написания
	void SetStylizedVariantArticleIndex(UInt32 aWordIndex, UInt32 aVariantIndex, UInt32 aArticleIndex) { m_StylizedVariantsMap[aWordIndex][aVariantIndex] = aArticleIndex; }

	/// Проверяет наличие заданной записи в списке слов
	bool HasListEntryId(const std::wstring &aListEntryID) const { return m_mListEntryId.find(aListEntryID) != m_mListEntryId.end(); }

	/// Добавляет запись в список атомик-статей
	int AddAtomicArticle(sld::wstring_ref aAtomicID, const std::vector<std::wstring>& aExternID);

	/// Устанавливает имя папки с дополнительными ресурсами
	void SetResourceFolderName(const std::wstring& aFolderName) { m_ResourceFolderName = aFolderName; }
	/// Возвращает имя папки с дополнительными ресурсами
	const std::wstring& GetResourceFolderName() const { return m_ResourceFolderName; }

	/// Устанавливает уровни каталога, на которых будет производиться поиск
	void SetSearchRange(ESldSearchRange aSearchRange) { m_SearchRange = aSearchRange; }
	/// Возвращает уровни каталога, на которых будет производиться поиск
	ESldSearchRange GetSearchRange() const { return m_SearchRange; }

private:

	/// Производит подготовку списка слов к последующему сжатию.
	int PrepareHierarchy();

	/// Рекурсивный метод занимающийся тем, что производит обход дерева иерархии.
	int CreateHierarchy(std::vector<ListElement> &newList,
						std::vector<ListElement> &oldList, UInt32 startPos, UInt32 endPos,
						UInt8 *hierarchy, UInt32 *hierarchyStart, UInt8 aSortedFlag, EHierarchyLevelType aType);

	/// Внутренний метод добавления новых элементов листа
	int AddListElement(ListElement&& aElem);

private:

	/// Идентификатор списка слов, используется для внутренних нужд компилятора (при построении списков слов полнотекстового поиска)
	std::wstring m_ListId;

	/// Массив элементов, из которых состоит список
	std::vector<ListElement>	m_List;
	/// Массив id элементов списка слов, для проверки уникальности id
	std::unordered_set<std::wstring> m_mListEntryId;

	/// Таблица локализованных названий списка слов
	std::vector<TListLocalizedNames> m_LocalizedNames;

	/// Таблица свойств вариантов написания
	std::vector<ListVariantProperty> m_VariantsProperty;

	/// здесь будет хранится готовая иерархия(в бинарном виде)
	std::vector<uint8_t> m_hierarchy;

	//////////////////////////////////////////////////////////////////////////
	// Свойства списка слов - флаги которые включают/отключают различные возможности
	//////////////////////////////////////////////////////////////////////////

	/// тип назначение списка (EWordListTypeEnum)
	UInt32 m_Usage;
	/// требуется ли списку слов сортировка
	bool m_isSorted;
	/// флаг того, что список слов иерархический
	bool m_isHierarchy;
	/// Флаг того, что данный список слов напрямую соотносится с переводами.
	/**
		Данный список слов напрямую отображается в статьи и он не нуждается в индексах, 
		т.к. нумерация статей и слов из списка совпадают(в пределах количества элементов 
		в списке слов).
	*/
	bool m_isDirectList;

	/// Флаг того, что некоторым словам из списка слов сопоставлены картинки
	bool m_isPicture;

	/// Флаг того, что некоторым словам из списка слов сопоставлено видео
	bool m_isVideo;

	/// Флаг того, что некоторым словам из списка слов сопоставлено сцена
	bool m_isScene;

	/// Флаг того, что некоторым словам из списка слов сопоставлена озвучка
	bool m_isSound;

	/// Флаг того, что список имеет настройки локализации для слов
	bool m_isLocalizedEntriesList;

	/// Флаг того, что список связан с другим листом
	bool m_isLinkedList;

	/// Количество бит необходимых для представления числа переводов на которые ссылается одна статья.
	UInt32 m_NumberOfArticlesSize;

	/// Количество бит необходимых для представления индекса картинки
	UInt32 m_PictureIndexSize;

	/// Количество бит необходимых для представления индекса видео
	UInt32 m_VideoIndexSize;

	/// Количество бит необходимых для представления индекса сцены
	UInt32 m_SceneIndexSize;

	/// Количество бит необходимых для представления индекса озвучки
	UInt32 m_SoundIndexSize;

	/// Размер ссылки на перевод в битах.
	UInt32 m_AticleIndexSize;

	/// Код языка, с которого производится перевод в данном списке слов.
	UInt32 m_LanguageCodeFrom;

	/// Код языка, на который производится перевод в данном списке слов.
	UInt32 m_LanguageCodeTo;

	/// Тип алфавита списка
	UInt32 m_AlphabetType;

	/// Способ сжатия данных
	CompressionConfig m_compressionConfig;

	/// Параметры списка, если список является списком слов полнотекстового поиска
	TFullTextSearchOptions m_FullTextSearchOptions;
	
	/// Количество бит, необходимых для представления номера списка, на который ссылается запись в поисковом списке
	UInt32 m_FullTextSearch_LinkListIndexSize;
	/// Номер списка, на который ссылаются все записи в поисковом списке в случае, если эта информация не сохраняется непосредственно в данных
	UInt32 m_FullTextSearch_DefaultLinkListIndex;
	/// Количество бит, необходимых для представления номера слова в списке слов, на которое ссылается запись в поисковом списке
	UInt32 m_FullTextSearch_LinkListEntryIndexSize;
	/// Количество бит, необходимых для представления номера перевода в записи списка слов, на который ссылается запись в поисковом списке
	UInt32 m_FullTextSearch_LinkTranslationIndexSize;
	/// Номер перевода, на который ссылаются все записи в поисковом списке в случае, если эта информация не сохраняется непосредственно в данных
	UInt32 m_FullTextSearch_DefaultLinkTranslationIndex;
	/// Количество бит, необходимых для представления величины смещения в статье, на которую ссылается запись в поисковом списке
	UInt32 m_FullTextSearch_ShiftIndexSize;

	/// Общее количество картинок в списке слов
	/// Учитываются картинки самих слов списка (иконки) и картинки в переводах, на которые ссылаются слова списка
	UInt32 m_TotalPictureCount;

	/// Общее количество видео в списке слов
	/// Учитываются видео в переводах, на которые ссылаются слова списка
	UInt32 m_TotalVideoCount;

	/// Общее количество сцен в списке слов
	/// Учитываются сцены в переводах (3d может быть только в переводе), на которые ссылаются слова списка
	UInt32 m_TotalSceneCount;

	/// Общее количество медиа контейнеров в списке слов
	/// Учитывает медиа контейнеры в переводах (медиа контейнер может быть только в переводе)
	UInt32 m_TotalMediaContainerCount;

	/// Общее количество озвучек в списке слов
	/// Учитываются озвучки самих слов списка и озвучки в переводах, на которые ссылаются слова списка
	UInt32 m_TotalSoundCount;
	
	/// Сортирован ли верхний уровень иерархии
	bool m_ZeroLevelSorted;

	/// Индекс списка, для которого данный список является вспомогательным
	Int32 m_SimpleSortedListIndex;

	/// Флаг того, что список имеет альтернативные заголовки
	bool m_HasAlternativeHeadwords;

	/// Количество слов, без учета альтернативыных заголовков
	UInt32 m_RealNumberOfWords;

	/// Количество форматированных вариантов написания
	UInt32 m_StylizedVariantsArticlesCount;

	/// Таблица соответствия индексов записей в списке слов и индексов статей с форматированными вариантами написания map<wordIndex, map<variantIndex, articleIndex>
	std::map<UInt32, std::map<UInt32, UInt32>> m_StylizedVariantsMap;

	/// Имя папки с дополнительными ресурсами
	std::wstring m_ResourceFolderName;

	/// Определяет дефолтные уровни каталога, на которых будет производиться поиск см #ESldSearchRange
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
