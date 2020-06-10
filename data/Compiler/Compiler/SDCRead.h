#ifndef _SDC_READ_H_
#define _SDC_READ_H_

#ifndef _CRT_SECURE_NO_WARNINGS
	#define _CRT_SECURE_NO_WARNINGS
	#define _CRT_NON_CONFORMING_SWPRINTFS
#endif

#include "Engine/SDC.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>


/// ��������� ������ ������� ����������� �������� (� ���������)
#define RESOURCE_TABLE_START_SIZE	(128)

/// ������ ������ ����� ������ � ������ ��� ���������� CRC ��������������� ������ ��������
#define CRC_DATA_BLOCK_SIZE			(0xFFFF)


/// ����� �������������� ��� ������ ������ �� ����������.
class CSDCRead
{
public:

	/// �����������
	CSDCRead(void);
	/// ����������
	~CSDCRead(void);

	/// ��������� ���������.
	SDCError Open(const Int8 *fileName);

	/// ��������� ���������.
	void Close(void);
	
	/// ���������� ��� ����������� � ����������
	UInt32 GetDatabaseType(void);

	/// ���������� ���������� �������� � �������� ����������.
	UInt32 GetNumberOfResources(void);

	/// �������� ������ �� ��� ������
	SDCError GetResource(UInt32 index, const void **ptr, UInt32 *size, UInt32 *res_type, UInt32 *res_index);

	/// �������� ������ �� ��� ���� � ������ ����� ������ �������� ������� ����.
	SDCError GetResourceByTypeIndex(UInt32 type, UInt32 index, const void **ptr, UInt32 *size);

	/// �������� ��� � ����� ������� �� ��� ������.
	SDCError GetTypeIndex(UInt32 index, UInt32 *res_type, UInt32 *res_index);

	/// ��������� ����������� ����������.
	SDCError CheckData(void);
	
	/// ��������� ������
	SDCError CloseResource(const void* aPtr);

private:

	/// �������� ���� ����������
	FILE* m_in;

	/// ��������� ����������
	SlovoEdContainerHeader m_Header;

	/// ������� ������������ ��������
	SlovoEdContainerResourcePosition* m_resTable;

	/// ������� ��������, ��� ����������� � ������
	ResourceMemType* m_loadedTable;
	
	/// ������ � ��������� ������� ����������� ��������
	UInt32 m_MaxLoadedTableElementsCount;
	
	/// ���������� ��������� � ������� ����������� ��������
	UInt32 m_LoadedTableElementsCount;
	
};

#endif // _SDC_READ_H_
