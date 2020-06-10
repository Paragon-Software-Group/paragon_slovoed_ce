#include "SldImageManager.h"


TImageElement::TImageElement()
{
	Data = NULL;
	Size = 0;
	ImageIndex = 0;
}

TImageElement::TImageElement(UInt8* aData, UInt32 aSize)
{
	Size = aSize;
	ImageIndex = 0;
	Data = (UInt8*)sldMemNew(Size);
	sldMemMove(Data, aData, Size);
}

TImageElement::~TImageElement()
{
	if (Data)
	{
		sldMemFree(Data);
		Data = NULL;
	}
}

TImageElement::TImageElement(const TImageElement& aImageElement)
{
	Data = NULL;
	Size = 0;
	ImageIndex = 0;
	*this = aImageElement;
}

TImageElement& TImageElement::operator=(const TImageElement& aImageElement)
{
	Size = aImageElement.Size;
	ImageIndex = aImageElement.ImageIndex;
	if (Data)
	{
		sldMemFree(Data);
		Data = NULL;
	}
	if(Size)
	{
		Data = (UInt8*)sldMemNew(Size);
		sldMemMove(Data, aImageElement.Data, Size);
	}
	return *this;
}