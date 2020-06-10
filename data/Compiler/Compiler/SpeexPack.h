#ifndef _SPEEX_PACK_H_
#define _SPEEX_PACK_H_

#include "sld_Types.h"
#include <string>
#include <map>
#include <algorithm>

using namespace std;

/// Версия контейнера
#define SPEEX_PACK_CONTAINER_VERSION	1

/// Номер сборки утилиты
#define SPEEX_PACK_APP_BUILD		2

/// Сигнатура файла-контейнера
#define SPEEX_PACK_SIGNATURE	0xAAFFFFAA

/// Максимальная длина строки-ключа в символах для файла контейнера (имя файла), включая завершающий 0
#define MAX_FILE_KEY_SIZE	128


/// Ошибки работы с контейнером
enum ESpeexPackError
{
	spOK = 0,
	
	spMemNullPointer,
	spMemBadAlloc,
	
	spEmptyFileKey,
	spEmptyFile,
	
	// файл в этом случае не добавляется, можно дальше продолжать добавлять новые файлы
	spDuplicatedFileKey,
	
	spToLargeFileKey,
	
	spErrorOpenFile,
	spErrorReadFile,
	spErrorWriteFile,
	
	spEmptyContainer,
	spItIsNotContainer,
	spCorruptedContainer,
	
	spFileNotFound,
	spAddingAnEmptyFile
};


/// Общий заголовок контейнера
typedef struct TSpeexPackContainerHeader
{
	/// Сигнатура файла-контейнера
	UInt32 Signature;
	/// Размер структуры заголовка
	UInt32 StructSize;
	/// Версия контейнера
	UInt32 Version;
	/// Размер всего файла-контейнера в байтах
	UInt32 FileSize;
	
	/// Смещение до таблицы заголовков файлов
	UInt32 ShiftToFilesTable;
	/// Размер таблицы заголовков файлов в байтах
	UInt32 FilesTableSize;
	/// Количество записей в таблице заголовков файлов (количество файлов в контейнере)
	UInt32 FilesCount;
	
	/// Смещение до начала блока данных всех файлов (данные располагаются непрерывно)
	UInt32 ShiftToData;
	/// Полный размер блока данных всех файлов
	UInt32 DataSize;
	
	/// Зарезервировано
	UInt32 Reserved[16];
	
} TSpeexPackContainerHeader;


/// Заголовок, описывающий один файл в контейнере
/// Элемент таблицы заголовков файлов
typedef struct TSpeexPackFileHeader
{
	/// Размер структуры заголовка
	UInt32 StructSize;
	/// Id файла
	UInt32 Id;
	/// Смещение от начала блока данных
	UInt32 Shift;
	/// Размер файла
	UInt32 Size;
	/// Флаги
	UInt32 Flags;
	/// Зарезервировано
	UInt32 Reserved[3];
	/// Юникодная строка-ключ файла
	UInt16 Key[MAX_FILE_KEY_SIZE];
	
} TSpeexPackFileHeader;


/// Структура, представляющая один файл в контейнере
typedef struct TSpeexPackFile
{
	/// Заголовок
	TSpeexPackFileHeader*	Header;
	/// Указатель на загруженные данные
	UInt8*					Data;
	
} TSpeexPackFile;


/// Класс для работы с контейнером файлов
/// Создание и сохранение контейнера в файл
class CSpeexPackWrite
{
public:
	
	/// Конструктор
	CSpeexPackWrite(void);
	/// Деструктор
	~CSpeexPackWrite(void);

public:

	/// Загружает существующий контейнер для изменения (добавления, удаления, замены файлов)
	UInt32 Load(const wchar_t* aFilename);
	
	/// Закрытие контейнера без сохранения
	void Close(void);

	/// Сохраняет контейнер в файл
	UInt32 Save(const wchar_t* aFilename);

	/// Добавляет файл в контейнер (файл читается с диска)
	UInt32 AddFile(const wchar_t* aFilename, const wchar_t* aFileKey = NULL);
	/// Добавляет файл в контейнер (файл уже загружен в память)
	UInt32 AddFile(const wchar_t* aFilename, const UInt8* aData, UInt32 aDataSize);

private:

	/// Очистка
	void Clear(void);

private:

	/// Файлы контейнера
	map<wstring, TSpeexPackFile> m_Data;

	/// Общее количество добавленных файлов
	UInt32 m_TotalFilesCount;
		
	/// Общий полный размер добавленных файлов
	UInt32 m_TotalDataSize;
	
};


/// Класс для работы с контейнером файлов
/// Чтение и извлечение файлов из контейнера
class CSpeexPackRead
{
public:
	
	/// Конструктор
	CSpeexPackRead(void);
	/// Деструктор
	~CSpeexPackRead(void);

public:

	/// Открывает и загружает контейнер
	UInt32 Open(const wchar_t* aFilename);
	
	/// Открыт ли контейнер
	bool IsOpen(void) const;
	
	/// Закрывает контейнер
	void Close(void);

public:

	/// Возвращает номер версии контейнера
	UInt32 GetContainerVersion(void) const;

	/// Возвращает общее количество файлов в контейнере
	UInt32 GetTotalFilesCount(void) const;
	
	/// Возвращает общий размер всех файлов в контейнере
	UInt32 GetTotalFilesSize(void) const;
	
	/// Возвращает данные конкретного файла в контейнере по ключу
	UInt32 GetFileData(const wchar_t* aKey, const UInt8** aData, UInt32* aSize);
	
	/// Возвращает структуру со всеми данными о файле по номеру файла в контейнере
	const TSpeexPackFile* GetFile(UInt32 aIndex);

private:

	/// Очистка
	void Clear(void);

private:

	/// Заголовок контейнера
	TSpeexPackContainerHeader m_Header;
	
	/// Таблица заголовков файлов контейнера
	TSpeexPackFileHeader* m_FilesTable;

	/// Блок данных всех файлов
	UInt8* m_Data;
	
	/// Файлы контейнера (для поиска по ключу)
	map<wstring, TSpeexPackFile> m_Files;
	
};

#endif // _SPEEX_PACK_H_
