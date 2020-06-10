#ifndef _SDC_WRITE_H_
#define _SDC_WRITE_H_

#include "Engine/SDC.h"

#include <cstdio>
#include <string>
#include <vector>

#include "util.h"

/// Класс создания и записи файла в формате SDC.
class CSDCWrite
{
public:

	struct CompressionConfig
	{
		// compression type to be used
		ESDCResourceCompressionType type;
		// minimal resource size to be compressed
		uint32_t minSize;
		// compression threshold - [0.0 .. 1.0]
		double threshold;
	};

	static const CompressionConfig DefaultCompression;
	static const CompressionConfig NoCompression;

	/// Конструктор
	CSDCWrite(void);
	/// Деструктор
	~CSDCWrite(void);

	/// Добавляем ресурс.
	SDCError AddResource(const void *aPtr, UInt32 aSize, UInt32 aType, UInt32 aIndex, const CompressionConfig &aCompression = DefaultCompression);
	/// Добавляем ресурс
	SDCError AddResource(MemoryRef aData, UInt32 aType, UInt32 aIndex, const CompressionConfig &aCompression = DefaultCompression);
	/// Добавляем ресурс.
	SDCError AddResource(MemoryRef aData, UInt32 aType, UInt32 aIndex, UInt32 maxResourceSize, const CompressionConfig &aCompression = DefaultCompression);
	/// Добавляем ресурс
	SDCError AddResource(MemoryBuffer&& aData, UInt32 aType, UInt32 aIndex, const CompressionConfig &aCompression = DefaultCompression);

	/// Добавляем ресурс.
	SDCError AddResource(const std::wstring &fileName, UInt32 aType, UInt32 aIndex);

	/// Сохраняем данные в контейнере SDC
	SDCError Write(const wchar_t *fileName);
	
	/// Устанавливаем идентификатор контейнера.
	void SetDictID(UInt32 DictID);
	/// Устанавливаем тип содержимого контейнера.
	void SetDatabaseType(UInt32 aDatabaseType);
	/// Устанавливаем флаг того, что это демо-база
	void SetInApp(UInt32 aIsInApp);

	/// Структура используемая для хранения данных о ресурсах
	struct Resource
	{
		/// Буфер с данными ресурса
		MemoryBuffer buf;
		/// Тип ресурса
		uint32_t Type;
		/// Номер ресурса
		uint32_t Index;
		/// Размер ресурса в распакованом виде
		uint32_t UncompressedSize;
		/// Тип используемой компрессии
		ESDCResourceCompressionType CompressionType;

		// настройки компрессии
		CompressionConfig compressionConfig;

		Resource(uint32_t aType, uint32_t aIndex, MemoryBuffer&& aData)
			: buf(std::move(aData)), Type(aType), Index(aIndex),
			  UncompressedSize(static_cast<uint32_t>(buf.size())),
			  CompressionType(eSDCResourceCompression_None)
		{}
	};

	/// Позволяет полностью изменить анстрйоки сжатия ресурсов
	void OverrideCompressionConfig(const CompressionConfig &aConfig);

private:

	/// Сохраняем данные в открытый файл.
	SDCError Write(FILE *out);
private:

	/// Таблица с уже добавленными ресурсами.
	std::vector<Resource> m_Data;

	/// Идентификатор контейнера.
	UInt32 m_DictID;

	/// Тип содержимого контейнера.
	UInt32 m_DatabaseType;
	/// Флаг того, что это демо-база
	UInt32 m_IsInApp;

	/// Переопределенные настроек сжатия
	bool useOverrideCompressionCfg;
	CompressionConfig overrideCompressionCfg;
};

#endif // _SDC_WRITE_H_
