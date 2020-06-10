#pragma once

#include "FileResourceManagement.h"

/// ����� ��� �������� ������������ �������
class CAbstractItemContainer
{
public:
	/// �����������
	CAbstractItemContainer() = default;

	/// ���������� ����� ���������� ����������� �������� � ����������
	UInt32 GetItemsCount() const { return c.count(); }

	/// ��������� ����������� ������ � ��������� �� ����� �����
	UInt32 AddItem(sld::wstring_ref aAbstractResourceName) { return c.addFile(aAbstractResourceName); }

	/// ���������� ������ ��� ������������ �������
	std::wstring GetFullItemName(UInt32 aIndex) const { return c.getFullName(aIndex); }

private:
	FileResourceContainer c;
};
