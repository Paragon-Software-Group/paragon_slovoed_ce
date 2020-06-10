#pragma once

#include <map>

#include "VideoContainer.h"

/// Класс для управления видео
class CVideoManager
{
public:
	/// Конструктор
	CVideoManager() = default;

public:

	/// Возвращает общее количество видео в глобальном массиве
	UInt32 GetVideoCount(void) const;

	/// Добавляет контейнер с видео
	AddedFileResourceMap AddVideoContainer(const CVideoContainer &aVideoContainer);

	/// Возвращает полное имя файла видео по глобальному индексу
	std::wstring GetFullVideoNameByVideoIndex(UInt32 aIndex) const;

private:

	/// Добавляет видео, используя полное имя файла видео
	UInt32 AddVideo(std::wstring aFullVideoName);

private:

	/// Глобальный массив видео
	/// Содержит полные имена файлов видео и их глобальные индексы
	std::map<std::wstring, UInt32> m_Videos;

	/// Массив уникальных картинок
	std::map<UInt32, std::wstring> m_UniqueVideos;

	/// Массив имен файлов картинок
	std::multimap<UInt32, std::wstring> m_VideoCRC32;
};
