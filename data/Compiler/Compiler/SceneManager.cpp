#include "SceneManager.h"

#include "Log.h"
#include "sld_Platform.h"
#include "Tools.h"

static int loadMateral(const std::wstring &aUrl, CMaterialManager &aMaterialManager, CImageContainer &aImageContainer)
{
	// ��������� ���� �� ����� �� ������
	const auto found = aUrl.find_last_of(L"/\\");
	if (found == std::wstring::npos)
	{
		sldILog("Error! CListLoader::ProcessMaterial : wrong path to material file!\n");
		return ERROR_CANT_OPEN_FILE;
	}
	const std::wstring path = aUrl.substr(0, found + 1);
	const std::string materialsDirectory = sld::as_utf8(path) + "materials/";
	const std::wstring texturesDirectory = path + L"images/";

	std::vector<std::string> files;
	GetFilesInDirectory(files, materialsDirectory);
	if (files.empty())
	{
		sldILog("Error! CListLoader::Processmaterial : No material files in directory!\n");
		return ERROR_CANT_OPEN_FILE;
	}

	for (auto&& fileName : files)
	{
		if (fileName.find_last_of(".material") != fileName.length() - 1)
			continue;

		// ��������� xml ����
		CorrectFilePathA(fileName);
		pugi::xml_document doc;
		doc.load_file(fileName.c_str());

		// ������ ���������� ��������� � ���� <Materials>
		for (auto materialNode : doc.child(L"Materials"))
		{
			sld::wstring_ref name = materialNode.attribute(L"name").value();
			if (name.empty())
				continue;

			CMaterial mat;
			mat.LoadFromXML(materialNode, aImageContainer, texturesDirectory);

			aMaterialManager.AddMaterial(name, mat);
		}
	}
	return ERROR_NO;
}

static int loadScene(const std::wstring &aUrl, CMeshManager &aMeshManager, CMaterialManager &aMaterialManager)
{
	// ��������� ���� �� ����� �� ������
	const auto found = aUrl.find_last_of(L"/\\");
	if (found == std::wstring::npos)
	{
		sldILog("Error! CListLoader::ProcessSceneXML : wrong path to scene file!\n");
		return ERROR_CANT_OPEN_FILE;
	}
	const std::wstring path = aUrl.substr(0, found + 1) + L"meshes/";

	// ��������� xml ����
	pugi::xml_document doc;
	doc.load_file(aUrl.c_str());

	// ���� ����� �������� �������� ������ �����, � ������ ���� �� ����
	for (auto node : doc.child(L"scene").child(L"nodes").children(L"node"))
	{
		pugi::xml_node entity = node.child(L"entity");
		if (entity.empty())
			continue;

		// TO DO : ��������� �� � ����� ���� ���� � �����?
		std::wstring meshPath = path + entity.attribute(L"meshFile").value();
		CorrectFilePathW(meshPath);
		UInt32 index = aMeshManager.AddMesh(meshPath);
		if (!index)
		{
			sldILog("Error! CListLoaser::ProcessSceneXML : wrong mesh index!\n");
			return ERROR_WRONG_INDEX;
		}

		for (auto subentity : entity.child(L"subentities").children(L"subentity"))
		{
			pugi::xml_attribute materialName = subentity.attribute(L"materialName");
			UInt32 index = aMaterialManager.GetMaterialIndexByFullMaterialName(materialName.value());
			if (!index)
			{
				sldILog("Error! CListLoader::ProcessSceneXML : Matierial not exist!\n");
				return ERROR_WRONG_STRING_ATTRIBUTE;
			}
		}
	}

	return ERROR_NO;
}

/***********************************************************************
* ��������� ��������� �� ������
*
* @param aVideoContainer - ������ �� ����������� ������
*
* @return ������� ������������ ���������� ������� ������� �� ���������� ����������� �������
************************************************************************/
AddedFileResourceMap CSceneManager::AddSceneContainer(const CSceneContainer &aSceneContainer,
													  CMaterialManager &aMaterialManager, CMeshManager &aMeshManager)
{
	const UInt32 sceneCount = aSceneContainer.GetSceneCount();
	if (sceneCount == 0)
		return AddedFileResourceMap(SLD_INDEX_SCENE_NO);

	// ��������� ���������� ������ ����
	std::vector<UInt32> map(sceneCount + 1, SLD_INDEX_SCENE_NO);
	for (UInt32 i = 1; i <= sceneCount; i++)
	{
		auto url = aSceneContainer.GetFullSceneName(i);
		if (url.empty())
		{
			sldILog("Error! No scene with index: %u\n", i);
			return AddedFileResourceMap(ERROR_WRONG_INDEX);
		}

		// ���������, ����� ����� ����� ��� ����
		uint32_t index = c.getIndex(url);
		if (index == c.InvalidIndex)
		{
			// ������� ������������ ����� � �����������
			int error = loadMateral(url, aMaterialManager, m_textures);
			if (error != ERROR_NO)
				return AddedFileResourceMap(error);

			// � ������ ��� � ���� ����� ����
			error = loadScene(url, aMeshManager, aMaterialManager);
			if (error != ERROR_NO)
				return AddedFileResourceMap(error);

			CScene scene;
			scene.LoadFromXML(url, aMeshManager, aMaterialManager);

			index = c.addFile(url);
			m_BinScene.emplace(index, std::move(scene));
		}

		map[i] = index;
	}

	return AddedFileResourceMap(SLD_INDEX_SCENE_NO, std::move(map));
}

/***********************************************************************
* ���������� �������������� ����� �� �������
*
* @return ��������� �� �����
************************************************************************/
const CScene* CSceneManager::GetSceneByIndex(UInt32 aIndex) const
{
	auto it = m_BinScene.find(aIndex);
	if (it != m_BinScene.cend())
		return &it->second;

	sldILog("Error! CSceneManager::GetSceneByIndex() - Invalid scene index: %d!\n", aIndex);
	return nullptr;
}
