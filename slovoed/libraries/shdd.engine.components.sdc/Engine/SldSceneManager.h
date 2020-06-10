#ifndef _SLD_SCENE_MANAGER_H_
#define _SLD_SCENE_MANAGER_H_

#include "SldTypes.h"

/// Один элемент, представляющий данные
struct TElement
{
	/// Индекс
	UInt32	Index;
	/// Указатель на загруженные данные
	UInt8*	Data;
	/// Размер загруженных данных
	UInt32	Size;

	TElement()
	{
		Data = NULL;
		Size = 0;
		Index = 0;
	}
	~TElement()
	{
		if (Data)
		{
			sldMemFree(Data);
			Data = NULL;
		}
	}

	TElement(const TElement& aElement)
	{
		*this = aElement;
	}

	TElement& operator=(const TElement &aElement)
	{
		Size = aElement.Size;
		Index = aElement.Index;
		Data = (UInt8*)sldMemNew(Size);
		sldMemMove(Data, aElement.Data, Size);
		return *this;
	}
};

#endif // _SLD_SCENE_MANAGER_H_
