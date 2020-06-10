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
	��������� ��� ���� ������ � ��������.

	@param aList - ����������� ������ ����
	
	@return - ��� ������
*/
int CWordListManager::AddList(CWordList&& aList)
{
	// �������� id ������ ���� �� ������������
	const wstring listId(aList.GetListId());
	if (!listId.empty() && GetWordList(listId) != nullptr)
	{
		sldILog("Error! CWordListManager::AddList : duplicated list id = %s\n", sld::as_ref(listId));
		return ERROR_DUPLICATED_LIST_ID;
	}

	m_Lists.push_back(new CWordList(std::move(aList)));
	return ERROR_NO;
}

/// ���������� ���������� ������� ����.
UInt32 CWordListManager::GetNumberOfLists(void) const
{
	return (UInt32)m_Lists.size();
}

/**
	���������� ��������� �� ������ ������ ����.

	@param index - ����� ������ ���� ����� �����������.

	@return - ������ ������ ���� ��� NULL � ������ ������.
*/
CWordList * CWordListManager::GetWordList(UInt32 index)
{
	return index < m_Lists.size() ? m_Lists[index] : nullptr;
}

const CWordList* CWordListManager::GetWordList(UInt32 index) const
{
	return index < m_Lists.size() ? m_Lists[index] : nullptr;
}

/// ���������� ������ ���� �� ��� Id ��� nullptr ���� ����� � ����� Id ���
CWordList* CWordListManager::GetWordList(sld::wstring_ref aListId)
{
	for (CWordList &list : lists())
	{
		if (list.GetListId() == aListId)
			return &list;
	}
	return nullptr;
}

/// ������������� ������� ����
int CWordListManager::PostProcessLists(UInt32 aLangCodeFrom, UInt32 aLangCodeTo, TSlovoedStyleManager* aStyles,
									   const bool aIsMultiLanguageBase)
{
	int error;

	// �������������
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

	// �������� id ���� ��������� �� ���� ������� �� ������������
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

	// �������� ������� ��������� ��������� � �������, �� ������� ��������� ������ ���� ��������������� ������
	const UInt32 Count = static_cast<UInt32>(m_Lists.size());
	for (UInt32 i = 0; i < Count; i++)
	{
		CWordList* pSearchList = m_Lists[i];
		assert(pSearchList);

		if (!pSearchList->IsFullTextSearchList() || pSearchList->GetType() == eWordListType_Atomic || pSearchList->GetType() == eWordListType_KES || pSearchList->GetType() == eWordListType_FC)
			continue;

		const TFullTextSearchOptions* pFullTextSearchOptions = pSearchList->GetFullTextSearchOptions();
		assert(pFullTextSearchOptions);

		// ���� ��������� ��������� ������ �� ���� ������ - ������ ��������� �� �����
		if (pFullTextSearchOptions->mAllowedSourceListId.size() == 1)
			continue;

		// �������� �� ���������� ���������� ��������� ��������� � �� ������� � ���� �������, �� ������� ��������� ��������� ���������� ������ � ������� i
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
			// ���������� ��� ������ ���� (��� ��� ���������� ������ ��������� ��������� �� ��� ������)
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
			// ���������� id �� ������ � ��������� ������ ���� ��������������� ������
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

	// ��� ������������� �������� ������� ���������, ������� ����� ������� ��������� ������ ��� ������ ��������� ���������
	// �������� id ���� ��������� �� ���� ������� �� ������������
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

			// ����� ������ �������� ��������� � ����������� ������
			wstring stylePreferencesText;
			// ��������� ������� ��������������� ����� ������� � ������, ������� ����� ��������� � ��������� �������� ���������
			for (const auto &it : listElement.Styles)
			{
				// ������, ��� ������� ����� �������� � ������� ����� (�, ��������������, ��� ����� �� ����� ����� � ����)
				// ����� �� ������ ���������������� ������ �� ������� ������ ������� �������� �������������� ������
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
			// ������ �� �����, ��� ����� �������������� ������� ���������
			// (�.�. ���� �� � ����� ������ ������ ����� ��� �������� ������ �� ����������)
			const UInt32 propNumber = list.GetNumberOfVariants();
			list.AddVariantProperty(propNumber, { eVariantStylePreferences, list.GetLanguageCodeFrom() });

			// �� ����� ������� ������� �� ��������� ������ �� ������, � ������� ���� ������� �������� ������
			// (�������������� ������ ������� �� ��������, �.�. �� ���������� ������� �� ���� ����, ����� �� ��� ��� 1 �������)
			for (ListElement &listElement : list.elements())
			{
				if (listElement.Name.size() < propNumber)
					listElement.Name.push_back(L"");
			}
		}
	}

	return ERROR_NO;
}
