#pragma once

#include "FileResourceManagement.h"

/// ����� ��� �������� ��������
class CImageContainer
{
public:
	CImageContainer() = default;

	/// ���������� ����� ���������� �������� � ����������
	UInt32 GetImageCount() const { return c.count(); }

	/// ������������� ���� � ����� � ����������
	void SetImageFolder(sld::wstring_ref aPath) { c.setFolder(aPath); }

	/// ��������� �������� � ��������� �� ����� �����
	UInt32 AddImage(sld::wstring_ref aImageName) { return c.addFile(aImageName); }

	/// ���������� ������ �������� �� �����
	UInt32 GetImageIndexByName(sld::wstring_ref aImageName) const { return c.getIndex(aImageName); }

	/// ���������� ������ ��� ��������
	std::wstring GetFullImageName(UInt32 aIndex) const { return c.getFullName(aIndex); }

private:
	FileResourceContainer c;
};
