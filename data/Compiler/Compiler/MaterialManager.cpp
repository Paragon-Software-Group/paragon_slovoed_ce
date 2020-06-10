#include "MaterialManager.h"

#include "Log.h"

/***********************************************************************
* ��������� ��������, ��������� ������ ��� ���������
*
* @param aMaterialName	- ��� ���������
* @param aMaterial		- ��� ��������
*
* @return ������ ������������ ���������
************************************************************************/
UInt32 CMaterialManager::AddMaterial(sld::wstring_ref aMaterialName, CMaterial aMaterial)
{
	if (aMaterialName.empty())
		return 0;

	// ���������, ����� ����� �������� ��� ����
	UInt32 index = c.getIndex(aMaterialName);
	if (index == c.InvalidIndex)
	{
		index = c.addFile(aMaterialName);
		m_IndexToMaterial.emplace(index, aMaterial);
	}
	return index;
}

/***********************************************************************
* ���������� ��� �������� �� ����������� �������
*
* @param aIndex		- ������ ���������
*
* @return ��������� �� ��������, nullptr � ������ ������
************************************************************************/
const CMaterial* CMaterialManager::GetMaterialByIndex(UInt32 aIndex) const
{
	if (aIndex == 0 || (aIndex > GetMaterialCount()))
	{
		sldILog("Error! CMaterialManager::GetMaterialByIndex : wrong material index!\n");
		return nullptr;
	}

	// ������� �������� � ���� ��������
	auto it = m_IndexToMaterial.find(aIndex);
	if (it == m_IndexToMaterial.end())
	{
		sldILog("Error! CMaterialManager::GetMaterialByIndex : material with index doesn't exist!\n");
		return nullptr;
	}

	return &it->second;
}

/***********************************************************************
* ������ ��������� �������� ������� �� ����������
*
* @param aImageMap - "�������" ������������ �������� � ���������� ��������
*
* @return ��� ������
************************************************************************/
int CMaterialManager::MapNameToGlobalIndex(const AddedFileResourceMap &aImageMap)
{
	for (auto&& material : m_IndexToMaterial)
		material.second.MapNameToGlobalIndex(aImageMap);
	return ERROR_NO;
}
