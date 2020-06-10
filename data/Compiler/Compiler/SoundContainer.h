#pragma once

#include "FileResourceManagement.h"
#include "sld2_Error.h"

/// ����� ��� �������� �������
class CSoundContainer
{
public:
	/// �����������
	CSoundContainer() : m_SpeexPackMode(false) {}

	/// ���������� ����� ���������� ������� � ����������
	UInt32 GetSoundCount() const { return c.count(); }

	/// ������������� ���� � ����� �� ������ ��� ��� ����� ���������� �������
	int SetSoundFolder(std::wstring aPath);
	/// ���������� ���� � ����� �� ������ ��� ��� ����� ���������� �������
	std::wstring GetSoundFolder() const { return c.folder(); };

	/// ��������� ������� � ��������� �� ����� �����
	UInt32 AddSound(sld::wstring_ref aSoundName) { return c.addFile(aSoundName); }

	/// ���������� ��� ������� �� �������
	std::wstring GetSoundNameByIndex(UInt32 aIndex) const { return to_string(c.getName(aIndex)); }

	/// ���������� ������ ��� �������
	std::wstring GetFullSoundName(UInt32 aIndex) const { return c.getFullName(aIndex); }

	/// ���������� ����� ������ � �����������
	bool GetSpeexPackMode(void) const { return m_SpeexPackMode; };

private:
	FileResourceContainer c;

	/// ����� ������ � ����������� (true) / �������� ������� �� ����� (false)
	bool m_SpeexPackMode;
};
