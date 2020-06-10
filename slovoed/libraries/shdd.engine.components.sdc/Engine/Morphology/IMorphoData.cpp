#include "IMorphoData.h"

#include "../SldCompare.h"

Int8* IMorphoData::StrTok(Int8** szToken, const Int8* szDelimit) const
{
	if (!szToken || !*szToken || !**szToken)
	{
		return NULL;
	}

	//если первые символы в строке разделители, тогда их нужно промотать, сделаем это
	Int8* szPtr = *szToken;
	bool bFind;
	do {
		bFind = false;
		for (Int32 k = 0; szDelimit[k]; k++)
		{
			if (**szToken == szDelimit[k])
			{
				bFind = true;
				(*szToken)++;
				szPtr = *szToken;
				break;
			}
		}
	} while (*szToken && **szToken && bFind);

	if (!**szToken)
		return NULL;

	//найдем первое вхождение разделителя в строке
	while (**szToken)
	{
		for (Int32 i = 0; szDelimit[i]; i++)
		{
			if (**szToken == szDelimit[i])
			{
				**szToken = 0;
				(*szToken)++;
				return szPtr;
			}
		}
		(*szToken)++;
	}

	return szPtr;
}

Int8* IMorphoData::StrReverse(Int8* szString) const
{
	// does the reverse operation inplace
	sld2::StrReverse(szString, szString + sld2::StrLen(szString) - 1);
	return szString;
}
