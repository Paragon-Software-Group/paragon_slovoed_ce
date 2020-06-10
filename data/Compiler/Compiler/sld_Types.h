#pragma once

#if _MSC_VER && !__INTEL_COMPILER
  #pragma warning(disable : 4503)
#endif

/// @file sld_Types.h

#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <string.h>
#include <wchar.h>

#include <climits>
#include <string>

#ifdef _MSC_VER
#  include <winerror.h>
#elif defined(_LINUX)
#  include <unistd.h>
#endif

#include "Engine/SldPlatform.h"
#include "Engine/SldDefines.h"
#include "Engine/SldTypeDefs.h"

#ifndef ERROR_NOT_ENOUGH_MEMORY
#  define ERROR_NOT_ENOUGH_MEMORY (0x03)
#endif

#ifdef _LINUX
int _wcsicmp(const wchar_t* str1, const wchar_t* str2);
int _wcsnicmp(const wchar_t* str1, const wchar_t* str2);
wchar_t* _wcslwr(wchar_t* aStr);
#endif

/// ��� ������� � ���������� ������ ��������� ������ ��� ������ ����
/**
��� ������� ������ ���� ����� ATST
��� ������� ������ ���� ����� BTST
*/
#define RESOURCE_TYPE_SEARCH_TREE_POINT	'TSTA'

/// ��� ������� ��� �������� ���������� ��������� ������ �� ������� ��������� ����� �� ������ ����
/** 
*	��� ������� ������ ���� ����� AIND
*	��� ������� ������ ���� ����� BIND
*/
#define RESOURCE_TYPE_INDEXES_COUNT		'CNIA'

/// ��� ������� ��� �������� ������� ������ �� ������� ��������� ����� �� ������ ����
/** 
*	��� ������� ������ ���� ����� AIND
*	��� ������� ������ ���� ����� BIND
*/
#define RESOURCE_TYPE_INDEXES_DATA		'DNIA'

/// ��� ������� ��� �������� ��������� ��������
/** 
*	��� ������� ������ ���� ����� AIQA
*	��� ������� ������ ���� ����� BIQA
*/
#define RESOURCE_TYPE_INDEXES_HEADER	'HNIA'

/// ��� ������� ��� �������� ������� �������� ������� ��� �������� (RESOURCE_TYPE_INDEXES_COUNT � RESOURCE_TYPE_INDEXES_DATA)
/** 
*	��� ������� ������ ���� ����� AIQA
*	��� ������� ������ ���� ����� BIQA
*/
#define RESOURCE_TYPE_INDEXES_QA		'AQIA'

/// ��� �������, ��������� ���������� �� �������������� ����������
/**
*	��� ������� ������ ���� ����� AAHI
*	��� ������� ������ ���� ����� BAHI
*/
#define RESOURCE_TYPE_ALTERNATIVE_HEADWORD_INFO	'IHAA'

/// ��� ������� ��� ������ ������ ������
#define RESOURCE_TYPE_ARTICLE_DATA		'DTRA'
/// ��� ������� ��� �������� ����� ������
#define RESOURCE_TYPE_ARTICLE_STYLE		'STRA'
/// ��� ������� ��� ������ ������ ������
#define RESOURCE_TYPE_ARTICLE_TREE		'TTRA'
/// ��� ������� ��� ������� �������� ������� ������
#define RESOURCE_TYPE_ARTICLE_QA		'QTRA'


/// ������� ������ ������
enum ESldCompressionType
{
	/// �� ������ ������, �������� �������� � ���� ������, �������� ������������� �� �����.
	eCompressionTypeNoCompression = 0,

};

/// �������� ������, �� ����������� ����� ������/�������� ������
enum ECompressionMode
{
	/// ������� ������
	eCompressionFast = 0,
	/// ������� ������
	eCompressionMedium,
	/// ������� ������
	eCompressionGood,
	/// ����������� ������
	eCompressionOptimal,
	/// ??? ������
	eCompression4,
	/// ������ ������
	eCompressionFull,
	/// ����� ������ ������. �� ���������
	eCompressionExtraFull
};

struct CompressionConfig
{
	ESldCompressionType type;
	ECompressionMode mode;

	CompressionConfig() : type(eCompressionTypeNoCompression), mode(eCompressionFast) {}
	CompressionConfig(ESldCompressionType aType, ECompressionMode aMode) : type(aType), mode(aMode) {}
};


/// ����� ������ ��� (��)����������� ������ ������.
#define SLD_TREE_BLOCKTYPE		(0)

/// ����� ������ ��� (��)����������� ���������� �������� ������� ����� � �������� � ����������� ����.
#define SLD_TREE_LIST_SHIFT	(0)
/// ����� ������ ��� (��)����������� ��������� ������ - ������� �����
#define SLD_TREE_TEXT	(1)


/// ���� � �������� ������������ �������
#define SLD_LANGUAGE_FROM	(0)

/// ���� �� ������� ������������ �������
#define SLD_LANGUAGE_TO		(1)


/// ������ ��������
#define SLD_INDEXES_VERSION		(1)

/// ��� ����� ��� ����� ��� ���� ������ ������� ��������-������������
#define SLD_SYMBOLS_DELIMITERS_LANGUAGE_CODE	(0)

/// ���� ����������� �� ������.
#define CHAR_CHAIN_NO_PATH_FOUND			(0xffffffff)

/// ���� ����������� �� ������.
#define CHAR_CHAIN_MAXIMUM_SIZE				(0xffffffff)

/// ���������� �������� ��� ������� ���������� ������� �������.
#define CHAR_CHAIN_FORCE_BIT_BORDER_COUNT	(32)

/// ������������ ������ �������� ��� ����� � �������� (UInt16)
#define SLD_MAX_STYLE_PREFIX_SIZE			(16)
/// ������������ ������ ��������� ��� ����� � �������� (UInt16)
#define SLD_MAX_STYLE_POSTFIX_SIZE			(16)

/// ������������ ������ ������
#define SLD_MAX_HEADER_SIZE					(64*1024-16)

#define SLD_TABLE_SPLIT_VERSION				(4)

/// ���������� ���� ����� ������� �������� ������� �� ���������
#define SLD_DEFAULT_QA_INTERLEAVE			(128)

/// ���������� ��� ������� ������������ � ������� �� ���������
#define SLD_DEFAULT_QA_INTERLEAVE_SIZE		(1024)

/// ����������� ���������� ����� ������� �������� �������
#define SLD_DEFAULT_MIN_QA_INTERLEAVE_SIZE	(16)

/// ����������� ���������� ����, ����������� ��� ���������� ������ �������� ������
#define SLD_MIN_WORDS_COUNT_FOR_QUICK_SEARCH	(512)


/// ���������, ����������� �������� ������ �������� ��������� ������ ����
typedef struct TListVariantProperty
{
	/// ����� �������� ���������
	UInt32 Number;
	/// ��� �������� ��������� (��. #EListVariantTypeEnum)
	UInt32 Type;
	/// ��� �����
	UInt32 LangCode;
	/// ���������������
	UInt32 Reserved[5];

}TListVariantProperty;

#define SLD_SIZE_VALUE_SCALE (100)

/// ���������� ���������� �� �������
typedef struct TSizeValue
{
	/// �����
	/// �����! ����� �������� � ���������� �� SLD_SIZE_VALUE_SCALE (100) ����.
	/// �.�. 1 �������� ��� 100, 1.01 - 101, .01 - 1 � �.�.
	Int32 Value;
	/// ������� ���������
	UInt32 Units;

	/// ������� TSizeValue � ���������� ���������� (�����������)
	TSizeValue() : Value(-1), Units(eMetadataUnitType_UNKNOWN) {}

	/// ������� TSizeValue c ����������� ����������
	/// �����! aValue ������ ���� "�������������" - �.�. 320 ������ 3.2, 100 ������ 1 � �.�.
	TSizeValue(Int32 aValue, EMetadataUnitType aUnits) : Value(aValue), Units(aUnits) {}

	/// ����������� �������� ��������
	void Set(Int32 aValue, EMetadataUnitType aUnits) { Value = aValue; Units = aUnits; }

	/// ���������� �������� �� ������ "��������"
	bool IsValid() const { return Units < eMetadataUnitType_end; }

	/// ��������� ������ � ���� "��������������" Float32
	Float32 AsFloat32() const { return (Float32)Value / (Float32)SLD_SIZE_VALUE_SCALE; }

	/// ���������� ����� ����� �������
	Int32 Integer() const { return Value / SLD_SIZE_VALUE_SCALE; }

	/// ���������� ������� ����� ������� - �� 0 �� 100
	UInt32 Fractional() const { return (Value >= 0 ? Value : -Value) % SLD_SIZE_VALUE_SCALE; }

} TSizeValue;

inline bool operator==(const TSizeValue &lhs, const TSizeValue &rhs)
{
	return lhs.Units == rhs.Units && lhs.Value == rhs.Value;
}
inline bool operator!=(const TSizeValue &lhs, const TSizeValue &rhs)
{
	return !(lhs == rhs);
}

/// ������� ���� ��� � ����� ������ ������� � ��������� ���������
#define SIZE_VALUE_UNITS	(0xFFFFFFFFUL)

/// ��������� �������� �����
typedef struct TStyleVariantHeader
{
	/// ������ ��������� � ������
	UInt32 structSize;
	
	/// ��� �������� (��. #EStyleVariantType)
	UInt32 VariantType;
	
	/// ����, ���� ����������, ������ ����� �������
	UInt32 Visible;
	
	/// ����������, ��� ������ �������������� ����� (��. #TTextType)
	/// ���� ��� - ����������, �� � ������ ���������� ������ 
	/// ���������� ������ ��� ��������� - �������� ������ ��� ��� ���-��
	UInt32 TextType;
	
	/// ������������ ������ �� ������ (��. #TFontLevel)
	UInt32 Level;
	
	/// �������� ������� ���������� ����� ������
	UInt32 ColorRed;
	/// �������� ������� ���������� ����� ������
	UInt32 ColorGreen;
	/// �������� ����� ���������� ����� ������
	UInt32 ColorBlue;
	/// �������� �����-������
	UInt32 ColorAlpha;

	/// �������� ������� ���������� ����� ����
	UInt32 BackgroundColorRed;
	/// �������� ������� ���������� ����� ����
	UInt32 BackgroundColorGreen;
	/// �������� ����� ���������� ����� ����
	UInt32 BackgroundColorBlue;
	/// �������� �����-������
	UInt32 BackgroundColorAlpha;
	
	/// ����� ������������
	UInt32 Bold;

	/// ����, ���� ����������, ������ ����� ������� ���������
	UInt32 Italic;

	/// ����� �������������
	UInt32 Underline;
	
	/// ����, ���� ����������, ������ ����� �����������
	UInt32 Strikethrough;

	/// ������ ������ (��. #TFontSize)
	/// ����� ������ 5 - ���������� ������ ������
	/// ���� == SIZE_VALUE_UNITS => ������ TextSizeValue
	UInt32 TextSize;

	/// ������ ������ (��. #TFontSize)
	/// ����� ������ 5 - ���������� ��������
	/// ���� == SIZE_VALUE_UNITS => ������ LineHeightValue
	UInt32 LineHeight;

	/// ��������� ������ (��. #ESldStyleFontFamilyEnum)
	UInt32 FontFamily;

	/// �������� ������ (��. #ESldStyleFontNameEnum)
	UInt32 FontName;
	
	/// �������
	UInt16 Prefix[SLD_MAX_STYLE_PREFIX_SIZE + 1];
	
	/// ��������
	UInt16 Postfix[SLD_MAX_STYLE_POSTFIX_SIZE + 1];

	/// ����, ���� ����������, ������ ����� ����������
	UInt32 Overline;

	/// ������ ������� ������� ��������
	Int32 BackgroundTopImageIndex;

	/// ������ ������ ������� ��������
	Int32 BackgroundBottomImageIndex;

	/// ���� ���������� ������ ���� - ����� ����������� ������ ������ ����� ���������� ��� ���������
	UInt32 Unclickable;

	/// ���� ����, ��� ��� ������������� ����� ������������ ����, �������� ������������ ����
	UInt32 UnderlineUseCustomColor;

	/// ������� ������������ ����� ��� ������������� 
	UInt32 UnderlineColorRed;

	/// ������� ������������ ����� ��� ������������� 
	UInt32 UnderlineColorGreen;

	/// ����� ������������ ����� ��� ������������� 
	UInt32 UnderlineColorBlue;

	/// ����� ������������ ����� ��� ������������� 
	UInt32 UnderlineColorAlpha;

	/// ������ ������ � ���� ��������� ��������� ���������
	TSizeValue TextSizeValue;

	/// ������ ������ ������ � ���� ��������� ��������� ���������
	TSizeValue LineHeightValue;

	/// ���������������
	UInt32 Reserved[14];

} TStyleVariantHeader;


/// �����, ������� �������� �� ������� ������� � ������� ��������� ������� ��� ������������ �������� �������� � ������� ���������
#define SLD_CMP_TABLE_FEATURE_SYMBOL_PAIR_TABLE_MASK	(0x1)

typedef struct WrongCMPType
{
	unsigned short	ch; 
	unsigned short	reserved1;
	int				Line;
}WrongCMPType;

/// ������� ���������, ����������� �� �������� �����
typedef struct TCompareTableSplit
{
	/// ��������� �� ��������� ������� ���������
	CMPHeaderType				*Header;

	/// ��������� �� ������ ����� ������� ��������(���� ����� �� ���� ������)
	CMPSimpleType				*Simple;

	/// ��������� �� ������ ������� ��������
	CMPComplexType				*Complex;

	/// ��������� �� ������ �������� ������������
	CMPDelimiterType			*Delimiter;

	/// ��������� �� ������ ������ �������� �����
	CMPNativeType				*Native;

	/// ��������� �� ������ ��������� ������������
	CMPHalfDelimiterType		*HalfDelimiter;

	/// ��������� �� ��������� ������� ������������ �������� � ������� � ������ ���������
	CMPSymbolPairTableHeader	*HeaderPairSymbols;

	/// ��������� �� ������ ��� ������������ �������� � ������� � ������ ��������� ��� ������� �����
	CMPSymbolPair				*NativePair;

	/// ��������� �� ������ ��� ������������ �������� � ������� � ������ ��������� ��� ���� ��������, ������� �� �����
	CMPSymbolPair				*CommonPair;

	/// ��������� �� ������ ��� ������������ �������� � ������� � ������ ��������� ��� ��������, ������������� � ������� ���� ������� �������
	CMPSymbolPair				*DictionaryPair;

	/// ������� ���� ��������
	UInt16						*SimpleMassTable;

	/// ������� ������� �������� ����� ������������ ��������
	UInt8						*MostUsageCharTable;

	/// ������ ���� �������
	UInt32						TableSize;

}TCompareTableSplit;

/// ������������, �����������, ����� ������� ���������� ������ � ������ ���� ��������������� ������
enum EFullTextSearchLinkType
{
	/// ������ ��������� �� id ������
	eLinkType_ArticleId = 0,
	/// ������ ��������� �� id ������� � ������ ����
	eLinkType_ListEntryId,
	/// ����������� ���
	eLinkType_Unknown = 0xFFFF
};

/// � ����� � ��������� ������ ���� ������ �����������
#define		SLD_SIMPLE_SORTED_NORMAL_WORD		(1)
/// � ����� � ��������� ������ ��� ������� �����������
#define		SLD_SIMPLE_SORTED_VIRTUAL_WORD		(-1)

/// ���������� � �������
struct TResourceInfo
{
	/// ������ ����� (������ ���� � �����), ������������ � �������.
	UInt32 NameIndex;
};

#define ATOMIC_ID_PREFIX		(L"AtomicArticle_")
