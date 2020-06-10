#include "SldBitInput.h"

/** ********************************************************************
* Инициализация 
*
* @param[in]	aData		- ссылка на объект блочного ввода данных
* @param[in]	aDataType	- тип данных которые планируется читать в данном классе
* @param[in]	aBlockSize	- размер нарезки ресурсов(нужно для вычисления нужного 
*							  ресурса по позиции)
*
* @return код ошибки
************************************************************************/
ESldError CSldBitInput::Init(CSDCReadMy &aData, UInt32 aDataType, UInt32 aBlockSize)
{
	m_data = &aData;
	m_DataType = aDataType;
	m_BlockSize = aBlockSize;
	m_CurrentDataIndex = MAX_UINT_VALUE;

	if (!m_CurrentData.resize(sld2::default_init, sld2::div_round_up(m_BlockSize, sizeof(UInt32))))
		return eMemoryNotEnoughMemory;

	return eOK;
}

/** ********************************************************************
* Получаем 1 бит из входного потока
*
* @param[out]	aBit	- указатель на переменную куда будет помещен этот самый 1 бит
*
* @return код ошибки
************************************************************************/
ESldError CSldBitInput::GetBit(UInt32 *aBit)
{
	// Если данные закончились - читаем новые.
	if (!m_Bit)
	{
		// Если данные закончились - зачитываем новый блок данных
		if (m_Shift >= MAX_COMPRESSED_DATA_RESOURCE_SIZE)
		{
			ESldError error;
			error = m_data->GetResourceData(m_CurrentData.data(), m_DataType, m_CurrentDataIndex+1, m_BlockSize);
			if (error != eOK)
				return error;
			m_Shift = 0;
		}

		// Заполняем хранилище данных.
		m_Bit = OUT_BIT_COUNT;
		m_BitBuffer = m_CurrentData[m_Shift++];
	}

	*aBit = m_BitBuffer & 1;
	m_Bit--;
	m_BitBuffer>>=1;
	return eOK;
}

/** ********************************************************************
* Получаем из входного потока слово нужной разрядности(1-32)
*
* @param[out]	aDataBuffer	- 
* @param[in]	aDataSize	- 
*
* @return код ошибки
************************************************************************/
ESldError CSldBitInput::GetData(UInt32 *aDataBuffer, UInt32 aDataSize)
{
	if (m_Bit < aDataSize)
	{
		if (m_Shift >= (MAX_COMPRESSED_DATA_RESOURCE_SIZE/(OUT_BIT_COUNT/8)))
		{
			ESldError error;
			error = m_data->GetResourceData(m_CurrentData.data(), m_DataType, m_CurrentDataIndex+1, m_BlockSize);
			if (error != eOK)
				return error;
			m_Shift = 0;
			m_ShiftBit = 0;
			m_CurrentDataIndex++;
		}

		if (m_ShiftBit > m_Bit)
		{
			m_BitBuffer |= m_CurrentData[m_Shift]>>(m_ShiftBit-m_Bit);
			UInt32 tmpBit = m_Bit;
			m_Bit += (OUT_BIT_COUNT - m_ShiftBit);
			m_ShiftBit+=(OUT_BIT_COUNT-tmpBit);
		}else
		{
			m_BitBuffer |= m_CurrentData[m_Shift]<<(m_Bit-m_ShiftBit);
			m_ShiftBit+=(OUT_BIT_COUNT-m_Bit);
			m_Bit = OUT_BIT_COUNT;
		}

		if (m_ShiftBit >= OUT_BIT_COUNT)
		{
			m_Shift++;
			m_ShiftBit = 0;
		}
		if (m_Bit < aDataSize)
			return GetData(aDataBuffer, aDataSize);
	}

	*aDataBuffer = m_BitBuffer&((1<<aDataSize)-1);
	m_BitBuffer>>=aDataSize;
	m_Bit-=aDataSize;
	return eOK;
}

/** ********************************************************************
* Перемещаем указатель текущей позиции чтения в указанную позицию(при 
* необходимости считываем данные).
*
* @param[in]	aPosition	- новая позиция чтения в БИТАХ
*
* @return код ошибки
************************************************************************/
ESldError CSldBitInput::GoTo(UInt32 aPosition)
{
	ESldError error;
	
	// Индекс необходимого ресурса с данными
	UInt32 NewDataIndex = aPosition/(m_BlockSize*8);

	if (NewDataIndex != m_CurrentDataIndex)
	{		
		m_CurrentDataIndex = NewDataIndex;
		
		error = m_data->GetResourceData(m_CurrentData.data(), m_DataType, m_CurrentDataIndex, m_BlockSize);
		if (error != eOK)
			return error;
	}

	m_Shift = (aPosition % (m_BlockSize*8))/OUT_BIT_COUNT;

	UInt32 bitShift = ((aPosition % (m_BlockSize*8))%OUT_BIT_COUNT);
	m_BitBuffer = m_CurrentData[m_Shift]>>(bitShift);
	m_Bit = OUT_BIT_COUNT - bitShift;
	m_ShiftBit = OUT_BIT_COUNT;
	if (m_ShiftBit >= OUT_BIT_COUNT)
	{
		m_Shift++;
		m_ShiftBit = 0;
	}

	return eOK;
}

/** ********************************************************************
* Возвращает текущее положение в битах.
*
* @return положение в битах
************************************************************************/
UInt32 CSldBitInput::GetCurrentPosition()
{
	UInt32 pos = 0;
	pos = m_Shift*OUT_BIT_COUNT + m_ShiftBit;
	pos -= m_Bit;
	pos += (m_CurrentDataIndex * m_BlockSize * 8);
	return pos;
}