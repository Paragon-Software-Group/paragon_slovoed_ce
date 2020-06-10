#include "VideoManager.h"

#include "SDC_CRC.h"
#include "Tools.h"

using namespace std;

/***********************************************************************
* ¬озвращает общее количество видео в глобальном массиве
*
* @return общее количество видео в глобальном массиве
************************************************************************/
UInt32 CVideoManager::GetVideoCount(void) const
{
	return (UInt32)m_UniqueVideos.size();
}

/***********************************************************************
* ƒобавл€ет контейнер с видео
*
* @param aVideoContainer - ссылка на добавл€емый объект
*
* @return таблица соответстви€ локального индекса ресурса из контейнера глобальному индексу
************************************************************************/
AddedFileResourceMap CVideoManager::AddVideoContainer(const CVideoContainer &aVideoContainer)
{
	UInt32 videoCount = aVideoContainer.GetVideoCount();
	if (videoCount == 0)
		return AddedFileResourceMap(SLD_INDEX_VIDEO_NO);

	// ќбновл€ем глобальный массив видео
	std::vector<UInt32> map(videoCount + 1, SLD_INDEX_PICTURE_NO);
	for (UInt32 i = 1; i <= videoCount; i++)
		map[i] = AddVideo(aVideoContainer.GetFullVideoName(i));

	return AddedFileResourceMap(SLD_INDEX_VIDEO_NO, std::move(map));
}

/***********************************************************************
* ¬озвращает полное им€ видео файла по глобальному индексу
*
* @param aIndex - глобальный индекс видео
*
* @return полное им€ файла видео или пуста€ строка, если такое видео не найдено
************************************************************************/
wstring CVideoManager::GetFullVideoNameByVideoIndex(UInt32 aIndex) const
{
	// Ќумераци€ видео начинаетс€ с 1
	if (aIndex==0 || (aIndex > GetVideoCount()))
		return std::wstring();

	auto it = m_UniqueVideos.find(aIndex);
	return it != m_UniqueVideos.end() ? it->second : std::wstring();
}

/***********************************************************************
* ƒобавл€ет видео, использу€ полное им€ видео файла
*
* @param aFullVideoName - полное им€ видео файла
*
* @return глобальный индекс добавленного видео файла
************************************************************************/
UInt32 CVideoManager::AddVideo(wstring aFullVideoName)
{
	if (aFullVideoName.empty())
		return SLD_INDEX_VIDEO_NO;

	// ѕровер€ем, может такое видео уже есть
	map<wstring, UInt32>::iterator it = m_Videos.find(aFullVideoName);
	if (it != m_Videos.end())
		return it->second;

	UInt32 imageCRC = -1;

	const MemoryBuffer buf = sld::read_file(aFullVideoName);
	if (buf.size())
	{
		imageCRC = CRC32(buf.data(), static_cast<UInt32>(buf.size()), SDC_CRC32_START_VALUE);

		auto itCRC = m_VideoCRC32.equal_range(imageCRC);

		for (auto it = itCRC.first; it != itCRC.second; ++it)
		{
			const MemoryBuffer buf2 = sld::read_file(it->second);
			if (buf.size() == buf2.size() && memcmp(buf.data(), buf2.data(), buf.size()) == 0)
			{
				const UInt32 index = m_Videos[it->second];
				m_Videos.emplace(aFullVideoName, index);
				return index;
			}
		}

	}


	// »ндекс добавл€емой картинки
	// Ќумераци€ картинок начинаетс€ с 1, 0 используетс€ как признак отсутстви€ картинки
	UInt32 Index = (UInt32)m_UniqueVideos.size() + 1;

	// ƒобавл€ем	
	m_Videos.emplace(aFullVideoName, Index);
	m_UniqueVideos.emplace(Index, aFullVideoName);
	m_VideoCRC32.emplace(imageCRC, aFullVideoName);

	return Index;
}
