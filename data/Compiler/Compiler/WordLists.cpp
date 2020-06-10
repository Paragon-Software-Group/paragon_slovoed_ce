#include "WordLists.h"

#include <unordered_map>

#include "Log.h"
#include "Tools.h"

CWordListManager::~CWordListManager()
{
	for (CWordList *list : m_Lists)
		delete list;
}

/**
	Добавляет еще один список в менеджер.

	@param aList - загруженный список слов
	
	@return - код ошибки
*/
int CWordListManager::AddList(CWordList&& aList)
{
	// Проверка id списка слов на уникальность
	const wstring listId(aList.GetListId());
	if (!listId.empty() && GetWordList(listId) != nullptr)
	{
		sldILog("Error! CWordListManager::AddList : duplicated list id = %s\n", sld::as_ref(listId));
		return ERROR_DUPLICATED_LIST_ID;
	}

	m_Lists.push_back(new CWordList(std::move(aList)));
	return ERROR_NO;
}

/// Возвращает количество списков слов.
UInt32 CWordListManager::GetNumberOfLists(void) const
{
	return (UInt32)m_Lists.size();
}

/**
	Возвращает указатель на объект списка слов.

	@param index - номер списка слов среди загруженных.

	@return - объект списка слов или NULL в случае ошибки.
*/
CWordList * CWordListManager::GetWordList(UInt32 index)
{
	return index < m_Lists.size() ? m_Lists[index] : nullptr;
}

const CWordList* CWordListManager::GetWordList(UInt32 index) const
{
	return index < m_Lists.size() ? m_Lists[index] : nullptr;
}

/// Возвращает список слов по его Id или nullptr если листа с таким Id нет
CWordList* CWordListManager::GetWordList(sld::wstring_ref aListId)
{
	for (CWordList &list : lists())
	{
		if (list.GetListId() == aListId)
			return &list;
	}
	return nullptr;
}

/// Постобработка списков слов
int CWordListManager::PostProcessLists(UInt32 aLangCodeFrom, UInt32 aLangCodeTo, TSlovoedStyleManager* aStyles,
									   const bool aIsMultiLanguageBase)
{
	int error;

	// Постобработка
	for (CWordList &list : lists())
	{
		UInt32 LangFrom = list.GetLanguageCodeFrom();
		UInt32 LangTo = list.GetLanguageCodeTo();

		if (!aIsMultiLanguageBase)
		{
			if (LangFrom != aLangCodeFrom && LangFrom != aLangCodeTo)
			{
				sldILog("Error! CWordListManager::PostProcessLists : Wrong language code from\n");
				return ERROR_WRONG_LANGUAGE_FROM;
			}

			if (LangTo != aLangCodeFrom && LangTo != aLangCodeTo)
			{
				sldILog("Error! CWordListManager::PostProcessLists : Wrong language code to\n");
				return ERROR_WRONG_LANGUAGE_TO;
			}
		}

		error = list.PostProcess();
		if (error != ERROR_NO)
			return error;
	}

	// Проверка id всех элементов во всех списках на уникальность
	bool hasDuplicates = false;
	std::unordered_map<sld::wstring_ref, sld::wstring_ref> mId;
	for (const CWordList &list : lists())
	{
		for (const ListElement &listElement : list.elements())
		{
			auto&& it = mId.emplace(listElement.Id, list.GetListId());
			if (!it.second)
			{
				sldILog("Error! Duplicated list entry id: `%s`, list: `%s`; first occurance in list: `%s`.\n",
						sld::as_ref(listElement.Id),
						sld::as_ref(list.GetListId()),
						sld::as_ref(it.first->second));
				hasDuplicates = true;
			}
		}
	}

	if (hasDuplicates)
		return ERROR_DUPLICATED_LIST_ENTRY_ID;

	// Проверка свойств вариантов написания у списков, на которые ссылаются списки слов полнотекстового поиска
	const UInt32 Count = static_cast<UInt32>(m_Lists.size());
	for (UInt32 i = 0; i < Count; i++)
	{
		CWordList* pSearchList = m_Lists[i];
		assert(pSearchList);

		if (!pSearchList->IsFullTextSearchList() || pSearchList->GetType() == eWordListType_Atomic || pSearchList->GetType() == eWordListType_KES || pSearchList->GetType() == eWordListType_FC)
			continue;

		const TFullTextSearchOptions* pFullTextSearchOptions = pSearchList->GetFullTextSearchOptions();
		assert(pFullTextSearchOptions);

		// Если разрешено ссылаться только на один список - ничего проверять не нужно
		if (pFullTextSearchOptions->mAllowedSourceListId.size() == 1)
			continue;

		// Проверка на совпадение количества вариантов написания и их свойств у всех списков, на которые разрешено ссылаться поисковому списку с номером i
		const CWordList *templateList = nullptr; // this get's updated on a "first" call to checkVariants()
		auto checkVariants = [&templateList](const CWordList *list) {
			if (list->IsFullTextSearchList() || list->GetLinkedList())
				return ERROR_NO;

			if (!templateList)
				templateList = list;

			const UInt32 count = templateList->GetNumberOfVariants();
			if (count != list->GetNumberOfVariants())
				return ERROR_WRONG_FULL_SEARCH_LIST_SOURCE_LIST_VARIANTS;

			for (UInt32 v = 0; v < count; v++)
			{
				const ListVariantProperty* templateVariant = templateList->GetVariantProperty(v);
				if (!templateVariant)
					return ERROR_NULL_POINTER;

				const ListVariantProperty* variant = list->GetVariantProperty(v);
				if (!variant)
					return ERROR_NULL_POINTER;

				if (variant->Type != templateVariant->Type)
					return ERROR_WRONG_FULL_SEARCH_LIST_SOURCE_LIST_VARIANTS;
			}
			return ERROR_NO;
		};

		if (pFullTextSearchOptions->mAllowedSourceListId.empty())
		{
			// Перебираем все списки слов (так как поисковому списку разрешено ссылаться на все списки)
			for (UInt32 j = 0; j < Count; j++)
			{
				if (j == i)
					continue;

				error = checkVariants(m_Lists[j]);
				if (error != ERROR_NO)
					return error;
			}
		}
		else
		{
			// Перебираем id из списка в свойствах списка слов полнотекстового поиска
			for (auto&& listId : pFullTextSearchOptions->mAllowedSourceListId)
			{
				const CWordList *sourceList = nullptr;
				for (UInt32 j = 0; j < Count; j++)
				{
					if (j == i)
						continue;

					if (m_Lists[j]->GetListId() == listId)
					{
						sourceList = m_Lists[j];
						break;
					}
				}

				if (!sourceList)
				{
					sldILog("Error! List: `%s`. Wrong full search list source list id: `%s`!\n",
							sld::as_ref(pSearchList->GetListId()), sld::as_ref(listId));
					return ERROR_WRONG_FULL_SEARCH_LIST_SOURCE_LIST_ID;
				}

				error = checkVariants(sourceList);
				if (error != ERROR_NO)
					return error;
			}
		}
	}

	// при необходимости создадим вариант написания, который будет хранить настройки стилей для разных вариантов написания
	// Проверка id всех элементов во всех списках на уникальность
	std::unordered_map<sld::wstring_ref, std::wstring> tagIndexes;
	for (CWordList &list : lists())
	{
		if (list.GetType() == eWordListType_SimpleSearch)
			continue;

		bool isStyleVariantNeeded = false;
		for (ListElement &listElement : list.elements())
		{
			if (listElement.Styles.empty())
				continue;

			isStyleVariantNeeded = true;

			// текст нового варианта написания с настройками стилей
			wstring stylePreferencesText;
			// настройки каждого использованного стиля добавим к строке, которая будет сохранена в отдельном варианте написания
			for (const auto &it : listElement.Styles)
			{
				// индекс, под которым стиль хранится в таблице стиле (и, соответственно, под каким он будет зашит в базу)
				// чтобы не делать последовательный проход по таблице стилей сначала проверим закэшированные записи
				auto cached = tagIndexes.find(it.second);
				if (cached == tagIndexes.end())
					cached = tagIndexes.emplace(it.second, aStyles->GetVariantStyleStr(it.second)).first;

				STString<128> wbuf(L"%i", it.first);
				stylePreferencesText += wbuf.str() + L"=\"" + cached->second + L"\";";
			}

			listElement.Name.push_back(std::move(stylePreferencesText));
		}

		if (isStyleVariantNeeded)
		{
			// теперь мы знаем, что нужен дополнительный вариант написания
			// (т.к. хотя бы в одной записи указан стиль для описания одного из заголовокв)
			const UInt32 propNumber = list.GetNumberOfVariants();
			list.AddVariantProperty(propNumber, { eVariantStylePreferences, list.GetLanguageCodeFrom() });

			// во время первого прохода мы заполнили только те записи, у которых были указаны варианты стилей
			// (дополнительные пустые строчки не зашиваем, т.к. до завершения прохода не было ясно, нужен ли нам еще 1 вариант)
			for (ListElement &listElement : list.elements())
			{
				if (listElement.Name.size() < propNumber)
					listElement.Name.push_back(L"");
			}
		}
	}

	return ERROR_NO;
}
