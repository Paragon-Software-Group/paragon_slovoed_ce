#include "StringFunctions.h"

/***********************************************************************
* Заменяет в строке aStr все подстроки aWhat на строки aFill
*
* @param aStr	- строка, в которой производится замена
* @param aWhat	- что меняем
* @param aFill	- на что меняем
*
* @return	количество произведенных замен
************************************************************************/
int str_all_string_replace(wstring& aStr, wstring aWhat, wstring aFill)
{
	// Негде или нечего менять
	if (aStr.empty() || aWhat.empty())
		return 0;
		// Негде или нечего менять
	if (aStr.empty() || aWhat.empty())
		return 0;

	int count = 0;
	size_t wsize = aWhat.size();
	
	wstring::iterator pos = aStr.begin();
	wstring::iterator end = aStr.end();
	wstring::iterator sub;
	wstring new_str(L"");

	while (true)
	{
		sub = search(pos, end, aWhat.begin(), aWhat.end());

		while (pos != sub)
			new_str += *pos++;

		if (pos == end)
			break;

		new_str += aFill;
		pos += wsize;
		count++;
	}

	aStr.clear();
	aStr = new_str;

	return count;
}
