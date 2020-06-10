#include "CompressByStore.h"

/// ѕроизводит постройку деревьев дл€ последующего сжати€
int CCompressByStore::BuildTrees()
{
	return ERROR_NO;
}

/// ƒобавл€ет текстовые данные перед сжатием.
/** ********************************************************************
* @param aDataPtr - указатель на буфер с данными.
* @param aDataSize - количество байт данных в буфере, которые необходимо 
*					сохранить.
* @param aDataType - тип данных которые сохран€ютс€ (номер стил€).
*
* @return код ошибки
************************************************************************/
int CCompressByStore::AddText(const UInt16 *aDataPtr, UInt32 aDataSize, UInt32 aDataType)
{
	if (m_Mode == eInitialization)
		return ERROR_NO;

	for (UInt32 i=0;i<aDataSize;i++)
	{
		int error = m_Out->AddBits(aDataPtr[i], sizeof(aDataPtr[0])*8);
		if (error != ERROR_NO)
			return error;
	}
	return ERROR_NO;
}

/// ƒобавл€ет пустые деревь€, если их реальное число меньше чем aTotalCount.
/***********************************************************************
* Ёто нужно, чтобы количество деревьев соответствовало количеству стилей,
* чего не происходит, если некоторые стили в словаре не содержат данных
*
* @param aTotalCount - общее количество деревьев, которое должно быть.
*
* @return код ошибки
************************************************************************/
int CCompressByStore::AddEmptyTrees(UInt32 aTotalCount)
{
	// “ут вообще нет деревьев - просто выходим
	return ERROR_NO;
}

/// ¬озвращает количество деревьев используемых при сжатии
/** ********************************************************************
* @return количество деревьев
************************************************************************/
UInt32 CCompressByStore::GetCompressedTreeCount() const
{
	return 0;
}

/// ¬озвращает данные указанного дерева.
MemoryRef CCompressByStore::GetCompressedTree(UInt32 aIndex)
{
	return nullptr;
}
