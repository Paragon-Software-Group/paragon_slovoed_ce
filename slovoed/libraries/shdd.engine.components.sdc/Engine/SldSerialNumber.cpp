#include "SldSerialNumber.h"
#include "SldCompare.h"
#include "SldTypes.h"

/** *********************************************************************
*
* Обрабатывает серийный номер
*
* @return код ошибки
*
************************************************************************/
ESldError CSldSerialNumber::ParseSerialNumber(const UInt16 *aSerialNumber,
	UInt16 *aPlatformID, UInt32 *aDictID, UInt32 *aNumber, UInt32 *aHASH, UInt32 *aDate, UInt32 *aIsBundle)
{
	if (!aSerialNumber || !aPlatformID || !aDictID || !aNumber || !aHASH || !aDate || !aIsBundle)
		return eMemoryNullPointer;

	if (CSldCompare::StrLen(aSerialNumber) != 24)
		return eSNWrongSerialNumber;

	// P7208E-65237-47690-4302
	if ((aSerialNumber[6] != '-') || (aSerialNumber[12]!= '-') || (aSerialNumber[18]!= '-'))
		return eSNWrongSerialNumber;


UInt8	*p;
UInt16	i, k;
UInt16	wCRC, dwTmp, wIndexLow;
UInt8	SnBuffer[32*4], szNumbers[32];
UInt8	*SerialNumber;
UInt8	dig[8];
UIntUnion	ct[2];
UInt8		SN[32*4];
UInt16		Word1, Word2;
UInt32		DWord1, DWord2;
	

	// Общие проверки на корректность серийного номера
	//////////////////////////////////////////////////////////////////////////

	aPlatformID[0] = aSerialNumber[0];
	aPlatformID[1] = aSerialNumber[1];
	aPlatformID[2] = 0;

	CSldCompare::StrUTF16_2_UTF8(SN, aSerialNumber);
	if (CSldCompare::StrLenA(SN) != 24)
		return eSNWrongSerialNumber;

	// Сохраняем серийный номер для того, чтобы не портить
	// исходных данных.
	//////////////////////////////////////////////////////////////////////////
	CSldCompare::StrCopyA(SnBuffer, SN);
	SerialNumber = SnBuffer;
	// Собираем цифровые блоки в единую цепочку.
	CSldCompare::StrCopyA(szNumbers, SerialNumber+7);
	CSldCompare::StrCopyA(szNumbers+5, szNumbers+6);
	CSldCompare::StrCopyA(szNumbers+10, szNumbers+11);
	SerialNumber[7] = 0;

	p = SnBuffer;
	while (*p)
	{
		// При необходимости приводим к большим буквам.
		if (*p >= 'a' && *p <= 'z')
			*p = *p-('a'-'A');
		p++;
	}
	

	// получаем длинное целое из десятичного представления.
	Decimal2Long(szNumbers, (UInt16)CSldCompare::StrLenA(szNumbers), dig, sizeof(dig));

	// переворачиваем порядок байт в удобный для дальнейшей обработки
	// (такой какой был при генерации).
	for (i=0;i<4;i++)
	{
		ct[0].ui_8[i] = dig[i];
		ct[1].ui_8[i] = dig[i+4];
	}

	// Получаем CRC номера из бинарных данных и обнуляем поля ответственные за CRC.
	//////////////////////////////////////////////////////////////////////////
	DataDemix(ct[1].ui_32, &wIndexLow, &wCRC);
	ct[1].ui_32 = wIndexLow;

	// распутываем часть с номером и HASH.
	ct[0].ui_32 -= QuasiCRC32(ct[1].ui_32);

	//////////////////////////////////////////////////////////////////////////
	// Вычисляем контрольную сумму для текстовой части серийного номера
	//////////////////////////////////////////////////////////////////////////
	p=SerialNumber;
	dwTmp=0x985E;
	while (*p)
	{	
		dwTmp += CRCcount(*p);
		p++;
	}

	//////////////////////////////////////////////////////////////////////////
	// Вычисляем контрольные суммы для бинарной части серийного номера
	//////////////////////////////////////////////////////////////////////////
	for (k=0;k<4;k++)
	{	
		dwTmp += CRCcount(ct[0].ui_8[k]);
		dwTmp += CRCcount(ct[1].ui_8[k]);
	}

	dwTmp = dwTmp & SLD_SN_CRC_MASK;
	if (wCRC != dwTmp)
	{
		return eSNWrongSerialNumber;
	}

	//////////////////////////////////////////////////////////////////////////
	// Далее считается, что номер настоящий(т.е. его контрольная сумма верна).
	//////////////////////////////////////////////////////////////////////////

	/* DATA FIELD FORMAT (Before packing)
	|    4    |    3    |    2    |    1    |    0    |
	 0III IIII IIII IIIH HHHH HHHI IIII IIIH HHHH HHHF

	Bites field meaning:
	I - Number of SN
	H - HASH
	F - Bundle flag
	*/

	
	DataDemix(ct[0].ui_32, &Word1, &Word2);
	DWord1 = Word1;
	DWord2 = Word2;
	DataDemix(ct[1].ui_32, &Word1, &Word2);
	DWord1 |= Word1 << 16;
	DWord2 |= Word2 << 16;

	*aNumber = DWord2;
	DWord1 >>= SLD_SN_BUNDLE_FLAG_SIZE;
	*aDate = DWord1 & ((1<<SLD_SN_DATE_SIZE) -1);
	DWord1 >>= SLD_SN_DATE_SIZE;
	*aHASH = DWord1;

	sldMemMove(aDictID, SerialNumber+2, 4);

	return eOK;
}

/** *********************************************************************
*
* Преобразует строчку с числом в десятичном виде в массив байт которые 
* представляют длинное целое.
*
* @param[in]	str			- строка из которой нужно получить число
* @param[in]	str_len		- количество символов в строчке
* @param[out]	d_array		- буфер в который нужно поместить собранное число
							  (память выделена снаружи метода)
* @param[in]	d_array_len	- размер буфера
*
* @return
*
***********************************************************************/
void CSldSerialNumber::Decimal2Long(const UInt8 *str, const UInt16 str_len, UInt8 *d_array, UInt16 d_array_len)
{
	UInt16	dig;
	Int16	i,k;

	sldMemZero(d_array, sizeof(*d_array)*d_array_len);

	for (k=str_len-1;k>=0;k--)
	{
		UInt16 next, shift;

		// Прибавляем к собранному числу очередное десятичное число.
		next = str[k]-'0';

		// Умножение с переносом
		shift = 0;
		for (i=0;i<d_array_len;i++)
		{
			dig = d_array[i] * 10;
			dig += shift;
			dig += next;
			next = 0;
			shift = dig>>8;
			d_array[i] = dig & 0xff;
		}

	}

}

/** *********************************************************************

* Разделяет биты расположенные поочередно в двойном слове по двум словам.
*
* @param[in]	Mix		- число которое требуется разделить
* @param[out]	CRC		- указатель на переменную куда нужно поместить 
*						  первое число.
* @param[out]	Index	- указатель на переменную куда нужно поместить 
*						  второе число.
*
* @return
*
***********************************************************************/
void CSldSerialNumber::DataDemix(UInt32 Mix, UInt16 *CRC, UInt16 *Index)
{
	UInt16	i;
	UInt32	Mask;
	Mask = 1;
	*CRC = 0;
	*Index = 0;
	for (i=0;i<16;i++)
	{
		*CRC |= (Mix & Mask)>>i;
		Mask <<= 1;
		*Index |= (Mix & Mask)>>(i+1);
		Mask <<= 1;
	}
}

/** *********************************************************************
*
* Расчитывает "типа CRC" для одного двойного слова.
*
* @param[in]	n - число для которого нужно расчитать "типа CRC"
*
* @return "типа CRC"
*
***********************************************************************/
UInt32 CSldSerialNumber::QuasiCRC32(UInt32 n)
{
	Int16 i;
	for (i=10; i>0;i--)
	{
		if (n & 1)
		{
			n=(n>>1)^0xEDB85364;
		}
		else
		{
			n>>=1;
		}
	}
	return n;
}

/** *********************************************************************
*
* Расчитывает CRC для одного символа.
*
* @param[in]	ch - символ для которого нужно расчитать CRC.
*
* @return CRC
*
***********************************************************************/
UInt16 CSldSerialNumber::CRCcount(UInt16 ch)
{
	UInt8 i;
	for (i=0; i<8; i++)
	{
		if (ch & 1)
		{
			ch=(ch>>1)^0x3DB5;
		}
		else
		{
			ch>>=1;
		}
	}

	return ch;
}