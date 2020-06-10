#pragma once

#include "FileResourceManagement.h"

/// Класс для хранения картинок
class CImageContainer
{
public:
	CImageContainer() = default;

	/// Возвращает общее количество картинок в контейнере
	UInt32 GetImageCount() const { return c.count(); }

	/// Устанавливает путь к папке с картинками
	void SetImageFolder(sld::wstring_ref aPath) { c.setFolder(aPath); }

	/// Добавляет картинку в контейнер по имени файла
	UInt32 AddImage(sld::wstring_ref aImageName) { return c.addFile(aImageName); }

	/// Возвращает индекс картинки по имени
	UInt32 GetImageIndexByName(sld::wstring_ref aImageName) const { return c.getIndex(aImageName); }

	/// Возвращает полное имя картинки
	std::wstring GetFullImageName(UInt32 aIndex) const { return c.getFullName(aIndex); }

private:
	FileResourceContainer c;
};
