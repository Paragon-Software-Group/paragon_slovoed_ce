#pragma once
#include "sld_Types.h"

#include <utility>
#include <vector>

/// ¬озвращает €вл€етс€ ли путь директорией, второе значение "пары" - возможна€ ошибка
std::pair<bool, int> IsDirectory(std::wstring aPath);

/// —коректировать путь. ѕод линукс могут быть проблемы с конструкци€ми типа "Media/1.mesh", нужно указать или полный путь или ./ плюс может быть нужно развернуть слеши в нужную сторону
void CorrectFilePathA(std::string& aPath);
	
/// —коректировать путь. ѕод линукс могут быть проблемы с конструкци€ми типа "Media/1.mesh", нужно указать или полный путь или ./ плюс может быть нужно развернуть слеши в нужную сторону
void CorrectFilePathW(std::wstring& aPath);

/// ѕолучает список файлов в директории
void GetFilesInDirectory(std::vector<std::string> &aOut, std::string aDirectory);

namespace sld {

// ќткрывает файл
FILE* fopen(std::wstring aPath, const wchar_t *aMode);

} // namespace sld
