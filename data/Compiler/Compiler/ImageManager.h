#pragma once

#include <map>

#include "ImageContainer.h"

/// Класс для управления картинками
class CImageManager
{
public:
	/// Возвращает общее количество картинок в глобальном массиве
	UInt32 GetImageCount(void) const;

	/// Добавляет контейнер с картинками
	AddedFileResourceMap AddImageContainer(const CImageContainer &aImageContainer);

	/// Возвращает полное имя файла картинки по глобальному индексу
	std::wstring GetFullImageNameByImageIndex(UInt32 aIndex) const;

private:

	/// Добавляет картинку, используя полное имя файла картинки
	UInt32 AddImage(std::wstring aFullImageName);

private:

	/// Глобальный массив картинок
	/// Содержит полные имена файлов картинок и их глобальные индексы
	std::map<std::wstring, UInt32> m_Images;

	/// Массив уникальных картинок
	std::map<UInt32, std::wstring> m_UniqueImages;

	/// Массив имен файлов картинок
	std::multimap<UInt32, std::wstring> m_ImageCRC32;
};
