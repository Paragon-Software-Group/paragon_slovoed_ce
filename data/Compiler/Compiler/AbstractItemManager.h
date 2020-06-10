#pragma once

#include "AbstractItemContainer.h"

/// Класс для управления видео
class CAbstractItemManager
{
public:
	/// Конструктор
	CAbstractItemManager() = default;

	/// Общее количество абстрактных ресурсов как имен файлов
	UInt32 GetItemsCount() const { return c.count(); }

	/// Добавляет контейнер с видео
	AddedFileResourceMap AddAbstractItemContainer(const CAbstractItemContainer &aAbstractItemContainer);

	/// Возвращает полное имя файла абстрактного ресурса по индексу
	std::wstring GetFullItemNameByItemIndex(UInt32 aIndex) const { return to_string(c.getName(aIndex)); }

private:
	FileResourceContainer c;
};
