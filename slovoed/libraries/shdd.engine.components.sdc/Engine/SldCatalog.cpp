#include "SldCatalog.h"

/** ********************************************************************
* Возвращает индекс ресурса для заданного глобального элемента
*
* @param[in]   aIndex		   - глобальный индекс элемента
* @param[out]  aResourceIndex - индекс ресурса
*
* @return код ошибки:
*    eMemoryNullPointer - aResourceIndex никуда не указывает
*    eCommonWrongIndex  - ресурс для данного индекса не содержится в кэше
*    eOK                - найден ресурс для заданного индекса
************************************************************************/
ESldError TResourceCache::FindResourceIndex(UInt32 aIndex, UInt32 *aResourceIndex) const
{
	if (!aResourceIndex)
		return eMemoryNullPointer;

	for (int i = 0; i < count; i++)
	{
		const TResourceCacheItem *item = &items[i];
		if (item->startIndex <= aIndex && item->endIndex >= aIndex)
		{
			*aResourceIndex = item->resourceIndex;
			return eOK;
		}
	}

	return eCommonWrongIndex;
}

/// Возвращает размер "массива" для заданного количества элементов
static UInt32 GetNewCapacity(UInt32 need)
{
	UInt32 newCapacity;

	// taken from Python's list
	newCapacity = (need >> 6) + (need < 9 ? 3 : 6);
	newCapacity += need;

	return newCapacity;
}

/// Удостоверяется что в кэше есть достаточно места для N новых записей
static ESldError ResourceCacheEnsureCapacity(TResourceCache *aResourceCache, UInt32 aAdditionalItemCount)
{
	UInt32 newCount = aResourceCache->count + aAdditionalItemCount;
	if (newCount <= aResourceCache->capacity)
		return eOK;

	UInt32 newCapacity = GetNewCapacity(newCount);

	auto newItems = sldMemReallocT(aResourceCache->items, newCapacity);
	if (!newItems)
		return eMemoryNotEnoughMemory;

	aResourceCache->items = newItems;
	aResourceCache->capacity = newCapacity;

	return eOK;
}

/** ********************************************************************
* Добавляет новую запись в кэш
*
* @param[in]  aResourceIndex - индекс ресурса
* @param[in]  aStartIndex    - глобальный индекс первого элемента в ресурсе
* @param[in]  aEndIndex      - глобальный индекс последнего элемента в ресурсе
*
* @return код ошибки
************************************************************************/
ESldError TResourceCache::PushResource(UInt32 aResourceIndex, UInt32 aStartIndex, UInt32 aEndIndex)
{
	for (int i = 0; i < count; i++)
	{
		TResourceCacheItem *item = &items[i];
		// Update the resource if we already have a record for it
		if (item->resourceIndex == aResourceIndex)
		{
			if (aStartIndex < item->startIndex)
				item->startIndex = aStartIndex;
			if (aEndIndex > item->endIndex)
				item->endIndex = aEndIndex;
			return eOK;
		}
	}

	ESldError err = ResourceCacheEnsureCapacity(this, 1);
	if (err != eOK)
		return err;

	TResourceCacheItem *item = &items[count++];
	item->resourceIndex = aResourceIndex;
	item->startIndex = aStartIndex;
	item->endIndex = aEndIndex;

	return eOK;
}

/// Возвращает указатель на элемент под заданным индексом
static inline const THierarchyElement* GetElement(const CSDCReadMy::Resource &aResource, UInt32 aIndex)
{
	return ((const THierarchyElement *)aResource.ptr()) + aIndex;
}

/** ********************************************************************
* Производим инициализацию класса доступа к каталогу
*
* @param[in]	aData			- 
* @param[in]	aCatalogType	- 
*
* @return код ошибки
************************************************************************/
ESldError CSldCatalog::Init(CSDCReadMy &aData, UInt32 aCatalogType)
{
	if (!aCatalogType)
		return eCommonWrongCatalogType;
	
	m_data = &aData;
	m_CatalogType = aCatalogType;
	m_CurrentResource = 0;

	auto res = m_data->GetResource(SLD_RESOURCE_HEADER, 0);
	if (res != eOK)
		return res.error();

	TDictionaryHeader* tmpDictHeader = (TDictionaryHeader*)res.ptr();
	if (tmpDictHeader->HeaderSize != res.size())
		return eInputWrongStructSize;

	m_EngineVersion = tmpDictHeader->Version;

	return GoToByShift(0);
}

/** *********************************************************************
* Возвращает количество слов имеющихся в текущем уровне иерархии(не включая 
* подуровни). 
*
* @param[out]	aNumberOfWords	- указатель на переменную в которую нужно записать
*								  количество слов в текущем уровне иерархии.
*
* @return код ошибки
************************************************************************/
ESldError CSldCatalog::GetNumberOfWords(Int32 *aNumberOfWords)
{
	if (!aNumberOfWords)
		return eMemoryNullPointer;
	
	*aNumberOfWords = m_LevelHeader.NumberOfWords;
	return eOK;
}

/** *********************************************************************
* Возвращает количество слов в корне списка 
*
* @param[out]	aNumberOfWords	- указатель на переменную в которую нужно записать
*								  количество слов на корневом уровне иерархии.
*
* @return код ошибки
************************************************************************/
ESldError CSldCatalog::GetNumberOfWordsInRoot(Int32 *aNumberOfWords)
{
	if (!aNumberOfWords)
		return eMemoryNullPointer;


	if(m_LevelHeader.GlobalShift == 0)
	{
		*aNumberOfWords = m_LevelHeader.NumberOfWords;
	}
	else if(m_CurrentResource == 0)
	{
		THierarchyLevelHeader levelHeader = *((THierarchyLevelHeader*)m_res.ptr());
		*aNumberOfWords = levelHeader.NumberOfWords;
	}
	else
	{
		auto res = m_data->GetResource(m_CatalogType, 0);
		if (res != eOK)
			return res.error();

		THierarchyLevelHeader levelHeader = *((THierarchyLevelHeader*)res.ptr());
		*aNumberOfWords = levelHeader.NumberOfWords;
	}

	return eOK;
}

/** *********************************************************************
* Возвращает номер базовой статьи в глобальном списке слов.
*
* @param[in]	aIndex	- номер слова в текущем уровне иерархии.
* @param[out]	aBase	- указатель на переменную в которую нужно записать номер слова 
*						  глобального списка в котором будет начало нового подуровня.
*			  			  Если MAX_UINT_VALUE, тогда указанный элемент является конечным
*						  и перейти по нему нельзя.
*
* @return код ошибки
************************************************************************/
ESldError CSldCatalog::GetBaseByIndex(UInt32 aIndex, UInt32 *aBase)
{
	if (aIndex >= m_LevelHeader.NumberOfWords)
		return eCommonWrongIndex;
	if (!aBase)
		return eMemoryNullPointer;

	ESldError error;
	const THierarchyElement *element;
	error = GetElementPtr(aIndex, &element);
	if (error != eOK)
		return error;
	*aBase = element->NextLevelGlobalIndex;

	return eOK;
}

/** *********************************************************************
*  Устанавливает новый базовый уровень.
*
* @param[in]	aIndex	- номер слова в текущем уровне иерархии.
*
* @return код ошибки
************************************************************************/
ESldError CSldCatalog::SetBaseByIndex(UInt32 aIndex)
{
	ESldError error;
	const THierarchyElement *element;

	// Если нужно сбросить на начало иерархии
	if (aIndex == MAX_UINT_VALUE)
	{
		error = GoToByShift(0);
		if (error != eOK)
			return error;

		return eOK;
	}

	if (aIndex >= m_LevelHeader.NumberOfWords)
		return eCommonWrongIndex;

	error = GetElementPtr(aIndex, &element);
	if (error != eOK)
		return error;

	if (element->NextLevelGlobalIndex == MAX_UINT_VALUE ||
		element->ShiftToNextLevel == MAX_UINT_VALUE)
		return eCommonLastLevel;

	error = GoToByShift(element->ShiftToNextLevel);
	if (error != eOK)
		return error;

	return eOK;
}

/** ********************************************************************
* Возвращает указатель на запрашиваемый элемент
*
* @param[in]	aIndex		- номер запрашиваемого элемента
* @param[out]	aElement	- по данному указателю будет записан указатель на элемент
*
* @return код ошибки
************************************************************************/
ESldError CSldCatalog::GetElementPtr(UInt32 aIndex, const THierarchyElement **aElement)
{
	if (aIndex >= m_LevelHeader.NumberOfWords)
		return eCommonWrongIndex;

	ESldError error;

	// Вычисляем индекс с учетом глобального смещения.
	aIndex += m_LevelHeader.GlobalShift;

	if (m_startResource != m_endResource)
	{
		// Для случая, когда текущий уровень не поместился полностью в один ресурс - 
		// ищем ресурс в котором может находится нужный нам элемент
		UInt32 resourceIndex;
		error = FindResourceIndexCached(aIndex, &resourceIndex);
		if (error != eOK)
			return error;

		error = LoadCurrentResource(resourceIndex);
		if (error != eOK)
			return error;
	}
	else if (m_CurrentResource != m_startResource)
	{
		// Если текущий ресурс не относится к данным текущего уровня, тогда загружаем
		// данные текущего уровня.
		error = LoadCurrentResource(m_startResource);
		if (error != eOK)
			return error;
	}

	UInt32 low = m_CurrentResource == m_startResource ? m_startIndex : 0;
	UInt32 hi = m_CurrentResource == m_endResource ? m_endIndex : SLD_CATALOG_NUMBER_OF_ELEMENTS;

	// Ищем в текущем ресурсе нужный элемент
	while (hi - low > 1)
	{
		UInt32 med = (hi + low) >> 1;
		const THierarchyElement *element = GetElement(m_res, med);

		if (aIndex >= element->BeginIndex && aIndex <= element->EndIndex)
		{
			low = med;
			break;
		}

		if (element->EndIndex < aIndex)
		{
			low = med;
		}
		else
		{
			hi = med;
		}
	}

	*aElement = GetElement(m_res, low);

	return eOK;
}

/** *********************************************************************
* Производит переход на новый уровень в соответствии со смещением
*
* @param[in]	aShift	- смещение в байтах относительно начала всех данных иерархии
*
* @return код ошибки
************************************************************************/
ESldError CSldCatalog::GoToByShift(UInt32 aShift)
{
	// Номер нужного нам ресурса
	UInt32 RequestResourceNumber = aShift/SLD_CATALOG_RESOURCE_SIZE;
	
	// Загружаем ресурс, только если он еще не загружен
	if (m_res.empty() || RequestResourceNumber != m_CurrentResource)
	{
		auto res = m_data->GetResource(m_CatalogType, RequestResourceNumber);
		if (res != eOK)
			return res.error();
		m_res = res.resource();
		m_CurrentResource = RequestResourceNumber;
	}

	UInt32 HeaderStartPos = aShift%SLD_CATALOG_RESOURCE_SIZE;

	// В 112 версии ядра размер хидера был изменен (с 16 до 32 байт)
	UInt32 hierarchyLevelHeaderSize = sizeof(THierarchyLevelHeader);
	if (m_EngineVersion < ENGINE_VERSION_HIERARCHY_LEVEL_HEADER)
	{
		hierarchyLevelHeaderSize = sizeof(UInt32) * 4;
	}

	// Если заголовок уровня полностью поместился в ресурс
	if (HeaderStartPos + hierarchyLevelHeaderSize <= m_res.size())
	{
		// тогда считываем его полностью
		sldMemMove(&m_LevelHeader, m_res.ptr() + HeaderStartPos, hierarchyLevelHeaderSize);

		m_startResource = m_CurrentResource;
		m_startIndex = (HeaderStartPos + hierarchyLevelHeaderSize) / sizeof(THierarchyElement);
	}else
	{
		// копируем начало заголовка
		UInt32 count = m_res.size() - HeaderStartPos;
		sldMemMove(&m_LevelHeader, m_res.ptr() + HeaderStartPos, count);

		m_CurrentResource++;
		auto res = m_data->GetResource(m_CatalogType, m_CurrentResource);
		if (res != eOK)
			return res.error();
		m_res = res.resource();

		// копируем остатки заголовка
		sldMemMove(((UInt8 *)&m_LevelHeader) + count, m_res.ptr(), hierarchyLevelHeaderSize - count);
		m_startResource = m_CurrentResource;
		m_startIndex = (hierarchyLevelHeaderSize - count) / sizeof(THierarchyElement);
	}

	// Вычислили позицию в элементах!
	UInt32 index = m_startResource*SLD_CATALOG_NUMBER_OF_ELEMENTS;
	index += m_startIndex;
	index += m_LevelHeader.NumberOfElements;

	m_endIndex = index % SLD_CATALOG_NUMBER_OF_ELEMENTS;
	m_endResource = index / SLD_CATALOG_NUMBER_OF_ELEMENTS;
	// Если элемент последний в ресурсе его индекс не равен нулю
	if(!m_endIndex && m_endResource)
	{
		m_endIndex = SLD_CATALOG_NUMBER_OF_ELEMENTS;
		m_endResource = m_CurrentResource;
	}
	// Иначе элемент находится в следующем ресурсе
	else if(m_CurrentResource != m_endResource)
	{
		m_CurrentResource = m_endResource;
		auto res = m_data->GetResource(m_CatalogType, m_CurrentResource);
		if (res != eOK)
			return res.error();
		m_res = res.resource();
	}
	return eOK;
}

/** ********************************************************************
* Возвращает путь в каталоге к элементу с глобальным номером(т.е. 
* "прямой" номер слова без учета иерархии)
*
* @param[in]	aIndex	- номер слова без учета иерархии
* @param[out]	aPath	- указатель на структуру, в которую будет прописан путь к 
*						  указанному элементу списка слов
*
* @return код ошибки
************************************************************************/
ESldError CSldCatalog::GetPathByGlobalIndex(UInt32 aIndex, TCatalogPath *aPath)
{
	ESldError error;
	
	// Сохраняем текущее состояние
	THierarchyLevelHeader tmpLevelHeader = m_LevelHeader;
	UInt32 tmpStartIndex = m_startIndex;
	UInt32 tmpEndIndex = m_endIndex;
	UInt32 tmpStartResource = m_startResource;
	UInt32 tmpEndResource = m_endResource;
	
	const THierarchyElement *element;

	if (!aPath)
		return eMemoryNullPointer;

	aPath->Clear();

	// Перемещаемся в начало и начинаем искать нужный нам элемент
	error = GoToByShift(0);
	if (error != eOK)
		return error;

	error = GetElementPtr(0, &element);
	if (error != eOK)
		return error;

	UInt32 startPos = 0;

	while (true)
	{
		if (aIndex >= startPos && aIndex < startPos + m_LevelHeader.NumberOfWords)
		{
			// Нас интересует номер слова, а не элемент.
			error = aPath->PushList(aIndex - startPos);
			if (error != eOK)
				return error;

			// Восстанавливаем прежний уровень
			m_LevelHeader = tmpLevelHeader;
			m_startIndex = tmpStartIndex;
			m_endIndex = tmpEndIndex;
			m_startResource = tmpStartResource;
			m_endResource = tmpEndResource;

			return eOK;
		}


		UInt32 low = 0;
		UInt32 med, med_prev;
		UInt32 hi = m_LevelHeader.NumberOfWords;

		while (hi - low > 1)
		{
			med_prev = med = (hi + low) >> 1;
			error = GetElementPtr(med, &element);
			if (error != eOK)
				return error;

			// Если попали на конечный элемент, тогда ищем ссылку на 
			// более глубокий уровень вложенности
			if (element->NextLevelGlobalIndex == MAX_UINT_VALUE)
			{
				med = element->EndIndex + 1 - m_LevelHeader.GlobalShift;
				if (med + 1 <= hi) {
					error = GetElementPtr(med, &element);
					if (error != eOK)
						return error;
				}
			}
			// Если мы в процессе поиска ссылки дошли до самого конца и 
			// ее так и не нашли, тогда границу поиска
			if (element->NextLevelGlobalIndex == MAX_UINT_VALUE && med + 1 >= hi)
			{
				hi = med_prev;
				continue;
			}

			if (element->NextLevelGlobalIndex <= aIndex)
			{
				low = med;
			}
			else
			{
				hi = med_prev;
			}
		}

		error = GetElementPtr(low, &element);
		if (error != eOK)
			return error;
		startPos = element->NextLevelGlobalIndex;

		error = aPath->PushList(element->BeginIndex - m_LevelHeader.GlobalShift);
		if (error != eOK)
			return error;

		error = SetBaseByIndex(low);
		if (error != eOK)
			return error;

		error = GetElementPtr(0, &element);
		if (error != eOK)
			return error;
	}

	// Восстанавливаем прежний уровень
	m_LevelHeader = tmpLevelHeader;
	m_startIndex = tmpStartIndex;
	m_endIndex = tmpEndIndex;
	m_startResource = tmpStartResource;
	m_endResource = tmpEndResource;

	return eOK;
}

/** ********************************************************************
* Возвращает индекс ресурса для запрашиваемого элемента
* Используется только для случаев когда уровень не помещается полностью в один ресурс
*
* @param[in]   aIndex         - глобальный номер запрашиваемого элемента
* @param[out]  aResourceIndex - индекс ресурса
*
* @return код ошибки
************************************************************************/
ESldError CSldCatalog::FindResourceIndexCached(UInt32 aIndex, UInt32 *aResourceIndex)
{
	ESldError error;

	error = m_resourceCache.FindResourceIndex(aIndex, aResourceIndex);
	if (error == eOK)
		return error;

	error = UpdateResourceCache();
	if (error != eOK)
		return error;

	error = m_resourceCache.FindResourceIndex(aIndex, aResourceIndex);
	if (error != eOK)
		return eCommonCatalogSearchError;

	return eOK;
}

/** ********************************************************************
* Загружает новый ресурс в "текущий"
*
* @param[in]  aResourceIndex - индекс ресурса
*
* @return код ошибки
************************************************************************/
ESldError CSldCatalog::LoadCurrentResource(UInt32 aResourceIndex)
{
	if (m_CurrentResource != aResourceIndex)
	{
		m_CurrentResource = aResourceIndex;
		auto res = m_data->GetResource(m_CatalogType, m_CurrentResource);
		if (res != eOK)
			return res.error();
		m_res = res.resource();
	}
	return eOK;
}

/** ********************************************************************
* Обновляет кэш ресурсов для текущего уровня иерархии
*
* @return код ошибки
************************************************************************/
ESldError CSldCatalog::UpdateResourceCache()
{
	ESldError error;

	const UInt32 numResources = m_endResource - m_startResource + 1;
	for (UInt32 i = 0; i < numResources; i++)
	{
		const UInt32 resourceIndex = m_startResource + i;
		const UInt32 startElementIndex = resourceIndex == m_startResource ? m_startIndex : 0;
		const UInt32 endElementIndex = resourceIndex == m_endResource ? m_endIndex : SLD_CATALOG_NUMBER_OF_ELEMENTS - 1;

		// skip adding resources where the hierarchy level header is contained inside one
		// resource right at the "end" of it and all of the hierarchy items for that
		// level are placed inside the next resource
		// (in which case start index is greater than the end index)
		if (startElementIndex > endElementIndex)
			continue;

		auto res = m_data->GetResource(m_CatalogType, resourceIndex);
		if (res != eOK)
			return res.error();

		const THierarchyElement *startElement = GetElement(res.resource(), startElementIndex);
		const THierarchyElement *endElement = GetElement(res.resource(), endElementIndex);

		error = m_resourceCache.PushResource(resourceIndex, startElement->BeginIndex, endElement->EndIndex);
		if (error != eOK)
			return error;
	}

	return eOK;
}
