#pragma once

#include "FileResourceManagement.h"
#include "sld2_Error.h"

/// Класс для хранения озвучки
class CSoundContainer
{
public:
	/// Конструктор
	CSoundContainer() : m_SpeexPackMode(false) {}

	/// Возвращает общее количество озвучек в контейнере
	UInt32 GetSoundCount() const { return c.count(); }

	/// Устанавливает путь к папке со звуком или имя файла контейнера озвучек
	int SetSoundFolder(std::wstring aPath);
	/// Возвращает путь к папке со звуком или имя файла контейнера озвучек
	std::wstring GetSoundFolder() const { return c.folder(); };

	/// Добавляет озвучку в контейнер по имени файла
	UInt32 AddSound(sld::wstring_ref aSoundName) { return c.addFile(aSoundName); }

	/// Возвращает имя озвучки по индексу
	std::wstring GetSoundNameByIndex(UInt32 aIndex) const { return to_string(c.getName(aIndex)); }

	/// Возвращает полное имя озвучки
	std::wstring GetFullSoundName(UInt32 aIndex) const { return c.getFullName(aIndex); }

	/// Возвращает режим работы с контейнером
	bool GetSpeexPackMode(void) const { return m_SpeexPackMode; };

private:
	FileResourceContainer c;

	/// Режим работы с контейнером (true) / обычными файлами на диске (false)
	bool m_SpeexPackMode;
};
