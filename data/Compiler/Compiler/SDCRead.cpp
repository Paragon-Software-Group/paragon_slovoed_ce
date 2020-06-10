#include "SDCRead.h"

#include "SDC_CRC.h"
#include "sld_Types.h"

CSDCRead::CSDCRead(void)
{
	m_in = NULL;
	memset(&m_Header, 0, sizeof(m_Header));
	m_resTable = NULL;
	m_loadedTable = NULL;
	m_MaxLoadedTableElementsCount = 0;
	m_LoadedTableElementsCount = 0;
}

CSDCRead::~CSDCRead(void)
{
	Close();
}

/** 
	��������� ���� ����������.

	@param fileName - ���� � �����.

	@return ��� ������.
*/
SDCError CSDCRead::Open(const Int8 *fileName)
{
	Close();

	m_in = fopen((const char*)fileName, "rb");
	if (!m_in)
		return SDC_READ_CANT_OPEN_FILE;

	if (fread(&m_Header, 1, sizeof(m_Header), m_in) != sizeof(m_Header))
	{
		Close();
		return SDC_READ_CANT_READ;
	}

	if (m_Header.Signature != SDC_SIGNATURE)
	{
		Close();
		return SDC_READ_WRONG_SIGNATURE;
	}

	m_resTable = (SlovoEdContainerResourcePosition*)malloc(m_Header.ResourceRecordSize*m_Header.NumberOfResources);
	if (!m_resTable)
	{
		Close();
		return SDC_MEM_NOT_ENOUGH_MEMORY;
	}

	if (fread(m_resTable, 1, m_Header.ResourceRecordSize*m_Header.NumberOfResources, m_in) !=
		m_Header.ResourceRecordSize*m_Header.NumberOfResources)
	{
		Close();
		return SDC_READ_CANT_READ;
	}
	
	m_MaxLoadedTableElementsCount = RESOURCE_TABLE_START_SIZE;
	m_loadedTable = (ResourceMemType*)malloc(m_MaxLoadedTableElementsCount*sizeof(m_loadedTable[0]));
	if (!m_loadedTable)
	{
		Close();
		return SDC_MEM_NOT_ENOUGH_MEMORY;
	}
	memset(m_loadedTable, 0, m_MaxLoadedTableElementsCount*sizeof(m_loadedTable[0]));

	return SDC_OK;
}

/** 
	��������� ���� � ����������� ������.

*/
void CSDCRead::Close(void)
{
	if (m_in)
	{
		fclose(m_in);
		m_in = NULL;
	}

	if (m_resTable)
	{
		free(m_resTable);
		m_resTable = NULL;
	}
	
	if (m_loadedTable)
	{
		for (UInt32 i=0;i<m_LoadedTableElementsCount;i++)
		{
			if (m_loadedTable[i].ptr)
				free(m_loadedTable[i].ptr);
		}
		free(m_loadedTable);
		m_loadedTable = NULL;
	}
	
	memset(&m_Header, 0, sizeof(m_Header));
	m_MaxLoadedTableElementsCount = 0;
	m_LoadedTableElementsCount = 0;
}

/***********************************************************************
* ���������� ��� ����������� � ����������
*
* @return ��� ����������� � ����������
************************************************************************/
UInt32 CSDCRead::GetDatabaseType(void)
{
	return m_Header.DatabaseType;
}

/** 
	���������� ���������� �������� � ����������.

	@return ���������� �������� ��� 0(���� ��������� �� ������).
*/
UInt32 CSDCRead::GetNumberOfResources(void)
{
	if (m_in)
		return m_Header.NumberOfResources;

	return 0;
}

/** 
	���������� ������ �� ��� ������.

	@param[in] index - ����� ������� �� �������(�� 0, �� #CSDCRead::GetNumberOfResources)
	@param[out] ptr - ��������� �� ��������� � ������� ����� ��������� ������ �������
	@param[out] size - ��������� �� ����������, � ������� ����� ������� ������ �������.
	@param[out] res_type - ��������� �� ����������, ���� ����� ������� ��� ���������� �������.
	@param[out] res_index - ��������� �� ����������, ���� ����� ������� ����� ���������� �������.

	� ������ ������ ptr � size ����������.

	@return ��� ������.
*/
SDCError CSDCRead::GetResource(UInt32 index, const void **ptr, UInt32 *size, UInt32 *res_type, UInt32 *res_index)
{
	if (!ptr)
		return SDC_MEM_NULL_POINTER;
	if (!size)
		return SDC_MEM_NULL_POINTER;

	if (index >= GetNumberOfResources())
		return SDC_READ_WRONG_INDEX;

	// ���� ������ ����� ����� ����������
	for (UInt32 i=0;i<m_LoadedTableElementsCount;i++)
	{
		if (m_loadedTable[i].Type == m_resTable[index].Type &&
			m_loadedTable[i].Index == m_resTable[index].Index)
		{
			*ptr = m_loadedTable[i].ptr;
			*size = m_loadedTable[i].Size;
			
			if (res_type)
				*res_type = m_loadedTable[i].Type;
			if (res_index)
				*res_index = m_loadedTable[i].Index;
				
			return SDC_OK;
		}
	}

	// ������� ��������� ���������� � ��������� ��.
	ResourceMemType tmpLoad;
	memset(&tmpLoad, 0, sizeof(tmpLoad));

	tmpLoad.Index = m_resTable[index].Index;
	tmpLoad.Type = m_resTable[index].Type;
	tmpLoad.Size = m_resTable[index].Size;
	
	tmpLoad.ptr = malloc(tmpLoad.Size);
	if (!tmpLoad.ptr)
		return SDC_MEM_NOT_ENOUGH_MEMORY;

	// ������������ �� ������ ������� ��� �������.
	if (fseek(m_in, m_resTable[index].Shift, SEEK_SET))
		return SDC_READ_CANT_POSITIONING;

	// ��������� ������
	if (fread(tmpLoad.ptr, 1, tmpLoad.Size, m_in) != tmpLoad.Size)
		return SDC_READ_CANT_READ;

	// ����������� ������ ������� ����������� ��������, ���� ����������
	if (m_LoadedTableElementsCount >= m_MaxLoadedTableElementsCount)
	{
		m_MaxLoadedTableElementsCount *= 2;
		
		ResourceMemType* tmpPtr = (ResourceMemType*)malloc(m_MaxLoadedTableElementsCount*sizeof(ptr[0]));
		if (!tmpPtr)
			return SDC_MEM_NOT_ENOUGH_MEMORY;
		memset(tmpPtr, 0, m_MaxLoadedTableElementsCount*sizeof(tmpPtr[0]));
		memmove(tmpPtr, m_loadedTable, m_LoadedTableElementsCount*sizeof(m_loadedTable[0]));
		free(m_loadedTable);
		m_loadedTable = tmpPtr;
	}
	
	// ��������� ���
	m_loadedTable[m_LoadedTableElementsCount] = tmpLoad;
	m_LoadedTableElementsCount++;

	// ���������� ������������ ��������.
	*ptr = tmpLoad.ptr;
	*size = tmpLoad.Size;
	if (res_type)
		*res_type = tmpLoad.Type;
	if (res_index)
		*res_index = tmpLoad.Index;

	return SDC_OK;
}

/** 
��������� ������

@param aPtr	- ��������� �� ������ ������������ �������

@return ��� ������.
*/
SDCError CSDCRead::CloseResource(const void* aPtr)
{
	if (!aPtr)
		return SDC_MEM_NULL_POINTER;
	
	// ���� �� ��������� ����� ����� ���������� ��������
	for (UInt32 i=0;i<m_LoadedTableElementsCount;i++)
	{
		if (m_loadedTable[i].ptr == aPtr)
		{
			free(m_loadedTable[i].ptr);
			
			UInt32 j = i;
			for (;j<m_LoadedTableElementsCount-1;j++)
				m_loadedTable[j] = m_loadedTable[j+1];
				
			memset(&m_loadedTable[j], 0, sizeof(m_loadedTable[j]));
			m_LoadedTableElementsCount--;
				
			return SDC_OK;
		}
	}
	
	return SDC_READ_RESOURCE_NOT_FOUND;
}

/** 
���������� ������ �� ���� � ������.

@param[in] type - ��� �������
@param[in] index - ����� ������� ����� ������ �������� ������� ����
@param[out] ptr - ��������� �� ��������� � ������� ����� ��������� ������ �������
@param[out] size - ��������� �� ����������, � ������� ����� ������� ������ �������.

� ������ ������ ptr � size ����������.

@return ��� ������.
*/
SDCError CSDCRead::GetResourceByTypeIndex(UInt32 type, UInt32 index, const void **ptr, UInt32 *size)
{
	if (!ptr)
		return SDC_MEM_NULL_POINTER;
	if (!size)
		return SDC_MEM_NULL_POINTER;

	/* ��� �������� �� ������, ��������� ����� � ������� ����� ���� ����� � �� ����������� ������ � ������� ��������� ���� �� ����
	if (index >= GetNumberOfResources())
		return SDC_READ_WRONG_INDEX;
	*/

	// ���� ������ ����� ����� ����������.
	for (UInt32 i=0;i<m_LoadedTableElementsCount;i++)
	{
		if (m_loadedTable[i].Type == type &&
			m_loadedTable[i].Index == index)
		{
			*ptr = m_loadedTable[i].ptr;
			*size = m_loadedTable[i].Size;
			
			return SDC_OK;
		}
	}

	// ���� ����� ���� ��������� ��������
	UInt32 ResourceCount = GetNumberOfResources();
	
	// �������� �����
	if (m_Header.IsResourceTableSorted)
	{
		if (m_resTable[0].Type > type || m_resTable[ResourceCount-1].Type < type)
			return SDC_READ_RESOURCE_NOT_FOUND;
		
		UInt32 low = 0;
		UInt32 hi = ResourceCount;
		UInt32 mid = 0;
	 
		while (low < hi)
		{
			mid = low + (hi-low)/2;
			
			if (m_resTable[mid].Type == type)
			{
				if (m_resTable[mid].Index == index)
					return GetResource(mid, ptr, size, NULL, NULL);
				else if (m_resTable[mid].Index > index)
					hi = mid;
				else
					low = mid + 1;
			}
			else if (m_resTable[mid].Type > type)
				hi = mid;
			else
				low = mid + 1;
		}
	
		if (m_resTable[hi].Type == type && m_resTable[hi].Index == index)
			return GetResource(hi, ptr, size, NULL, NULL);
	}
	// �������� �����
	else
	{
		for (UInt32 i=0;i<ResourceCount;i++)
		{
			if (m_resTable[i].Type == type &&
				m_resTable[i].Index == index)
			{
				return GetResource(i, ptr, size, NULL, NULL);
			}
		}
	}

	return SDC_READ_RESOURCE_NOT_FOUND;
}

/** 
	��������� ���������� �� ������ ��������� � ����������, ��� �� �����������.

	@return ��� ������
*/
SDCError CSDCRead::CheckData(void)
{
	if (!m_in)
		return SDC_READ_NOT_OPENED;

	UInt32 new_CRC = SDC_CRC32_START_VALUE;
	UInt32 fileSize = 0;
	UInt32 CRC = 0;

	fseek(m_in, 0, SEEK_END);
	fileSize = ftell(m_in);

	if (fileSize != m_Header.FileSize)
		return SDC_READ_WRONG_FILESIZE;
	
	// ��������� CRC � �������� ��� � ���������, �.�. ������ CRC32 ��� �������� ����������
	// ���������� � m_Header.CRC = 0.
	CRC = m_Header.CRC;
	m_Header.CRC = 0;
	
	// CRC ��������� ����������
	new_CRC = CRC32((UInt8*)&m_Header, sizeof(m_Header), new_CRC, true);
	// ��������������� �������� CRC.
	m_Header.CRC = CRC;

	// CRC ������� ��������
	new_CRC = CRC32((UInt8*)m_resTable, m_Header.NumberOfResources*m_Header.ResourceRecordSize, new_CRC, true);

	// ������������ �� ������ ������� �������.
	fseek(m_in, m_Header.HeaderSize + m_Header.NumberOfResources*m_Header.ResourceRecordSize, SEEK_SET);
	UInt32 data_size = m_Header.FileSize-ftell(m_in);
	
	void* data = malloc(CRC_DATA_BLOCK_SIZE);
	if (!data)
		return SDC_MEM_NOT_ENOUGH_MEMORY;
	
	//** ��������� CRC ��������������� ��� ������� ����� ������ **//
	// ������ ���������� ����� ������ ��� ������
	UInt32 readSize = 0;
	while (data_size)
	{
		readSize = (data_size > CRC_DATA_BLOCK_SIZE) ? CRC_DATA_BLOCK_SIZE : data_size;
		
		if (fread(data, readSize, 1, m_in) != 1)
		{
			free(data);
			return SDC_READ_CANT_READ;
		}
		
		data_size -= readSize;
		if (data_size)
		{
			// ��� �� ��������� ���� ������
			new_CRC = CRC32((UInt8*)data, readSize, new_CRC, false);
		}
		else
		{
			// ��� ��������� ���� ������
			new_CRC = CRC32((UInt8*)data, readSize, new_CRC, true);
		}
	}
	free(data);

	// ��������� ������������ �������� CRC � �������� CRC � ���������
	if (new_CRC != m_Header.CRC)
		return SDC_READ_WRONG_CRC;

	return SDC_OK;
}

/** 
	������� ���������� ��� � ������ ������� �� ��� ������.

	@param[in] index - ����� �������, ����� ���� �� 0 �� #CSDCRead::GetNumberOfResources
	@param[out] res_type - �� ������� ��������� ����� ������� ��� �������
	@param[out] res_index - �� ������� ��������� ����� ������� ����� �������

	@return ��� ������.
*/
SDCError CSDCRead::GetTypeIndex(UInt32 index, UInt32 *res_type, UInt32 *res_index)
{
	if (!res_type)
		return SDC_MEM_NULL_POINTER;
	if (!res_index)
		return SDC_MEM_NULL_POINTER;

	if (index >= GetNumberOfResources())
		return SDC_READ_WRONG_INDEX;

	*res_type = m_resTable[index].Type;
	*res_index = m_resTable[index].Index;

	return SDC_OK;
}
