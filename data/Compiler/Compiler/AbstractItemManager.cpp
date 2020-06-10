#include "AbstractItemManager.h"

/***********************************************************************
* ��������� ��������� � ���������
*
* @param aAbstractItemContainer - ������ �� ����������� ������
*
* @return ������� ������������ ���������� ������� ������� �� ���������� ����������� �������
************************************************************************/
AddedFileResourceMap CAbstractItemManager::AddAbstractItemContainer(const CAbstractItemContainer &aAbstractItemContainer)
{
	const UInt32 itemsCount = aAbstractItemContainer.GetItemsCount();
	if (itemsCount == 0)
		return AddedFileResourceMap(SLD_INDEX_VIDEO_NO);

	// ��������� ���������� ������ ��������
	std::vector<UInt32> map(itemsCount + 1, SLD_INDEX_VIDEO_NO);
	for (UInt32 i = 1; i <= itemsCount; i++)
	{
		auto name = aAbstractItemContainer.GetFullItemName(i);
		if (name.size())
			map[i] = c.addFile(name);
	}

	return AddedFileResourceMap(SLD_INDEX_VIDEO_NO, std::move(map));
}
