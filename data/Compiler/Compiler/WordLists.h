#pragma once

#include "WordList.h"
#include "Styles.h"
#include "StringFunctions.h"

class CSortTablesManager;

/// Класс который управляет списками слов.
class CWordListManager
{
public:
	CWordListManager() {}
	~CWordListManager();

	CWordListManager(const CWordListManager&) = delete;
	CWordListManager& operator=(const CWordListManager&) = delete;

	/// Добавляем список слов в менеджер.
	int AddList(CWordList&& aList);

	// Возвращает количество списков слов.
	UInt32 GetNumberOfLists(void) const;

	/// Возвращает список слов по его номеру.
	CWordList* GetWordList(UInt32 index);
	const CWordList* GetWordList(UInt32 index) const;

	/// Возвращает список слов по его Id
	CWordList* GetWordList(sld::wstring_ref aListId);

	/// Постобработка списков слов
	int PostProcessLists(UInt32 aLangCodeFrom, UInt32 aLangCodeTo, TSlovoedStyleManager* aStyles,
						 const bool aIsMultiLanguageBase);

private:

	/// Массив списков слов
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
