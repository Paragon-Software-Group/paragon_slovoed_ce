#include "SoundContainer.h"

#include "sld_Platform.h"

/***********************************************************************
* Устанавливает путь к папке со звуком или имя файла контейнера озвучек
*
* @param aPath - путь к папке со звуком или имя файла контейнера озвучек
*
* @return код ошибки
************************************************************************/
int CSoundContainer::SetSoundFolder(std::wstring aPath)
{
	c.setFolder(aPath);

	auto isDirectory = IsDirectory(c.folder());
	m_SpeexPackMode = !isDirectory.first;
	return isDirectory.second;
}
