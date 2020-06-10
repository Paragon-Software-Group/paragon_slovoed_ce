#pragma once

#include <string>
#include <vector>

#include "string_ref.h"

using namespace std;

/***********************************************************************
* �������� � ������ aStr ��� ��������� aWhat �� ������ aFill
*
* @param aStr	- ������, � ������� ������������ ������
* @param aWhat	- ��� ������
* @param aFill	- �� ��� ������
*
* @return	���������� ������������� �����
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
