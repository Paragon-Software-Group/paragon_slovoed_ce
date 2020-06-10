#ifndef _SDC_READ_H_
#define _SDC_READ_H_

#ifndef _CRT_SECURE_NO_WARNINGS
	#define _CRT_SECURE_NO_WARNINGS
	#define _CRT_NON_CONFORMING_SWPRINTFS
#endif

#include "Engine/SDC.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>


/// Начальный размер таблицы загруженных ресурсов (в элементах)
#define RESOURCE_TABLE_START_SIZE	(128)

/// Размер одного блока данных в байтах для вычисления CRC непосредственно данных ресурсов
#define CRC_DATA_BLOCK_SIZE			(0xFFFF)


/// Класс предназначеный для чтения данных из контейнера.
class CSDCRead
{
public:

	/// Конструктор
	CSDCRead(void);
	/// Деструктор
	~CSDCRead(void);

	/// Открываем контейнер.
	SDCError Open(const Int8 *fileName);

	/// Закрываем контейнер.
	void Close(void);
	
	/// Возвращает тип содержимого в контейнере
	UInt32 GetDatabaseType(void);

	/// Возвращает количество ресурсов в открытом контейнере.
	UInt32 GetNumberOfResources(void);

	/// Получает ресурс по его номеру
	SDCError GetResource(UInt32 index, const void **ptr, UInt32 *size, UInt32 *res_type, UInt32 *res_index);

	/// Получаем ресурс по его типу и номеру среди других ресурсов данного типа.
	SDCError GetResourceByTypeIndex(UInt32 type, UInt32 index, const void **ptr, UInt32 *size);

	/// Получаем тип и номер ресурса по его номеру.
	SDCError GetTypeIndex(UInt32 index, UInt32 *res_type, UInt32 *res_index);

	/// Проверяем целостность контейнера.
	SDCError CheckData(void);
	
	/// Закрывает ресурс
	SDCError CloseResource(const void* aPtr);

private:

	/// Открытый файл контейнера
	FILE* m_in;

	/// Заголовок контейнера
	SlovoEdContainerHeader m_Header;

	/// Таблица расположения ресурсов
	SlovoEdContainerResourcePosition* m_resTable;

	/// Таблица ресурсов, уже загруженных в память
	ResourceMemType* m_loadedTable;
	
	/// Размер в элементах таблицы загруженных ресурсов
	UInt32 m_MaxLoadedTableElementsCount;
	
	/// Количество элементов в таблице загруженных ресурсов
	UInt32 m_LoadedTableElementsCount;
	
};

#endif // _SDC_READ_H_
