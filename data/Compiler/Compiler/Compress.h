#pragma once

#include <utility>

#include "SDCWrite.h"
//#include "sld_Platform.h"


/// Максимальный размер ресурса с сжатыми данными, если данных больше чем на 1 ресурс, тогда остатки переносятся в следующий ресурс.
#define MAX_COMPRESSED_DATA_RESOURCE_SIZE	(0x8000)

/// Максимальный размер ресурса для таблиц быстрого доступа
#define MAX_QA_DATA_RESOURCE_SIZE			(0xFFFF)

/// Базовая версия листа с точками быстрого доступа
#define VERSION_LIST_BASE					(1)

/// Версия листа с прямым доступом к словам
#define VERSION_LIST_DIRECT_ACCESS			(2)

/// Текущая версия списка слов
#define VERSION_LIST_CURRENT				(VERSION_LIST_DIRECT_ACCESS)

class ThreadPool;
class SlovoEdProject;

/// Класс настроек сжатия словаря
class CompressConfig
{
public:
	/// Переназначает дефолтные настройки сжатия ресурсов
	void setResourceCompression(const CSDCWrite::CompressionConfig &aConfig) {
		m_resourcesCfg = { true, aConfig };
	}

	/// Возвращает настройки сжатия ресурсов
	std::pair<bool, CSDCWrite::CompressionConfig> resourceCompression() const {
		return m_resourcesCfg;
	}

	/// Нужно ли сжимать ресурсы листов
	void setCanCompressListResources(bool state) {
		m_compressListsResources = state;
	}
	bool canCompressListResources() const {
		return m_compressListsResources;
	}

private:
	std::pair<bool, CSDCWrite::CompressionConfig> m_resourcesCfg;
	bool m_compressListsResources = true;
};

/// Производит сжатие и упаковку словаря в контейнер
int Compress(ThreadPool &aThreadPool, SlovoEdProject *aProj, const CompressConfig &aConfig);
