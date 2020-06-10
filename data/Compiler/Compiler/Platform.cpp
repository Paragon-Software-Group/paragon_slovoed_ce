#include "sld_Platform.h"

#ifdef _MSC_VER
#  define NOMINMAX
#  define WIN32_LEAN_AND_MEAN
#  include <tchar.h>
#  include <windows.h>
#  include <atlbase.h>
#  include <conio.h>
#else
#  include <sys/types.h>
#  include <sys/stat.h>
#  include <dirent.h>
#endif

#include "StringFunctions.h"
#include "Tools.h"

/**
 * ¬озвращает €вл€етс€ ли путь директорией
 *
 * @param[in] aPath - путь в файловой системе
 *
 * @return - пара <€вл€етс€ ли путь папкой, ошибка>
 */
std::pair<bool, int> IsDirectory(wstring aPath)
{
#ifndef _LINUX
	// ѕо-умолчанию - каталог
	DWORD Attrs = 0;
	Attrs |= FILE_ATTRIBUTE_DIRECTORY;

	if (!aPath.empty())
	{
		Attrs = GetFileAttributesW(aPath.c_str());
		if (Attrs == INVALID_FILE_ATTRIBUTES)
			return{ false, ERROR_CANT_OPEN_FILE };
	}

	return{ (Attrs & FILE_ATTRIBUTE_DIRECTORY) != 0, ERROR_NO };
#else
	string path = sld::as_utf8(aPath);
	struct stat st;
	if (stat(path.c_str(), &st) == -1)
		return{ false, ERROR_CANT_OPEN_FILE };
	return{ (st.st_mode & S_IFDIR), ERROR_NO };
#endif
}

/** *************************************************************************
* GetFileInDirectory получает список файлов в директории
*
* @param  aOut - вектор с файлами внутри данной директории
*
* @param aDirectory -
*
**************************************************************************/
void GetFilesInDirectory(std::vector<string> &aOut, string aDirectory)
{
	aOut.clear();
#ifndef _LINUX
	HANDLE dir;
	WIN32_FIND_DATAA file_data;

	if ((dir = FindFirstFileA((aDirectory + "/*").c_str(), &file_data)) == INVALID_HANDLE_VALUE)
	{
		// ничего не нашли
		return;
	}

	do
	{
		const string file_name = file_data.cFileName;
		const string full_file_name = aDirectory + "/" + file_name;
		const bool is_directory = (file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

		if (file_name[0] == '.')
			continue;

		if (is_directory)
			continue;

		aOut.push_back(full_file_name);
	} while (FindNextFileA(dir, &file_data));

	FindClose(dir);
#else
	DIR *dir;
	struct dirent *ent;
	struct stat st;

	if (aDirectory[aDirectory.length() - 1] != '/')
	{
		aDirectory += '/';
	}
	dir = opendir(aDirectory.c_str());
	while ((ent = readdir(dir)) != NULL)
	{
		const string file_name = ent->d_name;
		const string full_file_name = aDirectory + file_name;

		if (file_name[0] == '.')
			continue;

		if (stat(full_file_name.c_str(), &st) == -1)
			continue;

		const bool is_directory = (st.st_mode & S_IFDIR) != 0;

		if (is_directory)
			continue;

		aOut.push_back(full_file_name);
	}
	closedir(dir);
#endif
}

#ifdef _LINUX
	int _wcsicmp(const wchar_t* str1, const wchar_t* str2)
	{
		return wcscasecmp(str1, str2);
	}
	
	int _wcsnicmp(const wchar_t* str1, const wchar_t* str2)
	{
		// надо еще проверить, скорее всего здесь будет wmemcmp + доп проверки
		// пока заглушка. 
		return wcscmp(str1, str2); //wmemcmp(str1, str2);
	}

	wchar_t* _wcslwr(wchar_t* aStr)
	{
		::setlocale(LC_ALL,""); 
		for (wchar_t *str = aStr; *str != '\0'; str++)
			*str = towlower(*str);
		return aStr;
	}
#endif

/// —коректировать путь. ѕод линукс могут быть проблемы с конструкци€ми типа "Media/1.mesh", нужно указать или полный путь или ./
void CorrectFilePathA(std::string& aPath)
{
#ifdef _LINUX
	if (aPath[0] != L'.' && aPath[0] != L'/')
	{
		char cCurrentPath[FILENAME_MAX];
		getcwd(cCurrentPath, sizeof(cCurrentPath));
		std::string currentPathA(cCurrentPath);	
		aPath = currentPathA + "/" + aPath;
	}
#endif
}

/// —коректировать путь. ѕод линукс могут быть проблемы с конструкци€ми типа "Media/1.mesh", нужно указать или полный путь или ./
void CorrectFilePathW(std::wstring& aPath)
{
#ifdef _LINUX
	if (aPath[0] != L'.' && aPath[0] != L'/')
	{
		char cCurrentPath[FILENAME_MAX];
		getcwd(cCurrentPath, sizeof(cCurrentPath));
		std::wstring currentPathW = sld::as_wide(cCurrentPath);
		aPath = currentPathW + L"/" + aPath;
	}
#endif
}

/**
 * ќткрывает файл
 *
 * @param[in] aPath  - путь к файлу
 * @param[in] aMode  - mode
 *
 * @return указатель на открытый поток или NULL
 */
FILE* sld::fopen(std::wstring aPath, const wchar_t *aMode)
{
#ifdef _MSC_VER
	UInt16* absolute_path = _wfullpath(nullptr, aPath.c_str(), 0);
	if (absolute_path == nullptr)
		return nullptr;

	std::wstring path = std::wstring(L"\\\\?\\") + absolute_path;
	free(absolute_path);

	return _wfopen(path.c_str(), aMode);
#elif defined(_LINUX)
	str_all_string_replace(aPath, L"\\", L"/");
	str_all_string_replace(aPath, L"//", L"/");

	std::string filenameUtf8 = sld::as_utf8(aPath);
	CorrectFilePathA(filenameUtf8);

	return ::fopen(filenameUtf8.c_str(), sld::as_utf8(aMode).c_str());
#endif
}
