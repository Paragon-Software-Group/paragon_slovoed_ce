/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
+
+	(C) Paragon Software, 2001-2009.
+	Author: Zhukov Boris
+	File:   StringCompare.h
+
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
+	
+	interface for the CStringCompare class.
+
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#pragma once

#include "sld_Types.h"
#include <memory.h>

// 
#define CMP_MASK_OF_INDEX_FLAG		(0x8000)
#define CMP_MASK_OF_INDEX			(0x7fff)
#define CMP_IGNORE_SYMBOL			(0)


//////////////////////////////////////////////////////////////////////////////////
//
//	CStringCompare - encapsulate string compare functions.
//
//////////////////////////////////////////////////////////////////////////////////
class CStringCompare  
{
public:
	CStringCompare();			// constructor
	~CStringCompare();	// destructor

	void EnsureSymbols(char *table);	// set symbols as used not only marked as used
										// but also their pair symbols (e.g. 'a' and 'A')
	int TestForQAW(unsigned short *str);	// Test string for ability to use it in quick access table
	int Init(const wchar_t *fileName, const wchar_t *secondaryFileName);	// load compare tables
	int Init(const UInt8* aMainTable, UInt32 aMainTableSize, const UInt8* aSecondaryTable, UInt32 aSecondaryTableSize);	// init by loaded compare tables
	bool IsCompare2() const;

	int StrWCMP(const wchar_t *str1, const wchar_t *str2, int table=0) const;	// Unicode string compare
	int StrWCMP16(const UInt16 *str1, const UInt16 *str2, int table=0) const;	// Unicode string compare

	int StrNWCMP16(const UInt16 *str1, const UInt16 *str2, UInt16 size);	// Unicode string compare
	
	static int StrWCmp(const wchar_t *str1, const wchar_t *str2);
	static int StrWCmp16(const UInt16 *str1, const UInt16 *str2);

	void TestWCMP(const unsigned short *unicode, int line);	// Собирает инофрмацию о том какие символы отсутствуют в таблице сравнения.
	void TestWCMPReset();	// Очищает информацию о том, какие символы отсутствуют в таблице сравнения.
	int  TestWCMPGetCount();	// Возвращает количество символов отсутсвующих в таблице сравнения
	void  TestWCMPGetWrong(int i, unsigned short *ch, int *line);	// Возвращает символ отсутсвующий в таблице сравнения
	UInt16 GetMass(UInt16 ch, int table = 0); // Возвращает массу символа
	static Int32 StrLen(const UInt16 *aStr);


private:
	UInt16 GetMass(UInt16 ch, const CMPSimpleType *simple, UInt16 simpleCount, UInt16 iNotFound) const;
	UInt16 GetComplex(const UInt16 *str, UInt16 index, UInt16 *mass, const CMPComplexType *complex) const;
	UInt16 StrUTF8_2_UTF16(UInt16 *unicode, const UInt8 *str);

	UInt32 norm[256]; // table with codes for no
	char	*m_UnicodeTable;
	UInt32	m_UnicodeTableSize;
	char	*m_SecondUnicodeTable;
	UInt32	m_SecondUnicodeTableSize;
	bool	m_SecondUnicode;

	int		m_NotFoundIdx;		// Количество не найденых в таблице сравнения символов.
	int		m_NotFoundMax;		// Размер буффера для не найденых в таблице сравнения символов.
	WrongCMPType	*m_NotFound;		// Буффер для не найденых символов
};

int UTF8_to_UNICODE(const unsigned char* utf8, unsigned short* unicode);
