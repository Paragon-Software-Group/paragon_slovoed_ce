#include "FileResourceManagement.h"

FileResourceContainer::FileResourceContainer()
{
	/**
	 * We have to special case index 0, as it's used as an "empty"/"invalid" index.
	 * Simply add an empty string to claim it [0], the only "special case" we have
	 * to handle with this is the total size which is 1 less than the underlying
	 * string map.
	 */
	m_files.insert(sld::wstring_ref());
}

/// Возвращает общее количество файлов в контейнере
uint32_t FileResourceContainer::count() const
{
	// see comment inside the ctor for why we substract 1
	return static_cast<uint32_t>(m_files.size() - 1);
}

/// Устанавливает путь к папке с файлами
void FileResourceContainer::setFolder(sld::wstring_ref aPath)
{
	while (aPath.ends_with('\\'))
		aPath.remove_suffix(1);
	m_folder = to_string(aPath);
}

/// Добавляет файл в контейнер по имени
uint32_t FileResourceContainer::addFile(sld::wstring_ref aName)
{
	return aName.empty() ? InvalidIndex : m_files.insert(aName);
}

/// Возвращает полное имя файла по индексу
std::wstring FileResourceContainer::getFullName(uint32_t aIndex) const
{
	sld::wstring_ref name = getName(aIndex);
	if (name.empty())
		return std::wstring();

	while (name.starts_with('\\'))
		name.remove_prefix(1);

	std::wstring fullName = folder();
	if (!fullName.empty())
		fullName.push_back('\\');
	fullName.append(name.data(), name.length());
	return fullName;
}
