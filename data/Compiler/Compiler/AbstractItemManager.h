#pragma once

#include "AbstractItemContainer.h"

/// ����� ��� ���������� �����
class CAbstractItemManager
{
public:
	/// �����������
	CAbstractItemManager() = default;

	/// ����� ���������� ����������� �������� ��� ���� ������
	UInt32 GetItemsCount() const { return c.count(); }

	/// ��������� ��������� � �����
	AddedFileResourceMap AddAbstractItemContainer(const CAbstractItemContainer &aAbstractItemContainer);

	/// ���������� ������ ��� ����� ������������ ������� �� �������
	std::wstring GetFullItemNameByItemIndex(UInt32 aIndex) const { return to_string(c.getName(aIndex)); }

private:
	FileResourceContainer c;
};
