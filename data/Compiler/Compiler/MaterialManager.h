#pragma once

#include "FileResourceManagement.h"
#include "Material.h"

/// ����� ��� ���������� 3d �����������
class CMaterialManager
{
public:

	/// �����������
	CMaterialManager() = default;

	/// ���������� ����� ���������� ���������� � ���������� �������
	UInt32 GetMaterialCount() const { return c.count(); }

	/// ��������� ��������, ��������� ������ ��� ���������
	UInt32 AddMaterial(sld::wstring_ref aMaterialName, CMaterial aMaterial);

	/// ���������� ���������� ������ ��������� �� ������� ����� �����
	UInt32 GetMaterialIndexByFullMaterialName(sld::wstring_ref aFullMaterialName) const { return c.getIndex(aFullMaterialName); }
	/// ���������� ������ ��� ����� ��������� �� ����������� �������
	std::wstring GetFullMaterialNameByMaterialIndex(UInt32 aIndex) const { return to_string(c.getName(aIndex)); }

	/// ���������� ��� �������� �� ����������� �������
	const CMaterial* GetMaterialByIndex(UInt32 aIndex) const;
	/// ������ ��������� �������� ������� �� ����������
	int MapNameToGlobalIndex(const AddedFileResourceMap &aImageMap);

private:
	FileResourceContainer c;

	/// ���������� ������ ����������
	/// �������� ���� ��������� � �� ���������� �������
	std::unordered_map<UInt32, CMaterial> m_IndexToMaterial;
};
