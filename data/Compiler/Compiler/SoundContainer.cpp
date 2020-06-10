#include "SoundContainer.h"

#include "sld_Platform.h"

/***********************************************************************
* ������������� ���� � ����� �� ������ ��� ��� ����� ���������� �������
*
* @param aPath - ���� � ����� �� ������ ��� ��� ����� ���������� �������
*
* @return ��� ������
************************************************************************/
int CSoundContainer::SetSoundFolder(std::wstring aPath)
{
	c.setFolder(aPath);

	auto isDirectory = IsDirectory(c.folder());
	m_SpeexPackMode = !isDirectory.first;
	return isDirectory.second;
}
