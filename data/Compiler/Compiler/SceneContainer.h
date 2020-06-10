#pragma once

#include "FileResourceManagement.h"

class CSceneContainer
{
public:
	/// �����������
	CSceneContainer() = default;

	/// ���������� ����� ���������� ���� � ����������
	UInt32 GetSceneCount() const { return c.count(); }

	/// ��������� ����� � ��������� �� ����� �����
	UInt32 AddScene(sld::wstring_ref aSceneName) { return c.addFile(aSceneName); }

	/// ���������� ������ ��� �����
	std::wstring GetFullSceneName(UInt32 aIndex) const { return c.getFullName(aIndex); }

private:
	FileResourceContainer c;
};
