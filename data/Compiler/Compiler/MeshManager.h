#pragma once

#include "FileResourceManagement.h"

class CMeshManager
{
public:
	/// �����������
	CMeshManager() = default;

	/// ���������� ����� ���������� ������� � ���������� �������
	UInt32 GetMeshCount() const { return c.count(); }

	/// ��������� ������, ��������� ������ ��� ����� ������
	UInt32 AddMesh(sld::wstring_ref aFullMeshName) { return c.addFile(aFullMeshName); }

	/// ���������� ���������� ������ ������ �� ������� ����� �����
	UInt32 GetMeshIndexByFullMeshName(sld::wstring_ref aFullMeshName) const { return c.getIndex(aFullMeshName); }
	/// ���������� ������ ��� ����� ������ �� ����������� �������
	std::wstring GetFullMeshNameByMeshIndex(UInt32 aIndex) const { return to_string(c.getName(aIndex)); }

private:
	FileResourceContainer c;
};
