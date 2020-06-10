#include "AbstractItemManager.h"

/***********************************************************************
* Добавляет контейнер с объектами
*
* @param aAbstractItemContainer - ссылка на добавляемый объект
*
* @return таблица соответствия локального индекса ресурса из контейнера глобальному индексу
************************************************************************/
AddedFileResourceMap CAbstractItemManager::AddAbstractItemContainer(const CAbstractItemContainer &aAbstractItemContainer)
{
	const UInt32 itemsCount = aAbstractItemContainer.GetItemsCount();
	if (itemsCount == 0)
		return AddedFileResourceMap(SLD_INDEX_VIDEO_NO);

	// Обновляем глобальный массив объектов
	std::vector<UInt32> map(itemsCount + 1, SLD_INDEX_VIDEO_NO);
	for (UInt32 i = 1; i <= itemsCount; i++)
	{
		auto name = aAbstractItemContainer.GetFullItemName(i);
		if (name.size())
			map[i] = c.addFile(name);
	}

	return AddedFileResourceMap(SLD_INDEX_VIDEO_NO, std::move(map));
}
