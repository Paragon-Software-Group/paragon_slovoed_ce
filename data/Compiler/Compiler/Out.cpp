#include "Out.h"

#include <string.h>

#include "Log.h"

/// ��� ����� ������� ������ ���������� ��� ����������.
#define OUT_SECURITY_SIZE	(16)

/// ����������� �������� ������
#define OUT_BIT_COUNT		(32)
/// ����� ������� ����� ��������� ��� ����, ����� �������� ������ ������ ���������� ���
#define OUT_BIT_MASK		((1LL<<OUT_BIT_COUNT)-1)

COut::COut(void)
{
	m_Byte = 0;
	m_Bit = 0;
}

/// ����� ������� ��� ���������� ������
void COut::ClearContent()
{
	m_buffer.clear();
	m_Byte = 0;
	m_Bit = 0;
}

// 	��������� ������ � �������� �����
/**

	@param[in] aData - ��������� �� ������
	@param[in] sizeBytes - ���������� ���� ������ ������� ���������� ��������

	@return ��� ������
*/
int COut::AddBytes(const void *aData, UInt32 sizeBytes)
{
	int error;
	for (UInt32 i=0;i<sizeBytes;i++)
	{
		error = AddBits(((const UInt8 *)aData)[i], 8);
		if (error != ERROR_NO)
			return error;
	}
	return ERROR_NO;
}

// 	��������� ������ � �������� �����
/**
	��������!!! ���� ���� �������� ����, ��� �������� ������!!!

	��������, ������ �����-�� ������(��� ����������� ������� �� 32�������):
	->00000000000000xxxxxxxxxxxxxxxxxxb, �� ��������� Intel(�� ����������� ����� ����� 
	PowerPC/DragonBall), � ��������� �� ������ ��� ����� ��������� ���:
	@code
	xxxx xxxx | xxxx xxxx | 0000 00xx | 0000 0000
	@endcode

	��� ����������� ����������� ����:
	@code
	00000000000000IHGFEDCBA987654321b
	@endcode

	� ������� N ����� ���������� ������ ������� ���� ��������� �����, � 
	������� 0 - �� �������� ����.

	���� ����� �������, ��� ����� � ��� ��� 1 �������� ����������� ����:
	@code
	00000NNN
	@endcode

	������ ������� � ���� ���� ������:

	@code
	5432 1NNN | 9876 DCBA | 000I HGFE | 0000 0000 | 000...
	���������� ������ ����� ^^^ � ����� �����.
	@endcode

	����� ����������� ������ ������� ����� �������� � ������ ��������� ������ 
	�������� ��� ���������.

	@param[in] aData - ����� �������������� ������
	@param[in] sizeBits - ���������� ��� ������ ������� ���������� ��������

	@return ��� ������
*/
int COut::AddBits(UInt64 aData, UInt32 sizeBits)
{
	// test for correct input data.
	if (sizeBits>=64)
	{
		sldILog("Error! COut::AddBits : Very big data size\n");
		return ERROR_WRONG_BINARY_DATA_SIZE;
	}

	// ������ ������ �� ������ ���� ������ 0!
	if (sizeBits==0)
	{
		sldILog("Error! COut::AddBits : zero data size\n");
		return ERROR_WRONG_BINARY_DATA_SIZE;
	}

	// ���� ����� �������������, ����� ����������� �����.
	if (m_Byte + OUT_SECURITY_SIZE >= m_buffer.size())
	{
		m_buffer.resize(m_buffer.empty() ? 1024 : m_buffer.size() * 2, 0);
	}

	UInt32 i;
	UInt64 tmpData = m_buffer[m_Byte];
	tmpData |= aData<<m_Bit;
	for (i=0;i<(sizeBits+m_Bit)/OUT_BIT_COUNT;i++)
	{
		m_buffer[m_Byte] = (tmpData & OUT_BIT_MASK);
		tmpData >>= OUT_BIT_COUNT;
		m_Byte++;
	}

	m_buffer[m_Byte] = (tmpData&OUT_BIT_MASK);
	m_Bit = (sizeBits+m_Bit)%OUT_BIT_COUNT;
	return ERROR_NO;
}


// ���������� ���������� ������ ���������� ������� ������ � �����.
UInt32 COut::GetCurrentPos() const
{
	return m_Byte*OUT_BIT_COUNT+m_Bit;
}

// ���������� ����������� ������.
MemoryRef COut::GetData() const
{
	const size_t size = m_Byte * (OUT_BIT_COUNT / 8) + (m_Bit ? (OUT_BIT_COUNT / 8) : 0);
	return MemoryRef(m_buffer.data(), size);
}
