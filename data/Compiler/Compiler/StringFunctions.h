#pragma once

#include <string>
#include <vector>

#include "string_ref.h"

using namespace std;

/***********************************************************************
* Заменяет в строке aStr все подстроки aWhat на строки aFill
*
* @param aStr	- строка, в которой производится замена
* @param aWhat	- что меняем
* @param aFill	- на что меняем
*
* @return	количество произведенных замен
************************************************************************/
int str_all_string_replace(wstring& aStr, wstring aWhat, wstring aFill);

template <typename String>
inline void split(sld::wstring_ref aStr, const wchar_t aSep, std::vector<String> &aTokens)
{
	size_t end = 0;
	while ((end = aStr.find(aSep)) != sld::wstring_ref::npos) {
		aTokens.emplace_back(String(aStr.substr(0, end)));
		aStr.remove_prefix(end + 1);
	}
	if (aStr.size())
		aTokens.emplace_back(String(aStr));
}
