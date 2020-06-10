#pragma once

#include <vector>

#include "sld_Types.h"
#include "sld2_Error.h"
#include "util.h"

/// ������ ����� ���������� ������, ��������� ������ ����� ������.
class COut
{
public:
	/// ����������� �� ���������
	COut(void);

	/// ����� ������� ��� ���������� ������
	void ClearContent();

	/// ��������� ������ � �������� �����
	int AddBytes(const void *aData, UInt32 sizeBytes);

	/// ��������� ������ � �������� �����
	int AddBits(UInt64 aData, UInt32 sizeBits);

	/// ���������� ���������� ������ ���������� ������� ������ � �����.
	UInt32 GetCurrentPos() const;

	/// ���������� ����������� ������.
	MemoryRef GetData() const;

private:
	/// ����� � ������� ���������� ������
	std::vector<UInt32> m_buffer;
	/// ���������� ��� ����������� � ��������� �����
	UInt32 m_Bit;
	/// ����� �������� �����
	UInt32 m_Byte;
};
