#pragma once

#include "WordList.h"
#include "Styles.h"
#include "StringFunctions.h"

class CSortTablesManager;

/// ����� ������� ��������� �������� ����.
class CWordListManager
{
public:
	CWordListManager() {}
	~CWordListManager();

	CWordListManager(const CWordListManager&) = delete;
	CWordListManager& operator=(const CWordListManager&) = delete;

	/// ��������� ������ ���� � ��������.
	int AddList(CWordList&& aList);

	// ���������� ���������� ������� ����.
	UInt32 GetNumberOfLists(void) const;

	/// ���������� ������ ���� �� ��� ������.
	CWordList* GetWordList(UInt32 index);
	const CWordList* GetWordList(UInt32 index) const;

	/// ���������� ������ ���� �� ��� Id
	CWordList* GetWordList(sld::wstring_ref aListId);

	/// ������������� ������� ����
	int PostProcessLists(UInt32 aLangCodeFrom, UInt32 aLangCodeTo, TSlovoedStyleManager* aStyles,
						 const bool aIsMultiLanguageBase);

private:

	/// ������ ������� ����
	vector<CWordList*> m_Lists;

public:

	auto lists() -> sld::DereferencingRange<decltype(m_Lists.begin())>
	{
		return{ m_Lists.begin(), m_Lists.end() };
	}

	auto lists() const -> sld::DereferencingRange<decltype(m_Lists.begin())>
	{
		return{ m_Lists.begin(), m_Lists.end() };
	}
};
