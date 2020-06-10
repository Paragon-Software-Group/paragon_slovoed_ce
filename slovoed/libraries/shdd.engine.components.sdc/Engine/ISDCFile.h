#pragma once
#ifndef _I_SDC_FILE_H_
#define _I_SDC_FILE_H_

#include "SDC.h"

class ISDCFile
{
public:
	/// Деструктор
	virtual ~ISDCFile(void) {}

	/// Проверяет открыт ли файл. 1 - если открыт, 0 если нет
	virtual Int8 IsOpened() const = 0;

	/**
	 * Считывает блок данных из файла
	 *
	 * @param[in] aDestPtr - указатель куда будут записаны прочитаные данные
	 * @param[in] aSize    - размер блока данных (в байтах)
	 * @param[in] aOffset  - смещение (в байтах) относительно начала файла откуда
	 *                       производить чтение
	 *
	 * @return размер считанного блока данных (в байтах)
	 */
	virtual UInt32 Read(void *aDestPtr, UInt32 aSize, UInt32 aOffset) = 0;

	/// Возвращает размер файла в байтах
	virtual UInt32 GetSize() const = 0;
};

#endif // _I_SDC_FILE_H_
