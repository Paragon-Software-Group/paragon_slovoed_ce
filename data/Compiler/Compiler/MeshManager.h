#pragma once

#include "FileResourceManagement.h"

class CMeshManager
{
public:
	/// Конструктор
	CMeshManager() = default;

	/// Возвращает общее количество моделей в глобальном массиве
	UInt32 GetMeshCount() const { return c.count(); }

	/// Добавляет модель, используя полное имя файла модели
	UInt32 AddMesh(sld::wstring_ref aFullMeshName) { return c.addFile(aFullMeshName); }

	/// Возвращает глобальный индекс модели по полному имени файла
	UInt32 GetMeshIndexByFullMeshName(sld::wstring_ref aFullMeshName) const { return c.getIndex(aFullMeshName); }
	/// Возвращает полное имя файла модели по глобальному индексу
	std::wstring GetFullMeshNameByMeshIndex(UInt32 aIndex) const { return to_string(c.getName(aIndex)); }

private:
	FileResourceContainer c;
};
