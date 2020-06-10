#pragma once

#include "FileResourceManagement.h"

///  ласс дл€ хранени€ абстрактного ресурса
class CAbstractItemContainer
{
public:
	///  онструктор
	CAbstractItemContainer() = default;

	/// ¬озвращает общее количество абстрактных ресурсов в контейнере
	UInt32 GetItemsCount() const { return c.count(); }

	/// ƒобавл€ет абстрактный ресурс в контейнер по имени файла
	UInt32 AddItem(sld::wstring_ref aAbstractResourceName) { return c.addFile(aAbstractResourceName); }

	/// ¬озвращает полное им€ абстрактного ресурса
	std::wstring GetFullItemName(UInt32 aIndex) const { return c.getFullName(aIndex); }

private:
	FileResourceContainer c;
};
