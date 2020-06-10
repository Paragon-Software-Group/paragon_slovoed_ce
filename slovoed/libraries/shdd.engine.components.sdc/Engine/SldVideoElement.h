#ifndef _VIDEO_ELEMENT_H_
#define _VIDEO_ELEMENT_H_

#include "SldTypes.h"

/// Объект элемента, содержащий видео
class CSldVideoElement
{
public:
	/// Конструктор объекта по-умолчанию
	CSldVideoElement();
	/// Конструктор объекта, позволяющий создать объект с корректными данными
	CSldVideoElement(const UInt32 aiIndex, const UInt8 *const apData, const UInt32 aiSize);

	/// Метод, определяющий корректность хранимых данных
	UInt8 IsValid() const;
	/// Очистка объекта (данные не удаляются)
	void Clear() { *this = CSldVideoElement(); }

	/// Возращается идентификатор видео
	UInt32 GetIndex() const { return m_iIndex; }
	/// Возвращается указатель на загруженные данные
	UInt8 *GetData () const { return m_pData;  }
	/// Возвращается размер загруженных данных
	UInt32 GetSize () const { return m_iSize;  }

private:
	/// Идентификатор видеоконтента
	UInt32 m_iIndex;
	/// Указатель на загруженные данные
	UInt8 *m_pData;
	/// Размер загруженных данных
	UInt32 m_iSize;
};

#endif // _VIDEO_ELEMENT_H_
