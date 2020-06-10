#pragma once

#include "FileResourceManagement.h"

/// Класс для хранения видео
class CVideoContainer
{
public:
	/// Конструктор
	CVideoContainer() = default;

	/// Возвращает общее количество видео в контейнере
	UInt32 GetVideoCount() const { return c.count(); }

	/// Устанавливает путь к папке с видео
	void SetVideoFolder(sld::wstring_ref aPath) { c.setFolder(aPath); }

	/// Добавляет видео в контейнер по имени файла
	UInt32 AddVideo(sld::wstring_ref aVideoName) { return c.addFile(aVideoName); }

	/// Возвращает полное имя видео
	std::wstring GetFullVideoName(UInt32 aIndex) const { return c.getFullName(aIndex); }

private:
	FileResourceContainer c;
};
