#pragma once

#include "ImageContainer.h"
#include "MaterialManager.h"
#include "MeshManager.h"
#include "Scene.h"
#include "SceneContainer.h"

class CSceneManager
{
public:

	/// �����������
	CSceneManager() {}

	/// ���������� ����� ���������� ���� � ���������� �������
	UInt32 GetSceneCount() const { return c.count(); }

	/// ��������� ��������� �� �������
	AddedFileResourceMap AddSceneContainer(const CSceneContainer &aSceneContainer,
										   CMaterialManager &aMaterialManager, CMeshManager &aMeshManager);

	/// ���������� ������ ��� ����� ����� �� ����������� �������
	std::wstring GetFullSceneNameBySceneIndex(UInt32 aIndex) const { return to_string(c.getName(aIndex)); }

	/// ���������� �������������� ����� �� �������
	const CScene* GetSceneByIndex(UInt32 aIndex) const;

	/// ���������� ��������� � ����������
	const CImageContainer& textures() const { return m_textures; }

protected:
	FileResourceContainer c;

	/// ���������� ������ �������������� ����
	/// �������� �������������� ����� � �� ���������� �������
	std::map<UInt32, CScene> m_BinScene;

	/// ��������� � ����������
	CImageContainer m_textures;
};
