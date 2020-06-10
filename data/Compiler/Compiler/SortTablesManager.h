#pragma once

#include <string>
#include <map>
#include <unordered_set>
#include <vector>

#include "Log.h"
#include "sld_Types.h"
#include "util.h"

using namespace std;


/// “аблицы сортировки дл€ конкретного €зыка
struct TLanguageSortTable
{
	UInt32 LangCode;

	MemoryBuffer MainTable;
	std::wstring MainFileName;

	MemoryBuffer SecondTable;
	std::wstring SecondFileName;

	TLanguageSortTable(UInt32 lang) : LangCode(lang) {}
};

///  ласс дл€ работы с таблицами сортировки
class CSortTablesManager
{
public:
	
	///  онструктор
	CSortTablesManager(void);

public:
	
	/// ƒобавл€ет основную таблицу сортировки
	int AddMainSortTable(UInt32 aLangCode, const wstring &aFilename);

	/// ƒобавл€ет вспомогательную таблицу сортировки
	int AddSecondarySortTable(UInt32 aLangCode, const wstring &aFilename);

	/// ƒобавл€ет в таблицы сортировки символы из списков слов
	int AddDictionarySymbolPairsTable(UInt32 aLangCode, const std::unordered_set<UInt16> &aSymbolMap);

	/// ¬озвращает таблицу сортировки по коду €зыка
	const TLanguageSortTable* GetLanguageSortTable(UInt32 aLangCode) const;

	/// ¬озвращает общее количество загруженных таблиц сортировки (основных и вспомогательных)
	UInt32 GetTotalSortTablesCount(void) const;

private:
	/// ћассив имен файлов основных таблиц сортировки (используетс€ дл€ контрол€ того, чтобы не загружать одну и ту же таблицу сравнени€ несколько раз)
	map<UInt32, unordered_set<wstring> > m_SortTablesFiles;

	/// ћассив имен файлов вспомогательных таблиц сортировки (используетс€ дл€ контрол€ того, чтобы не загружать одну и ту же таблицу сравнени€ несколько раз)
	map<UInt32, unordered_set<wstring> > m_SecondarySortTablesFiles;

	/// ћассив таблиц сортировки
	map<UInt32, TLanguageSortTable> m_SortTables;

public:
	auto tables() const -> sld::MapDerefRange<decltype(m_SortTables.begin())> {
		return{ m_SortTables.begin(), m_SortTables.end() };
	}
};
