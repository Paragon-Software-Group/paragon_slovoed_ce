#pragma once

#include <map>

#include "SoundContainer.h"

/// —труктура описывает один элемент озвучки
struct TSoundElement
{
	// полное им€ файла озвучки: путь к папке со звуком или им€ файла-контейнера + им€ файла
	// это то же самое, что и ключ в глобальном массиве всех озвучек
	std::wstring FullSoundName;
	// им€ файла озвучки
	// в случае SpeexPackMode, это им€, по которому из контейнера надо запрашивать озвучку
	std::wstring SoundName;
	// брать озвучку из контейнера или нет
	bool SpeexPackMode;
	// им€ файла контейнера озвучек, если SpeexPackMode
	std::wstring SpeexPackFilename;
	// глобальный индекс озвучки
	UInt32 Index;

	// конструктор
	TSoundElement() : SpeexPackMode(false), Index(0) {}
};

///  ласс дл€ управлени€ озвучкой
class CSoundManager
{
public:
	///  онструктор
	CSoundManager() = default;
	/// ƒеструктор
	~CSoundManager(void);

public:

	/// ¬озвращает общее количество озвучек в глобальном массиве
	UInt32 GetSoundCount(void) const;

	/// ƒобавл€ет контейнер с озвучкой
	AddedFileResourceMap AddSoundContainer(const CSoundContainer &aSoundContainer);

	/// ¬озвращает полное им€ файла озвучки по глобальному индексу
	std::wstring GetFullSoundNameBySoundIndex(UInt32 aIndex) const;

	/// ¬озвращает озвучку по глобальному индексу
	const TSoundElement* GetSoundBySoundIndex(UInt32 aIndex) const;

private:

	/// ƒобавл€ет озвучку в глобальный массив
	UInt32 AddSound(std::wstring aFullSoundName, std::wstring aSoundName, bool aSpeexPackMode, std::wstring aSpeexPackFilename);

private:

	/// √лобальный массив озвучек дл€ поиска по полному имени файла озвучки (освобождаем пам€ть только в этом массиве)
	///  люч массива - полное им€ файла озвучки: путь к папке со звуком или им€ файла-контейнера + им€ файла
	std::map<std::wstring, TSoundElement*> m_SoundsByName;

	/// √лобальный массив озвучек дл€ поиска по глобальному индексу озвучки (копи€ массива m_SoundsByName, пам€ть тут освобождать не надо)
	std::map<UInt32, TSoundElement*> m_SoundsByIndex;
};
