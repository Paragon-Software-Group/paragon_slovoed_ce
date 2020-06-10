#ifndef _C_SLD_SYMBOLS_TABLE_
#define _C_SLD_SYMBOLS_TABLE_

#include "SldPlatform.h"
#include "SldTypes.h"
#include "SldError.h"
#include "SldSDCReadMy.h"


namespace sld2
{
	namespace symbolsTable
	{
		/// Возвращает сортированный массив Emoji-символов
		inline Span<const UInt16> GetFullEmojiTable()
		{
			static const UInt16 fullEmojiTable[]
			{
				0x0023,
				0x0030,
				0x0031,
				0x0032,
				0x0033,
				0x0034,
				0x0035,
				0x0036,
				0x0037,
				0x0038,
				0x0039,
				0x203C,
				0x2049,
				0x2139,
				0x2194,
				0x2195,
				0x2196,
				0x2197,
				0x2198,
				0x2199,
				0x21A9,
				0x21AA,
				0x231A,
				0x231B,
				0x24C2,
				0x25AA,
				0x25AB,
				0x25B6,
				0x25C0,
				0x25FB,
				0x25FC,
				0x25FD,
				0x25FE,
				0x2600,
				0x2601,
				0x260E,
				0x2611,
				0x2614,
				0x2615,
				0x261D,
				0x263A,
				0x2648,
				0x2649,
				0x264A,
				0x264B,
				0x264C,
				0x264D,
				0x264E,
				0x264F,
				0x2650,
				0x2651,
				0x2652,
				0x2653,
				0x2660,
				0x2663,
				0x2665,
				0x2666,
				0x2668,
				0x267B,
				0x267F,
				0x2693,
				0x26A0,
				0x26A1,
				0x26AA,
				0x26AB,
				0x26BD,
				0x26BE,
				0x26C4,
				0x26C5,
				0x26D4,
				0x26EA,
				0x26F2,
				0x26F3,
				0x26F5,
				0x26FA,
				0x26FD,
				0x2702,
				0x2708,
				0x2709,
				0x270C,
				0x270F,
				0x2712,
				0x2714,
				0x2716,
				0x2733,
				0x2734,
				0x2744,
				0x2747,
				0x2757,
				0x2764,
				0x27A1,
				0x2934,
				0x2935,
				0x2B05,
				0x2B06,
				0x2B07,
				0x2B1B,
				0x2B1C,
				0x2B50,
				0x2B55,
				0x303D,
				0x3297,
				0x3299
			};
			return sld2::make_span(fullEmojiTable);
		}

		/// Возвращает сортированный массив словоедовских Emoji-символов
		inline Span<const UInt16> GetSldEmojiTable()
		{
			static const UInt16 sldEmojiTable[]
			{
				0x2194,
				0x25B6,
				0x25BC
			};
			return sld2::make_span(sldEmojiTable);
		}

		/// Возвращает сортированный массив пробельных символов
		inline Span<const UInt16> GetSpaceTable()
		{
			static const UInt16 spaceTable[]
			{
				0x0009,
				0x000a,
				0x000b,
				0x000c,
				0x000d,
				0x0020,
				0x0085,
				0x00a0,
				0x1680,
				0x180e,
				0x2000,
				0x2001,
				0x2002,
				0x2003,
				0x2004,
				0x2005,
				0x2006,
				0x2007,
				0x2008,
				0x2009,
				0x200a,
				0x200b,
				0x200c,
				0x200d,
				0x2028,
				0x2029,
				0x202f,
				0x205f,
				0x2060,
				0x3000
			};
			return sld2::make_span(spaceTable);
		}
	} //symbolsTable
} // sld2

/// Класс, представляющий таблицу символов для определенного языка либо таблицу символов-разделителей
class CSldSymbolsTable
{
public:
	
	/// Конструктор
	CSldSymbolsTable(void);

public:

	/// Инициализация
	ESldError Init(CSDCReadMy &aData, UInt32 aResourceType, UInt32 aResourceIndex);
	
	/// Получает код языка таблицы
	ESldLanguage GetLanguageCode() const;
	
	/// Проверяет, принадлежит ли символ языку
	bool IsSymbolBelongToLanguage(UInt16 aCode) const;

	/// Объединяет таблицы символов
	ESldError MergeTables(const CSldSymbolsTable* aTable);

private:

	/// Заголовок таблицы
	TLanguageSymbolsTableHeader m_Header;
	
	/// Таблица
	sld2::DynArray<UInt8> m_Table;
	
};

#endif
