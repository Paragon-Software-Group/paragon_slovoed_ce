#include "Scene.h"

#include "sld_Platform.h"
#include "Tools.h"

using namespace std;

CScene::CScene(void) : m_numberOfNodes(0)
{
	memset(&m_header, 0, sizeof(TSceneHeader));
	memset(&m_enviroment, 0, sizeof(TSceneEnviroment));
}

/// Convert pugi::xml_node to SceneNode
void createSceneNode(pugi::xml_node &node, TSceneNode &result, 
					 CMeshManager& aMeshManager, CMaterialManager& aMaterialManager,
					 wstring aFolderName)
{
	pugi::xml_node pos_node = node.child(L"position");
	TPosition pos;
	if(pos_node.empty()) return;
	else
	{
		pos.x = pos_node.attribute(L"x").as_float();
		pos.y = pos_node.attribute(L"y").as_float();
		pos.z = pos_node.attribute(L"z").as_float();
	}
	result.point = pos;

	pugi::xml_node rotation_node = node.child(L"rotation");
	TQuaternion rotation;
	if(rotation_node.empty()) return;
	else
	{
		rotation.x = rotation_node.attribute(L"qx").as_float();
		rotation.y = rotation_node.attribute(L"qy").as_float();
		rotation.z = rotation_node.attribute(L"qz").as_float();
		rotation.w = rotation_node.attribute(L"qw").as_float();
	}
	result.rotation = rotation;

	pugi::xml_node scale_node = node.child(L"scale");
	TScale size;
	if(scale_node.empty()) return;
	else
	{
		size.x = scale_node.attribute(L"x").as_float();
		size.y = scale_node.attribute(L"y").as_float();
		size.z = scale_node.attribute(L"z").as_float();
	}
	result.nodeScale = size;

	/// Анимации
	pugi::xml_node animations_node = node.child(L"animations");

	/// Обход всех элементов <animation>
	for(auto it = animations_node.begin(); it != animations_node.end(); it++)
	{
		TAnimation animation;
		animation.length = it->attribute(L"length").as_double();
		animation.name   = it->attribute(L"name").value();

		for(auto keyframe_node = it->begin(); keyframe_node != it->end(); keyframe_node++)
		{
			TKeyframe keyframe;
			keyframe.time = keyframe_node->attribute(L"time").as_double();
			pugi::xml_node pos_node = keyframe_node->child(L"position");
			if (!pos_node.empty()) 
			{
				keyframe.pos.x = pos_node.attribute(L"x").as_float();
				keyframe.pos.y = pos_node.attribute(L"y").as_float();
				keyframe.pos.z = pos_node.attribute(L"z").as_float();
			}

			pugi::xml_node rotation_node = keyframe_node->child(L"rotation");
			if (!rotation_node.empty()) 
			{
				keyframe.rotation.x = rotation_node.attribute(L"qx").as_float();
				keyframe.rotation.y = rotation_node.attribute(L"qy").as_float();
				keyframe.rotation.z = rotation_node.attribute(L"qz").as_float();
				keyframe.rotation.w = rotation_node.attribute(L"qw").as_float();
			}

			pugi::xml_node scale_node = keyframe_node->child(L"scale");
			if (!scale_node.empty()) 
			{
				keyframe.size.x = scale_node.attribute(L"x").as_float();
				keyframe.size.y = scale_node.attribute(L"y").as_float();
				keyframe.size.z = scale_node.attribute(L"z").as_float();
			}
			animation.keyframes.push_back(keyframe);
		}
		result.animations.push_back(animation);
	}

	aFolderName.append(L"meshes/");
	pugi::xml_node entityNode = node.child(L"entity");
	wstring meshName = entityNode.attribute(L"meshFile").value();
	meshName = aFolderName + meshName;
	result.entity.mesh = aMeshManager.GetMeshIndexByFullMeshName(meshName);
	
	pugi::xml_node subentitiesNode = entityNode.child(L"subentities");
	/// Обход всех элементов <subentity>
	for(auto it = subentitiesNode.begin(); it != subentitiesNode.end(); it++)
	{
		TSubentity subentityElement;
		subentityElement.index = it->attribute(L"index").as_uint();
		wstring materialName = it->attribute(L"materialName").value();
		subentityElement.Material = aMaterialManager.GetMaterialIndexByFullMaterialName(materialName);
		result.entity.sceneNodes.push_back(subentityElement);
		result.entity.numberOfSubentities++;
	}
}

/// Загружаем объект из файла xml
void CScene::LoadFromXML(wstring aFullSceneName, CMeshManager& aMeshManager, 
						 CMaterialManager& aMaterialManager)
{
	// Выделение пути до папки со сценой
	std::wstring::size_type found = aFullSceneName.find_last_of(L"/\\");
	wstring path = aFullSceneName.substr(0,found + 1);

	m_header.sizeScene = 0;
	// Файл сцены содержит линейный список нодов, в каждом ноде по мешу
	pugi::xml_document doc;
	// Открываем xml файл
	doc.load_file(aFullSceneName.c_str());
	pugi::xml_node sceneNode = doc.child(L"scene");
	pugi::xml_node environmentNode = sceneNode.child(L"environment");

	pugi::xml_node cAmbient = environmentNode.child(L"colourAmbient");
	if(!cAmbient.empty())
	{
		m_enviroment.colourAmbient.setRed(cAmbient.attribute(L"r").as_uint());
		m_enviroment.colourAmbient.setGreen(cAmbient.attribute(L"g").as_uint());
		m_enviroment.colourAmbient.setBlue(cAmbient.attribute(L"b").as_uint());
	}
	pugi::xml_node cBackground = environmentNode.child(L"colourBackground");
	if(!cAmbient.empty())
	{
		m_enviroment.colourBackground.setRed(cBackground.attribute(L"r").as_uint());
		m_enviroment.colourBackground.setGreen(cBackground.attribute(L"g").as_uint());
		m_enviroment.colourBackground.setBlue(cBackground.attribute(L"b").as_uint());
	}

	pugi::xml_node nodes = sceneNode.child(L"nodes");
	for(auto it = nodes.begin(); it != nodes.end(); it++)
	{
		pugi::xml_node node = *it;
		if(wcscmp(node.name(), L"node"))
			continue;

		TSceneNode sceneNodeElement;
		createSceneNode(node, sceneNodeElement, aMeshManager, aMaterialManager, path);
		if (sceneNodeElement.animations.size() != 0)
			m_header.hasAnimation = true;

		m_sceneNodes.push_back(sceneNodeElement);
		m_numberOfNodes++;
	}
	m_header.version = 0x00000001;
	m_header.sizeHeader = sizeof(TSceneHeader);
	m_header.sizeScene = m_header.sizeHeader + sizeof(TSceneEnviroment) + sizeof(UInt32);


	for(auto it = m_sceneNodes.begin(); it != m_sceneNodes.end(); it++)
	{
		m_header.sizeScene += sizeof(TPosition) + sizeof(TQuaternion) + sizeof(TScale) + sizeof(UInt32) + sizeof(UInt32);
		for(auto itEntity = it->entity.sceneNodes.begin(); itEntity != it->entity.sceneNodes.end(); itEntity++)
			m_header.sizeScene += sizeof(TSubentity);

		for(auto itAnimation = it->animations.begin(); itAnimation != it->animations.end(); itAnimation++)
			m_header.sizeScene += sizeof(itAnimation->length) + sizeof(UInt16) * 128 + itAnimation->keyframes.size() * sizeof(TKeyframe);
	}

	
}

/// Сериализует объект
MemoryBuffer CScene::Serialize() const
{
	MemoryBuffer buf(m_header.sizeScene);
	uint8_t *buffer = buf.data();

	memcpy(buffer, &m_header, sizeof(TSceneHeader));
	buffer += sizeof(TSceneHeader);

	memcpy(buffer, &m_enviroment, sizeof(TSceneEnviroment));
	buffer += sizeof(TSceneEnviroment);

	memcpy(buffer, &m_numberOfNodes, sizeof(UInt32));
	buffer += sizeof(UInt32);

	for(auto it = m_sceneNodes.begin(); it != m_sceneNodes.end(); it++)
	{
		memcpy(buffer, &it->point, sizeof(TPosition));
		buffer += sizeof(TPosition);

		memcpy(buffer, &it->rotation, sizeof(TQuaternion));
		buffer += sizeof(TQuaternion);

		memcpy(buffer, &it->nodeScale, sizeof(TScale));
		buffer += sizeof(TScale);

		memcpy(buffer, &it->entity.mesh, sizeof(UInt32));
		buffer += sizeof(UInt32);

		memcpy(buffer, &it->entity.numberOfSubentities, sizeof(UInt32));
		buffer += sizeof(UInt32);
		for(auto itEntity = it->entity.sceneNodes.begin(); itEntity != it->entity.sceneNodes.end(); itEntity++)
		{
			memcpy(buffer, &(*itEntity), sizeof(TSubentity));
			buffer += sizeof(TSubentity);
		}

		if (m_header.hasAnimation)
		{
			UInt32 numberOfAnimations = static_cast<UInt32>(it->animations.size());
			memcpy(buffer, &numberOfAnimations, sizeof(UInt32));
			buffer += sizeof(UInt32);

			for (auto animation = it->animations.begin(); animation != it->animations.end(); animation++)
			{
				// Общая длина
				UInt32 length = animation->length;
				memcpy(buffer, &length, sizeof(length));
				buffer += sizeof(length);

				// Название анимации
				std::u16string name = sld::as_utf16(animation->name);
				UInt32 name_length = static_cast<UInt32>(name.length());
				memcpy(buffer, &name_length, sizeof(name_length));
				buffer += sizeof(name_length);

				memcpy(buffer, name.c_str(), name_length * sizeof(char16_t));
				buffer += name_length * sizeof(char16_t);

				// Число ключевых кадров
				UInt32 size = static_cast<UInt32>(animation->keyframes.size());
				memcpy(buffer, &size, sizeof(UInt32));
				buffer += sizeof(UInt32);

				for (auto keyframe = animation->keyframes.begin(); keyframe != animation->keyframes.end(); keyframe++)
				{
					// Временная отметка кадра
					memcpy(buffer, &keyframe->time, sizeof(keyframe->time));
					buffer += sizeof(keyframe->time);
					// Position
					memcpy(buffer, &keyframe->pos, sizeof(TPosition));
					buffer += sizeof(TPosition);
					// Rotation
					memcpy(buffer, &keyframe->rotation, sizeof(TQuaternion));
					buffer += sizeof(TQuaternion);
					// Size
					memcpy(buffer, &keyframe->size, sizeof(TScale));
					buffer += sizeof(TScale);
				}
			}
		}
	}
	return buf;
}
