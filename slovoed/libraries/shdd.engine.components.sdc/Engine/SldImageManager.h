#ifndef _SLD_IMAGE_MANAGER_H_
#define _SLD_IMAGE_MANAGER_H_

#include "SldTypes.h"


/// Один элемент, представляющий картинку
struct TImageElement
{
	/// Индекс картинки
	UInt32		ImageIndex;
	/// Указатель на загруженные данные
	UInt8*		Data;
	/// Размер загруженных данных
	UInt32		Size;

	TImageElement();
	TImageElement(UInt8* aData, UInt32 aSize);
	~TImageElement();

	TImageElement(const TImageElement& aImageElement);

	TImageElement& operator=(const TImageElement& aImageElement);
};

#endif
