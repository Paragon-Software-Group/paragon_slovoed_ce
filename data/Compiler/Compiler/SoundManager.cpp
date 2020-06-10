#include "SoundManager.h"

using namespace std;

/***********************************************************************
* ƒеструктор
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
* ¬озвращает общее количество озвучек в глобальном массиве
*
* @return общее количество озвучек в глобальном массиве
************************************************************************/
UInt32 CSoundManager::GetSoundCount(void) const
{
	return (UInt32)m_SoundsByName.size();
}

/***********************************************************************
* ƒобавл€ет контейнер с озвучкой
*
* @param aSoundContainer - ссылка на добавл€емый объект
*
* @return таблица соответстви€ локального индекса ресурса из контейнера глобальному индексу
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

	// ќбновл€ем глобальный массив
	std::vector<UInt32> map(soundsCount + 1, SLD_INDEX_SOUND_NO);
	for (UInt32 i = 1; i <= soundsCount; i++)
		map[i] = AddSound(aSoundContainer.GetFullSoundName(i), aSoundContainer.GetSoundNameByIndex(i), speexPackMode, speexPackName);

	return AddedFileResourceMap(SLD_INDEX_SOUND_NO, std::move(map));
}

/***********************************************************************
* ¬озвращает полное им€ файла озвучки по глобальному индексу
*
* @param aIndex - глобальный индекс озвучки
*
* @return полное им€ файла озвучки или пуста€ строка, если така€ озвучка не найдена
************************************************************************/
wstring CSoundManager::GetFullSoundNameBySoundIndex(UInt32 aIndex) const
{
	auto fit = m_SoundsByIndex.find(aIndex);
	return fit != m_SoundsByIndex.end() ? fit->second->FullSoundName : std::wstring();
}

/***********************************************************************
* ¬озвращает озвучку по глобальному индексу
*
* @param aIndex - глобальный индекс озвучки
*
* @return указатель на структуру с озвучкой или NULL, если така€ озвучка не найдена
************************************************************************/
const TSoundElement* CSoundManager::GetSoundBySoundIndex(UInt32 aIndex) const
{
	auto fit = m_SoundsByIndex.find(aIndex);
	return fit != m_SoundsByIndex.end() ? fit->second : nullptr;
}

/***********************************************************************
* ƒобавл€ет озвучку, использу€ полное им€ файла озвучки
*
* @param aFullSoundName - полное им€ файла озвучки
*
* @return глобальный индекс добавленного файла озвучки
************************************************************************/
UInt32 CSoundManager::AddSound(wstring aFullSoundName, wstring aSoundName, bool aSpeexPackMode, wstring aSpeexPackFilename)
{
	if (aFullSoundName.empty() || aSoundName.empty())
		return SLD_INDEX_SOUND_NO;

	// ѕровер€ем, может така€ озвучка уже есть
	auto fit = m_SoundsByName.find(aFullSoundName);
	if (fit != m_SoundsByName.end())
		return fit->second->Index;

	TSoundElement* pSound = new TSoundElement;

	pSound->FullSoundName = aFullSoundName;
	pSound->SoundName = aSoundName;
	pSound->SpeexPackMode = aSpeexPackMode;
	pSound->SpeexPackFilename = aSpeexPackFilename;

	// »ндекс добавл€емой озвучки
	// Ќумераци€ озвучек начинаетс€ с 1, 0 используетс€ как признак отсутстви€ озвучки
	pSound->Index = GetSoundCount() + 1;

	// ƒобавл€ем
	m_SoundsByName.emplace(pSound->FullSoundName, pSound);
	m_SoundsByIndex.emplace(pSound->Index, pSound);

	return pSound->Index;
}
