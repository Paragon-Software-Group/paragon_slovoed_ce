#pragma once

#include <map>

#include "SoundContainer.h"

/// ��������� ��������� ���� ������� �������
struct TSoundElement
{
	// ������ ��� ����� �������: ���� � ����� �� ������ ��� ��� �����-���������� + ��� �����
	// ��� �� �� �����, ��� � ���� � ���������� ������� ���� �������
	std::wstring FullSoundName;
	// ��� ����� �������
	// � ������ SpeexPackMode, ��� ���, �� �������� �� ���������� ���� ����������� �������
	std::wstring SoundName;
	// ����� ������� �� ���������� ��� ���
	bool SpeexPackMode;
	// ��� ����� ���������� �������, ���� SpeexPackMode
	std::wstring SpeexPackFilename;
	// ���������� ������ �������
	UInt32 Index;

	// �����������
	TSoundElement() : SpeexPackMode(false), Index(0) {}
};

/// ����� ��� ���������� ��������
class CSoundManager
{
public:
	/// �����������
	CSoundManager() = default;
	/// ����������
	~CSoundManager(void);

public:

	/// ���������� ����� ���������� ������� � ���������� �������
	UInt32 GetSoundCount(void) const;

	/// ��������� ��������� � ��������
	AddedFileResourceMap AddSoundContainer(const CSoundContainer &aSoundContainer);

	/// ���������� ������ ��� ����� ������� �� ����������� �������
	std::wstring GetFullSoundNameBySoundIndex(UInt32 aIndex) const;

	/// ���������� ������� �� ����������� �������
	const TSoundElement* GetSoundBySoundIndex(UInt32 aIndex) const;

private:

	/// ��������� ������� � ���������� ������
	UInt32 AddSound(std::wstring aFullSoundName, std::wstring aSoundName, bool aSpeexPackMode, std::wstring aSpeexPackFilename);

private:

	/// ���������� ������ ������� ��� ������ �� ������� ����� ����� ������� (����������� ������ ������ � ���� �������)
	/// ���� ������� - ������ ��� ����� �������: ���� � ����� �� ������ ��� ��� �����-���������� + ��� �����
	std::map<std::wstring, TSoundElement*> m_SoundsByName;

	/// ���������� ������ ������� ��� ������ �� ����������� ������� ������� (����� ������� m_SoundsByName, ������ ��� ����������� �� ����)
	std::map<UInt32, TSoundElement*> m_SoundsByIndex;
};
