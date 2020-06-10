#include "VideoManager.h"

#include "SDC_CRC.h"
#include "Tools.h"

using namespace std;

/***********************************************************************
* ���������� ����� ���������� ����� � ���������� �������
*
* @return ����� ���������� ����� � ���������� �������
************************************************************************/
UInt32 CVideoManager::GetVideoCount(void) const
{
	return (UInt32)m_UniqueVideos.size();
}

/***********************************************************************
* ��������� ��������� � �����
*
* @param aVideoContainer - ������ �� ����������� ������
*
* @return ������� ������������ ���������� ������� ������� �� ���������� ����������� �������
************************************************************************/
AddedFileResourceMap CVideoManager::AddVideoContainer(const CVideoContainer &aVideoContainer)
{
	UInt32 videoCount = aVideoContainer.GetVideoCount();
	if (videoCount == 0)
		return AddedFileResourceMap(SLD_INDEX_VIDEO_NO);

	// ��������� ���������� ������ �����
	std::vector<UInt32> map(videoCount + 1, SLD_INDEX_PICTURE_NO);
	for (UInt32 i = 1; i <= videoCount; i++)
		map[i] = AddVideo(aVideoContainer.GetFullVideoName(i));

	return AddedFileResourceMap(SLD_INDEX_VIDEO_NO, std::move(map));
}

/***********************************************************************
* ���������� ������ ��� ����� ����� �� ����������� �������
*
* @param aIndex - ���������� ������ �����
*
* @return ������ ��� ����� ����� ��� ������ ������, ���� ����� ����� �� �������
************************************************************************/
wstring CVideoManager::GetFullVideoNameByVideoIndex(UInt32 aIndex) const
{
	// ��������� ����� ���������� � 1
	if (aIndex==0 || (aIndex > GetVideoCount()))
		return std::wstring();

	auto it = m_UniqueVideos.find(aIndex);
	return it != m_UniqueVideos.end() ? it->second : std::wstring();
}

/***********************************************************************
* ��������� �����, ��������� ������ ��� ����� �����
*
* @param aFullVideoName - ������ ��� ����� �����
*
* @return ���������� ������ ������������ ����� �����
************************************************************************/
UInt32 CVideoManager::AddVideo(wstring aFullVideoName)
{
	if (aFullVideoName.empty())
		return SLD_INDEX_VIDEO_NO;

	// ���������, ����� ����� ����� ��� ����
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


	// ������ ����������� ��������
	// ��������� �������� ���������� � 1, 0 ������������ ��� ������� ���������� ��������
	UInt32 Index = (UInt32)m_UniqueVideos.size() + 1;

	// ���������	
	m_Videos.emplace(aFullVideoName, Index);
	m_UniqueVideos.emplace(Index, aFullVideoName);
	m_VideoCRC32.emplace(imageCRC, aFullVideoName);

	return Index;
}
