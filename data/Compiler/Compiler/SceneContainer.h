#pragma once

#include "FileResourceManagement.h"

class CSceneContainer
{
public:
	/// Конструктор
	CSceneContainer() = default;

	/// Возвращает общее количество сцен в контейнере
	UInt32 GetSceneCount() const { return c.count(); }

	/// Добавляет сцену в контейнер по имени файла
	UInt32 AddScene(sld::wstring_ref aSceneName) { return c.addFile(aSceneName); }

	/// Возвращает полное имя сцены
	std::wstring GetFullSceneName(UInt32 aIndex) const { return c.getFullName(aIndex); }

private:
	FileResourceContainer c;
};
