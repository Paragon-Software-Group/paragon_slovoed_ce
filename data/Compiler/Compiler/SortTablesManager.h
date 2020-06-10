#pragma once

#include <string>
#include <map>
#include <unordered_set>
#include <vector>

#include "Log.h"
#include "sld_Types.h"
#include "util.h"

using namespace std;


/// ������� ���������� ��� ����������� �����
struct TLanguageSortTable
{
	UInt32 LangCode;

	MemoryBuffer MainTable;
	std::wstring MainFileName;

	MemoryBuffer SecondTable;
	std::wstring SecondFileName;

	TLanguageSortTable(UInt32 lang) : LangCode(lang) {}
};

/// ����� ��� ������ � ��������� ����������
class CSortTablesManager
{
public:
	
	/// �����������
	CSortTablesManager(void);

public:
	
	/// ��������� �������� ������� ����������
	int AddMainSortTable(UInt32 aLangCode, const wstring &aFilename);

	/// ��������� ��������������� ������� ����������
	int AddSecondarySortTable(UInt32 aLangCode, const wstring &aFilename);

	/// ��������� � ������� ���������� ������� �� ������� ����
	int AddDictionarySymbolPairsTable(UInt32 aLangCode, const std::unordered_set<UInt16> &aSymbolMap);

	/// ���������� ������� ���������� �� ���� �����
	const TLanguageSortTable* GetLanguageSortTable(UInt32 aLangCode) const;

	/// ���������� ����� ���������� ����������� ������ ���������� (�������� � ���������������)
	UInt32 GetTotalSortTablesCount(void) const;

private:
	/// ������ ���� ������ �������� ������ ���������� (������������ ��� �������� ����, ����� �� ��������� ���� � �� �� ������� ��������� ��������� ���)
	map<UInt32, unordered_set<wstring> > m_SortTablesFiles;

	/// ������ ���� ������ ��������������� ������ ���������� (������������ ��� �������� ����, ����� �� ��������� ���� � �� �� ������� ��������� ��������� ���)
	map<UInt32, unordered_set<wstring> > m_SecondarySortTablesFiles;

	/// ������ ������ ����������
	map<UInt32, TLanguageSortTable> m_SortTables;

public:
	auto tables() const -> sld::MapDerefRange<decltype(m_SortTables.begin())> {
		return{ m_SortTables.begin(), m_SortTables.end() };
	}
};
