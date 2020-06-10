#pragma once

#include <cstddef>
#include <cstdint>
#include <deque>
#include <vector>

#include "string_ref.h"
#include "Tools.h"

class FileResourceContainer
{
public:
	enum : uint32_t { InvalidIndex = 0 };

	FileResourceContainer();

	FileResourceContainer(const FileResourceContainer&) = delete;
	FileResourceContainer& operator=(const FileResourceContainer&) = delete;
	FileResourceContainer(FileResourceContainer&&) = default;
	FileResourceContainer& operator=(FileResourceContainer&&) = default;

	/// Возвращает общее количество файлов в контейнере
	uint32_t count() const;

	/// Устанавливает путь к папке с файлами
	void setFolder(sld::wstring_ref aPath);
	/// Возвращает путь к папке с файлами
	const std::wstring& folder() const { return m_folder; }

	/// Добавляет файл в контейнер по имени
	uint32_t addFile(sld::wstring_ref aName);

	/// Возвращает имя файла по индексу
	sld::wstring_ref getName(uint32_t aIndex) const {
		return m_files.get(aIndex);
	}

	/// Возвращает индекс файла по имени
	uint32_t getIndex(sld::wstring_ref aName) const {
		const uint32_t index = m_files.get(aName);
		return index == ~0u ? InvalidIndex : index;
	}

	/// Возвращает полное имя файла по индексу
	std::wstring getFullName(uint32_t aIndex) const;

private:
	// путь к папке с файлами
	std::wstring m_folder;
	// имена файлов
	sld::StringRefMap m_files;
};

// a mapping table generated during file resource container addition
// XXX: better names welcome
class AddedFileResourceMap
{
public:
	AddedFileResourceMap()
		: invalidIndex_(~0u) {}

	explicit AddedFileResourceMap(uint32_t invalidIndex)
		: invalidIndex_(invalidIndex) {}

	AddedFileResourceMap(uint32_t invalidIndex, std::vector<uint32_t>&& data)
		: invalidIndex_(invalidIndex), data_(std::move(data)) {}

	uint32_t invalidIndex() const { return invalidIndex_; }

	size_t size() const { return data_.size(); }
	bool empty() const { return data_.empty(); }

	uint32_t operator[](uint32_t aIndex) const {
		return aIndex < data_.size() ? data_[aIndex] : invalidIndex_;
	}
private:
	uint32_t invalidIndex_;
	std::vector<uint32_t> data_;
};
