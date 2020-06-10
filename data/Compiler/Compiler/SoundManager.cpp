#include "SoundManager.h"

using namespace std;

/***********************************************************************
* ����������
************************************************************************/
CSoundManager::~CSoundManager(void)
{
	for (auto&& it : m_SoundsByName)
	{
		if (it.second)
			delete it.second;
	}
}

/***********************************************************************
* ���������� ����� ���������� ������� � ���������� �������
*
* @return ����� ���������� ������� � ���������� �������
************************************************************************/
UInt32 CSoundManager::GetSoundCount(void) const
{
	return (UInt32)m_SoundsByName.size();
}

/***********************************************************************
* ��������� ��������� � ��������
*
* @param aSoundContainer - ������ �� ����������� ������
*
* @return ������� ������������ ���������� ������� ������� �� ���������� ����������� �������
************************************************************************/
AddedFileResourceMap CSoundManager::AddSoundContainer(const CSoundContainer &aSoundContainer)
{
	UInt32 soundsCount = aSoundContainer.GetSoundCount();
	if (soundsCount == 0)
		return AddedFileResourceMap(SLD_INDEX_SOUND_NO);

	std::wstring speexPackName;
	const bool speexPackMode = aSoundContainer.GetSpeexPackMode();
	if (speexPackMode)
		speexPackName = aSoundContainer.GetSoundFolder();

	// ��������� ���������� ������
	std::vector<UInt32> map(soundsCount + 1, SLD_INDEX_SOUND_NO);
	for (UInt32 i = 1; i <= soundsCount; i++)
		map[i] = AddSound(aSoundContainer.GetFullSoundName(i), aSoundContainer.GetSoundNameByIndex(i), speexPackMode, speexPackName);

	return AddedFileResourceMap(SLD_INDEX_SOUND_NO, std::move(map));
}

/***********************************************************************
* ���������� ������ ��� ����� ������� �� ����������� �������
*
* @param aIndex - ���������� ������ �������
*
* @return ������ ��� ����� ������� ��� ������ ������, ���� ����� ������� �� �������
************************************************************************/
wstring CSoundManager::GetFullSoundNameBySoundIndex(UInt32 aIndex) const
{
	auto fit = m_SoundsByIndex.find(aIndex);
	return fit != m_SoundsByIndex.end() ? fit->second->FullSoundName : std::wstring();
}

/***********************************************************************
* ���������� ������� �� ����������� �������
*
* @param aIndex - ���������� ������ �������
*
* @return ��������� �� ��������� � �������� ��� NULL, ���� ����� ������� �� �������
************************************************************************/
const TSoundElement* CSoundManager::GetSoundBySoundIndex(UInt32 aIndex) const
{
	auto fit = m_SoundsByIndex.find(aIndex);
	return fit != m_SoundsByIndex.end() ? fit->second : nullptr;
}

/***********************************************************************
* ��������� �������, ��������� ������ ��� ����� �������
*
* @param aFullSoundName - ������ ��� ����� �������
*
* @return ���������� ������ ������������ ����� �������
************************************************************************/
UInt32 CSoundManager::AddSound(wstring aFullSoundName, wstring aSoundName, bool aSpeexPackMode, wstring aSpeexPackFilename)
{
	if (aFullSoundName.empty() || aSoundName.empty())
		return SLD_INDEX_SOUND_NO;

	// ���������, ����� ����� ������� ��� ����
	auto fit = m_SoundsByName.find(aFullSoundName);
	if (fit != m_SoundsByName.end())
		return fit->second->Index;

	TSoundElement* pSound = new TSoundElement;

	pSound->FullSoundName = aFullSoundName;
	pSound->SoundName = aSoundName;
	pSound->SpeexPackMode = aSpeexPackMode;
	pSound->SpeexPackFilename = aSpeexPackFilename;

	// ������ ����������� �������
	// ��������� ������� ���������� � 1, 0 ������������ ��� ������� ���������� �������
	pSound->Index = GetSoundCount() + 1;

	// ���������
	m_SoundsByName.emplace(pSound->FullSoundName, pSound);
	m_SoundsByIndex.emplace(pSound->Index, pSound);

	return pSound->Index;
}
