#include "SortTablesManager.h"

#include <algorithm>
#include <iterator>

#include "sld_Platform.h"
#include "Tools.h"
#include "util.h"


/***********************************************************************
* Конструктор
***********************************************************************/
CSortTablesManager::CSortTablesManager(void) {}

/***********************************************************************
* Возвращает таблицу сортировки по коду языка
*
* @param aLangCode	- код языка таблицы сортировки
*
* @return указатель на таблицу сортировки или NULL, если таблица не найдена
***********************************************************************/
const TLanguageSortTable* CSortTablesManager::GetLanguageSortTable(UInt32 aLangCode) const
{
	const auto fit = m_SortTables.find(aLangCode);
	if (fit != m_SortTables.end())
		return &(fit->second);

	return NULL;
}

static int loadSortTable(UInt32 aLangCode, const std::wstring &aFilename,
								 std::map<UInt32, TLanguageSortTable> &aTables,
								 std::map<UInt32, std::unordered_set<std::wstring>> &aFiles,
								 bool(*check)(const TLanguageSortTable&),
								 void(*init)(TLanguageSortTable&, MemoryBuffer&, sld::wstring_ref))
{
	wchar_t lang_code_str[5] = {0};
	char* cptr = (char*)&aLangCode;
	for (int i = 0; i < 4; i++)
		lang_code_str[i] = cptr[i];

	// Проверяем, загружали или нет мы уже этот файл для данного языка
	auto&& fit = aFiles.find(aLangCode);
	if (fit != aFiles.end() && fit->second.find(aFilename) != fit->second.end())
	{
		STString<512> wbuf(L"Warning! Duplicated sort tables specified: language=\"%s\", filename=\"%s\"", lang_code_str, aFilename);
		sldILog("%s\n", wbuf);
		sld::printf(eLogStatus_Warning, "\n%s", wbuf);
		return ERROR_NO;
	}

	// Проверяем, может мы уже загружали для данного языка таблицу сортировки
	auto it = aTables.find(aLangCode);
	if (it != aTables.end() && check(it->second))
	{
		STString<512> wbuf(L"Warning! Sort table redifinition: language=\"%s\", filename=\"%s\". Used first table.", lang_code_str, aFilename);
		sldILog("%s\n", wbuf);
		sld::printf(eLogStatus_Warning, "\n%s", wbuf);
		return ERROR_NO;
	}

	// Загружаем таблицу
	auto fileData = sld::read_file(aFilename);
	if (fileData.empty())
	{
		sldILog("Error! Can't read sort table file \"%s\"\n", sld::as_ref(aFilename));
		return ERROR_CANT_READ_FILE;
	}

	// Добавляем таблицу
	if (it == aTables.end())
		it = aTables.emplace(aLangCode, TLanguageSortTable(aLangCode)).first;
	init(it->second, fileData, aFilename);

	aFiles[aLangCode].insert(aFilename);

	return ERROR_NO;
}

/***********************************************************************
* Добавляет основную таблицу сортировки
*
* @param aLangCode	- код языка таблицы сортировки
* @param aFilename	- имя файла с таблицей сортировки
*
* @return код ошибки
***********************************************************************/
int CSortTablesManager::AddMainSortTable(UInt32 aLangCode, const wstring &aFilename)
{
	static const auto check = [](const TLanguageSortTable &table) { return !table.MainTable.empty(); };
	static const auto init = [](TLanguageSortTable &table, MemoryBuffer &fileData, sld::wstring_ref fileName) {
		table.MainTable = std::move(fileData);
		table.MainFileName = to_string(fileName);
	};

	return loadSortTable(aLangCode, aFilename, m_SortTables, m_SortTablesFiles, check, init);
}

/***********************************************************************
* Добавляет вспомогательную таблицу сортировки
*
* @param aLangCode	- код языка таблицы сортировки
* @param aFilename	- имя файла с таблицей сортировки
*
* @return код ошибки
***********************************************************************/
int CSortTablesManager::AddSecondarySortTable(UInt32 aLangCode, const wstring &aFilename)
{
	static const auto check = [](const TLanguageSortTable &table) { return !table.SecondTable.empty(); };
	static const auto init = [](TLanguageSortTable &table, MemoryBuffer &fileData, sld::wstring_ref) {
		table.SecondTable = std::move(fileData);
	};

	return loadSortTable(aLangCode, aFilename, m_SortTables, m_SecondarySortTablesFiles, check, init);
}

/***********************************************************************
* Добавляет в таблицы сортировки символы из списков слов
*
* @param aLangCode	- код языка таблицы сортировки
* @param aSymbolMap	- указатель на массив всех символов из слов списков слов с основным языком aLangCode
*
* @return код ошибки
***********************************************************************/
int CSortTablesManager::AddDictionarySymbolPairsTable(UInt32 aLangCode, const std::unordered_set<UInt16> &aSymbolMap)
{
	// TODO: символы добавляются (если они есть) только в основную таблицу сортировки.
	// Нужно ли добавлять их во вспомогательную?

	map<UInt32, TLanguageSortTable>::iterator fit = m_SortTables.find(aLangCode);
	if (fit == m_SortTables.end())
	{
		sldILog("Error! CSortTablesManager::AddDictionarySymbolPairsTable : sort table doesn't exist!\n");
		return ERROR_NULL_SORT_TABLE;
	}

	// Загруженная из файла таблица сортировки
	TLanguageSortTable* pSortTable = &(fit->second);

	if (pSortTable->MainTable.empty())
	{
		sldILog("Error! CSortTablesManager::AddDictionarySymbolPairsTable : pMainTable doesn't exist!\n");
		return ERROR_NULL_POINTER;
	}

	// Нечего добавлять
	if (aSymbolMap.empty())
		return ERROR_NO;

	TCompareTableSplit TableSplit;
	memset(&TableSplit, 0, sizeof(TableSplit));

	TableSplit.Header = (CMPHeaderType*)pSortTable->MainTable.data();

	if (TableSplit.Header->Version < SLD_TABLE_SPLIT_VERSION)
	{
		sldILog("Error! CSortTablesManager::AddDictionarySymbolPairsTable : wrong sort table file format!\n");
		return ERROR_WRONG_SORT_TABLE_FILE_FORMAT;
	}
	TableSplit.Simple = (CMPSimpleType*)(TableSplit.Header + 1);
	TableSplit.Complex = (CMPComplexType*)(TableSplit.Simple + TableSplit.Header->SimpleCount);
	TableSplit.Delimiter = (CMPDelimiterType*)(TableSplit.Complex + TableSplit.Header->ComplexCount);
	TableSplit.Native = (CMPNativeType*)(TableSplit.Delimiter + TableSplit.Header->DelimiterCount);
	TableSplit.HalfDelimiter = (CMPHalfDelimiterType*)(TableSplit.Native + TableSplit.Header->NativeCount);

	// В таблице сортировки нет таблиц соответствия - выходим
	if (!(TableSplit.Header->FeatureFlag & SLD_CMP_TABLE_FEATURE_SYMBOL_PAIR_TABLE_MASK))
		return ERROR_NO;
	
	TableSplit.HeaderPairSymbols = (CMPSymbolPairTableHeader*)(TableSplit.HalfDelimiter + TableSplit.Header->HalfDelimiterCount);

	UInt8* ptr = (UInt8*)(TableSplit.HeaderPairSymbols) + TableSplit.HeaderPairSymbols->structSize;
	if (TableSplit.HeaderPairSymbols->NativePairCount)
	{
		TableSplit.NativePair = (CMPSymbolPair*)ptr;
		ptr += TableSplit.HeaderPairSymbols->NativePairCount * sizeof(TableSplit.NativePair[0]);
	}

	if (TableSplit.HeaderPairSymbols->CommonPairCount)
	{
		TableSplit.CommonPair = (CMPSymbolPair*)ptr;
		//ptr += TableSplit.HeaderPairSymbols->CommonPairCount * sizeof(TableSplit.CommonPair[0]);
	}

	if (TableSplit.HeaderPairSymbols->DictionaryPairCount)
	{
		// Такого не может быть, именно эту часть мы и хотим заполнить, в самой таблице этих данных нет
		sldILog("Error! CSortTablesManager::AddDictionarySymbolPairsTable : wrong sort table file format!\n");
		return ERROR_WRONG_SORT_TABLE_FILE_FORMAT;
	}

	TableSplit.TableSize = static_cast<UInt32>(pSortTable->MainTable.size());
	
	if (!TableSplit.HeaderPairSymbols->NativePairCount && !TableSplit.HeaderPairSymbols->CommonPairCount)
	{
		// У нас нет ни одной таблицы соответствия в самой таблице сортировки,
		// мы не можем построить аналогичную таблицу соответствия для символов списков слов
		return ERROR_NO;
	}

	// Массив пар символов из списков слов: символ в верхнем регистре - символ в нижнем регистре
	map<UInt16, UInt16> mDictPairs;

	// Символ, который нужно добавить (если найдем для него пару)
	for (UInt16 Symbol : aSymbolMap)
	{
		UInt16 PairSymbol = 0;
		UInt8 IsSymbolUppercase = 0;

		// Ищем парный символ в первой таблице
		for (UInt32 i=0;i<TableSplit.HeaderPairSymbols->NativePairCount;i++)
		{
			if (Symbol == TableSplit.NativePair[i].up)
			{
				PairSymbol = TableSplit.NativePair[i].low;
				IsSymbolUppercase = 1;
				break;
			}
			else if (Symbol == TableSplit.NativePair[i].low)
			{
				PairSymbol = TableSplit.NativePair[i].up;
				break;
			}
		}

		if (!PairSymbol)
		{
			// Ищем парный символ во второй таблице
			for (UInt32 i=0;i<TableSplit.HeaderPairSymbols->CommonPairCount;i++)
			{
				if (Symbol == TableSplit.CommonPair[i].up)
				{
					PairSymbol = TableSplit.CommonPair[i].low;
					IsSymbolUppercase = 1;
					break;
				}
				else if (Symbol == TableSplit.CommonPair[i].low)
				{
					PairSymbol = TableSplit.CommonPair[i].up;
					break;
				}
			}
		}

		if (PairSymbol)
		{
			if (IsSymbolUppercase)
				mDictPairs[Symbol] = PairSymbol;
			else
				mDictPairs[PairSymbol] = Symbol;
		}
	}

	if (mDictPairs.empty())
		return ERROR_NO;

	UInt32 AddCount = (UInt32)mDictPairs.size();
	UInt32 AddMemSize = AddCount*sizeof(TableSplit.DictionaryPair[0]);

	// Обновляем информацию в заголовке о количестве символов в третьей таблице
	TableSplit.HeaderPairSymbols->DictionaryPairCount = AddCount;

	MemoryBuffer newSortTable(TableSplit.TableSize + AddMemSize);
	memcpy(newSortTable.data(), pSortTable->MainTable.data(), TableSplit.TableSize);

	pSortTable->MainTable = std::move(newSortTable);

	CMPSymbolPair* DictTable = (CMPSymbolPair*)(pSortTable->MainTable.data() + TableSplit.TableSize);
	for (const auto &dpit : enumerate(mDictPairs))
	{
		DictTable[dpit.index].up = dpit->first;
		DictTable[dpit.index].low = dpit->second;
	}

	return ERROR_NO;
}

/***********************************************************************
* Возвращает общее количество загруженных таблиц сортировки (основных и вспомогательных)
*
* @return количество загруженных таблиц сортировки
***********************************************************************/
UInt32 CSortTablesManager::GetTotalSortTablesCount(void) const
{
	UInt32 count = 0;
	for (auto&& table : m_SortTables)
	{
		if (table.second.MainTable.size())
			count++;
		if (table.second.SecondTable.size())
			count++;
	}
	return count;
}
