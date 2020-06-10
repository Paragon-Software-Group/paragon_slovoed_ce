#ifndef _SLD_BIT_INPUT_H_
#define _SLD_BIT_INPUT_H_

#include "SldPlatform.h"
#include "SldError.h"
#include "SldSDCReadMy.h"

/// Количество бит на единицу хранения данных.
#define OUT_BIT_COUNT	(32)

/// Класс предназначенный для бинарного чтения данных
/** ********************************************************************
*	Класс предназначенный для бинарного чтения данных
*	В-частности, данный класс умеет получать данные как по 1 биту, так и 
*	любым количеством до 32 бит включительно. Кроме того данный класс 
*	отвечает за хранение позиции чтения, ее перемещения при операциях 
*	чтения или по требованию извне.
************************************************************************/
class CSldBitInput
{
public:
	CSldBitInput(void) :
		m_data(NULL),
		m_DataType(0),
		m_BlockSize(0),
		m_CurrentDataIndex(MAX_UINT_VALUE),
		m_Bit(0),
		m_BitBuffer(0),
		m_Shift(0),
		m_ShiftBit(0)
		{}

	/// Инициализация
	ESldError Init(CSDCReadMy &aData, UInt32 aDataType, UInt32 aBlockSize);

	///	Перемещаем указатель текущей позиции чтения в указанную позицию(при 
	///	необходимости считываем данные).
	ESldError GoTo(UInt32 aPosition);
	
	/// Получаем 1 бит из входного потока
	ESldError GetBit(UInt32 *aBit);

	/// Получаем  из входного потока слово нужной разрядности(1-32)
	ESldError GetData(UInt32 *aDataBuffer, UInt32 aDataSize);

	/// Возвращает текущее положение в битах.
	UInt32 GetCurrentPosition();


private:
	/// Указатель на класс блочного чтения.
	CSDCReadMy		*m_data;

	/// Тип ресурсов которые умеет читать данный класс.
	UInt32			m_DataType;
	/// Размер одного ресурса в байтах.
	UInt32			m_BlockSize;

	/// Здесь хранится текущий считанный ресурс.
	sld2::DynArray<UInt32>	m_CurrentData;
	/// Номер текущего считанного ресурса.
	UInt32			m_CurrentDataIndex;

	/// Количество бит оставшихся в буфере(может быть от 8)
	UInt32			m_Bit;
	/// Битовый буфер
	UInt32			m_BitBuffer;
	/// Смещение текущего байта от начала текущего ресурса
	UInt32			m_Shift;
	/// Количество бит которые были использованы из текущего элемента массива считанных данных.
	UInt32			m_ShiftBit;

};

#endif
