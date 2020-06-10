#include "MaterialManager.h"

#include "Log.h"

/***********************************************************************
* Добавляет материал, используя полное имя материала
*
* @param aMaterialName	- имя материала
* @param aMaterial		- сам материал
*
* @return индекс добавленного материала
************************************************************************/
UInt32 CMaterialManager::AddMaterial(sld::wstring_ref aMaterialName, CMaterial aMaterial)
{
	if (aMaterialName.empty())
		return 0;

	// Проверяем, может такой материал уже есть
	UInt32 index = c.getIndex(aMaterialName);
	if (index == c.InvalidIndex)
	{
		index = c.addFile(aMaterialName);
		m_IndexToMaterial.emplace(index, aMaterial);
	}
	return index;
}

/***********************************************************************
* Возвращаем сам материал по глобальному индексу
*
* @param aIndex		- индекс материала
*
* @return указатель на материал, nullptr в случае ошибки
************************************************************************/
const CMaterial* CMaterialManager::GetMaterialByIndex(UInt32 aIndex) const
{
	if (aIndex == 0 || (aIndex > GetMaterialCount()))
	{
		sldILog("Error! CMaterialManager::GetMaterialByIndex : wrong material index!\n");
		return nullptr;
	}

	// Находим материал с этим индексом
	auto it = m_IndexToMaterial.find(aIndex);
	if (it == m_IndexToMaterial.end())
	{
		sldILog("Error! CMaterialManager::GetMaterialByIndex : material with index doesn't exist!\n");
		return nullptr;
	}

	return &it->second;
}

/***********************************************************************
* Замена локальных индексов текстур на глобальные
*
* @param aImageMap - "маппинг" соответствий локалных и глобальных индексов
*
* @return код ошибки
************************************************************************/
int CMaterialManager::MapNameToGlobalIndex(const AddedFileResourceMap &aImageMap)
{
	for (auto&& material : m_IndexToMaterial)
		material.second.MapNameToGlobalIndex(aImageMap);
	return ERROR_NO;
}
