/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
+
+	(C) Paragon Software, 2001-2003.
+	Author: Zhukov Boris
+	File:   StringCompare.cpp
+
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
+
+	implementation of the CStringCompare class.
+
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#include "StringCompare.h"
#include <stdio.h>
#include <string.h>
#include "sld2_Error.h"
#include "sld_Platform.h"
#include "Log.h"
#include "Tools.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CStringCompare::CStringCompare()
{
	m_UnicodeTable = NULL;
	m_UnicodeTableSize = 0;
	m_SecondUnicodeTable = NULL;
	m_SecondUnicodeTableSize = 0;
	m_SecondUnicode = false;

	m_NotFoundIdx = m_NotFoundMax = 0;
	m_NotFound = NULL;
}

CStringCompare::~CStringCompare()
{
	if (m_UnicodeTable)
		delete[] m_UnicodeTable;
	if (m_SecondUnicodeTable)
		delete [] m_SecondUnicodeTable;
	if (m_NotFound)
		delete[] m_NotFound;

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
+
+ FUNCTION:		Init
+
+ DESCRIPTION:	This function load compare table.
+
+ PARAMETERS:	char *fileName - pointer to null-terminated string, that
+					contain path to compare table.
+
+ RETURNED:		error code
+
+ REVISION HISTORY:
+
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int CStringCompare::Init(const wchar_t *fileName, const wchar_t *secondaryFileName)
{
	if (m_UnicodeTable)
		delete[] m_UnicodeTable; // free memory from previous compare table

	// open input stream
	auto fileData = sld::read_file(fileName);
	if (fileData.empty())
	{
		sldILog("Error! CStringCompare::Init : can't open file.\n");
		return ERROR_WRONG_SORT_FILE;
	}

	m_UnicodeTableSize = static_cast<UInt32>(fileData.size());
	m_UnicodeTable = (char*)fileData.release();

	if (m_SecondUnicodeTable) 
	{
		delete[] m_SecondUnicodeTable; // free memory from previous compare table
		m_SecondUnicodeTable = NULL;
	}

	if (secondaryFileName && wcslen(secondaryFileName))
	{
		// open input stream
		fileData = sld::read_file(secondaryFileName);
		if (fileData.empty())
		{
			sldILog("Error! CStringCompare::Init : can't open file.\n");
			return ERROR_WRONG_SORT_FILE;
		}

		m_SecondUnicodeTableSize = static_cast<UInt32>(fileData.size());
		m_SecondUnicodeTable = (char*)fileData.release();
	}

	return ERROR_NO;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
+
+ FUNCTION:		Init
+
+ DESCRIPTION:	This function init by loaded compare tables
+
+ PARAMETERS:	aMainTable		- pointer to loaded main compare table.
+ PARAMETERS:	aSecondaryTable	- pointer to loaded secondary compare table.
+
+ RETURNED:		error code
+
+ REVISION HISTORY:
+
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int CStringCompare::Init(const UInt8* aMainTable, UInt32 aMainTableSize, const UInt8* aSecondaryTable, UInt32 aSecondaryTableSize)
{
	if (!aMainTable)
	{
		sldILog("Error! CStringCompare::Init : Null input pointer!\n");
		return ERROR_NULL_POINTER;
	}
	
	if (m_UnicodeTable)
	{
		delete [] m_UnicodeTable;
		m_UnicodeTable = NULL;
	}

	m_UnicodeTableSize = aMainTableSize;
	m_UnicodeTable = new char[m_UnicodeTableSize];
	if (!m_UnicodeTable)
		return eMemoryNotEnough;

	memmove(m_UnicodeTable, aMainTable, m_UnicodeTableSize);

	if (m_SecondUnicodeTable) 
	{
		delete [] m_SecondUnicodeTable;
		m_SecondUnicodeTable = NULL;
	}

	if (aSecondaryTable)
	{
		m_SecondUnicodeTableSize = aSecondaryTableSize;
		m_SecondUnicodeTable = new char[m_SecondUnicodeTableSize];
		if (!m_SecondUnicodeTable)
			return eMemoryNotEnough;

		memmove(m_SecondUnicodeTable, aSecondaryTable, m_SecondUnicodeTableSize);
	}

	return ERROR_NO;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
+
+ FUNCTION:		TestForQAW
+
+ DESCRIPTION:	This function return length of symbol, that pointed by
+				"str".
+
+ PARAMETERS:	unsigned char *str - pointer to string(symbol)
+
+ RETURNED:		int - size of symbol
+
+ REVISION HISTORY:
+
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int CStringCompare::TestForQAW(unsigned short *str)
{
// Вычисляем положение частей таблицы сравнения.
CMPHeaderType *header = (CMPHeaderType *)m_UnicodeTable;
CMPSimpleType *simple = (CMPSimpleType *)(header+1);
CMPComplexType *complex = (CMPComplexType *)(simple+header->SimpleCount);
UInt16 mass_idx, mass[5];

	// if this is unused(skip at comparing) symbol
	if ((mass_idx = GetMass(*str, simple, header->SimpleCount, CMP_IGNORE_SYMBOL)) == CMP_IGNORE_SYMBOL)
		return 1;

	// Если это простой символ.
	if (!(mass_idx&CMP_MASK_OF_INDEX_FLAG))
	{
		return 1;
	}

	// Если это сложный символ, тогда сразу возвращаем его длинну.
	return GetComplex(str, mass_idx&CMP_MASK_OF_INDEX, mass, complex);
}

int CStringCompare::StrWCMP(const wchar_t *str1, const wchar_t *str2, int table) const
{
	// входные таблицы сравнения все в utf-16, поэтому создаем соотв. строки и вызываем соотв. функции
	const std::u16string string1 = sld::as_utf16(str1);
	const std::u16string string2 = sld::as_utf16(str2);
	return StrWCMP16((const UInt16*)string1.c_str(), (const UInt16*)string2.c_str(), table);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
+
+ FUNCTION:		StrWCMP16
+
+ DESCRIPTION:	This function compares two unicode(UTF-16) strings
+
+ PARAMETERS:	char *str1 - first string for compare
+				char *str2 - second string for compare
+				void *vTablePtr - pointer to table of comparison.
+
+ RETURNED:		int - size of symbol
+
+ REVISION HISTORY:
+
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int CStringCompare::StrWCMP16(const UInt16 *str1, const UInt16 *str2, int table) const
{
UInt16 mass1[5],mass2[5];
UInt16 *cmp1=mass1, *cmp2=mass2;
UInt16 last_char_mass1, last_char_mass2;
// Вычисляем положение частей таблицы сравнения.
CMPHeaderType *header;
	if (table == 0)
		header = (CMPHeaderType *)m_UnicodeTable;
	else
		header = (CMPHeaderType *)m_SecondUnicodeTable;

	if (!header)
	{
		return wcscmp((wchar_t*)str1, (wchar_t*)str2);
	}


CMPSimpleType *simple = (CMPSimpleType *)(header+1);
CMPComplexType *complex = (CMPComplexType *)(simple+header->SimpleCount);


	memset(mass1,0,sizeof(mass1));
	memset(mass2,0,sizeof(mass2));

	// Compare strings while both strings not finished
	while ((*str1 || *(cmp1+1)) || (*str2 || *(cmp2+1)))
	{

		while (*str1 && (last_char_mass1 = GetMass(*str1, simple, header->SimpleCount, CMP_IGNORE_SYMBOL)) == CMP_IGNORE_SYMBOL)
			str1++;
		while (*str2 && (last_char_mass2 = GetMass(*str2, simple, header->SimpleCount, CMP_IGNORE_SYMBOL)) == CMP_IGNORE_SYMBOL)
			str2++;
/*
		if (!(*str1 || *(cmp1+1)))
			break;
		if (!(*str2 || *(cmp2+1)))
			break;
*/
		if (*str1 && !*(cmp1+1))
		{
			if (last_char_mass1&CMP_MASK_OF_INDEX_FLAG)
			{
				// Otherwise calculate conversion value separately and
				// shift pointer by properly number of bytes
				// (not only 1, may be bigger).
				str1 += GetComplex(str1, last_char_mass1&CMP_MASK_OF_INDEX, mass1, complex);
				cmp1 = mass1;
			}else
			{
				cmp1 = mass1;
				cmp1[0] = last_char_mass1;
				cmp1[1] = 0;
				str1++;
			}
		}else
		{
			cmp1++;
		}

		if (*str2 && !*(cmp2+1))
		{
			if (last_char_mass2&CMP_MASK_OF_INDEX_FLAG)
			{
				// Otherwise calculate conversion value separately and
				// shift pointer by properly number of bytes
				// (not only 2, may be bigger).
				str2 += GetComplex(str2, last_char_mass2&CMP_MASK_OF_INDEX, mass2, complex);
				cmp2 = mass2;
			}else
			{
				cmp2 = mass2;
				cmp2[0] = last_char_mass2;
				cmp2[1] = 0;
				str2++;
			}
		}else
		{
			cmp2++;
		}

		// Compare. If first symbol smaller then other, then first 
		// string smaller then second. return -1.
		if (*cmp1 < *cmp2)
		{
			return -1;
		}
		else if (*cmp2 < *cmp1)
		{
			return 1;
		}
	}

	// Сравниваем остаточные веса.
	if (*(cmp1+1) > *(cmp2+1))
		return 1;
	if (*(cmp1+1) < *(cmp2+1))
		return -1;

	// skip unused symbols
	while (*str1 && (last_char_mass1 = GetMass(*str1, simple, header->SimpleCount, CMP_IGNORE_SYMBOL)) == CMP_IGNORE_SYMBOL)
		str1++;
	while (*str2 && (last_char_mass2 = GetMass(*str2, simple, header->SimpleCount, CMP_IGNORE_SYMBOL)) == CMP_IGNORE_SYMBOL)
		str2++;

	// if strings are equal (both string are empty now)
	if (*str1 == *str2)
	{
		return 0;
	}else
	if (!(*str1))
	{
		return -1;
	}else
	{
		return 1;
	}
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
+
+ FUNCTION:		StrNWCMP16
+
+ DESCRIPTION:	This function compares two unicode(UTF-16) strings
+
+ PARAMETERS:	char *str1 - first string for compare
+				char *str2 - second string for compare
+				void *vTablePtr - pointer to table of comparison.
+
+ RETURNED:		int - size of symbol
+
+ REVISION HISTORY:
+
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int CStringCompare::StrNWCMP16(const UInt16 *str1, const UInt16 *str2, UInt16 size)
{
UInt16 mass1[5],mass2[5];
UInt16 *cmp1=mass1, *cmp2=mass2;
UInt16 last_char_mass1, last_char_mass2;
// Вычисляем положение частей таблицы сравнения.
CMPHeaderType *header = (CMPHeaderType *)m_UnicodeTable;
CMPSimpleType *simple = (CMPSimpleType *)(header+1);
CMPComplexType *complex = (CMPComplexType *)(simple+header->SimpleCount);
UInt16 len_1, len_2, shift;


	memset(mass1,0,sizeof(mass1));
	memset(mass2,0,sizeof(mass2));
	len_1 = len_2 = 0;

	// Compare strings while both strings not finished
	while ((*str1 || *(cmp1+1) || *str2 || *(cmp2+1)) && len_1 < size && len_2 < size)
	{

		while (*str1 && (last_char_mass1 = GetMass(*str1, simple, header->SimpleCount, CMP_IGNORE_SYMBOL)) == CMP_IGNORE_SYMBOL)
		{
			str1++;
			len_1++;
		}
		while (*str2 && (last_char_mass2 = GetMass(*str2, simple, header->SimpleCount, CMP_IGNORE_SYMBOL)) == CMP_IGNORE_SYMBOL)
		{
			str2++;
			len_2++;
		}
		/*
		if (!(*str1 || *(cmp1+1)))
		break;
		if (!(*str2 || *(cmp2+1)))
		break;
		*/
		if (*str1 && !*(cmp1+1))
		{
			if (last_char_mass1&CMP_MASK_OF_INDEX_FLAG)
			{
				// Otherwise calculate conversion value separately and
				// shift pointer by properly number of bytes
				// (not only 1, may be bigger).
				shift = GetComplex(str1, last_char_mass1&CMP_MASK_OF_INDEX, mass1, complex);
				str1 += shift;
				len_1 += shift;
				cmp1 = mass1;
			}else
			{
				cmp1 = mass1;
				cmp1[0] = last_char_mass1;
				cmp1[1] = 0;
				str1++;
				len_1++;
			}
		}else
		{
			cmp1++;
		}

		if (*str2 && !*(cmp2+1))
		{
			if (last_char_mass2&CMP_MASK_OF_INDEX_FLAG)
			{
				// Otherwise calculate conversion value separately and
				// shift pointer by properly number of bytes
				// (not only 2, may be bigger).
				shift += GetComplex(str2, last_char_mass2&CMP_MASK_OF_INDEX, mass2, complex);
				str2 += shift;
				len_2 += shift;
				cmp2 = mass2;
			}else
			{
				cmp2 = mass2;
				cmp2[0] = last_char_mass2;
				cmp2[1] = 0;
				str2++;
				len_2++;
			}
		}else
		{
			cmp2++;
		}

		// Compare. If first symbol smaller then other, then first 
		// string smaller then second. return -1.
		if (*cmp1 < *cmp2)
		{
			return -1;
		}
		else if (*cmp2 < *cmp1)
		{
			return 1;
		}
	}

	return 0;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
+
+ FUNCTION:		GetMass
+
+ DESCRIPTION:	This function returns mass of the symbol "ch". If symbol 
+				not found, then return CMP_IGNORE_SYMBOL
+
+ PARAMETERS:	UInt16 ch - symbol to find
+				CMPSimpleType *simple - pointer to table with simple 
+								symbols description
+				UInt16 simpleCount - number of simple table items.
+
+ RETURNED:		returns mass of the symbol "ch". If symbol 
+				not found, then return CMP_IGNORE_SYMBOL
+
+ REVISION HISTORY:
+
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
UInt16 CStringCompare::GetMass(UInt16 ch, const CMPSimpleType *simple, UInt16 simpleCount, UInt16 iNotFound) const
{
UInt16 low, med, hi;
	low = 0;
	hi = simpleCount-1;
	med = (hi+low)/2;
	while (hi-low > 1)
	{
		if (simple[med].ch > ch)
			hi = med;
		else if (simple[med].ch < ch)
			low = med;
		else return simple[med].mass;

		med = (hi+low)/2;
	}

	if (simple[low].ch == ch)
		return simple[low].mass;
	if (simple[hi].ch == ch)
		return simple[hi].mass;

	// Символ не найден, поэтому возвращаем вес символа соответсвующий игнорированию.
	return iNotFound;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
+
+ FUNCTION:		GetMass
+
+ DESCRIPTION:	This function returns mass of the symbol "ch". If symbol 
+				not found, then return CMP_IGNORE_SYMBOL
+
+ PARAMETERS:	UInt16 ch - symbol to find
+
+ RETURNED:		returns mass of the symbol "ch". If symbol 
+				not found, then return CMP_IGNORE_SYMBOL
+
+ REVISION HISTORY:
+
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

UInt16 CStringCompare::GetMass(UInt16 ch, int table)
{
	// Вычисляем положение частей таблицы сравнения.
	CMPHeaderType *header;
	if (table == 0)
		header = (CMPHeaderType *)m_UnicodeTable;
	else
		header = (CMPHeaderType *)m_SecondUnicodeTable;

	CMPSimpleType *simple = (CMPSimpleType *)(header+1);

	return GetMass(ch, simple, header->SimpleCount, CMP_IGNORE_SYMBOL);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
+
+ FUNCTION:		GetComplex
+
+ DESCRIPTION:	This function finds description for the complex symbols,
+				and substitute one with masses.
+
+ PARAMETERS:	UInt16 *str - pointer to string for matching with complex 
+								symbols
+				UInt16 index - index of start complex table item
+				UInt16 *mass - pointer to the array for masses of the 
+								complex symbol.
+				CMPComplexType *complex - pointer to the table with 
+								description of the complex symbols.
+
+ RETURNED:		number of chars we need skip.
+
+ REVISION HISTORY:
+
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
UInt16 CStringCompare::GetComplex(const UInt16 *str, UInt16 index, UInt16 *mass, const CMPComplexType *complex) const
{
Int16 best_len = 0;
Int16 best_index = 0;
UInt16 count;

	// Ищем наиболее подходящий сложный символ(символ наиболее полно совпадающий с строчкой)
	while (complex[index].chain[0] == *str)
	{
		// считаем количество совпадений.
		count = 1;// одно совпадение уже есть.
		while (count < 4 && complex[index].chain[count] == str[count] && str[count])
		{
			count++;
		}

		if (best_len < count && (count >= 4 || complex[index].chain[count] == 0))
		{
			best_len = count;
			best_index = index;
		}
		index++;
	}

	// если хоть один символ был найден
	if (best_len)
	{
		// тогда возвращаем данные по нему.
		mass[0] = complex[best_index].mass[0];
		mass[1] = complex[best_index].mass[1];
		mass[2] = complex[best_index].mass[2];
		mass[3] = complex[best_index].mass[3];
		return best_len;
	}

	// ничего не найдено - возвращаем символ игнорировать, и сообщаем, что 
	// текущий символ нужно пропустить.
	mass[0] = CMP_IGNORE_SYMBOL;
	return 1;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
+
+ FUNCTION:		StrUTF8_2_UTF16
+
+ DESCRIPTION:	This function converts from UTF8 to UTF16(standard unicode)
+
+ PARAMETERS:	UInt16 *unicode - pointer to buffer for unicode
+				UInt16 *str - ASCII string
+
+ RETURNED:		????
+
+ REVISION HISTORY:
+
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

UInt16 CStringCompare::StrUTF8_2_UTF16(UInt16 *unicode, const UInt8 *str)
{
	return UTF8_to_UNICODE(str, unicode);
}

int UTF8_to_UNICODE(const unsigned char* utf8, unsigned short* unicode)
{
	const unsigned char* src = utf8;
	unsigned short u;
	if(unicode)
	{
		unsigned short* dst = unicode;
		while(*src)
		{
			if(!(*src & 0x80))
				*dst = *src;
			else
				if((*src & 0xE0)==0xC0)
				{
					u = *src;
					*dst = (u & 0x1F) << 6;
					src++;
					if((*src & 0xC0)!=0x80)
						return -1;
					*dst |= (*src & 0x3F);
				}else
				if((*src & 0xF0)==0xE0)
				{
					u = *src;
					*dst = (u << 12)&0xFFFF;
					src++;
					if((*src & 0xC0)!=0x80)
						return -1;
					u = *src;
					*dst |= (u & 0x3F) << 6;
					src++;
					if((*src & 0xC0)!=0x80)
						return -1;
					*dst |= (*src & 0x3F);
				}
				else
					return -1;
			src++;
			dst++;
		};
		*dst = 0;
		return (int)(dst-unicode)+1;
	};
	int cnt = 0;
	while(*src)
	{
		if(!(*src & 0x80))
			;
		else
			if((*src & 0xE0)==0xC0)
			{
				src++;
				if((*src & 0xC0)!=0x80)
					return -1;
			}else
			if((*src & 0xF0)==0xE0)
			{
				src++;
				if((*src & 0xC0)!=0x80)
					return -1;
				src++;
				if((*src & 0xC0)!=0x80)
					return -1;
			}
			else
				return -1;
		src++;
		cnt++;
	};
	return cnt+1;
}

/** *********************************************************************
*
* <b>FUNCTION:     TestWCMP</b>
*
* Функция получения информации о том, имеются ли символы входящие в 
* строку в таблице сравнения. Это нужно для того, чтобы при создании 
* словаря не пропустить символов и осознанно выдать им вес.
*
* @param[in]	unicode - pointer to unicode string
*
***********************************************************************/
void CStringCompare::TestWCMP(const unsigned short *unicode, int line)
{
UInt16 char_mass;
// Вычисляем положение частей таблицы сравнения.
CMPHeaderType *header = (CMPHeaderType *)m_UnicodeTable;
CMPSimpleType *simple = (CMPSimpleType *)(header+1);
//CMPComplexType *complex = (CMPComplexType *)(simple+header->SimpleCount);


	while (*unicode)
	{
	int i;
		char_mass = GetMass(*unicode, simple, header->SimpleCount, CMP_MASK_OF_INDEX);
		if (char_mass == CMP_MASK_OF_INDEX)
		{
			for (i=0;i<m_NotFoundIdx;i++)
			{
				if (m_NotFound[i].ch == *unicode)
					break;
			}

			if (i < m_NotFoundIdx)
			{
				unicode++;
				continue;
			}

			if (m_NotFoundIdx >= m_NotFoundMax)
			{
			WrongCMPType *tmpBuf;
				m_NotFoundMax += 32;
				tmpBuf = new WrongCMPType[m_NotFoundMax];
				if (m_NotFound)
				{
					memmove(tmpBuf, m_NotFound, sizeof(WrongCMPType)*m_NotFoundIdx);
					delete[] m_NotFound;
				}
				m_NotFound = tmpBuf;
			}

			m_NotFound[m_NotFoundIdx].ch = *unicode;
			m_NotFound[m_NotFoundIdx].Line = line;
			m_NotFoundIdx++;
		}
		unicode++;
	}
}
void CStringCompare::TestWCMPReset()
{
	if (m_NotFound)
	{
		delete[] m_NotFound;
		m_NotFound = NULL;
		m_NotFoundIdx = 0;
		m_NotFoundMax = 0;
	}
}

int  CStringCompare::TestWCMPGetCount()
{
	return m_NotFoundIdx;
}

void CStringCompare::TestWCMPGetWrong(int i, unsigned short *ch, int *line)
{
	if (i <0 || i>m_NotFoundIdx)
	{
		*ch = 0;
		*line = -1;
		return;
	}

	*ch = m_NotFound[i].ch;
	*line = m_NotFound[i].Line;
}

bool CStringCompare::IsCompare2() const
{
	return m_SecondUnicodeTable!=NULL;
}

int CStringCompare::StrWCmp(const wchar_t *str1, const wchar_t *str2)
{
	const std::u16string string1 = sld::as_utf16(str1);
	const std::u16string string2 = sld::as_utf16(str2);
	return StrWCmp16((const UInt16*)string1.c_str(), (const UInt16*)string2.c_str());
}

int CStringCompare::StrWCmp16(const UInt16 *str1, const UInt16 *str2)
{
	while (*str1 && *str2 && *str1 == *str2)
	{
		str1++;
		str2++;
	}

	if (*str1 > *str2)
		return 1;
	else
	if (*str1 < *str2)
		return -1;
	else
		return 0;
}


	/** *********************************************************************
* Определяет длину строки
*
* @param[in]	aStr	- указатель на строку
*
* @return длина строки или 0, если передан нулевой указатель на строку
************************************************************************/
Int32 CStringCompare::StrLen(const UInt16 *aStr)
{
	if (!aStr)
		return 0;

	Int32 count = 0;
	while (*aStr)
	{
		aStr++;
		count++;
	}

	return count;
}