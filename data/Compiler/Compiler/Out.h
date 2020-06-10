#pragma once

#include <vector>

#include "sld_Types.h"
#include "sld2_Error.h"
#include "util.h"

/// Данный класс занимается сбором, хранением данных после сжатия.
class COut
{
public:
	/// Конструктор по умолчанию
	COut(void);

	/// Метод очищает все внутренние буфера
	void ClearContent();

	/// Добавляет данные в выходной поток
	int AddBytes(const void *aData, UInt32 sizeBytes);

	/// Добавляет данные в выходной поток
	int AddBits(UInt64 aData, UInt32 sizeBits);

	/// Возвращает количество данных переданных данному классу в битах.
	UInt32 GetCurrentPos() const;

	/// Возвращает сохраненные данные.
	MemoryRef GetData() const;

private:
	/// Буфер в который помещаются данные
	std::vector<UInt32> m_buffer;
	/// количество бит сохраненных в последнем байте
	UInt32 m_Bit;
	/// Номер текущего байта
	UInt32 m_Byte;
};
