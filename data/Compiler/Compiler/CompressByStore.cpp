#include "CompressByStore.h"

/// ���������� ��������� �������� ��� ������������ ������
int CCompressByStore::BuildTrees()
{
	return ERROR_NO;
}

/// ��������� ��������� ������ ����� �������.
/** ********************************************************************
* @param aDataPtr - ��������� �� ����� � �������.
* @param aDataSize - ���������� ���� ������ � ������, ������� ���������� 
*					���������.
* @param aDataType - ��� ������ ������� ����������� (����� �����).
*
* @return ��� ������
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

/// ��������� ������ �������, ���� �� �������� ����� ������ ��� aTotalCount.
/***********************************************************************
* ��� �����, ����� ���������� �������� ��������������� ���������� ������,
* ���� �� ����������, ���� ��������� ����� � ������� �� �������� ������
*
* @param aTotalCount - ����� ���������� ��������, ������� ������ ����.
*
* @return ��� ������
************************************************************************/
int CCompressByStore::AddEmptyTrees(UInt32 aTotalCount)
{
	// ��� ������ ��� �������� - ������ �������
	return ERROR_NO;
}

/// ���������� ���������� �������� ������������ ��� ������
/** ********************************************************************
* @return ���������� ��������
************************************************************************/
UInt32 CCompressByStore::GetCompressedTreeCount() const
{
	return 0;
}

/// ���������� ������ ���������� ������.
MemoryRef CCompressByStore::GetCompressedTree(UInt32 aIndex)
{
	return nullptr;
}
