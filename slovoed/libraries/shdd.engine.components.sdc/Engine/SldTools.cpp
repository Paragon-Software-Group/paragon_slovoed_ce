#include "SldTools.h"

#include "SldSerialNumber.h"
#include "SldInputText.h"


/** *********************************************************************
* Инициализирует генератор случайных чисел.
*
* @param[out] aRandomSeed - Структура хранящая данные для генератора случайных числе.
* @param[in] aSeed - базовое число для вычисления случайного числа.
*
* @return    случайное число.
*
***********************************************************************/
void SldInitRandom(TRandomSeed *aRandomSeed, UInt32 aSeed)
{
	aRandomSeed->Seed = aSeed;
	aRandomSeed->y = 362436000;
	aRandomSeed->z = 521288629;
	aRandomSeed->c = 7654321;
}

/** *********************************************************************
* Генерирует случайное число основываясь на начальном числе. Начальное число будет изменено.
*
* @param[in/out] aRandomSeed - Структура хранящая данные для генератора случайных числе.
*
* @return    случайное число.
*
***********************************************************************/
UInt32 SldGetRandom(TRandomSeed *aRandomSeed)
{
	// This is the KISS Random Numbers Generator:
	// http://www.cs.ucl.ac.uk/staff/d.jones/GoodPracticeRNG.pdf
	/*
	static unsigned int x = 123456789,y = 362436000,z = 521288629,c = 7654321; // Seed variables 
	unsigned int KISS()
	{ unsigned long long t, a = 698769069ULL;
	x = 69069*x+12345; y ^= (y<<13); y ^= (y>>17); y ^= (y<<5); // y must never be set to zero! 
	t = a*z+c; c = (t>>32); // Also avoid setting z=c=0! 
	return x+y+(z=t); }
	*/
	TRandomSeed tmpRandomSeed;
	if (!aRandomSeed)
	{
		aRandomSeed = &tmpRandomSeed;
	}
	UInt32 t, a;
	a = 698769069;
	aRandomSeed->Seed = 69069*aRandomSeed->Seed+12345; 
	aRandomSeed->y ^= (aRandomSeed->y<<13); 
	aRandomSeed->y ^= (aRandomSeed->y>>17); 
	aRandomSeed->y ^= (aRandomSeed->y<<5); /* y must never be set to zero! */ 
	t = (((a>>16)*aRandomSeed->z) + (a*(aRandomSeed->z>>16)))>>16;
	aRandomSeed->z=a*aRandomSeed->z+aRandomSeed->c;
	aRandomSeed->c = t;

	return aRandomSeed->Seed+aRandomSeed->y+aRandomSeed->z; 
}

/** *********************************************************************
* Сохраняет регистрационные данные для указанного словаря.
*
* @param aDictID		- идентификатор словаря данные которого мы будем сохранять
* @param aRegData		- Регистрационные данные для сохранения.
* @param aRandomSeed	- данные генератора случайных чисел
* @param aLayerAccess	- Указатель на класс прослойки который будет заниматься сохранением данных.
*
* @return код ошибки
************************************************************************/
ESldError SldSaveRegistrationData(UInt32 aDictID, const TRegistrationData *aRegData, TRandomSeed *aRandomSeed, ISldLayerAccess *aLayerAccess)
{
	ESldError error;
	// вычисляем контрольную сумму для aDictID, которая будет являться 
	UInt32 crc = 0;
	UInt32 n = aDictID;
	UInt32 i=0;
	while (n)
	{
		crc ^= (n^i)&4;
		i++;
		n>>=4;
	}
	n = crc;

	UInt32 from[SLD_SN_ELEMENT_COUNT_VERSION_1], to[SLD_SN_ELEMENT_COUNT_VERSION_1];
	sldMemZero(from, sizeof(from));
	sldMemZero(to, sizeof(to));

	from[SLD_SN_DATA_VERSION] = SLD_SN_SAVE_VERSION_1;
	from[SLD_SN_DATA_V1_RANDOM] = SldGetRandom(aRandomSeed);
	from[SLD_SN_DATA_V1_HASH] = from[SLD_SN_DATA_V1_RANDOM]^aRegData->HASH;
	from[SLD_SN_DATA_V1_DATE] = from[SLD_SN_DATA_V1_RANDOM]^aRegData->Date;
	from[SLD_SN_DATA_V1_NUMBER] = from[SLD_SN_DATA_V1_RANDOM]^aRegData->Number;
	from[SLD_SN_DATA_V1_CLICKS] = from[SLD_SN_DATA_V1_RANDOM]^aRegData->Clicks;
	from[SLD_SN_DATA_V1_DICTID] = from[SLD_SN_DATA_V1_RANDOM]^aDictID;
	from[SLD_SN_DATA_V1_CRC] = 0;

	// Вычисляем контрольную сумму сохраняемых данных, чтобы потом можно было проверить корректность считаных данных.
	for (i=0;i<SLD_SN_ELEMENT_COUNT_VERSION_1-1;i++)
		from[SLD_SN_DATA_V1_CRC]+=CSldSerialNumber::QuasiCRC32(from[i]);

	// Перемешиваем биты сохраняемых данных.
	for (i=0;i<sizeof(from)*8;i++)
	{
		to[i/32] |= (from[n]&1)<<(i%32);
		from[n]>>=1;
		n++;
		if (n>=SLD_SN_ELEMENT_COUNT_VERSION_1)
			n = 0;
	}

	error = aLayerAccess->SaveSerialData(aDictID, (const UInt8 *)to, sizeof(to));
	if (error != eOK)
		return error;

	return eOK;
}

/** *********************************************************************
* Загружает регистрационные данные для текущего словаря.
*
* @param aDictID		- идентификатор словаря данные которого мы будем считывать
* @param aRegData		- Регистрационные данные для сохранения.
* @param aLayerAccess	- Указатель на класс прослойки который будет заниматься сохранением данных.
*
* @return код ошибки
************************************************************************/
ESldError SldLoadRegistrationData(UInt32 aDictID, TRegistrationData *aRegData, ISldLayerAccess *aLayerAccess)
{
	ESldError error;

	// вычисляем контрольную сумму для aDictID, которая будет являться 
	UInt32 crc = 0;
	UInt32 n = aDictID;
	UInt32 i=0;
	while (n)
	{
		crc ^= (n^i)&4;
		i++;
		n>>=4;
	}
	n = crc;

	UInt32 size = 0;
	error = aLayerAccess->LoadSerialData(aDictID, NULL, &size);
	if (error != eOK)
		return error;

	UIntUnion *from, *to;
	from = (UIntUnion *)sldMemNew(size);
	if (!from)
		return eMemoryNotEnoughMemory;
	to = (UIntUnion *)sldMemNewZero(size);
	if (!to)
	{
		sldMemFree(from);
		return eMemoryNotEnoughMemory;
	}

	error = aLayerAccess->LoadSerialData(aDictID, from->ui_8, &size);
	if (error != eOK)
	{
		sldMemFree(from);
		sldMemFree(to);
		return error;
	}

	// Восстанавливаем данные (перед сохранением они были перемешаны)
	for (i=0;i<size*8;i++)
	{
		to[n].ui_32 >>= 1;
		to[n].ui_32 |= (from[i/32].ui_32&1)<<(31);
		from[i/32].ui_32 >>= 1;

		n++;
		if (n>=SLD_SN_ELEMENT_COUNT_VERSION_1)
			n = 0;
	}

	// Проверяем контрольную сумму.
	crc = 0;
	// Вычисляем контрольную сумму сохраняемых данных, чтобы потом можно было проверить корректность считаных данных.
	for (i=0;i<SLD_SN_ELEMENT_COUNT_VERSION_1-1;i++)
		crc+=CSldSerialNumber::QuasiCRC32(to[i].ui_32);
	if (crc != to[SLD_SN_DATA_V1_CRC].ui_32)
	{
		sldMemFree(from);
		sldMemFree(to);
		return eSNWrongDataCRC;
	}

	if (aDictID != (to[SLD_SN_DATA_V1_DICTID].ui_32^to[SLD_SN_DATA_V1_RANDOM].ui_32))
	{
		sldMemFree(from);
		sldMemFree(to);
		return eSNWrongLoadedData;
	}

	if (aRegData)
	{
		aRegData->Number = to[SLD_SN_DATA_V1_NUMBER].ui_32^to[SLD_SN_DATA_V1_RANDOM].ui_32;
		aRegData->HASH = to[SLD_SN_DATA_V1_HASH].ui_32^to[SLD_SN_DATA_V1_RANDOM].ui_32;
		aRegData->Date = to[SLD_SN_DATA_V1_DATE].ui_32^to[SLD_SN_DATA_V1_RANDOM].ui_32;
		aRegData->Clicks = to[SLD_SN_DATA_V1_CLICKS].ui_32^to[SLD_SN_DATA_V1_RANDOM].ui_32;
	}

	sldMemFree(from);
	sldMemFree(to);

	return eOK;
}

/**
 * Создает объект чтения данных согласно методу сжатия(представления) данных
 *
 * @param[in] aCompressionMethod - метод сжатия
 *
 * @return готовый объект получения данных (или null в случае ошибки)
 */
sld2::UniquePtr<ISldInput> sld2::CreateInput(UInt32 aCompressionMethod)
{
	switch (aCompressionMethod)
	{
	case SLD_COMPRESSION_METHOD_TEXT:
		return make_unique<CSldInputText>();
	default: break;
	}
	return nullptr;
}
