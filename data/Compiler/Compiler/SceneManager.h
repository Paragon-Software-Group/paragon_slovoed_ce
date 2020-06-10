#pragma once

#include "ImageContainer.h"
#include "MaterialManager.h"
#include "MeshManager.h"
#include "Scene.h"
#include "SceneContainer.h"

class CSceneManager
{
public:

	/// Конструктор
	CSceneManager() {}

	/// Возвращает общее количество сцен в глобальном массиве
	UInt32 GetSceneCount() const { return c.count(); }

	/// Добавляет контейнер со сценами
	AddedFileResourceMap AddSceneContainer(const CSceneContainer &aSceneContainer,
										   CMaterialManager &aMaterialManager, CMeshManager &aMeshManager);

	/// Возвращает полное имя файла сцены по глобальному индексу
	std::wstring GetFullSceneNameBySceneIndex(UInt32 aIndex) const { return to_string(c.getName(aIndex)); }

	/// Возвращает бинаризованную сцену по индексу
	const CScene* GetSceneByIndex(UInt32 aIndex) const;

	/// Возвращает контейнер с текстурами
	const CImageContainer& textures() const { return m_textures; }

protected:
	FileResourceContainer c;

	/// Глобальный массив бинаризованных сцен
	/// Содержит бинаризованные сцены и их глобальные индексы
	std::map<UInt32, CScene> m_BinScene;

	/// Контейнер с текстурами
	CImageContainer m_textures;
};
