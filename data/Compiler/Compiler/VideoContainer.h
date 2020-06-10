#pragma once

#include "FileResourceManagement.h"

/// ����� ��� �������� �����
class CVideoContainer
{
public:
	/// �����������
	CVideoContainer() = default;

	/// ���������� ����� ���������� ����� � ����������
	UInt32 GetVideoCount() const { return c.count(); }

	/// ������������� ���� � ����� � �����
	void SetVideoFolder(sld::wstring_ref aPath) { c.setFolder(aPath); }

	/// ��������� ����� � ��������� �� ����� �����
	UInt32 AddVideo(sld::wstring_ref aVideoName) { return c.addFile(aVideoName); }

	/// ���������� ������ ��� �����
	std::wstring GetFullVideoName(UInt32 aIndex) const { return c.getFullName(aIndex); }

private:
	FileResourceContainer c;
};
