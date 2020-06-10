///////////////////////////////////////////////////////////////////////////////////
///
/// @file LanguageSpecific.h
///
/// Copyright Paragon Software (SHDD), 2006
///
/// Файл содержит описания классов и функций для поддержки 
/// морфологических особенностей, специфических для какого-нибудь 
/// конкретного языка
///
/// \author Капустин Михаил
///
///////////////////////////////////////////////////////////////////////////////////

#ifndef LANGUAGE_SPECIFIC_V2_H
#define LANGUAGE_SPECIFIC_V2_H

#include "../SldPlatform.h"

/// Тип символов, с которыми мы работаем. 
/** В данном случае мы используем однобайтные символы. */
typedef char MCHAR;

/// Тип двух указателей на MCHAR (char) 
typedef MCHAR * ArrOf2MCHARS[2];

#define FIRST_ARRAY_SIZE	7
#define SECOND_ARRAY_SIZE	4
#define	MAX_ARRAY_ITEM_SIZE	32


class LanguageSpecificData_v2
{
	static const Int32 MAX_PRONOUNS_IN_GROUP = 10;
	static const Int32 MAX_PRONOUN_LENGTH = 50;

	ArrOf2MCHARS firstPronounsArray[FIRST_ARRAY_SIZE];
	ArrOf2MCHARS secondPronounsArray[SECOND_ARRAY_SIZE];
	ArrOf2MCHARS subtablesNames;
	
	char pronounsRevArrays[2][MAX_PRONOUNS_IN_GROUP][MAX_PRONOUN_LENGTH];

	const MCHAR * GetPronounForm(bool vectNumb, Int32 pronNumb,  bool reverse = true, bool baseForm = false) const;
	UInt16 FillPronounArrays();
public:
	LanguageSpecificData_v2();
	~LanguageSpecificData_v2();
	
	Int32 GetPronounsVectorSize(bool numb) const;
	const MCHAR * GetRevesedPronoun(bool vectNumb, Int32 pronNumb) const;
	const MCHAR * GetPronounBaseForm(bool vectNumb, Int32 pronNumb) const;
	Int32 GetSubtablesNumber() const;
	const MCHAR * GetSubtableName(Int32 numb) const;
	
	/// Проверяет, может ли являться слово отделяемой приставкой в немецком языке
	/// Word - проверяемое слово
	bool IsWordGermDetachablePrefix(const MCHAR* Word) const;
};


#endif // LANGUAGE_SPECIFIC_V2_H
