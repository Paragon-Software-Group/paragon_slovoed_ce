#pragma once

#include <map>

#include "VideoContainer.h"

/// ����� ��� ���������� �����
class CVideoManager
{
public:
	/// �����������
	CVideoManager() = default;

public:

	/// ���������� ����� ���������� ����� � ���������� �������
	UInt32 GetVideoCount(void) const;

	/// ��������� ��������� � �����
	AddedFileResourceMap AddVideoContainer(const CVideoContainer &aVideoContainer);

	/// ���������� ������ ��� ����� ����� �� ����������� �������
	std::wstring GetFullVideoNameByVideoIndex(UInt32 aIndex) const;

private:

	/// ��������� �����, ��������� ������ ��� ����� �����
	UInt32 AddVideo(std::wstring aFullVideoName);

private:

	/// ���������� ������ �����
	/// �������� ������ ����� ������ ����� � �� ���������� �������
	std::map<std::wstring, UInt32> m_Videos;

	/// ������ ���������� ��������
	std::map<UInt32, std::wstring> m_UniqueVideos;

	/// ������ ���� ������ ��������
	std::multimap<UInt32, std::wstring> m_VideoCRC32;
};
