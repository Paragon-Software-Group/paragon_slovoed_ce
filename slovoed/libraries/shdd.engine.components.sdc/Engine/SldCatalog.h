#ifndef _SLD_CATALOG_H_
#define _SLD_CATALOG_H_

#include "SldError.h"
#include "SldSDCReadMy.h"

/// Максимальный размер ресурса. На самом деле все ресурсы кроме последнего должны быть этого размера.
#define SLD_CATALOG_RESOURCE_SIZE	(32*1024)

#define SLD_CATALOG_NUMBER_OF_ELEMENTS	(SLD_CATALOG_RESOURCE_SIZE/sizeof(THierarchyElement))

/// Элемент кэша
struct TResourceCacheItem
{
	/// Индекс ресура
	UInt32 resourceIndex;
	/// Индекс первого и последнего элемента в данном ресурсе
	UInt32 startIndex, endIndex;
};

/// Структура отвечающая за кэширование номеров ресурсов
struct TResourceCache
{
	TResourceCache() : items(NULL), count(0), capacity(0) {}
	~TResourceCache() { if (items) sldMemFree(items); }

	/// Возвращает индекс ресурса для заданного глобального элемента
	ESldError FindResourceIndex(UInt32 aIndex, UInt32 *aResourceIndex) const;

	/// Добавляет новую запись в кэш
	ESldError PushResource(UInt32 aResourceIndex, UInt32 aStartIndex, UInt32 aEndIndex);

	/// Элементы кэша
	TResourceCacheItem *items;
	/// Количество элементов в кэше
	UInt32 count;
	/// Количество элементов кэша под которых выделена память
	UInt32 capacity;
};


/// Класс отвечающий за работу с иерархией списка слов.
class CSldCatalog
{
public:
	/// Конструктор
	CSldCatalog(void):
		m_data(NULL),
		m_CatalogType(0),
		m_CurrentResource(MAX_UINT_VALUE)
		{
			sldMemZero(&m_LevelHeader, sizeof(m_LevelHeader));
		};

	/// Конструктор копирования
	CSldCatalog(const CSldCatalog &aOld) = delete;
	CSldCatalog& operator=(const CSldCatalog &aOld) = delete;

	/// Производим инициализацию класса доступа к каталогу
	ESldError Init(CSDCReadMy &aData, UInt32 aCatalogType);

	/// Возвращает количество слов имеющихся в текущем уровне иерархии
	/// (не включая подуровни). 
	ESldError GetNumberOfWords(Int32 *aNumberOfWords);

	/// Возвращает количество слов в корне списка
	ESldError GetNumberOfWordsInRoot(Int32 *aNumberOfWords);

	/// Возвращает глобальный индекс базовой статьи
	/** Если MAX_UINT_VALUE, тогда указанный элемент является конечным
		и перейти по нему нельзя.
	*/
	ESldError GetBaseByIndex(UInt32 aIndex, UInt32 *aBase);

	/// Устанавливает новый базовый уровень.
	ESldError SetBaseByIndex(UInt32 aIndex);

	/// Производит переход но новый уровень в соответствии со смещением
	ESldError GoToByShift(UInt32 aShift);

	/// Возвращает путь в каталоге к элементу с глобальным номером(т.е. "прямой" номер слова без учета иерархии)
	ESldError GetPathByGlobalIndex(UInt32 aIndex, TCatalogPath *aPath);

	/// Проверяет сортированность текущего уровня иерархии
	UInt8 IsLevelSorted() { return m_LevelHeader.IsSortedLevel; }

	/// Возвращает тип для текущего уровня иерархии
	EHierarchyLevelType GetLevelType() { return (EHierarchyLevelType)m_LevelHeader.LevelType; }

private:
	/// Возвращает указатель на запрашиваемый элемент
	ESldError GetElementPtr(UInt32 aIndex, const THierarchyElement **aElement);

	/// Возвращает индекс ресурса для запрашиваемого элемента (в случае уровней не влезающих в один ресурс)
	ESldError FindResourceIndexCached(UInt32 aIndex, UInt32 *aResourceIndex);

	/// Загружает новый ресурс в "текущий"
	ESldError LoadCurrentResource(UInt32 aResourceIndex);

	/// Обновляет кэш ресурсов для текущего уровня иерархии
	ESldError UpdateResourceCache();

private:
	/// класс отвечающий за получение данных из контейнера
	CSDCReadMy				*m_data;

	/// тип ресурса в котором хранятся данные иерархии.
	UInt32					m_CatalogType;

	/// индекс текущего ресурса
	UInt32					m_CurrentResource;

	/// Ресурс с загруженной иерархией.
	CSDCReadMy::Resource	m_res;

	/// Указатель на заголовок текущего уровня.
	THierarchyLevelHeader	m_LevelHeader;

	/// номер элемента который является началом массива элементов текущего уровня
	UInt32					m_startIndex;

	/// номер элемента который является концом массива элементов текущего уровня
	UInt32					m_endIndex;

	/// номер ресурса который является первым для текущего уровня
	UInt32					m_startResource;

	/// номер ресурса который является последним для текущего уровня
	UInt32					m_endResource;

	/// Версия словарной базы
	UInt32					m_EngineVersion;

	/// Кэш индексов ресурсов
	TResourceCache			m_resourceCache;
};

#endif