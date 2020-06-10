#include "Out.h"

#include <string.h>

#include "Log.h"

/// При каком остатке буфера происходит его увеличение.
#define OUT_SECURITY_SIZE	(16)

/// Разрядность хранения данных
#define OUT_BIT_COUNT		(32)
/// Маска которую нужно применять для того, чтобы оставить только нужное количество бит
#define OUT_BIT_MASK		((1LL<<OUT_BIT_COUNT)-1)

COut::COut(void)
{
	m_Byte = 0;
	m_Bit = 0;
}

/// Метод очищает все внутренние буфера
void COut::ClearContent()
{
	m_buffer.clear();
	m_Byte = 0;
	m_Bit = 0;
}

// 	Добавляет данные в выходной поток
/**

	@param[in] aData - указатель на данные
	@param[in] sizeBytes - количество байт данных которые необходимо добавить

	@return код ошибки
*/
int COut::AddBytes(const void *aData, UInt32 sizeBytes)
{
	int error;
	for (UInt32 i=0;i<sizeBytes;i++)
	{
		error = AddBits(((const UInt8 *)aData)[i], 8);
		if (error != ERROR_NO)
			return error;
	}
	return ERROR_NO;
}

// 	Добавляет данные в выходной поток
/**
	ВНИМАНИЕ!!! Ниже идет описание того, как хранятся данные!!!

	Допустим, пришли какие-то данные(для наглядности считаем их 32битными):
	->00000000000000xxxxxxxxxxxxxxxxxxb, на платформе Intel(на практически любой кроме 
	PowerPC/DragonBall), с разбивкой по байтам они будут выглядеть как:
	@code
	xxxx xxxx | xxxx xxxx | 0000 00xx | 0000 0000
	@endcode

	Для наглядности пронумеруем биты:
	@code
	00000000000000IHGFEDCBA987654321b
	@endcode

	С помощью N будем обозначать данные которые были сохранены ранее, с 
	помощью 0 - не значащие биты.

	Итак будем считать, что ранее у нас был 1 частично заполненный байт:
	@code
	00000NNN
	@endcode

	Теперь добавим к нему наши данные:

	@code
	5432 1NNN | 9876 DCBA | 000I HGFE | 0000 0000 | 000...
	Дальнейшая запись будет ^^^ с этого места.
	@endcode

	Такая организация данных кажется более логичной и должна требовать меньше 
	операций при обработке.

	@param[in] aData - число представляющее данные
	@param[in] sizeBits - количество бит данных которые необходимо добавить

	@return код ошибки
*/
int COut::AddBits(UInt64 aData, UInt32 sizeBits)
{
	// test for correct input data.
	if (sizeBits>=64)
	{
		sldILog("Error! COut::AddBits : Very big data size\n");
		return ERROR_WRONG_BINARY_DATA_SIZE;
	}

	// Размер данных не должен быть равным 0!
	if (sizeBits==0)
	{
		sldILog("Error! COut::AddBits : zero data size\n");
		return ERROR_WRONG_BINARY_DATA_SIZE;
	}

	// Если буфер заканчивается, тогда увеличиваем буфер.
	if (m_Byte + OUT_SECURITY_SIZE >= m_buffer.size())
	{
		m_buffer.resize(m_buffer.empty() ? 1024 : m_buffer.size() * 2, 0);
	}

	UInt32 i;
	UInt64 tmpData = m_buffer[m_Byte];
	tmpData |= aData<<m_Bit;
	for (i=0;i<(sizeBits+m_Bit)/OUT_BIT_COUNT;i++)
	{
		m_buffer[m_Byte] = (tmpData & OUT_BIT_MASK);
		tmpData >>= OUT_BIT_COUNT;
		m_Byte++;
	}

	m_buffer[m_Byte] = (tmpData&OUT_BIT_MASK);
	m_Bit = (sizeBits+m_Bit)%OUT_BIT_COUNT;
	return ERROR_NO;
}


// Возвращает количество данных переданных данному классу в битах.
UInt32 COut::GetCurrentPos() const
{
	return m_Byte*OUT_BIT_COUNT+m_Bit;
}

// Возвращает сохраненные данные.
MemoryRef COut::GetData() const
{
	const size_t size = m_Byte * (OUT_BIT_COUNT / 8) + (m_Bit ? (OUT_BIT_COUNT / 8) : 0);
	return MemoryRef(m_buffer.data(), size);
}
