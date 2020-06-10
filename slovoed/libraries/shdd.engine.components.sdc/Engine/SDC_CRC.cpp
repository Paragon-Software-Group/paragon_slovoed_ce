#include "SDC_CRC.h"

/** 
Вычисляет контрольную сумму блока данных.

В качестве основного полинома можно использовать следующие
числа: 0x04C11DB7 / 0xEDB88320 / 0x82608EDB
Чаще всего используется 0xEDB88320.

@param buf - указатель на буффер с данными для вычисления CRC32
@param size - размер данных для вычисления CRC32
@param start_value - начальное значение(результат предыдущего 
вызова данной функции). При первом вызове должно быть 0xFFFFFFFFUL.
@param inverse_flag - флаг, инвертировать биты в crc в конце вычисления или нет 
*/
UInt32 CRC32(const UInt8 *buf, UInt32 size, UInt32 start_value, bool inverse_flag)
{
	UInt32 crc_table[256];
	UInt32 crc;

	for (UInt32 i = 0; i < 256; i++)
	{
		crc = i;
		for (UInt32 j = 0; j < 8; j++)
			crc = crc & 1 ? (crc >> 1) ^ 0xEDB88320UL : crc >> 1;

		crc_table[i] = crc;
	}

	crc = start_value;

	while (size)
	{
		crc = crc_table[(crc ^ *buf++) & 0xFF] ^ (crc >> 8);
		size--;
	}
	
	if (inverse_flag)
		crc ^= 0xFFFFFFFFUL;

	return crc;
}
