#include "WordList.h"

#include <algorithm>
#include <sstream>
#include <unordered_map>

#include "Log.h"
#include "StringCompare.h"
#include "StringFunctions.h"
#include "Tools.h"

namespace {

struct ListElement_Less
{
	const CStringCompare *cmp;
	ListElement_Less(const CStringCompare *cmp) : cmp(cmp) {}

	bool operator()(const ListElement &lhs, const ListElement &rhs) const
	{
		int res = cmp->StrWCMP(lhs.Name[0].c_str(), rhs.Name[0].c_str(), 0);
		if (res != 0)
			return res < 0;

		res = cmp->StrWCmp(lhs.Name[0].c_str(), rhs.Name[0].c_str());
		if (res != 0)
			return res < 0;

		if (cmp->IsCompare2())
			return cmp->StrWCMP(lhs.Name[0].c_str(), rhs.Name[0].c_str(), 1) < 0;
		return false;
	}
};

// Simple tree implementation
namespace tree
{
// XXX: Considering it's use case this is overly complicated and wasteful
// Some of the cleanup/optimization opportunities:
// - simplify this all into a simple node *struct* with only minimal helpers; maybe even roll a
//   function "sortHierarchical()" and make it nested there
// - make it rvalue/move aware; considering that we *can* move the list elements in here
//   and then steal them out of it when constructing the sorted vector back this would be a huge win
// - use an ArenaAllocator for the nodes

template<class T> class TreeNode
{
public:
	TreeNode(TreeNode *parent, const T &data) : m_data(data), m_parent(parent) {}

	~TreeNode()
	{
		for (auto it = m_children.begin(); it != m_children.end(); ++it)
			delete *it;
		m_children.clear();
	}

	TreeNode(const TreeNode&) = delete;
	TreeNode &operator=(const TreeNode&) = delete;

	TreeNode* appendChild(const T &data)
	{
		TreeNode *child = new TreeNode(this, data);
		m_children.push_back(child);
		return child;
	}

	const T& data() const { return m_data; }
	TreeNode *parent() const { return m_parent; }

	template <class Predicate, class Compare>
	void sort(Predicate pred, Compare comp)
	{
		if (pred(m_data))
			stable_sort(m_children.begin(), m_children.end(), [&comp](TreeNode *first, TreeNode *second) { return comp(first->data(), second->data()); });

		for (TreeNode *node : m_children)
			node->sort(pred, comp);
	}

	template <class Function>
	void forEach(Function fn) const
	{
		for (TreeNode *node : m_children)
		{
			fn(node->data());
			node->forEach(fn);
		}
	}

private:
	T m_data;
	TreeNode *m_parent;
	std::vector<TreeNode*> m_children;
};

template<class T> class Tree : public TreeNode<T>
{
public:
	Tree(const T &data = T()) : TreeNode<T>(0, data) {}
};
};

} // anon namespace

CWordList::CWordList(void)
{
	m_Usage = eWordListType_Unknown;
	m_isHierarchy = false;
	m_isSorted = false;
	m_isDirectList = false;
	m_isPicture = false;
	m_isVideo = false;
	m_isScene = false;
	m_isSound = false;
	m_isLocalizedEntriesList = false;
	m_isLinkedList = false;
	m_NumberOfArticlesSize = 0;
	m_PictureIndexSize = 0;
	m_VideoIndexSize = 0;
	m_SceneIndexSize = 0;
	m_SoundIndexSize = 0;
	m_AticleIndexSize = 0;
	m_AlphabetType = 0;
	m_compressionConfig.type = eCompressionTypeNoCompression;
	m_compressionConfig.mode = eCompressionOptimal;

	m_FullTextSearch_LinkListIndexSize = 0;
	m_FullTextSearch_DefaultLinkListIndex = 0;
	m_FullTextSearch_LinkListEntryIndexSize = 0;
	m_FullTextSearch_LinkTranslationIndexSize = 0;
	m_FullTextSearch_DefaultLinkTranslationIndex = 0;
	m_FullTextSearch_ShiftIndexSize = 0;

	m_TotalPictureCount				= 0;
	m_TotalVideoCount				= 0;
	m_TotalSceneCount				= 0;
	m_TotalSoundCount				= 0;
	m_TotalMediaContainerCount		= 0;
	m_RealNumberOfWords				= 0;
	m_StylizedVariantsArticlesCount	= 0;

	m_ZeroLevelSorted = false;
	m_SimpleSortedListIndex = SLD_DEFAULT_LIST_INDEX;

	m_HasAlternativeHeadwords = false;

	m_SearchRange = eSearchRangeDefault;
}

/// Устанавливает ListId
void CWordList::SetListId(wstring aId)
{
	m_ListId = aId;
}

/// Возвращает ListId
const std::wstring& CWordList::GetListId(void) const
{
	return m_ListId;
}

/***********************************************************************
* Возвращает количество блоков локализованных строк
*
* @return количество блоков локализованных строк
************************************************************************/
UInt32 CWordList::GetLocalizedListStringsCount(void) const
{
	return (UInt32)m_LocalizedNames.size();
}

/***********************************************************************
* Возвращает указатель на структуру, содержащую свойства варианта написания, по индексу варианта
*
* @param aIndex	- индекс запрашиваемого варианта написания
*
* @return	указатель на структуру или NULL, если таковая не найдена
************************************************************************/
const ListVariantProperty* CWordList::GetVariantProperty(UInt32 aIndex) const
{
	return aIndex < m_VariantsProperty.size() ? &m_VariantsProperty[aIndex] : nullptr;
}

const TListLocalizedNames* CWordList::GetLocalizedListStringsByIndex(UInt32 aIndex) const
{
	UInt32 Count = GetLocalizedListStringsCount();
	if (aIndex >= Count)
		return NULL;
	
	return &m_LocalizedNames[aIndex];
}

/**
	Производит обработку считанных данных, подготавливая их к последующему использованию.

	@return код ошибки.
*/
int CWordList::PostProcess()
{
	// Если нет локализованных строк ни на одном языке - добавляем пустую запись для языка по умолчанию
	if (m_LocalizedNames.empty())
	{
		TListLocalizedNames tmpNames = {};
		tmpNames.structSize = sizeof(tmpNames);
		tmpNames.LanguageCode = SldLanguage::Default;
		m_LocalizedNames.push_back(tmpNames);
	}

	// считаем максимальное количество вариантов написания встреченных в листе
	unsigned numberOfVariants = 0;
	for (auto&& elem : m_List)
	{
		if (elem.Name.size() > numberOfVariants)
			numberOfVariants = static_cast<unsigned>(elem.Name.size());
	}

	// проверяем количество вариантов написания и их свойств
	if (m_VariantsProperty.empty() && numberOfVariants == 1)
	{
		// случай, когда VarPropCount == 0 и numberOfVariants == 1 нормален. В этом случае используется 1 дефолтный вариант - show
		m_VariantsProperty.push_back({ eVariantShow, m_LanguageCodeFrom });
	}
	else if (m_VariantsProperty.size() != numberOfVariants)
	{
		// количество вариантов написания и их свойств не совпадает
		sldILog("Error! List: `%s`. Variants property count (%u) != actual number of variants (%u)!\n",
				sld::as_ref(m_ListId), m_VariantsProperty.size(), numberOfVariants);
		sld::printf(eLogStatus_Error, "\nError! Variants property count != actual number of variants!");
		return -1;
	}

	// Производим нормализацию глубин, т.е. делаем так, чтобы самый высокий уровень был 0.
	UInt32 top_level = 0xFFFFFFFFUL;
	for (const ListElement &elem : elements())
	{
		if (top_level > elem.Level)
			top_level = elem.Level;
	}
	if (top_level)
	{
		for (ListElement &elem : elements())
			elem.Level -= top_level;
	}

	return ERROR_NO;
}

/**
 * Возвращает указатель на элемент списка слов
 *
 * @param aIndex - номер запрашиваемого элемента
 *
 * @return указатель на элемент списка слов или NULL, если элемент не найден
 */
const ListElement* CWordList::GetListElement(UInt32 aIndex) const
{
	return aIndex >= GetNumberOfElements() ? nullptr : &(m_List[aIndex]);
}

/**
	Возвращает количество элементов в каталоге/списке слов

	@return код ошибки.
*/
UInt32 CWordList::GetNumberOfElements() const
{
	return (UInt32)m_List.size();
}

/**
Удаляет ссылку на перевод

@param[in] aElementIndex	- номер элемента списка слов, данные которого мы хотим получить
@param[in] aArticleIndex	- номер статьи (перевода) среди ассоциированных с элементом списка слов

@return результат выполнения функции
*/
bool CWordList::RemoveArticleRef(UInt32 aElementIndex, UInt32 aArticleIndex)
{
	if (aElementIndex >= GetNumberOfElements())
		return false;

	auto &refs = m_List[aElementIndex].ArticleRefs;
	if (aArticleIndex >= refs.size())
		return false;

	refs.erase(std::next(refs.begin(), aArticleIndex));
	return true;
}

/**
	Возвращает настройки полнотекстового поиска

	@return указатель на структуру c настройками полнотекстового поиска
*/
const TFullTextSearchOptions* CWordList::GetFullTextSearchOptions(void) const
{
	return &m_FullTextSearchOptions;
}

void CWordList::SetFullTextSearchOptions(const TFullTextSearchOptions & aOptions)
{
	// XXX: Should we check them? Skip it for now...
	m_FullTextSearchOptions = aOptions;
}

/// Устанавливает тип назначения списка.
void CWordList::SetType(UInt32 aType)
{
	m_Usage = aType;
}

/// Возвращает тип назначения списка.
UInt32 CWordList::GetType(void) const
{
	return m_Usage;
}

/// Устанавливает флаг того, требуется ли списку слов сортировка
void CWordList::SetSorted(bool aIsSorted)
{
	m_isSorted = aIsSorted;
	m_ZeroLevelSorted = aIsSorted;
}

/// Возвращает флаг того, требуется ли списку слов сортировка
bool CWordList::GetSorted() const
{
	return m_isSorted;
}

/// Устанавливает флаг того, содержит ли список локализацию слов на разные языки
void CWordList::SetLocalizedEntries(bool aIsLocalizedEntries)
{
	m_isLocalizedEntriesList = aIsLocalizedEntries;
}

/// Возвращает флаг того, содержит ли список локализацию слов на разные языки
bool CWordList::GetLocalizedEntries() const
{
	return m_isLocalizedEntriesList;
}

/// Устанавливает флаг того, что список слов иерархический
void CWordList::SetHierarchy(bool aIsHierarchy)
{
	m_isHierarchy = aIsHierarchy;
}

/// Возвращает флаг того, что список слов иерархический
bool CWordList::GetHierarchy() const
{
	return m_isHierarchy;
}

/// Устанавливает флаг того, что список связан с другим листом
void CWordList::SetLinkedList(bool aIsLinkedList)
{
	m_isLinkedList = aIsLinkedList;
}

/// Возвращает флаг того, что список связан с другим листом
bool CWordList::GetLinkedList() const
{
	return m_isLinkedList;
}

/// Возвращает флаг того, что на одно слово списка слов может приходится несколько переводов
bool CWordList::GetIsNumberOfArticles() const
{
	for (auto&& elem : m_List)
	{
		if (elem.ArticleRefs.size() != 1)
			return true;
	}
	return m_FullTextSearchOptions.LinkType == eLinkType_ArticleId &&
			IsOneArticleReferencedByManyListEntries();
}

// builds a hierarchical tree out of a "flat" list elements vector with levels
static void buildListTree(tree::Tree<ListElement> &aTree, const vector<ListElement> &aList)
{
	tree::TreeNode<ListElement> *parentNode = &aTree;
	tree::TreeNode<ListElement> *lastNode = 0;
	unsigned int level = 0;
	for (auto&& elem : aList)
	{
		if (elem.Level > level)
		{
			if (elem.Level - 1 != level)
				throw std::runtime_error("Wrong hierarchy level!");

			parentNode = lastNode;
			level = elem.Level;
		}
		else if (elem.Level < level)
		{
			for (auto i = 0U; i < level - elem.Level; ++i)
			{
				if (!parentNode)
					throw std::runtime_error("Memory null pointer access!");

				parentNode = parentNode->parent();
			}
			level = elem.Level;
		}
		if (!parentNode)
			throw std::runtime_error("Memory null pointer access!");
		lastNode = parentNode->appendChild(elem);
	}
}

/**
 * Производит сортировку списка слов (если возможно)
 *
 * @param[in]  aCompare - указатель на сортировщик строк, может быть nullptr
 */
int CWordList::DoSort(const CStringCompare *aCompare)
{
	if (GetSorted() && aCompare)
	{
		const ListElement_Less less(aCompare);
		if (GetHierarchy())
		{
			// В листах с иерархией сортируем через дерево
			ListElement rootElement;
			rootElement.childrenIsSorted = m_ZeroLevelSorted;

			tree::Tree<ListElement> tree(rootElement);

			buildListTree(tree, m_List);
			tree.sort([](const ListElement &element) { return element.childrenIsSorted; }, less);

			m_List.clear();
			tree.forEach([this](const ListElement &element) { m_List.push_back(element); });
		}
		else
		{
			// В листах без иерархии просто сортируем список "в лоб"
			stable_sort(m_List.begin(), m_List.end(), less);
		}
	}

	UInt32 MaxArticlesCount = 0;
	// Заполняем смещения до следующих элементов такой-же глубины вложенности
	for (auto elem = m_List.begin(); elem != m_List.cend(); ++elem)
	{
		UInt32 shiftToUnilevelElement = 0;
		for (auto elem2 = std::next(elem); elem2 != m_List.cend(); ++elem2)
		{
			if (elem2->Level == elem->Level)
			{
				shiftToUnilevelElement = std::distance(elem, elem2);
				break;
			}
			// Если мы поднялись на уровень выше, чем наш текущий - значит ставим 0,
			// т.к. это означает, что мы пытаемся погрузится в другую ветвь каталога.
			if (elem->Level > elem2->Level)
				break;
		}
		elem->Shift2Next = shiftToUnilevelElement;

		const UInt32 aRefSize = static_cast<UInt32>(elem->ArticleRefs.size());
		if (MaxArticlesCount < aRefSize)
			MaxArticlesCount = aRefSize;
	}

	// После того, как мы нашли максимальное число переводов на 1 статью, можно рассчитать
	// количество бит необходимое для хранения.
	if (MaxArticlesCount)
		MaxArticlesCount >>= 1;

	m_NumberOfArticlesSize = 1 + GetCodeLen(MaxArticlesCount);

	// Создаем иерархию
	return PrepareHierarchy();
}

/**
	Устанавливает флаг того, что данный список слов напрямую отображается в статьи и он не 
	нуждается в индексах, т.к. нумерация статей и слов из списка совпадают(в пределах количества
	элементов в списке слов).

	@param aIsDirectList - флаг.
*/
void CWordList::SetDirectList(bool aIsDirectList)
{
	m_isDirectList = aIsDirectList;
}

/// Возвращает флаг того, что данный список слов напрямую соответствует переводам.
bool CWordList::GetDirectList() const
{
	return m_isDirectList;
}

/// Возвращает флаг того, что данный список слов является списком слов полнотекстового поиска
bool CWordList::IsFullTextSearchList(void) const
{
	if (IsFullTextSearchListType() || m_FullTextSearchOptions.IsOptionsPresent)
		return true;
	
	return false;
}

/// Возвращает флаг того, является ла данный лист вспомогательным листом для поиска (поиск ссылается на этот лист, этот лист это "фразы"->статьи)
bool CWordList::IsFullTextSearchAuxillaryList(void) const
{
	return (m_Usage == eWordListType_FullTextAuxiliary);
}

/// Возвращает флаг, является ли тип данного списка слов типом списка слов полнотекстового поиска
bool CWordList::IsFullTextSearchListType(void) const
{
	return m_Usage >= eWordListType_FullTextSearchBase && m_Usage <= eWordListType_FullTextSearchLast;
}

/// Возвращает флаг того, что данный список слов полнотекстового поиска является суффиксным
UInt32 CWordList::IsSuffixFullTextSearchList(void) const
{
	return m_FullTextSearchOptions.isSuffixSearch ? 1 : 0;
}

/// Возвращает флаг, содержит или нет данный список для каждого слова все словоформы
UInt32 CWordList::IsFullMorphologyWordFormsList(void) const
{
	return m_FullTextSearchOptions.isFullMorphologyWordForms ? 1 : 0;
}

/// Возвращает ID морфологической базы, с учетом которой собирался данный список
/// В списке должны быть только базовые формы
UInt32 CWordList::GetMorphologyId(void) const
{
	return m_FullTextSearchOptions.MorphologyId;
}

/// Возвращает флаг, ссылается или нет на один перевод (статью) несколько элементов списка слов
/// Если да, то в таком случае по этому списку слов нельзя сортировать статьи
bool CWordList::IsOneArticleReferencedByManyListEntries() const
{
	// holds the first list entry id encountered for a given article id
	std::unordered_map<sld::wstring_ref, sld::wstring_ref> map;
	for (const ListElement &elem : elements())
	{
		for (const ArticleRefStruct &ref : elem.ArticleRefs)
		{
			const auto it = map.find(ref.ID);
			if (it == map.end())
				map.emplace(ref.ID, elem.Id);
			else if (it->second != elem.Id)
				return true;
		}
	}
	return false;
}

/**
	Производит подготовку списка слов к последующему сжатию.

	@return код ошибки.
*/
int CWordList::PrepareHierarchy()
{
	if (!GetHierarchy())
		return ERROR_NO;
	
	/// Список слов подготовленный к последующему сжатию. Больше никаких работ над ним производится не должно.
	vector<ListElement>	newList;
	// Резервируем память в объеме превышающем требования
	const size_t hierarchySize = m_List.size() * (sizeof(THierarchyLevelHeader) + sizeof(THierarchyElement));
	m_hierarchy.resize(hierarchySize, 0);
	UInt32 actualSize = 0; // Здесь будет количество реально использованной памяти
	int error = CreateHierarchy(newList, m_List, 0, (UInt32)m_List.size(), m_hierarchy.data(), &actualSize, (UInt8)m_ZeroLevelSorted, eLevelTypeNormal);
	if (error != ERROR_NO)
		return error;
	m_hierarchy.resize(actualSize);

	m_List = std::move(newList);

	return ERROR_NO;
}

/**
	Метод создает из структурированного списка слов, список слов подготовленный
	к сжатию(расположение слов изменено на более оптимальное) и отдельно структуру
	иерархии, которая должна быть просто сохранена.

	@param newList[out] - список слов который необходимо наполнить из старого списка
	@param oldList[in] - список слов подлежащий обработке
	@param startPos[in] - номер элемента списка с которого начинается новая ветвь иерархии
	@param hierarchy[out] - указатель на память с данными структуры списка слов.
	@param hierarchyStart[in|out] - смещение с которого нужно проводить запись в структуру иерархии.
	@param hierarchySize[in] - размер буфера для иерархии

	@return код ошибки.
*/
int CWordList::CreateHierarchy(vector<ListElement> &newList,
									   vector<ListElement> &oldList, UInt32 startPos, UInt32 endPos, 
									   UInt8 *hierarchy, UInt32 *hierarchyStart, UInt8 aSortedFlag, EHierarchyLevelType aType)
{
	UInt32 currentLevel = oldList[startPos].Level;
	UInt32 countElement = 0;	// Количество записей
	UInt32 countWordsInList = 0;	// Количество слов в данном уровне списка слов
	UInt32 startIndex = (UInt32)newList.size();
	// Определяем количество элементов имеющих такой-же уровень вложенности в текущей ветви
	UInt32 i=startPos;
	UInt32 prevEndElement = 0;
	while (i<endPos)
	{
		assert(oldList[i].Level == currentLevel);

		UInt32 newEndPos = endPos;
		if (oldList[i].Shift2Next!=0)
			newEndPos = i+oldList[i].Shift2Next;

		// Если это финальный элемент, тогда считаем сколько их накопилось
		if (i+1 >= newEndPos)
			prevEndElement++;
		else
		{
			// Если у нас была цепочка конечных элементов, тогда заменяем ее на 1 элемент
			// с указанием границ.
			if (prevEndElement)
				countElement -= prevEndElement-1;
			// Учет элементов начинаем заново.
			prevEndElement = 0;
		}

		// Добавляем элемент текущего уровня
		newList.push_back(oldList[i]);
		countElement++;
		countWordsInList++;

		// Если это последний элемент данного уровня вложенности - выходим.
		if (oldList[i].Shift2Next == 0)
			break;

		i += oldList[i].Shift2Next;
	}
	if (prevEndElement)
		countElement -= prevEndElement-1;

	THierarchyLevelHeader header;
	memset(&header, 0, sizeof(header));
	header.CurrentLevel = currentLevel;
	header.NumberOfWords = countWordsInList;
	header.NumberOfElements = countElement;
	header.GlobalShift = startIndex;
	header.IsSortedLevel = aSortedFlag;
	header.LevelType = aType;

	// Копируем заголовок данного уровня в память.
	memmove(hierarchy+(*hierarchyStart), &header, sizeof(header));
	*hierarchyStart += sizeof(header);

	// Создаем элемент иерархии с которым мы будем сравнивать память перед записью.
	// Это поможет обнаруживать ошибки, когда например мы пытаемся записать данные
	// в место которое уже было использовано.
	//////////////////////////////////////////////////////////////////////////
	THierarchyElement ZeroElement;
	memset(&ZeroElement, 0, sizeof(ZeroElement));
	//////////////////////////////////////////////////////////////////////////

	THierarchyElement element;
	UInt32 localStart = *hierarchyStart;
	*hierarchyStart += sizeof(THierarchyElement)*header.NumberOfElements;
	countElement = countWordsInList = 0;

	// Производим обработку каждого элемента текущего уровня вложенности
	//////////////////////////////////////////////////////////////////////////
	i=startPos;
	//UInt32 elementsToSkip=1;
	while (i<endPos)
	{
		// Если дошли до уровня выше - выходим, т.к. это значит, что текущая ветвь закончилась.
		assert (oldList[i].Level == currentLevel);

		memset(&element, 0, sizeof(element));
		element.NextLevelGlobalIndex = (UInt32)newList.size();
		element.BeginIndex = element.EndIndex = startIndex + countWordsInList;

		UInt32 newEndPos = endPos;
		if (oldList[i].Shift2Next!=0)
			newEndPos = i+oldList[i].Shift2Next;

		// Проверяем, есть ли еще уровни ниже текущего.
		if (i+1 < newEndPos)
		{
			element.ShiftToNextLevel = *hierarchyStart;
			// Обрабатываем тот уровень на который указывает данный элемент.
			int error = CreateHierarchy(newList, oldList, i+1, newEndPos,
				hierarchy, hierarchyStart, (UInt8)oldList[i].childrenIsSorted, oldList[i].HierarchyLevelType);
			if (error != ERROR_NO)
				return error;

			// Записываем в память новый элемент
			UInt8 *ptr = hierarchy+localStart+countElement*sizeof(element);
			assert(memcmp(ptr, &ZeroElement, sizeof(ZeroElement)) == 0);
			memmove(ptr, &element, sizeof(element));
			countElement++;

			countWordsInList += 1;
			// Если это последний элемент данного уровня вложенности - выходим.
//			if (oldList[i].Shift2Next == 0)
//				break;
			i = newEndPos;
		}else
		{
			// Указываем, что это финальный элемент
			element.ShiftToNextLevel = LIST_END;
			element.NextLevelGlobalIndex = LIST_END;

			// Считаем количество слов текущего идущих подряд и являющихся конечными
			UInt32 continuous_words = 0;
			UInt32 Shift2Next;
			Shift2Next = oldList[i+continuous_words].Shift2Next;
			while (i+continuous_words < endPos && Shift2Next == 1)
			{
				continuous_words++;
				Shift2Next = oldList[i+continuous_words].Shift2Next;
			}

			// Проверяем достигли мы конца данного уровня или нет.
			if (i+continuous_words +1 >= endPos)
				continuous_words++;
			element.EndIndex += continuous_words-1;
			countWordsInList += continuous_words;

			UInt8 *ptr = hierarchy+localStart+countElement*sizeof(element);
			assert(memcmp(ptr, &ZeroElement, sizeof(ZeroElement)) == 0);
			memmove(ptr, &element, sizeof(element));
			countElement++;
			i+=continuous_words;
		}

	}

	assert(header.NumberOfElements == countElement);

	return ERROR_NO;
}

/**
	Возвращает количество вариантов написания которое существует 
	для одного слова из списка слов.

	@return количество вариантов написания.
*/
UInt32 CWordList::GetNumberOfVariants() const
{
	return static_cast<UInt32>(m_VariantsProperty.size());
}

/**
	Возвращает размер числа представляющего количество переводов для элемента
	списка слов. Любое значение #CListLoader::GetNumberOfArticles должно 
	поместится в переменную указанного размера. 
	Размер в БИТАХ !!!!

	@return количество бит для представления количества переводов в статьях
*/
UInt32 CWordList::GetNumberOfArticlesSize(void) const
{
	return m_NumberOfArticlesSize;
}

/// Устанавливает количество бит для представления количества переводов в статьях
void CWordList::SetNumberOfArticlesSize(UInt32 aCount)
{
	m_NumberOfArticlesSize = aCount;
}

/***********************************************************************
* Устанавливает флаг того, что в данном списке слов используются картинки
*
* @param aIsListHasIcons - флаг того, что в данном списке слов используются картинки
************************************************************************/
void CWordList::SetPicture(bool aIsListHasIcons)
{
	m_isPicture = aIsListHasIcons;
}

/// Возвращает флаг того, что в данном списке слов используются картинки
bool CWordList::GetPicture() const
{
	return m_isPicture;
}

/***********************************************************************
* Устанавливает флаг того, что в данном списке слов используются картинки
*
* @param aIsListHasIcons - флаг того, что в данном списке слов используются картинки
************************************************************************/
void CWordList::SetVideo(bool aIsListHasVideo)
{
	m_isVideo = aIsListHasVideo;
}

/// Возвращает флаг того, что в данном списке слов используется видео
bool CWordList::GetVideo() const
{
	return m_isVideo;
}

/***********************************************************************
* Устанавливает флаг того, что в данном списке слов используются сцены
*
* @param aIsListHasScene - флаг того, что в данном списке слов используются сцены
************************************************************************/
void CWordList::SetScene(bool aIsListHasScene)
{
	m_isScene = aIsListHasScene;
}

/// Возвращает флаг того, что в данном списке слов используется сцена
bool CWordList::GetScene() const
{
	return m_isScene;
}

/***********************************************************************
* Устанавливает флаг того, что в данном списке слов используется озвучка
*
* @param aIsListHasSounds - флаг того, что в данном списке слов используется озвучка
************************************************************************/
void CWordList::SetSound(bool aIsListHasSounds)
{
	m_isSound = aIsListHasSounds;
}

/// Возвращает флаг того, что в данном списке слов используется озвучка
bool CWordList::GetSound() const
{
	return m_isSound;
}

/// Возвращает количество бит для представления индекса картинки
UInt32 CWordList::GetPictureIndexSize() const
{
	return m_PictureIndexSize;
}

/// Устанавливает количество бит для представления индекса картинки
void CWordList::SetPictureIndexSize(UInt32 aIndexSize)
{
	m_PictureIndexSize = aIndexSize;
}

/// Возвращает количество бит для представления индекса видео
UInt32 CWordList::GetVideoIndexSize() const
{
	return m_VideoIndexSize;
}

/// Устанавливает количество бит для представления индекса картинки
void CWordList::SetVideoIndexSize(UInt32 aIndexSize)
{
	m_VideoIndexSize = aIndexSize;
}

/// Возвращает количество бит для представления индекса сцены
UInt32 CWordList::GetSceneIndexSize() const
{
	return m_SceneIndexSize;
}

/// Устанавливает количество бит для представления индекса сцены
void CWordList::SetSceneIndexSize(UInt32 aIndexSize)
{
	m_SceneIndexSize = aIndexSize;
}

/// Возвращает количество бит для представления индекса озвучки
UInt32 CWordList::GetSoundIndexSize() const
{
	return m_SoundIndexSize;
}

/// Устанавливает количество бит для представления индекса озвучки
void CWordList::SetSoundIndexSize(UInt32 aIndexSize)
{
	m_SoundIndexSize = aIndexSize;
}

/// Возвращает иерархию подготовленную для хранения
MemoryRef CWordList::GetHierarchyData() const
{
	return sld::make_memref(m_hierarchy.data(), m_hierarchy.size());
}

/** ********************************************************************
* Возвращает размер ссылки на перевод(в битах).
*
* @return размер в битах
************************************************************************/
UInt32 CWordList::GetAticleIndexSize() const
{
	if (IsFullTextSearchList())
		return m_FullTextSearch_LinkListEntryIndexSize;
	else
		return m_AticleIndexSize;
}

/** ********************************************************************
* Устанавливаем размер ссылки на перевод(в битах).
*
* @param aIndexSize - размер ссылки в битах.
************************************************************************/
void CWordList::SetArticleIndexSize(UInt32 aIndexSize)
{
	m_AticleIndexSize = aIndexSize;
}

/**
 * Устанавливает настройки сжатия данных
 * 
 * @param[in] aConfig - настройки сжатия
*/
void CWordList::SetCompressionMethod(CompressionConfig aConfig)
{
	m_compressionConfig = aConfig;
}

/// Возвращает настройки сжатия
CompressionConfig CWordList::GetCompressionMethod(void) const
{
	return m_compressionConfig;
}

/// Устанавливаем код языка с которого производится перевод в данном списке слов.
void CWordList::SetLanguageCodeFrom(UInt32 aLanguageCodeFrom)
{
	m_LanguageCodeFrom = aLanguageCodeFrom;
}

/// Устанавливаем код языка на который производится перевод в данном списке слов.
void CWordList::SetLanguageCodeTo(UInt32 aLanguageCodeTo)
{
	m_LanguageCodeTo = aLanguageCodeTo;
}

/// Возвращает код языка с которого производится перевод в данном списке слов.
UInt32 CWordList::GetLanguageCodeFrom() const
{
	return m_LanguageCodeFrom;
}

/// Возвращает код языка на который производится перевод в данном списке слов.
UInt32 CWordList::GetLanguageCodeTo() const
{
	return m_LanguageCodeTo;
}

/// Получает общее количество картинок в списке слов
UInt32 CWordList::GetTotalPictureCount(void) const
{
	return m_TotalPictureCount;
}

/// Устанавливает общее количество картинок в списке слов
void CWordList::SetTotalPictureCount(UInt32 aCount)
{
	m_TotalPictureCount = aCount;
}

/// Получает общее количество видео в списке слов
UInt32 CWordList::GetTotalVideoCount(void) const
{
	return m_TotalVideoCount;
}

/// Устанавливает общее количество видео в списке слов
void CWordList::SetTotalVideoCount(UInt32 aCount)
{
	m_TotalVideoCount = aCount;
}

/// Получает общее количество сцен в списке слов
UInt32 CWordList::GetTotalSceneCount(void) const
{
	return m_TotalSceneCount;
}

/// Устанавливает общее количество сцен в списке слов
void CWordList::SetTotalSceneCount(UInt32 aCount)
{
	m_TotalSceneCount = aCount;
}

/// Получает общее количество озвучек в списке слов
UInt32 CWordList::GetTotalSoundCount(void) const
{
	return m_TotalSoundCount;
}

/// Устанавливает общее количество озвучек в списке слов
void CWordList::SetTotalSoundCount(UInt32 aCount)
{
	m_TotalSoundCount = aCount;
}

/// Устанавливает тип алфавита для основного языка списка
void CWordList::SetAlphabetType(EAlphabetType aType)
{
	m_AlphabetType = aType;
}

int CWordList::AddVariantProperty(UInt32 aNumber, const ListVariantProperty &aVariantProperty)
{
	if (aNumber != m_VariantsProperty.size() || aVariantProperty.Type == eVariantUnknown || aVariantProperty.LangCode == ~0u)
	{
		sldILog("Error! CWordList::AddVariantProperty : Can't add new variant property!\n");
		return ERROR_WRONG_STRING_ATTRIBUTE;
	}

	m_VariantsProperty.push_back(aVariantProperty);
	return ERROR_NO;
}

/**
 * Получает индекс слова в списке слов по id слова
 *
 * @param[in] aName - id слова, которое мы ищем
 *
 * @return индекс слова в данном списке слов, или SLD_DEFAULT_WORD_INDEX если слова с таким id нету
 */
UInt32 CWordList::GetIndexByName(sld::wstring_ref aName) const
{
	UInt32 index = 0;
	for (auto&& elem : m_List)
	{
		if (elem.Level == 0)
		{
			if (elem.Id == aName)
				return index;
			index++;
		}
	}
	return SLD_DEFAULT_WORD_INDEX;
}

/**
	Устанавливает индекс списка, для которого данный список является вспомогательным

	@param[in] aIndex	- индекс базового списка
*/
void CWordList::SetSimpleSortedListIndex(Int32 aListIndex)
{
	m_SimpleSortedListIndex = aListIndex;
}

/**
	Записывает несортированные индексы слов в сортированный список вторым вариантом написания

	@param[in] aNotSortedList	- указатель на несортированный список

	@return код ошибки.
*/
int CWordList::DoSimpleSortedList(const CWordList *aNotSortedList)
{
	map<wstring, UInt32> elementID_ElementNumber;
	for (const auto listElem : enumerate(aNotSortedList->m_List))
		elementID_ElementNumber[L"for_search_" + listElem->Id] = static_cast<UInt32>(listElem.index);

	vector<ListElement> altElements;
	wstringstream ss;

	for (ListElement &listElem : m_List)
	{ 
		ss << Int32(SLD_SIMPLE_SORTED_NORMAL_WORD) << (wchar_t)L'|' << UInt32(elementID_ElementNumber[listElem.Id]);
		listElem.Name.push_back(ss.str());
		ss.str(L"");

		ss << SLD_SIMPLE_SORTED_VIRTUAL_WORD << (wchar_t)L'|' << elementID_ElementNumber[listElem.Id];

		for (const std::wstring &altName : listElem.AltName)
		{
			altElements.push_back(listElem);
			altElements.back().Name.front() = altName;
			altElements.back().Name.back() = ss.str();
			altElements.back().Id = listElem.Id + L"_alternative_name_" + std::to_wstring((long long)altElements.size());
		}
		ss.str(L"");
	}

	for (ListElement &listElem : altElements)
		m_List.push_back(std::move(listElem));

	// добавляем свойство для только что записанных несортированных индексов
	m_VariantsProperty.push_back({ eVariantNotSortedRealGlobalIndex, 0 });

	return ERROR_NO;
}

/**
	Преобразует список во вспомогательный сортированный список

	@return код ошибки.
*/
int CWordList::ConvertListToSimpleSortedList()
{
	SetSorted(true);
	SetHierarchy(false);
	SetType(eWordListType_SimpleSearch);

	ListVariantProperty ShowVariantProperty(eVariantShow, m_LanguageCodeFrom);
	size_t showVariantIndex = 0;
	for (auto&& currentVariant : enumerate(m_VariantsProperty))
	{
		if (currentVariant->Type == eVariantShow)
		{
			ShowVariantProperty = *currentVariant;
			showVariantIndex = currentVariant.index;
			break;
		}
	}

	m_VariantsProperty.clear();
	m_VariantsProperty.push_back(ShowVariantProperty);

	for (ListElement &element : elements())
	{
		if (showVariantIndex >= element.Name.size())
		{
			sldILog("Error! Simple Sorted list creation: no Show Variant text for list element `%s`, list: `%s`.\n",
					sld::as_ref(element.Id), sld::as_ref(GetListId()));
			return ERROR_EMPTY_SHOW_VARIANT;
		}

		element.Id = L"for_search_" + element.Id;
		element.Level = 0;

		wstring name = std::move(element.Name[showVariantIndex]);
		element.Name.clear();
		element.Name.push_back(std::move(name));

		element.SoundIndex = SLD_INDEX_SOUND_NO;
		element.PictureIndex = SLD_INDEX_PICTURE_NO;
		element.VideoIndex = SLD_INDEX_VIDEO_NO;
		element.SceneIndex = SLD_INDEX_SCENE_NO;
	}

	SetListId(L"for_search_" + GetListId());

	return ERROR_NO;
}

/**
Добавляет альтернативные заголовки в список

@return код ошибки.
*/
int CWordList::AddAlternativeHeadwords()
{
	map<wstring, set<wstring>> altname_lisentryid;
	set<wstring> realWords;

	for (const ListElement &listElem : m_List)
	{
		realWords.insert(listElem.Name.front());

		for (const std::wstring &altName : listElem.AltName)
			altname_lisentryid[altName].insert(listElem.Id);
	}

	UInt32 altNameCount = 0;
	for (auto &it : altname_lisentryid)
	{
		if (realWords.count(it.first))
			continue;

		ListElement element;
		element.Name.push_back(it.first);
		element.Id = m_ListId + L"_alternative_name_" + std::to_wstring(altNameCount);

		for (const std::wstring &id : it.second)
			element.IDForAlternativeWords.insert(id);

		m_List.push_back(std::move(element));
		altNameCount++;
	}

	return ERROR_NO;
}

/**
Обработка альтернативных заголовков

@return код ошибки.
*/
int CWordList::PostProccessAlternativeHeadwords()
{
	map<wstring, UInt32> elementID_ElementIndex;
	for (const auto listElem : enumerate(m_List))
		elementID_ElementIndex[listElem->Id] = static_cast<UInt32>(listElem.index);

	for (ListElement &listElem : m_List)
	{
		for (const std::wstring &id : listElem.IDForAlternativeWords)
		{
			auto it = elementID_ElementIndex.find(id);
			if (it != elementID_ElementIndex.end())
				listElem.IndexesForAlternativeWords.insert(it->second);
		}
	}

	return ERROR_NO;
}

/**
Возвращает индекс статьи с форматированным вариантом написания

@param[in]	aWordIndex		- номер слова в списке слов
@param[in]	aVariantIndex	- номер варианта написания

@return строка с содержимым узла
*/
UInt32 CWordList::GetStylizedVariantArticleIndex(UInt32 aWordIndex, UInt32 aVariantIndex) const
{
	auto fit = m_StylizedVariantsMap.find(aWordIndex);
	if (fit != m_StylizedVariantsMap.end())
	{
		auto fit2 = fit->second.find(aVariantIndex);
		if (fit2 != fit->second.end())
			return fit2->second;
	}

	return STYLIZED_VARIANT_NO;
}

/** ********************************************************************
* Добавляет запись в список атомик-статей
*
* @param aAtomicID	- ID объекта
* @param aExternID	- ID слинкованных с объектом элементов (KES, FC...)
*
* @return код ошибки
************************************************************************/
int CWordList::AddAtomicArticle(sld::wstring_ref aAtomicID, const vector<wstring>& aExternID)
{
	ListElement newElement;

	newElement.Id = ATOMIC_ID_PREFIX;
	newElement.Id.append(aAtomicID.data(), aAtomicID.size());
	newElement.Name.push_back(to_string(aAtomicID));

	// Проверяем id элемента списка слов на уникальность в пределах списка слов
	if (m_mListEntryId.find(newElement.Id) != m_mListEntryId.end())
	{
		sldILog("Error! Duplicated list_entry_id: '%s'\n", sld::as_ref(newElement.Id));
		return ERROR_DUPLICATED_LIST_ENTRY_ID;
	}

	for (const std::wstring &id : aExternID)
	{
		ArticleRefStruct tmpRef;
		tmpRef.ID = id;
		newElement.ArticleRefs.push_back(tmpRef);
	}

	// Добавляем элемент в список
	m_mListEntryId.insert(newElement.Id);
	m_List.push_back(newElement);

	return ERROR_NO;
}

// Внутренний метод добавления новых элементов листа
// Добавляет новый элемент в лист осуществляя все проверки и т.п.
int CWordList::AddListElement(ListElement&& newElement)
{
	// Проверяем id элемента списка слов на уникальность в пределах списка слов
	if (m_mListEntryId.find(newElement.Id) != m_mListEntryId.end())
	{
		sldILog("Error! Duplicated list_entry_id: %s\n", sld::as_ref(newElement.Id));
		return ERROR_DUPLICATED_LIST_ENTRY_ID;
	}

	// Добавляем элемент в список
	m_mListEntryId.insert(newElement.Id);
	m_List.push_back(std::move(newElement));

	return ERROR_NO;
}
