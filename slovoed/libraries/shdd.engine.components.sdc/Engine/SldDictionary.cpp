#include "SldDictionary.h"

#include "SldCSSDataManager.h"
#include "SldDefines.h"
#include "SldMetadataManager.h"
#include "SldStringStore.h"
#include "SldTools.h"


/// Стандартный конструктор
CSldDictionary::CSldDictionary(void)
{
	Clear();
}

/// Деструктор
CSldDictionary::~CSldDictionary(void)
{
	Close();
}

/// Инициализация членов по умолчанию
void CSldDictionary::Clear(void)
{
	m_LayerAccess = NULL;
	m_Header = NULL;
	m_AdditionalInfo = NULL;
	m_Annotation = NULL;
	m_DictionaryVersionInfo = NULL;
	m_ListInfo = NULL;
	m_List = NULL;
	m_ListCount = 0;
	m_PrevListIndex = 0;
	m_ListIndex = 0;
	m_SwitchThematicsBitmap = 0;
	sldMemZero(&m_SoundBuilder, sizeof(m_SoundBuilder));
	m_AtomicInfo = NULL;
	m_MetadataStringsListIndex = SLD_DEFAULT_LIST_INDEX;
	m_cssDataStringsListIndex = SLD_DEFAULT_LIST_INDEX;
}

/** *********************************************************************
* Открывает словарь с указанным идентификатором, проверяет корректность открытой
* базы
*
* @param[in]	aFile			- указатель на открытый файл контейнера с sdc базой
* @param[in]	aLayerAccess	- указатель на класс, организующий доступ к 
*								  внеядерным ресурсам (чтение/сохранение данных),
*								  а также занимающийся сборкой перевода и озвучки
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::Open(ISDCFile *aFile, ISldLayerAccess *aLayerAccess)
{
	ESldError error = LoadBaseHeader(aFile);
	if (error != eOK)
	{
		Close();
		return error;
	}

	m_LayerAccess = aLayerAccess;

	// Проверка, поддерживает ли ядро версию словаря
	if (m_Header->Version < SLOVOED2_MIN_ENGINE_VERSION)
	{
		Close();
		return eCommonTooOldDictionaryVersion;
	}
	if (m_Header->Version > SLOVOED2_MAX_ENGINE_VERSION)
	{
		Close();
		return eCommonTooHighDictionaryVersion;
	}

	//////////////////////////////////////////////////////////////////////////
	// Загружаем дополнительную информацию о базе
	auto res = m_data.GetResource(SLD_RESOURCE_INFO, eInfoIndex_CommonInfo);
	if (res)
	{
		// Выделяем память для структуры с описанием базы
		m_AdditionalInfo = (TAdditionalInfo*)sldMemNewZero(sizeof(*m_AdditionalInfo));
		if (!m_AdditionalInfo)
		{
			Close();
			return eMemoryNotEnoughMemory;
		}

		// Простая проверка на корректность ресурса с описанием базы
		TAdditionalInfo* tmpAdditionalInfo = (TAdditionalInfo*)res.ptr();
		if (tmpAdditionalInfo->StructSize != res.size())
		{
			Close();
			return eInputWrongStructSize;
		}

		// Копируем данные
		sldMemMove(m_AdditionalInfo, tmpAdditionalInfo, tmpAdditionalInfo->StructSize);
	}

	// Загружаем аннотацию к базе
	res = m_data.GetResource(SLD_RESOURCE_INFO, eInfoIndex_Annotation);
	if (res)
	{
		// Выделяем память для аннотации
		m_Annotation = (UInt16*)sldMemNewZero(res.size());
		if (!m_Annotation)
		{
			Close();
			return eMemoryNotEnoughMemory;
		}

		// Копируем данные
		sldMemMove(m_Annotation, res.ptr(), res.size());
	}
	//////////////////////////////////////////////////////////////////////////

	// Выделяем память для информации о версии словарной базы
	m_DictionaryVersionInfo = (TDictionaryVersionInfo*)sldMemNewZero(sizeof(*m_DictionaryVersionInfo));
	if (!m_DictionaryVersionInfo)
	{
		Close();
		return eMemoryNotEnoughMemory;
	}
	
	// Загружаем информацию о версии словарной базы
	res = m_data.GetResource(SLD_RESOURCE_DICTIONARY_VERSION_INFO, 0);
	if (res)
	{
		// Простая проверка на корректность данных
		if (((TDictionaryVersionInfo*)res.ptr())->structSize != res.size())
		{
			Close();
			return eInputWrongStructSize;
		}
		
		// Копируем данные
		sldMemMove(m_DictionaryVersionInfo, res.ptr(), ((TDictionaryVersionInfo*)res.ptr())->structSize);
	}
	
	
	// Инициализируем систему сравнения строчек
	error = m_CMP.Open(m_data, m_Header->NumberOfLanguageSymbolsTable, m_Header->NumberOfLanguageDelimitersSymbolsTable);
	if (error != eOK)
	{
		Close();
		return error;
	}

	// Инициализируем менеджера морфологий
	if (m_Header->HasMorphologyData)
	{
		m_MorphologyManager = sld2::make_unique<CSldMorphology>();
		if (!m_MorphologyManager)
		{
			Close();
			return eMemoryNotEnoughMemory;
		}

		error = m_MorphologyManager->Init(m_data);
		if (error != eOK)
		{
			Close();
			return error;
		}
	}
	
	// Загружаем информацию о списках слов.
	res = m_data.GetResource(SLD_RESOURCE_HEADER, 1);
	if (res != eOK)
	{
		Close();
		return res.error();
	}
	
	// Проверка на корректность заголовков списков слов.
	TListHeader* tmpListHeader = (TListHeader*)res.ptr();
	if ((res.size() % m_Header->NumberOfLists) != 0 || res.size() / tmpListHeader->HeaderSize != m_Header->NumberOfLists)
	{
		Close();
		return eInputWrongStructSize;
	}

	// Выделяем память для массива указателей на информацию о списках слов
	m_ListInfo = sldMemNewZero<CSldListInfo*>(m_Header->NumberOfLists);
	if (!m_ListInfo)
	{
		Close();
		return eMemoryNotEnoughMemory;
	}
	
	// Копируем данные
	for (UInt32 i=0;i<m_Header->NumberOfLists;i++)
	{
		m_ListInfo[i] = sldNew<CSldListInfo>();
		if (!m_ListInfo)
		{
			Close();
			return eMemoryNotEnoughMemory;
		}
			
		error = m_ListInfo[i]->Init(m_data, (TListHeader*)(res.ptr() + tmpListHeader->HeaderSize*i));
		if (error != eOK)
		{
			Close();
			return error;
		}
	}

	// Выделяем память для массива указателей на списки слов
	m_List = sldMemNewZero<ISldList*>(m_Header->NumberOfLists);
	if (!m_List)
	{
		Close();
		return eMemoryNotEnoughMemory;
	}
	m_ListCount = m_Header->NumberOfLists;
	
	// Переводы
	if (m_Header->NumberOfArticles)
	{
		auto articles = sld2::make_unique<CSldArticles>(*this, *m_LayerAccess, m_data, *m_Header, error);
		if (!articles || error != eOK)
		{
			Close();
			return articles ? error : eMemoryNotEnoughMemory;
		}

		m_Articles = sld2::move(articles);
	}

	// Локализованные строчки
	{
		auto strings = sld2::make_unique<CSldLocalizedString>(m_data, m_Header->NumberOfStringsLanguages, error);
		if (!strings || error != eOK)
		{
			Close();
			return strings ? error : eMemoryNotEnoughMemory;
		}

		m_Strings = sld2::move(strings);
	}

	// Загружаем тематики блоков switch
	if (IsDictionaryHasSwitchThematics())
	{
		res = m_data.GetResource(RESOURCE_TYPE_THEMATIC, 0);
		if (res != eOK)
		{
			Close();
			return error;
		}
		// Простая проверка на корректность ресурса
		if (res.size() != sizeof(UInt32))
		{
			Close();
			return eInputWrongStructSize;
		}
		// Копируем данные
		sldMemMove(&m_SwitchThematicsBitmap, res.ptr(), res.size());
	}

	res = m_data.GetResource(SLD_AO_ARTICLE_TABLE, 0);
	if (res && res.size())
	{
		m_AtomicInfo = (TAtomicObjectInfo*)sldMemNew(res.size());
		sldMemMove(m_AtomicInfo, res.ptr(), res.size());
	}

	error = InitRegistration();
	if (error != eOK)
	{
		Close();
		return error;
	}

	// Инициализация менеджера бинарных метаданных
	res = m_data.GetResource(SLD_RESOURCE_STRUCTURED_METADATA_HEADER, 0);
	if (res && res.size())
	{
		auto manager = sld2::make_unique<CSldMetadataManager>(m_data, res.resource(), error);
		if (!manager || error != eOK)
		{
			Close();
			return manager ? error : eMemoryNotEnoughMemory;
		}

		m_MetadataManager = sld2::move(manager);
		// there is no point initializing the list index if there are no structured metadata
		// resources or if we can't load them
		m_MetadataStringsListIndex = FindListIndex(eWordListType_StructuredMetadataStrings);
	}

	// Инициализация менеджера css метаданных
	res = m_data.GetResource(SLD_RESOURCE_CSS_DATA_HEADER, 0);
	if (res && res.size())
	{
		auto manager = sld2::make_unique<CSldCSSDataManager>(*this, m_data, res.resource(), error);
		if (!manager || error != eOK)
		{
			Close();
			return manager ? error : eMemoryNotEnoughMemory;
		}

		m_cssDataManager = sld2::move(manager);
		m_cssDataStringsListIndex = FindListIndex(eWordListType_CSSDataStrings);
	}
	
	// Выбираем метод для декодирования озвучки
	m_SoundBuilder[1] = &ISldLayerAccess::BuildSoundWrong;
	m_SoundBuilder[2] = &ISldLayerAccess::BuildSoundRight;
	SLD_VALIDATE_SECURITY(&m_RegistrationData, &m_RandomSeed, m_SoundBuilder);
	
	return eOK;
}

/** *********************************************************************
* Закрывает словарь и освобождает все ресурсы и память
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::Close(void)
{
	ESldError error;

	if (m_List)
	{
		Int32 Count;
		error = GetNumberOfLists(&Count);
		if (error != eOK)
			return error;
			
		for (Int32 i=0;i<Count;i++)
		{
			if (m_List[i])
			{
				sldDelete(m_List[i]);
				m_List[i] = NULL;
			}
		}
		sldMemFree(m_List);
		m_List = NULL;
		m_ListCount = 0;
	}

	if (m_ListInfo)
	{
		if (m_Header)
		{
			for (UInt32 i=0;i<m_Header->NumberOfLists;i++)
			{
				if (m_ListInfo[i])
				{
					sldDelete(m_ListInfo[i]);
					m_ListInfo[i] = NULL;
				}
			}
		}
		sldMemFree(m_ListInfo);
		m_ListInfo = NULL;
	}

	m_Articles = nullptr;

	error = m_CMP.Close();
	if (error != eOK)
		return error;

	m_Strings = nullptr;
	m_MetadataManager = nullptr;
	m_cssDataManager = nullptr;

	if (m_LayerAccess)
		m_LayerAccess = NULL;

	m_data.Close();

	if (m_Header)
	{
		sldMemFree(m_Header);
		m_Header = NULL;
	}

	if (m_AdditionalInfo)
	{
		sldMemFree(m_AdditionalInfo);
		m_AdditionalInfo = NULL;
	}

	if (m_Annotation)
	{
		sldMemFree(m_Annotation);
		m_Annotation = NULL;
	}
	
	if (m_DictionaryVersionInfo)
	{
		sldMemFree(m_DictionaryVersionInfo);
		m_DictionaryVersionInfo = NULL;
	}

	m_MorphologyManager = nullptr;

	if (m_AtomicInfo)
	{
		sldMemFree(m_AtomicInfo);
	}
	
	Clear();

	return eOK;
}

/** *********************************************************************
* Устанавливает список слов в качестве текущего; таблица сравнения также
* переключается на язык нового списка
*
* @param[in]	aIndex	- номер списка слов
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::SetCurrentWordlist(Int32 aIndex)
{
	ESldError error;
	Int32 ListCount = 0;
	UInt32 LanguageCode = 0;
	UInt32 ListUsage = 0;
	
	error = GetNumberOfLists(&ListCount);
	if (error != eOK)
		return error;
	
	if (aIndex >= ListCount || aIndex < 0)
		return eCommonWrongIndex;

	if (!m_List)
		return eMemoryNullPointer;
	
	// Сохраняем состояние текущего списка слов
	if (m_List[m_ListIndex] && m_List[m_ListIndex]->isInit())
	{
		error = m_List[m_ListIndex]->SaveCurrentState();
		if (error != eOK)
			return error;
	}
	
	// Перед сменой текущего индекса списка слов запоминаем индекс предыдущего списка (если нужно)
	if (m_ListInfo && m_ListInfo[m_ListIndex])
	{
		ListUsage = m_ListInfo[m_ListIndex]->GetUsage();
		if (ListUsage!=eWordListType_Unknown && ListUsage!=eWordListType_RegularSearch && ListUsage != eWordListType_CustomList)
			m_PrevListIndex = m_ListIndex;
	}
	
	// Меняем индекс текущего списка слов
	m_ListIndex = aIndex;
	
	if (!m_List[m_ListIndex])
	{
		auto pList = sld2::make_unique<CSldList>();
		if (!pList)
			return eMemoryNotEnoughMemory;
		
		error = AddList(sld2::move(pList), m_ListIndex);
		if (error != eOK)
			return error;
	}
	
	// Восстанавливаем ранее сохраненное состояние списка слов
	if (m_List[m_ListIndex] && m_List[m_ListIndex]->isInit())
	{
		error = m_List[m_ListIndex]->RestoreState();
		if (error != eOK)
			return error;
	}

	// Если список слов уже был проинициализирован
	if (m_List[m_ListIndex]->isInit())
	{
		// Если мы переключились на непоисковый список - делаем предыдущим его и переключаем таблицу сравнения (у поискового списка нет таблицы сравнения)
		if (m_ListInfo && m_ListInfo[m_ListIndex])
		{
			ListUsage = m_ListInfo[m_ListIndex]->GetUsage();
			if (ListUsage!=eWordListType_Unknown && ListUsage!=eWordListType_RegularSearch && ListUsage != eWordListType_CustomList)
			{
				// Переключаем таблицу сравнения
				error = m_CMP.SetDefaultLanguage(m_ListInfo[m_ListIndex]->GetLanguageFrom());
				if (error != eOK)
					return error;
				
				m_PrevListIndex = m_ListIndex;
			}
		}

		//	Ничего больше делать не нужно
		return eOK;
	}

	// Инициализация
	error = m_List[m_ListIndex]->Init(m_data, GetLayerAccess(), m_ListInfo[m_ListIndex], &m_CMP, m_RegistrationData.HASH);
	if (error != eOK)
		return error;

	// Переключаем таблицу сравнения
	error = m_CMP.SetDefaultLanguage(m_ListInfo[m_ListIndex]->GetLanguageFrom());
	if (error != eOK)
		return error;
	
	// Если мы переключились на непоисковый список - делаем предыдущим его
	if (m_ListInfo && m_ListInfo[m_ListIndex])
	{
		ListUsage = m_ListInfo[m_ListIndex]->GetUsage();
		if (ListUsage!=eWordListType_Unknown && ListUsage!=eWordListType_RegularSearch && ListUsage != eWordListType_CustomList)
			m_PrevListIndex = m_ListIndex;
	}
	
	// Завершаем инициализацию списка слов
	error = m_List[m_ListIndex]->SetBase(MAX_UINT_VALUE);
	if (error != eOK)
		return error;

	error = m_List[m_ListIndex]->SetLocalization(SldLanguage::Default);
	if (error != eOK)
		return error;
	
	Int32 WordsCount = 0;
	error = m_List[m_ListIndex]->GetNumberOfWords(&WordsCount);
	if (error != eOK)
		return error;
	
	if (WordsCount)
	{
		error = m_List[m_ListIndex]->GetWordByIndex(0);
		if (error != eOK)
			return error;
	}

	if(m_ListInfo[m_ListIndex]->GetHeader()->Version > VERSION_LIST_BASE)
	{
		const Int32 simpleSortedListIndex = m_ListInfo[m_ListIndex]->GetSimpleSortedListIndex();
		if(simpleSortedListIndex != SLD_DEFAULT_LIST_INDEX)
		{
			if (!m_List[simpleSortedListIndex])
			{
				auto pList = sld2::make_unique<CSldList>();
				if (!pList)
					return eMemoryNotEnoughMemory;
		
				error = AddList(sld2::move(pList), simpleSortedListIndex);
				if (error != eOK)
					return error;

				error = m_List[simpleSortedListIndex]->Init(m_data, GetLayerAccess(), m_ListInfo[simpleSortedListIndex], &m_CMP, m_RegistrationData.HASH);
				if (error != eOK)
					return error;
			}

			error = m_List[m_ListIndex]->InitSortedList(m_List[simpleSortedListIndex]);
			if (error != eOK)
				return error;
		}
	}
	else if (ListUsage == eWordListType_SimpleSearch && GetDictionaryHeader()->Version > ENGINE_VERSION_HIERARCHY_LEVEL_HEADER)
	{
		const Int32 baseListIndex = m_ListInfo[m_ListIndex]->GetSimpleSortedListIndex();
		if (m_List[baseListIndex])
		{
			error = m_List[baseListIndex]->InitSortedList(m_List[m_ListIndex]);
			if (error != eOK)
				return error;
		}
	}

	return eOK;
}

/** *********************************************************************
* Получает индекс текущего списка слов
*
* @param[out]	aIndex	- указатель на переменную, куда сохраняется индекс
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetCurrentWordList(Int32* aIndex) const
{
	if (!aIndex)
		return eMemoryNullPointer;
		
	ESldError error;
	Int32 ListCount;

	error = GetNumberOfLists(&ListCount);
	if (error != eOK)
		return error;

	if (GetCurrentListIndex() >= ListCount || GetCurrentListIndex() < 0)
		return eCommonWrongIndex;

	*aIndex = GetCurrentListIndex();

	return eOK;
}

/***********************************************************************
* Добавляет список слов
*
* @param[in]	aList		- указатель на список слов для добавления
* @param[in]	aListIndex	- номер, под которым мы хотим добавить список в базу
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::AddList(sld2::UniquePtr<ISldList> aList, Int32 aListIndex)
{
	if (!aList)
		return eMemoryNullPointer;
	
	ESldError error;
	Int32 ListCount;
	
	error = GetNumberOfLists(&ListCount);
	if (error != eOK)
		return error;
	
	if (aListIndex >= ListCount)
	{
		UInt32 NewListCount = aListIndex + 1;
		
		// Реорганизуем массив указателей на списки слов
		ISldList** tmpList = (ISldList**)sldMemNewZero(sizeof(ISldList*)*NewListCount);
		if (!tmpList)
			return eMemoryNotEnoughMemory;

		sldMemMove(tmpList, m_List, sizeof(ISldList*)*ListCount);
		sldMemFree(m_List);
		m_List = tmpList;
		
		// Реорганизуем массив указателей на информацию о списках слов
		CSldListInfo** tmpListInfo = (CSldListInfo**)sldMemNewZero(sizeof(CSldListInfo*)*NewListCount);
		if (!tmpListInfo)
			return eMemoryNotEnoughMemory;

		sldMemMove(tmpListInfo, m_ListInfo, sizeof(CSldListInfo*)*ListCount);
		sldMemFree(m_ListInfo);
		m_ListInfo = tmpListInfo;
		
		m_ListCount = NewListCount;
	}
	
	if (m_List[aListIndex])
		return eCommonWrongIndex;

	if (!m_ListInfo[aListIndex])
	{
		const CSldListInfo* listInfo = NULL;
		error = aList->GetWordListInfo(&listInfo);
		if (error != eOK)
			return error;
		
		if(listInfo->GetUsage() == eWordListType_CustomList)
		{
			m_ListInfo[aListIndex] = (CSldListInfo*)listInfo;
		}
		else
		{
			m_ListInfo[aListIndex] = sldNew<CSldListInfo>();
			if (!m_ListInfo[aListIndex])
				return eMemoryNotEnoughMemory;

			error = m_ListInfo[aListIndex]->Init(m_data, listInfo->GetHeader());
			if (error != eOK)
				return error;
		}
	}

	m_List[aListIndex] = aList.release();

	return eOK;
}

/** *********************************************************************
* Удаляет список слов
*
* @param[in]	aListIndex	- индекс списка, который нужно удалить из словаря
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::RemoveList(Int32 aListIndex)
{
	ESldError error;
	Int32 ListCount;

	error = GetNumberOfLists(&ListCount);
	if (error != eOK)
		return error;
	
	if (aListIndex >= ListCount)
		return eCommonWrongIndex;
	
	if (!m_List[aListIndex])
		return eCommonWrongIndex;
		
	if (!m_ListInfo[aListIndex])
		return eCommonWrongIndex;

	sldDelete(m_List[aListIndex]);
	m_List[aListIndex] = NULL;

	sldDelete(m_ListInfo[aListIndex]);
	m_ListInfo[aListIndex] = NULL;

	if (m_ListCount > (aListIndex + 1))
	{
		sldMemMove(&m_List[aListIndex], &m_List[aListIndex + 1], (m_ListCount - aListIndex - 1) * sizeof(m_List[0]));
		sldMemMove(&m_ListInfo[aListIndex], &m_ListInfo[aListIndex + 1], (m_ListCount - aListIndex - 1) * sizeof(m_ListInfo[0]));
	}

	m_ListCount--;

	return eOK;
}

/** *********************************************************************
* Устанавливает базовую позицию для текущего списка слов,
* относительно которой будут считаться номера слов
*
* @param[in]	aIndex	- номер слова относительно текущей базовой позиции
*						  (MAX_UINT_VALUE - сбросить базу на начало списка слов)
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::SetBase(Int32 aIndex)
{
	return SetBase(GetCurrentListIndex(), aIndex);
}

/** *********************************************************************
* Устанавливает базовую позицию для списка слов по номеру списка,
* относительно которой будут считаться номера слов
*
* @param[in]	aListIndex	- номер списка слов
* @param[in]	aIndex		- номер слова относительно текущей базовой позиции
*							  (MAX_UINT_VALUE - сбросить базу на начало списка слов)
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::SetBase(Int32 aListIndex, Int32 aIndex)
{
	ISldList* pList = 0;
	ESldError error = GetWordList(aListIndex, &pList);
	if (error != eOK)
		return error;
	
	if (!pList)
		return eMemoryNullPointer;
		
	return pList->SetBase(aIndex);
}

/** *********************************************************************
* Получает слово по локальному номеру слова для текущего списка слов
* (в пределах текущего уровня вложенности)
*
* @param[in]	aWordIndex	- номер слова по порядку, начиная с 0
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetWordByIndex(Int32 aWordIndex)
{
	return GetWordByIndex(GetCurrentListIndex(), aWordIndex);
}

/** *********************************************************************
* Получает слово по локальному номеру слова и номеру списка слов
* (в пределах текущего уровня вложенности)
*
* @param[in]	aListIndex	- номер списка слов
* @param[in]	aWordIndex	- номер слова по порядку, начиная с 0
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetWordByIndex(Int32 aListIndex, Int32 aWordIndex)
{
	ISldList* pList = 0;
	ESldError error = GetWordList(aListIndex, &pList);
	if (error != eOK)
		return error;
	
	if (!pList)
		return eMemoryNullPointer;
		
	return pList->GetWordByIndex(aWordIndex);
}

/** *********************************************************************
* Получает полное количество слов в списке слов по номеру списка
*
* @param[in]	aListIndex	- номер списка слов
* @param[out]	aCount		- указатель на переменную, куда будет записан результат
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetTotalWordCount(Int32 aListIndex, Int32* aCount)
{
	if (!aCount)
		return eMemoryNullPointer;
	*aCount = 0;
	
	ISldList* pList = 0;
	ESldError error = GetWordList(aListIndex, &pList);
	if (error != eOK)
		return error;
	
	if (!pList)
		return eMemoryNullPointer;
	
	return pList->GetTotalWordCount(aCount);
}

/** *********************************************************************
* Получает слово по глобальному номеру слова в текущем списке слов
*
* ВНИМАНИЕ: в случае, когда запрашиваемое слово находится внутри папки,
* производится обновление информации о каталоге, но индекс начала папки не изменяется
*
* @param[in]	aGlobalWordIndex	- глобальный номер слова
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetWordByGlobalIndex(Int32 aGlobalWordIndex)
{
	ISldList* pList = 0;
	ESldError error = GetWordList(GetCurrentListIndex(), &pList);
	if (error != eOK)
		return error;

	if (!pList)
		return eMemoryNullPointer;

	error = pList->GetWordByGlobalIndex(aGlobalWordIndex);
	if (error != eOK)
		return error;

	// если список с иерархией и переход к слову происходит по глобальному индексу,
	// то состояние каталога и базовый индекс более не будут синхронизированы c текущим индексом списка
	if (pList->HasHierarchy() && error == eOK)
		pList->SetCatalogSynchronizedFlag(0);

	return eOK;
}
	
/** *********************************************************************
* Получает слово по номеру списка слов, глобальному номеру слова в списке слов и
* номеру варианта написания; меняет состояние иерархии
*
* @param[in]	aListIndex			- номер списка слов
* @param[in]	aGlobalWordIndex	- глобальный номер слова
* @param[in]	aVariantIndex		- номер варианта написания в списке слов
* @param[out]	aWord				- указатель, по которому записывается указатель на слово
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetWordByGlobalIndex(Int32 aListIndex, Int32 aGlobalWordIndex, Int32 aVariantIndex, UInt16** aWord)
{
	if (!aWord)
		return eMemoryNullPointer;

	*aWord = 0;
	
	ISldList* pList = 0;
	ESldError error = GetWordList(aListIndex, &pList);
	if (error != eOK)
		return error;
	
	if (!pList)
		return eMemoryNullPointer;
	
	error = pList->GoToByGlobalIndex(aGlobalWordIndex);
	if (error != eOK)
		return error;

	// @todo delete
	// если список с иерархией и переход к слову происходит по глобальному индексу, то текущий индекс списка
	// более не будет синхронизирован с состоянием каталога и базовым индексом
	//if (pList->HasHierarchy())
	//	pList->SetCatalogSynchronizedFlag(0);
	
	return pList->GetCurrentWord(aVariantIndex, aWord);
}

/** *********************************************************************
* Ищет ближайшее слово в текущем списке слов, которое больше или равно заданному
*
* @param[in]	aText	- искомое слово
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetWordByText(const UInt16* aText)
{
	return GetWordByText(GetCurrentListIndex(), aText);
}

/** *********************************************************************
* Ищет ближайшее слово в указанном списке слов, которое больше или равно заданному
*
* @param[in]	aListIndex	- номер списка слов
* @param[in]	aText		- искомое слово
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetWordByText(Int32 aListIndex, const UInt16* aText)
{
	ISldList* pList = 0;
	ESldError error = GetWordList(aListIndex, &pList);
	if (error != eOK)
		return error;
	
	if (!pList)
		return eMemoryNullPointer;

	return pList->GetWordByText(aText);
}

/** *********************************************************************
* В текущем списке слов ищет слово, наиболее похожее на заданное
*
* @param[in]	aText		- искомое слово
* @param[out]	aResultFlag	- сюда сохраняется флаг результата:
*							  1 - похожее слово найдено,
*							  0 - похожее слово не найдено
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetMostSimilarWordByText(const UInt16* aText, UInt32* aResultFlag)
{
	return GetMostSimilarWordByText(GetCurrentListIndex(), aText, aResultFlag);
}

/** *********************************************************************
* В указанном списке слов ищет слово, наиболее похожее на заданное
*
* @param[in]	aListIndex	- номер списка слов
* @param[in]	aText		- искомое слово
* @param[out]	aResultFlag	- сюда сохраняется флаг результата:
*							  1 - похожее слово найдено,
*							  0 - похожее слово не найдено
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetMostSimilarWordByText(Int32 aListIndex, const UInt16* aText, UInt32* aResultFlag)
{
	const auto defaultLanguage = GetCMP().GetDefaultLanguage();

	ISldList* pList = 0;
	ESldError error = GetWordList(aListIndex, &pList);
	if (error != eOK)
		return error;

	if (!pList)
		return eMemoryNullPointer;

	const CSldListInfo * listInfo;
	error = pList->GetWordListInfo(&listInfo);
	if (error != eOK)
		return error;

	const auto currentLanguage = listInfo->GetLanguageFrom();
	if (defaultLanguage != currentLanguage)
	{
		error = GetCMP().SetDefaultLanguage(currentLanguage);
		if (error != eOK)
			return error;
	}

	error = pList->GetMostSimilarWordByText(aText, aResultFlag);
	if (error != eOK)
		return error;

	if (defaultLanguage != currentLanguage)
	{
		error = GetCMP().SetDefaultLanguage(defaultLanguage);
		if (error != eOK)
			return error;
	}

	return eOK;
}

/** *********************************************************************
* По номеру варианта написания получает последнее найденное слово
* в текущем списке слов
*
* @param[in]	aVariantIndex	- номер варианта написания
* @param[out]	aWord			- указатель, по которому записывается указатель на слово
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetCurrentWord(Int32 aVariantIndex, UInt16** aWord)
{
	return GetCurrentWord(GetCurrentListIndex(), aVariantIndex, aWord);
}

/** *********************************************************************
* По номеру варианта написания получает последнее найденное слово
* в указанном списке слов
*
* @param[in]	aListIndex		- номер списка слов
* @param[in]	aVariantIndex	- номер варианта написания
* @param[out]	aWord			- указатель, по которому записывается указатель на слово
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetCurrentWord(Int32 aListIndex, Int32 aVariantIndex, UInt16** aWord)
{
	ISldList* pList = 0;
	ESldError error = GetWordList(aListIndex, &pList);
	if (error != eOK)
		return error;
	
	if (!pList)
		return eMemoryNullPointer;
	
	return pList->GetCurrentWord(aVariantIndex, aWord);
}

/** *********************************************************************
* Возвращает указатель на eVariantLabel последнего найденного слова, если она есть
*
* @param[out]	aLabel	- указатель, по которому записывается указатель на метку
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetCurrentWordLabel(UInt16** aLabel)
{
	ISldList* pList = 0;
	ESldError error = GetWordList(GetCurrentListIndex(), &pList);
	if (error != eOK)
		return error;

	if (!pList)
		return eMemoryNullPointer;

	return pList->GetCurrentWordLabel(aLabel);
}


/** *********************************************************************
* В текущем списке слов получает локальный номер текущего слова
* (в пределах текущего уровня вложенности)
*
* @param[out]	aIndex	- сюда сохраняется номер слова
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetCurrentIndex(Int32* aIndex)
{
	return GetCurrentIndex(GetCurrentListIndex(), aIndex);
}


/** *********************************************************************
* В указанном списке слов получает локальный номер текущего слова
* (в пределах текущего уровня вложенности)
*
* @param[in]	aListIndex	- номер списка слов
* @param[out]	aIndex		- сюда сохраняется номер слова
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetCurrentIndex(Int32 aListIndex, Int32* aIndex)
{
	ISldList* pList = 0;
	ESldError error = GetWordList(aListIndex, &pList);
	if (error != eOK)
		return error;
	
	if (!pList)
		return eMemoryNullPointer;

	error = pList->GetCurrentIndex(aIndex);
	if (error != eOK)
		return error;
	
	// такое может случиться, если слово было получено с помощью функции GetWordByGlobalIndex
	if (*aIndex < 0)
		return eCommonCantFindIndex;

	return eOK;

}

/** *********************************************************************
* В текущем списке слов получает глобальный номер текущего слова
* (номер слова в пределах всего списка слов)
*
* @param[out]	aIndex	- сюда сохраняется номер слова
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetCurrentGlobalIndex(Int32* aIndex)
{
	return GetCurrentGlobalIndex(GetCurrentListIndex(), aIndex);
}

/** *********************************************************************
* В указанном списке слов получает глобальный номер текущего слова
* (номер слова в пределах всего списка слов)
*
* @param[in]	aListIndex	- номер списка слов
* @param[out]	aIndex		- сюда сохраняется номер слова
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetCurrentGlobalIndex(Int32 aListIndex, Int32* aIndex)
{
	ISldList* pList = 0;
	ESldError error = GetWordList(aListIndex, &pList);
	if (error != eOK)
		return error;
	
	if (!pList)
		return eMemoryNullPointer;

	return pList->GetCurrentGlobalIndex(aIndex);
}

/** *********************************************************************
* Конвертирует локальный номер слова в пределах текущего уровня вложенности в текущем списке слов
* в глобальный номер слова в пределах текущего списка слов
*
* @param[in]	aLocalIndex		- локальный номер слова в пределах текущего уровня вложенности
* @param[out]	aGlobalIndex	- указатель на переменную, куда сохраняется глобальный номер слова
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::LocalIndex2GlobalIndex(Int32 aLocalIndex, Int32* aGlobalIndex)
{
	return LocalIndex2GlobalIndex(GetCurrentListIndex(), aLocalIndex, aGlobalIndex);
}

/** *********************************************************************
* Конвертирует локальный номер слова в пределах текущего уровня вложенности в указанном списке слов
* в глобальный номер слова в пределах указанного списка слов
*
* @param[in]	aListIndex		- номер списка слов
* @param[in]	aLocalIndex		- локальный номер слова в пределах текущего уровня вложенности
* @param[out]	aGlobalIndex	- указатель на переменную, куда сохраняется глобальный номер слова
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::LocalIndex2GlobalIndex(Int32 aListIndex, Int32 aLocalIndex, Int32* aGlobalIndex)
{
	ISldList* pList = 0;
	ESldError error = GetWordList(aListIndex, &pList);
	if (error != eOK)
		return error;
	
	if (!pList)
		return eMemoryNullPointer;
	
	error = pList->LocalIndex2GlobalIndex(aLocalIndex, aGlobalIndex);
	if (error != eOK)
		return error;
	
	if (*aGlobalIndex < 0)
		return eCommonWrongIndex;

	return eOK;
}

/** *********************************************************************
* Получает номер списка слов по локальному номеру слова в текущем списке слов
* Для обычного списка (не поискового) возвращается SLD_DEFAULT_LIST_INDEX
* Для поискового списка возвращается номер списка слов, в котором было найдено слово
* Для списка полнотекстового поиска возвращается индекс базового списка для первого результата поиска
*
* @param[in]	aLocalWordIndex	- локальный номер слова
* @param[out]	aRealListIndex	- указатель на переменную, в которую сохраняется номер списка
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetRealListIndex(Int32 aLocalWordIndex, Int32* aRealListIndex)
{
	return GetRealListIndex(GetCurrentListIndex(), aLocalWordIndex, aRealListIndex);
}

/** *********************************************************************
* Получает номер списка слов по локальному номеру слова в указанном списке слов
* Для обычного списка (не поискового) возвращается SLD_DEFAULT_LIST_INDEX
* Для поискового списка возвращается номер списка слов, в котором было найдено слово
* Для списка полнотекстового поиска возвращается индекс базового списка для первого результата поиска
*
* @param[in]	aListIndex		- номер списка слов
* @param[in]	aLocalWordIndex	- локальный номер слова
* @param[out]	aRealListIndex	- указатель на переменную, в которую сохраняется номер списка
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetRealListIndex(Int32 aListIndex, Int32 aLocalWordIndex, Int32* aRealListIndex)
{
	if (!aRealListIndex)
		return eMemoryNullPointer;
	
	ISldList* pList = 0;
	ESldError error = GetWordList(aListIndex, &pList);
	if (error != eOK)
		return error;
	
	if (!pList)
		return eMemoryNullPointer;
	
	if (aLocalWordIndex < 0)
		return eCommonWrongIndex;
	
	// Выполняются проверки, что aLocalWordIndex корректен
	Int32 GlobalWordIndex = 0;
	error = pList->GetRealGlobalIndex(aLocalWordIndex, &GlobalWordIndex);
	if (error != eOK)
		return error;
	
	if (GlobalWordIndex < 0)
		return eCommonWrongIndex;
	
	*aRealListIndex = aListIndex;
	
	error = pList->GetRealListIndex(aLocalWordIndex, aRealListIndex);
	if (error != eOK)
		return error;

	return eOK;
}

/** *********************************************************************
* Получает глобальный номер слова по локальному номеру слова в текущем списке слов
* Для обычного списка (не поискового) возвращается SLD_DEFAULT_WORD_INDEX
* Для поискового списка возвращается глобальный номер слова из списка слов, в котором было найдено слово
* Для списка полнотекстового поиска возвращается глобальный номер первого результата из базового списка
*
* @param[in]	aLocalWordIndex		- локальный номер слова
* @param[out]	aGlobalWordIndex	- указатель на переменную, в которую сохраняется глобальный номер слова
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetRealGlobalIndex(Int32 aLocalWordIndex, Int32* aGlobalWordIndex)
{	
	return GetRealGlobalIndex(GetCurrentListIndex(), aLocalWordIndex, aGlobalWordIndex);
}

/** *********************************************************************
* Получает глобальный номер слова по локальному номеру слова в указанном списке слов
* Для обычного списка (не поискового) возвращается SLD_DEFAULT_WORD_INDEX
* Для поискового списка возвращается глобальный номер слова из списка слов, в котором было найдено слово
* Для списка полнотекстового поиска возвращается глобальный номер первого результата из базового списка
*
* @param[in]	aListIndex			- номер списка слов
* @param[in]	aLocalWordIndex		- локальный номер слова
* @param[out]	aGlobalWordIndex	- указатель на переменную, в которую сохраняется глобальный номер слова
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetRealGlobalIndex(Int32 aListIndex, Int32 aLocalWordIndex, Int32* aGlobalWordIndex)
{
	ISldList* pList = 0;
	ESldError error = GetWordList(aListIndex, &pList);
	if (error != eOK)
		return error;
	
	if (!pList)
		return eMemoryNullPointer;
	
	if (aLocalWordIndex < 0)
		return eCommonWrongIndex;
	
	error = pList->GetRealGlobalIndex(aLocalWordIndex, aGlobalWordIndex);
	if (error != eOK)
		return error;
	
	return eOK;
}

/** *********************************************************************
* Получает реальные индексы списка и записи в реальном списке в по локальному номеру слова в указанном списке слов
*
* @param[in]	aLocalWordIndex		- локальный номер слова
* @param[in]	aTrnslationIndex	- номер индексов которые мы хотим получить (можно получить через GetNumberOfTranslations() для списков типа IsFullTextSearchList())
* @param[out]	aRealListIndex		- указатель на переменную, в которую сохраняется индекс списка
* @param[out]	aGlobalWordIndex	- указатель на переменную, в которую сохраняется глобальный номер слова
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetRealIndexes(Int32 aLocalWordIndex, Int32 aTrnslationIndex, Int32* aRealListIndex, Int32* aGlobalWordIndex)
{
	return GetRealIndexes(GetCurrentListIndex(), aLocalWordIndex, aTrnslationIndex, aRealListIndex, aGlobalWordIndex);
}

/** *********************************************************************
* Получает реальные индексы списка и записи в реальном списке в по локальному номеру слова в указанном списке слов
*
* @param[in]	aListIndex			- номер списка слов
* @param[in]	aLocalWordIndex		- локальный номер слова
* @param[in]	aTrnslationIndex	- номер индексов которые мы хотим получить (можно получить через GetNumberOfTranslations() для списков типа IsFullTextSearchList())
* @param[out]	aRealListIndex		- указатель на переменную, в которую сохраняется индекс списка
* @param[out]	aGlobalWordIndex	- указатель на переменную, в которую сохраняется глобальный номер слова
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetRealIndexes(Int32 aListIndex, Int32 aLocalWordIndex, Int32 aTrnslationIndex, Int32* aRealListIndex, Int32* aGlobalWordIndex)
{
	ISldList* pList = 0;
	ESldError error = GetWordList(aListIndex, &pList);
	if (error != eOK)
		return error;

	if (!pList)
		return eMemoryNullPointer;

	Int32 globalIndex = SLD_DEFAULT_WORD_INDEX;
	error = pList->LocalIndex2GlobalIndex(aLocalWordIndex, &globalIndex);
	if (error != eOK)
		return error;

	return pList->GetRealIndexes(globalIndex, aTrnslationIndex, aRealListIndex, aGlobalWordIndex);
}

/** *********************************************************************
* Получает  количество реальных индексов в текущем списке слов
*
* @param[in]	aLocalWordIndex		- локальный номер слова
* @param[out]	aRealListIndex		- указатель на переменную, в которую сохраняется количество индексов
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetRealIndexesCount(Int32 aLocalWordIndex, Int32* aRealIndexesCount)
{
	return GetRealIndexesCount(GetCurrentListIndex(), aLocalWordIndex, aRealIndexesCount);
}

/** *********************************************************************
* Получает  количество реальных индексов в указанном списке слов
*
* @param[in]	aListIndex			- номер списка слов
* @param[in]	aLocalWordIndex		- локальный номер слова
* @param[out]	aRealListIndex		- указатель на переменную, в которую сохраняется количество индексов
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetRealIndexesCount(Int32 aListIndex, Int32 aLocalWordIndex, Int32* aRealIndexesCount)
{
	ISldList* pList = 0;
	ESldError error = GetWordList(aListIndex, &pList);
	if (error != eOK)
		return error;

	if (!pList)
		return eMemoryNullPointer;

	Int32 globalIndex = SLD_DEFAULT_WORD_INDEX;
	error = pList->LocalIndex2GlobalIndex(aLocalWordIndex, &globalIndex);
	if (error != eOK)
		return error;

	return pList->GetReferenceCount(globalIndex, aRealIndexesCount);
}

/** *********************************************************************
* Получает номер картинки текущего слова в текущем списке слов
*
* @param[out]	aPictureIndexes	- вектор с индексами картинок для текущего слова
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetCurrentWordPictureIndex(CSldVector<Int32> & aPictureIndexes)
{
	return GetCurrentWordPictureIndex(GetCurrentListIndex(), aPictureIndexes);
}

/** *********************************************************************
* Получает номер картинки текущего слова в указанном списке слов
*
* @param[in]	aListIndex		- номер списка слов
* @param[out]	aPictureIndexes	- вектор с индексами картинок для текущего слова
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetCurrentWordPictureIndex(Int32 aListIndex, CSldVector<Int32> & aPictureIndexes)
{		
	aPictureIndexes.clear();

	// Сохраняем данные из системы защиты.
	ESldError error = SldSaveRegistrationData(GetDictionaryHeader()->DictID, &m_RegistrationData, &m_RandomSeed, GetLayerAccess());
	if (error != eOK)
		return error;
		
	ISldList* pList = 0;
	error = GetWordList(aListIndex, &pList);
	if (error != eOK)
		return error;
	
	if (!pList)
		return eMemoryNullPointer;

	return pList->GetPictureIndex(aPictureIndexes);
}

/** *********************************************************************
* Получает номер видео текущего слова в текущем списке слов
*
* @param[out]	aVideoIndex	- указатель на переменную, в которую будет записан номер видео
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetCurrentWordVideoIndex(Int32* aVideoIndex)
{
	return GetCurrentWordVideoIndex(GetCurrentListIndex(), aVideoIndex);
}

/** *********************************************************************
* Получает номер видео текущего слова в указанном списке слов
*
* @param[in]	aListIndex	- номер списка слов
* @param[out]	aVideoIndex	- указатель на переменную, в которую будет записан номер видео
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetCurrentWordVideoIndex(Int32 aListIndex, Int32* aVideoIndex)
{
	if (!aVideoIndex)
		return eMemoryNullPointer;

	ISldList* pList = 0;
	ESldError error = GetWordList(aListIndex, &pList);
	if (error != eOK)
		return error;

	if (!pList)
		return eMemoryNullPointer;

	return pList->GetVideoIndex(aVideoIndex);
}


/** *********************************************************************
* Получает номер видео текущего слова в текущем списке слов
*
* @param[out]	aVideoIndex	- указатель на переменную, в которую будет записан номер видео
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetCurrentWordSceneIndex(Int32* aSceneIndex)
{
	return GetCurrentWordSceneIndex(GetCurrentListIndex(), aSceneIndex);
}

/** *********************************************************************
* Получает номер видео текущего слова в указанном списке слов
*
* @param[in]	aListIndex	- номер списка слов
* @param[out]	aVideoIndex	- указатель на переменную, в которую будет записан номер видео
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetCurrentWordSceneIndex(Int32 aListIndex, Int32* aSceneIndex)
{
	if (!aSceneIndex)
		return eMemoryNullPointer;

	ISldList* pList = 0;
	ESldError error = GetWordList(aListIndex, &pList);
	if (error != eOK)
		return error;

	if (!pList)
		return eMemoryNullPointer;

	return pList->GetSceneIndex(aSceneIndex);
}

/** *********************************************************************
* Получает вектор индексов озвучек текущего слова в текущем списке слов
*
* @param[out]	aSoundIndexes	- вектор с индексами озвучки для текущего слова
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetCurrentWordSoundIndex(CSldVector<Int32> & aSoundIndexes)
{
	return GetCurrentWordSoundIndex(GetCurrentListIndex(), aSoundIndexes);
}

/** *********************************************************************
* Получает вектор индексов озвучек текущего слова в указанном списке слов
*
* @param[in]	aListIndex		- номер списка слов
* @param[out]	aSoundIndexes	- вектор с индексами озвучки для текущего слова
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetCurrentWordSoundIndex(Int32 aListIndex, CSldVector<Int32> & aSoundIndexes)
{		
	aSoundIndexes.clear();

	ISldList* pList = 0;
	ESldError error = GetWordList(aListIndex, &pList);
	if (error != eOK)
		return error;
	
	if (!pList)
		return eMemoryNullPointer;

	return pList->GetSoundIndex(aSoundIndexes);
}

/** *********************************************************************
* Получает картинку по номеру
*
* @param[in]	aPictureIndex	- индекс запрашиваемой картинки
* @param[in]	aMaxSideSize	- максимальный размер стороны картинки (ширина или высота) в пикселах
* @param[out]	aImageElement	- сюда сохраняется указатель на структуру с данными
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetWordPicture(Int32 aPictureIndex, Int32 aMaxSideSize, TImageElement* aImageElement)
{
	if (!aImageElement)
		return eMemoryNullPointer;
	
	// Очищаем выходные данные
	sldMemZero(aImageElement, sizeof(TImageElement));

	// Загружаем ресурс с картинкой
	
	//*******************************************************************
	// TODO:	нужно сделать обработку параметра aMaxSideSize,
	//			а также реализовать обработку разных типов картинок
	//*******************************************************************
	
	auto res = m_data.GetResource(RESOURCE_TYPE_IMAGE, aPictureIndex);
	if (res != eOK)
		return res.error();

	// Копируем данные
	aImageElement->Data = sldMemNew<UInt8>(res.size());
	if (!aImageElement->Data)
		return eMemoryNotEnoughMemory;

	sldMemMove(aImageElement->Data, res.ptr(), res.size());
	aImageElement->ImageIndex = aPictureIndex;
	aImageElement->Size = res.size();

	return eOK;
}

/** *********************************************************************
* Получает видео по номеру
*
* @param[in]	aVideoIndex		- индекс запрашиваемого видео
* @param[out]	aVideoElement	- сюда сохраняется указатель на структуру с данными
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetWordVideo(const UInt32 aVideoIndex, CSldVideoElement* aVideoElement)
{
	if (!aVideoElement)
		return eMemoryNullPointer;

	// Очищаем выходные данные
	aVideoElement->Clear();

	// Загружаем ресурс с видео
	auto res = m_data.GetResource(RESOURCE_TYPE_VIDEO, aVideoIndex);
	if (res != eOK)
		return res.error();

	*aVideoElement = CSldVideoElement(aVideoIndex, res.ptr(), res.size());

	return aVideoElement->IsValid() ? eOK : eCommonWrongMediaSourceType;
}

/** *********************************************************************
* Освобождает ресурс с видео
*
* @param[in]	aVideoElement	- указатель на структуру с данными
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::ReleaseWordVideo(CSldVideoElement* aVideoElement)
{
	if (aVideoElement->GetData())
		sldMemFree(aVideoElement->GetData());

	aVideoElement->Clear();

	return eOK;
}

/// Загружает ресурс по типу и номеру
static ESldError LoadTElement(CSDCReadMy &aReader, UInt32 aType, UInt32 aIndex, TElement* aElement)
{
	if (!aElement)
		return eMemoryNullPointer;

	// Очищаем выходные данные
	sldMemZero(aElement, sizeof(TElement));

	// Загружаем ресурс со сценой
	auto res = aReader.GetResource(aType, aIndex);
	if (res != eOK)
		return res.error();

	// Копируем данные
	aElement->Data = sldMemNew<UInt8>(res.size());
	if (!aElement->Data)
		return eMemoryNotEnoughMemory;

	sldMemMove(aElement->Data, res.ptr(), res.size());
	aElement->Index = aIndex;
	aElement->Size = res.size();

	return eOK;
}

/** *********************************************************************
* Получает 3D сцену по номеру
*
* @param[in]	aSceneIndex		- индекс запрашиваемой сцены
*
* @param[in]	aSceneElement	- указатель на структуру с данными
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetWordScene(Int32 aSceneIndex, TElement* aSceneElement)
{
	return LoadTElement(m_data, RESOURCE_TYPE_SCENE, aSceneIndex, aSceneElement);
}

/** *********************************************************************
* Освобождает ресурсы
*
* @param[in]	aElement	- указатель на структуру с данными
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::ReleaseWordResources(TElement* aElement)
{
	if (!aElement)
		return eMemoryNullPointer;

	if (aElement->Data)
	{
		sldMemFree(aElement->Data);
	}
	sldMemZero(aElement, sizeof(TElement));

	return eOK;
}

/** *********************************************************************
* Получает модель по номеру
*
* @param[in]	aModelIndex		- индекс запрашиваемой модели
*
* @param[in]	aModelElement	- указатель на структуру с данными
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetWordMesh(Int32 aModelIndex, TElement* aModelElement)
{
	return LoadTElement(m_data, RESOURCE_TYPE_MESH, aModelIndex, aModelElement);
}

/** *********************************************************************
* Получает материал по номеру
*
* @param[in]	aMaterialIndex		- индекс запрашиваемого материала
*
* @param[in]	aMaterialElement	- указатель на структуру с данными
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetWordMaterial(Int32 aMaterialIndex, TElement* aMaterialElement)
{
	return LoadTElement(m_data, RESOURCE_TYPE_MATERIAL, aMaterialIndex, aMaterialElement);
}

/** *********************************************************************
* Получает абстрактный ресурс по номеру
*
* @param[in]	aAbstractResourceIndex		- индекс запрашиваемого материала
*
* @param[out]	aAbstractResourceElement	- указатель на структуру с данными
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetWordAbstractResource(Int32 aAbstractResourceIndex, TElement* aAbstractResourceElement)
{
	return LoadTElement(m_data, RESOURCE_TYPE_ABSTRACT, aAbstractResourceIndex, aAbstractResourceElement);
}

/** *********************************************************************
* Получает бинарный ресурс по номеру
*
* @param[in]	aResourceIndex		- индекс запрашиваемого бинарного ресурса
*
* @param[out]	aResource			- указатель на структуру с данными
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetBinaryResourceByIndex(const Int32 aResourceIndex, TElement* aResource)
{
	return LoadTElement(m_data, RESOURCE_TYPE_BINARY, aResourceIndex, aResource);
}

/** *********************************************************************
* Получает информацию о загруженной картинке
*
* @param[in]	aImageElement	- указатель на структуру с данными картинки
* @param[out]	aFormatType		- сюда сохраняется формат картинки (см. #EPictureFormatType)
* @param[out]	aWidth			- сюда сохраняется ширина картинки (если не передан NULL)
*								  если полученное значение равно -1, то ширину определить не удалось
* @param[out]	aHeight			- сюда сохраняется высота картинки (если не передан NULL)
*								  если полученное значение равно -1, то высоту определить не удалось
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetPictureInfo(TImageElement* aImageElement, UInt32* aFormatType, TSizeValue* aWidth, TSizeValue* aHeight) const
{
	if (!aFormatType || !aImageElement || !aImageElement->Data)
		return eMemoryNullPointer;
	
	*aFormatType = ePictureFormatType_UNKNOWN;
	if (aWidth)
		*aWidth = TSizeValue();
	if (aHeight)
		*aHeight = TSizeValue();
	
	if (!aImageElement->Size)
		return eOK;
	
	UInt8* DataPtr = aImageElement->Data;
	UInt32 DataSize = aImageElement->Size;

	const UInt8 PNGSignature[] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 0};
	const UInt8 GIFSignature[] = {0x47, 0x49, 0x46, 0x38, 0};
	const UInt8 JPGSignature[] = {0xFF, 0xD8, 0xFF, 0};
	
	TSizeValue width, height;
	// PNG
	if (sld2::StrEqual(DataPtr, PNGSignature, sld2::StrLen(PNGSignature)))
	{
		*aFormatType = ePictureFormatType_PNG;
		
		UInt32 offset = 8 + 4 + 4; //skip signature, chunk length and chunk type (IHDR)
		UInt32 uSize = 0;
		
		uSize = *((UInt32*)(DataPtr + offset));
		uSize = ByteSwap32(uSize);
		width.Set(uSize * SLD_SIZE_VALUE_SCALE, eMetadataUnitType_px);
		
		offset += 4;
		uSize = *((UInt32*)(DataPtr + offset));
		uSize = ByteSwap32(uSize);
		height.Set(uSize * SLD_SIZE_VALUE_SCALE, eMetadataUnitType_px);
	}
	// GIF
	else if (sld2::StrEqual(DataPtr, GIFSignature, sld2::StrLen(GIFSignature)))
	{
		// GIF-signature: GIF87a || GIF89a
		const UInt8 GIFSignatureFull[] = "GIF8Xa";
		DataPtr += CSldCompare::StrLenA(GIFSignatureFull);

		*aFormatType = ePictureFormatType_GIF;

		Int32 iSize = 0;

		iSize = (DataPtr[0]) + (DataPtr[1] << 8);
		width.Set(iSize * SLD_SIZE_VALUE_SCALE, eMetadataUnitType_px);

		iSize = (DataPtr[2]) + (DataPtr[3] << 8);
		height.Set(iSize * SLD_SIZE_VALUE_SCALE, eMetadataUnitType_px);
	}
	// JPG
	else if (sld2::StrEqual(DataPtr, JPGSignature, sld2::StrLen(JPGSignature)))
	{
		*aFormatType = ePictureFormatType_JPG;
		Int32 iwidth, iheight;
		GetJpgInfo(DataPtr, DataSize, &iwidth, &iheight);
		width.Set(iwidth * SLD_SIZE_VALUE_SCALE, eMetadataUnitType_px);
		height.Set(iheight * SLD_SIZE_VALUE_SCALE, eMetadataUnitType_px);
	}
	// SVG
	else
	{
		ESldError error = CSldMetadataParser::GetSvgInfo(aImageElement, aFormatType, &width, &height);
		if (error != eOK)
			return error;
	}

	if (aWidth)
		*aWidth = width;
	if (aHeight)
		*aHeight = height;
		
	return eOK;
}

/** *********************************************************************
* Получает информацию о JPG картинке
*
* @param[in]	apiData		- указатель на массив с данными картинки
* @param[out]	aiDataSize	- размер массива с данными картинки
* @param[out]	apiWidth	- сюда сохраняется ширина картинки в пикселах (если не передан NULL)
* @param[out]	apiHeight	- сюда сохраняется высота картинки в пикселах (если не передан NULL)
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetJpgInfo(UInt8 *const apiData, const UInt32 aiDataSize, Int32 *const apiWidth, Int32 *const apiHeight) const
{
	for (UInt32 iDataIndex(0); iDataIndex < aiDataSize;) 
	{
		while  (apiData[iDataIndex++] != 0xFF);
		switch (apiData[iDataIndex++])
		{
		case 0xFF:	// skip this block
		case 0xD8:	// image start token
			continue;

		case 0x00:	// wrong marker
		case 0xD9:	// image end token
			return eCommonWrongShiftSize;

		case 0xC0:	// jpeg baseline
		case 0xC1: case 0xC2: case 0xC3:
		case 0xC5: case 0xC6: case 0xC7:
		case 0xC9: case 0XCA: case 0xCB:
		case 0xCD: case 0xCE: case 0xCF:
		{
			iDataIndex += 2; // segment length
			iDataIndex += 1; // data precision

			// image height
			UInt16 iHeight(0);
			iHeight += apiData[iDataIndex++] << 8;
			iHeight += apiData[iDataIndex++];

			// image width
			UInt16 iWidth(0);
			iWidth += apiData[iDataIndex++] << 8;
			iWidth += apiData[iDataIndex++];

			if (apiHeight)
				*apiHeight = iHeight;

			if (apiWidth)
				*apiWidth = iWidth;

			return eOK;
		}

		default:
			UInt32 iBlockIndex(iDataIndex);
			iDataIndex += apiData[iBlockIndex++] << 8;
			iDataIndex += apiData[iBlockIndex++];
		}
	}

	return eCommonWrongMediaSourceType;
}


/** *********************************************************************
* Инициирует процедуру декодирования и проигрывания озвучки с заданным индексом;
* результатом вызова функции станет передача декодированных wav-данных
* в функцию сборки озвучки класса прослойки 
*
* @param[in]	aSoundIndex	- индекс озвучки
* @param[in]	aIsLast		- флаг, если установлен, значит данное слово 
*							  будет последним при декодировании звука и прослойка 
*							  получит команду начать воспроизведение
* @param[in]	aStartPos	- указатель на переменную, в которой хранится позиция, 
*							  в которую нужно начинать помещать декодированный звук.
*							  Если будет озвучено 1 слово или каждый раз данные будут 
*							  озвучиваться отдельно, можно передавать NULL.
* @param[in]	aExternFlag	- флаг, если установлен, сообщает,
*							  что озвучка будет предоставлена извне самой оболочкой
* @param[in]	aBeginPlay	- обозначает с какого места проигрывать озвучку [0;100], в %
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::PlaySoundByIndex(Int32 aSoundIndex, UInt8 aIsLast, UInt32* aStartPos, UInt32 aExternFlag, UInt8 aBeginPlay)
{
	CSDCReadMy::ResourceHandle loadedRes;
	TResourceType res = {};
	
	UInt8 IsExternalData = (GetDictionaryHeader()->SoundSourceType != eMediaSourceType_Database || aExternFlag);
	if (IsExternalData)
	{
		// Озвучка находится не в базе, ее предоставляет сам клиент
		ESldError error = GetLayerAccess()->LoadSoundByIndex(aSoundIndex, &res.Pointer, &res.Size);
		if (error != eOK)
			return error;
	}
	else
	{
		// Загружаем ресурс с озвучкой
		loadedRes = m_data.GetResource(RESOURCE_TYPE_SOUND, aSoundIndex);
		if (loadedRes != eOK)
			return loadedRes.error();
		res.Pointer = loadedRes.ptr();
		res.Size = loadedRes.size();
	}
	
	// Общий заголовок звуковых данных
	const TSoundFileHeader* soundHeader = (const TSoundFileHeader*)res.Pointer;
	
	// Проверка корректности данных
	if (soundHeader->structSize != sizeof(*soundHeader))
		return eSoundSpeexDataCorrupted;
	
	m_RegistrationData.Clicks++;
	SLD_VALIDATE_SECURITY(&m_RegistrationData, &m_RandomSeed, m_SoundBuilder);
	
	// Выбираем соответствующую функцию для декодирования в зависимости от формата
	switch (soundHeader->SoundFormat)
	{
		case SLD_SOUND_FORMAT_SPX:
		{
			return SpeexDecode(GetLayerAccess(), m_SoundBuilder[0], res.Pointer, res.Size, aIsLast, aStartPos, aBeginPlay);
		}
		case SLD_SOUND_FORMAT_WAV:
		{
			return WavDecode(GetLayerAccess(), m_SoundBuilder[0], res.Pointer, res.Size, aIsLast, aStartPos);
		}
		case SLD_SOUND_FORMAT_MP3:
		{
			return Mp3Decode(GetLayerAccess(), m_SoundBuilder[0], res.Pointer, res.Size, aIsLast, aStartPos);
		}
		case SLD_SOUND_FORMAT_OGG:
		{
			return OggDecode(GetLayerAccess(), m_SoundBuilder[0], res.Pointer, res.Size, aIsLast, aStartPos);
		}
		default:
		{
			return eSoundWrongDataFormat;
		}
	}
	return eOK;
}

/** *********************************************************************
* Инициирует процедуру декодирования и проигрывания озвучки с заданным индексом;
* результатом вызова функции станет передача декодированных wav-данных
* в функцию сборки озвучки класса прослойки 
*
* @param[in]	aSoundIndex	- индекс озвучки
* @param[in]	aIsLast		- флаг, если установлен, значит данное слово 
*							  будет последним при декодировании звука и прослойка 
*							  получит команду начать воспроизведение
* @param[in]	aStartPos	- указатель на переменную, в которой хранится позиция, 
*							  в которую нужно начинать помещать декодированный звук.
*							  Если будет озвучено 1 слово или каждый раз данные будут 
*							  озвучиваться отдельно, можно передавать NULL.
* @param[in]	aExternFlag	- флаг, если установлен, сообщает,
*							  что озвучка будет предоставлена извне самой оболочкой
* @param[in]	aBeginPlay	- обозначает с какого места проигрывать озвучку [0;100], в %
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::PlaySoundByIndex(Int32 aSoundIndex, UInt8 aIsLast, UInt32* aStartPos, UInt32 aExternFlag)
{
	return PlaySoundByIndex(aSoundIndex, aIsLast, aStartPos, aExternFlag, 0);
}

/** *********************************************************************
* Инициирует процедуру декодирования и проигрывания озвучки с заданным индексом;
* результатом вызова функции станет передача декодированных wav-данных
* в функцию сборки озвучки класса прослойки 
*
* @param[in]	aSoundIndex	- индекс озвучки
* @param[in]	aIsLast		- флаг, если установлен, значит данное слово 
*							  будет последним при декодировании звука и прослойка 
*							  получит команду начать воспроизведение
* @param[in]	aStartPos	- указатель на переменную, в которой хранится позиция, 
*							  в которую нужно начинать помещать декодированный звук.
*							  Если будет озвучено 1 слово или каждый раз данные будут 
*							  озвучиваться отдельно, можно передавать NULL.
* @param[in]	aExternFlag	- флаг, если установлен, сообщает,
*							  что озвучка будет предоставлена извне самой оболочкой
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::PlaySoundByIndexWithPosition(Int32 aSoundIndex, UInt8 aIsLast, UInt32* aStartPos,UInt8 aBeginPlay,UInt32 aExternFlag)
{
	return PlaySoundByIndex(aSoundIndex, aIsLast, aStartPos, aExternFlag, aBeginPlay);
}


/** *********************************************************************
* Проигрывает озвучку по имени
*
* @param[in]	aText		- указатель на строку с именем звука
* @param[out]	aResultFlag	- флаг, найдена ли подходящая озвучка (1 - найдена, 0 - не найдена). 
*					Озвучка ищется в текущем листе.
*							  
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::PlaySoundByText(const UInt16 *aText, UInt32 *aResultFlag)
{
	if (!aText || !aResultFlag)
		return eMemoryNullPointer;

	ESldError error;
	CSldVector<Int32> soundIndexes;
	if (GetCurrentLanguageFrom() == SldLanguage::Chinese)
	{

		const UInt16 *tempText = aText;
		while (*tempText) {
			if (!isChinSoundCh(*tempText)) {
				*aResultFlag = 0;
				return eOK;
			}
			++tempText;
		}
		
		error = SearchChineseSound(aText, 0, soundIndexes, aResultFlag);
		if (error != eOK)
			return error;
		// Если разбиение слова нашлось, то проигрываем его.
		if (*aResultFlag)
		{
			UInt32 startPos = 0;
			for (UInt32 index = 0; index < soundIndexes.size(); index++)
			{
				if (index >= MAX_SEARCH_DEPTH)
					break;

				error = PlaySoundByIndex(soundIndexes[index], index == (soundIndexes.size() - 1), &startPos);
				if (error != eOK)
					return error;
			}
		}
		return eOK;
	}
	else
	{
		error = GetSoundIndexByText(aText, soundIndexes);
		if (error != eOK)
			return error;

		*aResultFlag = soundIndexes.empty() ? 0 : 1;
		if (!*aResultFlag)
			return eOK;

		return PlaySoundByIndex(soundIndexes.front(), 1, 0);
	}
}

ESldError CSldDictionary::GetDecoderObject(UInt32 aSoundIndex, SldSpxDecoder *aDecoder, UInt32 aExternFlag)
{
	if(!aDecoder)
		return eMemoryNullPointer;

	ESldError error;
	CSDCReadMy::ResourceHandle loadedRes;
	TResourceType res = {};

	UInt8 IsExternalData = (GetDictionaryHeader()->SoundSourceType != eMediaSourceType_Database || aExternFlag);
	if (IsExternalData)
	{
		// Озвучка находится не в базе, ее предоставляет сам клиент
		error = GetLayerAccess()->LoadSoundByIndex(aSoundIndex, &res.Pointer, &res.Size);
		if (error != eOK)
			return error;
	}
	else
	{
		// Загружаем ресурс с озвучкой
		loadedRes = m_data.GetResource(RESOURCE_TYPE_SOUND, aSoundIndex);
		if (loadedRes != eOK)
			return loadedRes.error();
		res.Pointer = loadedRes.ptr();
		res.Size = loadedRes.size();
	}
	
	// Общий заголовок звуковых данных
	const TSoundFileHeader* soundHeader = (const TSoundFileHeader*)res.Pointer;
	
	// Проверка корректности данных
	if (soundHeader->structSize != sizeof(*soundHeader))
		return eSoundSpeexDataCorrupted;

	m_RegistrationData.Clicks++;
	SLD_VALIDATE_SECURITY(&m_RegistrationData, &m_RandomSeed, m_SoundBuilder);

	// Выбираем соответствующую функцию для декодирования в зависимости от формата
	switch (soundHeader->SoundFormat)
	{
		case SLD_SOUND_FORMAT_SPX:
		{
			if (IsExternalData)
				return aDecoder->Init(res.Pointer, res.Size);
			else
				return aDecoder->Init(loadedRes.resource());
		}
		case SLD_SOUND_FORMAT_WAV:
		{
			error = WavDecode(GetLayerAccess(), m_SoundBuilder[0], res.Pointer, res.Size, 1, NULL);
			return error != eOK ? error : eSoundNotSpx;
		}
		case SLD_SOUND_FORMAT_MP3:
		{
			error = Mp3Decode(GetLayerAccess(), m_SoundBuilder[0], res.Pointer, res.Size, 1, NULL);
			return error != eOK ? error : eSoundNotSpx;
		}
		case SLD_SOUND_FORMAT_OGG:
		{
			error = OggDecode(GetLayerAccess(), m_SoundBuilder[0], res.Pointer, res.Size, 1, NULL);
			return error != eOK ? error : eSoundNotSpx;
		}
		default:
		{
			return eSoundWrongDataFormat;
		}
	}

	return eOK;
}

/** *********************************************************************
* Возвращает информацию об озвучке
*
* @param[in]	aSoundIndex	- индекс озвучки
* @param[out]	aSoundType	- тип озвучки
*
* @todo Пока возвращает тип озвучки, можно добавить еще несколько параметров
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetSoundInfo(Int32 aSoundIndex, TSoundElement* aSoundInfo)
{
	UInt8 IsExternalData = (GetDictionaryHeader()->SoundSourceType != eMediaSourceType_Database);
	if (IsExternalData)
	{
		// Озвучка находится не в базе, ее предоставляет сам клиент
		TResourceType res = {};
		ESldError error = GetLayerAccess()->LoadSoundByIndex(aSoundIndex, &res.Pointer, &res.Size);
		return error == eOK ? aSoundInfo->Init(res.Pointer, res.Size) : error;
	}
	else
	{
		// Загружаем ресурс с озвучкой
		auto res = m_data.GetResource(RESOURCE_TYPE_SOUND, aSoundIndex);
		return res ? aSoundInfo->Init(res.ptr(), res.size()) : res.error();
	}
}

/** *********************************************************************
* Генерирует тоновый сигнал, который используется в случае незарегистрированной озвучки
* Память для звукового сигнала выделяется здесь, после использования
* пямять нужно освободить вызовом функции sldMemFree()
*
* @param[out]	aDataPtr	- указатель, по которому нужно записать указатель на память с сгенерированным звуком
* @param[out]	aDataSize	- указатель, по которому нужно записать размер памяти с сгенерированным звуком
* @param[out]	aFrequency	- указатель, по которому нужно записать частоту дискретизации сгенерированного звука
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GenerateToneSound(UInt8** aDataPtr, UInt32* aDataSize, UInt32* aFrequency)
{
	if (!aDataPtr || !aDataSize || !aFrequency)
		return eMemoryNullPointer;
	
	*aDataPtr = NULL;
	*aDataSize = 0;
	*aFrequency = 0;
	
	UInt16 volume[] = {117, 97, 78};
	UInt32 MaxFrameSize = 640;
	UInt32 FrameSize = 320;
	UInt32 DataBlockCount = 10;
	UInt32 VolumeCount = sizeof(volume)/sizeof(volume[0]);
	
	// Выделяем память
	UInt32 MemBlockSize = sizeof(Int16)*MaxFrameSize*DataBlockCount;
	UInt32 MemSize = MemBlockSize*VolumeCount;
	Int16* pData = (Int16*)sldMemNewZero(MemSize);
	if (!pData)
		return eMemoryNotEnoughMemory;
	
	// Генерируем сигнал
	Int16* ptr = pData;
	for (UInt32 i=0;i<VolumeCount;i++)
	{
		if (i)
			ptr += MemBlockSize/sizeof(Int16);
			
		for (UInt32 k=0;k<DataBlockCount;k++)
		{
			for (UInt32 j=0;j<MaxFrameSize;j++)
			{
				ptr[k*FrameSize+j] = (Int16)(GetSin(j+i*320)*volume[i]);
			}
		}
	}
	
	*aDataPtr = (UInt8*)pData;
	*aDataSize = MemSize;
	*aFrequency = 22050;

	return eOK;
}

/** *********************************************************************
* Освобождает ресурс с картинкой
*
* @param[in]	aImageElement	- указатель на структуру с данными
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::ReleaseWordPicture(TImageElement* aImageElement) const
{
	if (!aImageElement)
		return eMemoryNullPointer;
		
	if (aImageElement->Data)
	{
		sldMemFree(aImageElement->Data);
	}
	sldMemZero(aImageElement, sizeof(TImageElement));

	return eOK;
}

/** *********************************************************************
* Возвращает количество слов у текущего списка слов на текущем уровне вложенности
*
* @param[out]	aNumberOfWords	- указатель на буфер для количества слов.
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetNumberOfWords(Int32* aNumberOfWords)
{			
	return GetNumberOfWords(GetCurrentListIndex(), aNumberOfWords);
}

/** *********************************************************************
* Возвращает количество слов у указанного списка слов на текущем уровне вложенности
*
* @param[in]	aListIndex		- номер списка слов
* @param[out]	aNumberOfWords	- указатель на буфер для количества слов
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetNumberOfWords(Int32 aListIndex, Int32* aNumberOfWords)
{
	ISldList* pList = 0;
	ESldError error = GetWordList(aListIndex, &pList);
	if (error != eOK)
		return error;
	
	if (!pList)
		return eMemoryNullPointer;
			
	return pList->GetNumberOfWords(aNumberOfWords);
}

/** *********************************************************************
* Возвращает количество списков слов
*
* @param[out]	aNumberOfLists	- указатель на буфер для количества слов.
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetNumberOfLists(Int32* aNumberOfLists) const
{
	if (!aNumberOfLists)
		return eMemoryNullPointer;

	*aNumberOfLists = m_ListCount;
	return eOK;
}

/** *********************************************************************
* Определяет, имеет или нет слово с локальным номером в текущем списке слов поддерево иерархии
*
* @param[in]	aIndex			- локальный номер слова на текущем уровне иерархии
* @param[out]	aIsHierarchy	- указатель на переменную, в которую будет возвращен флаг:
*								  1 - слово имеет поддерево,
*								  0 - слово не имеет поддерева
* @param[out]	aLevelType		- указатель, по которому будет записан тип поддерева (см #EHierarchyLevelType)
*								  (можно передать NULL)
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::isWordHasHierarchy(Int32 aIndex, UInt32* aIsHierarchy, EHierarchyLevelType* aLevelType)
{
	return isWordHasHierarchy(GetCurrentListIndex(), aIndex, aIsHierarchy, aLevelType);
}

/** *********************************************************************
* Определяет, имеет или нет слово с локальным номером в указанном списке слов поддерево иерархии
*
* @param[in]	aListIndex		- номер списка слов
* @param[in]	aIndex			- локальный номер слова на текущем уровне иерархии
* @param[out]	aIsHierarchy	- указатель на переменную, в которую будет возвращен флаг:
*								  1 - слово имеет поддерево,
*								  0 - слово не имеет поддерева
* @param[out]	aLevelType		- указатель, по которому будет записан тип поддерева (см #EHierarchyLevelType)
*								  (можно передать NULL)
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::isWordHasHierarchy(Int32 aListIndex, Int32 aIndex, UInt32* aIsHierarchy, EHierarchyLevelType* aLevelType)
{
	ISldList* pList = 0;
	ESldError error = GetWordList(aListIndex, &pList);
	if (error != eOK)
		return error;
	
	if (!pList)
		return eMemoryNullPointer;
	
	if (aIndex < 0)
		return eCommonWrongIndex;
		
	error = pList->isWordHasHierarchy(aIndex, aIsHierarchy, aLevelType);
	if (error != eOK)
		return error;
	
	return eOK;
}

/** *********************************************************************
* Определяет, сортирован или нет список слов с указанным индексом
*
* @param[in]	aListIndex	- индекс списка слов
* @param[out]	aIsSorted	- указатель на переменную, в которую будет сохранен флаг:
*							  1 - список сортированный,
*							  0 - список не сортированный
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::IsListSorted(Int32 aListIndex, UInt32* aIsSorted) const
{
	if (!aIsSorted)
		return eMemoryNullPointer;

	const CSldListInfo* listInfo = NULL;
	ESldError error = GetWordListInfo(aListIndex, &listInfo);
	if (error != eOK)
		return error;

	*aIsSorted = listInfo->IsSortedList();
	return eOK;
}

/** *********************************************************************
* Определяет, сопоставлены ли некоторым словам из списка слов картинки
* 
* @param[in]	aListIndex	- индекс списка слов
* @param[out]	aIsPicture	- указатель на переменную, в которую будет сохранен флаг:
*							  1 - сопоставлены,
*							  0 - НЕ сопоставлены
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::IsListHasPicture(Int32 aListIndex, UInt32* aIsPicture) const
{
	if (!aIsPicture)
		return eMemoryNullPointer;

	const CSldListInfo* listInfo = NULL;
	ESldError error = GetWordListInfo(aListIndex, &listInfo);
	if (error != eOK)
		return error;

	*aIsPicture = listInfo->IsPicture();
	return eOK;
}

/** *********************************************************************
* Определяет, сопоставлена или нет некоторым словам из списка слов озвучка
* 
* @param[in]	aListIndex	- индекс списка слов
* @param[out]	aIsSound	- указатель на переменную, в которую будет сохранен флаг:
*							  1 - сопоставлена,
*							  0 - НЕ сопоставлена
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::IsListHasSound(Int32 aListIndex, UInt32* aIsSound) const
{
	if (!aIsSound)
		return eMemoryNullPointer;

	const CSldListInfo* listInfo = NULL;
	ESldError error = GetWordListInfo(aListIndex, &listInfo);
	if (error != eOK)
		return error;

	*aIsSound = listInfo->IsSound();
	return eOK;
}

/** *********************************************************************
* Определяет, сопоставлено или нет некоторым словам из списка слов видео
* 
* @param[in]	aListIndex	- индекс списка слов
* @param[out]	aIsSound	- указатель на переменную, в которую будет сохранен флаг:
*							  1 - сопоставлено,
*							  0 - НЕ сопоставлено
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::IsListHasVideo(Int32 aListIndex, UInt32* aIsVideo)
{
	if (!aIsVideo)
		return eMemoryNullPointer;

	const CSldListInfo* listInfo = NULL;
	ESldError error = GetWordListInfo(aListIndex, &listInfo);
	if (error != eOK)
		return error;

	*aIsVideo = listInfo->IsVideo();
	return eOK;
}

/** *********************************************************************
* Определяет, сопоставлено или нет некоторым словам из списка слов 3d
* 
* @param[in]	aListIndex	- индекс списка слов
* @param[out]	aIsScene	- указатель на переменную, в которую будет сохранен флаг:
*							  1 - сопоставлено,
*							  0 - НЕ сопоставлено
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::IsListHasScene(Int32 aListIndex, UInt32* aIsScene)
{
	if (!aIsScene)
		return eMemoryNullPointer;

	ISldList* pList = 0;
	ESldError error = GetWordList(aListIndex, &pList);
	if (error != eOK)
		return error;

	if (!pList)
		return eMemoryNullPointer;

	return pList->IsListHasScene(aIsScene);
}

/** *********************************************************************
* Возвращает флаг того, имеет или нет список слов локализованные строки
*
* @param[in]	aListIndex			- индекс списка слов
* @param[out]	aIsLocalizedStrings	- указатель на переменную, в которую будет сохранен флаг:
*									  1 - имеет,
*									  0 - НЕ имеет
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::IsListHasLocalizedStrings(Int32 aListIndex, UInt32* aIsLocalizedStrings) const
{
	if (!aIsLocalizedStrings)
		return eMemoryNullPointer;

	ESldError error;
	Int32 ListCount = 0;

	error = GetNumberOfLists(&ListCount);
	if (error != eOK)
		return error;

	if (aListIndex >= ListCount || aListIndex < 0)
		return eCommonWrongIndex;
	
	const CSldListInfo * listInfo;
	error = GetWordListInfo(aListIndex, &listInfo);
	if (error != eOK)
		return error;

	*aIsLocalizedStrings = listInfo->IsListHasLocalizedStrings();
	return eOK;
}

/** *********************************************************************
* Возвращает номер старшей версии словарной базы
*
* @param[out]	aVersion			- указатель, по которому будет записан результат
* @param[in]	aDictionaryIndex	- индекс словаря, для которого мы хотим получить информацию
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetDictionaryMajorVersion(UInt32* aVersion, const Int32 aDictionaryIndex) const
{
	if (!aVersion)
		return eMemoryNullPointer;
	if (!m_DictionaryVersionInfo)
		return eMemoryNullPointer;
	
	*aVersion = m_DictionaryVersionInfo->MajorVersion;
	
	return eOK;
}

/** *********************************************************************
* Возвращает номер младшей версии словарной базы
*
* @param[out]	aVersion			- указатель, по которому будет записан результат
* @param[in]	aDictionaryIndex	- индекс словаря, для которого мы хотим получить информацию
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetDictionaryMinorVersion(UInt32* aVersion, const Int32 aDictionaryIndex) const
{
	if (!aVersion)
		return eMemoryNullPointer;
	if (!m_DictionaryVersionInfo)
		return eMemoryNullPointer;
	
	*aVersion = m_DictionaryVersionInfo->MinorVersion;
	
	return eOK;
}

/** *********************************************************************
* Возвращает структуру с дополнительной информацией по базе
*
* @param[out]	aAdditionalInfo	- указатель, по которому будет записан результат
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetAdditionalInfo(TAdditionalInfo *aAdditionalInfo)
{
	if (!aAdditionalInfo)
		return eMemoryNullPointer;

	if (m_AdditionalInfo)
		*aAdditionalInfo = *m_AdditionalInfo;

	m_AdditionalInfo ?
		sldMemMove(aAdditionalInfo, m_AdditionalInfo, sizeof(TAdditionalInfo)) :
		sldMemZero(aAdditionalInfo, sizeof(TAdditionalInfo));

	return eOK;
}

/** *********************************************************************
* Возвращает строку с аннотацией к базе
*
* ВНИМАНИЕ! Память для результирующей строки выделяется в этом методе
* и должна быть освобождена в вызывающем методе вызовом функции sldMemFree()
*
* @param[out]	aAnnotation	- указатель, по которому будет записана итоговая строка
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetAnnotation(UInt16 **aAnnotation)
{
	if (!(aAnnotation && *aAnnotation))
		return eMemoryNullPointer;

	const Int32 iAnnotationLength = CSldCompare::StrLen(m_Annotation) + 1;
	const Int32 iAnnotationSize = iAnnotationLength * sizeof((*aAnnotation)[0]);

	*aAnnotation = (UInt16*)sldMemNewZero(iAnnotationSize);
	if (!(*aAnnotation))
		return eMemoryNotEnoughMemory;

	if (m_Annotation)
	{
		sldMemMove(*aAnnotation, m_Annotation, iAnnotationSize);
	}

	return eOK;
}

/** *********************************************************************
* Возвращает константу, которая обозначает бренд словарной базы (см. EDictionaryBrandName)
*
* @param[out]	aBrand				- указатель, по которому будет записан результат
* @param[in]	aDictionaryIndex	- индекс словаря, для которого мы хотим получить информацию
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetDictionaryBrand(UInt32* aBrand, const Int32 aDictionaryIndex) const
{
	if (!aBrand)
		return eMemoryNullPointer;
	if (!m_DictionaryVersionInfo)
		return eMemoryNullPointer;
	
	*aBrand = m_DictionaryVersionInfo->Brand;
	
	return eOK;
}

/** *********************************************************************
* Возвращает хэш словарной базы
*
* @param[out]	aHash				- указатель, по которому будет записан результат
* @param[in]	aDictionaryIndex	- индекс словаря, для которого мы хотим получить информацию
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetDictionaryHash(UInt32* aHash, const Int32 aDictionaryIndex) const
{
	if (!aHash)
		return eMemoryNullPointer;
	
	if (GetDictionaryHeader() == NULL)
		return eMemoryNullPointer;
	
	*aHash = GetDictionaryHeader()->HASH;
	
	return eOK;
}

/** *********************************************************************
* Возвращает ID словарной базы
*
* @param[out]	aDictID				- указатель, по которому будет записан результат
* @param[in]	aDictionaryIndex	- индекс словаря, для которого мы хотим получить информацию
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetDictionaryID(UInt32* aDictID, const Int32 aDictionaryIndex) const
{
	if (!aDictID)
		return eMemoryNullPointer;
	
	if (GetDictionaryHeader() == NULL)
		return eMemoryNullPointer;
	
	*aDictID = GetDictionaryHeader()->DictID;
	
	return eOK;
}

/** *********************************************************************
* Возвращает ID словарной базы, сконвертированный в строку
*
* @param[out]	aDictID	- указатель, по которому будет записан результат
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetDictionaryStringID(UInt16* aDictID) const
{
	if (!aDictID)
		return eMemoryNullPointer;

	if (GetDictionaryHeader() == NULL)
		return eMemoryNullPointer;

	UInt8 flag = 0;

	ESldError error = IsETBDatabase(&flag);
	if (error != eOK)
		return error;

	/// 8ми-значная .datf база
	if (flag != 0)
	{
		for (UInt8 i = 0; i < 8; i++)
		{
			aDictID[i] = '0';
		}

		UInt16 idStr[9];
		sldMemZero(idStr, 9*sizeof(UInt16));

		CSldCompare::UInt32ToStr(GetDictionaryHeader()->DictID, idStr, 16);
		UInt32 len = CSldCompare::StrLen(idStr);

		for (UInt8 i = 0; i < len; i++)
		{
			aDictID[8-len+i] = idStr[i];
		}
		aDictID[8] = 0;
	}

	/// 4х-значная .sdc база
	else
	{
		for (UInt8 i = 0; i < 4; i++)
		{
			sldMemMove(aDictID + i, (char*)&GetDictionaryHeader()->DictID + i, 1);
		}
	}

	return eOK;
}

/** *********************************************************************
* Возвращает маркетинговое общее количество слов в словаре
*
* @param[out]	aWordsCount	- указатель, по которому будет записан результат
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetMarketingTotalWordsCount(Int32* aWordsCount) const
{
	if (!aWordsCount)
		return eMemoryNullPointer;
	
	*aWordsCount = GetDictionaryHeader()->MarketingTotalWordsCount;
	
	return eOK;
}

/** *********************************************************************
* Возвращает общее количество картинок в словаре
*
* @param[out]	aCount	- указатель, по которому будет записан результат
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetTotalPictureCount(Int32* aCount) const
{
	if (!aCount)
		return eMemoryNullPointer;

	*aCount = GetDictionaryHeader()->TotalPictureCount;

	return eOK;
}

/** *********************************************************************
* Возвращает общее количество озвучек в словаре
*
* @param[out]	aCount	- указатель, по которому будет записан результат
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetTotalSoundCount(Int32* aCount) const
{
	if (!aCount)
		return eMemoryNullPointer;

	*aCount = GetDictionaryHeader()->TotalSoundCount;

	return eOK;
}

/** *********************************************************************
* Возвращает максимальный размер перевода в данном словаре, в символах
*
* @param[out]	aWordMaxSize	- указатель, по которому будет записан результат
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetTranslateMaxBlockSize(Int32* aWordMaxSize) const
{
	if (!aWordMaxSize)
		return eMemoryNullPointer;

	*aWordMaxSize = GetDictionaryHeader()->ArticlesBufferSize;

	return eOK;
}


/** *********************************************************************
* Возвращает максимальный размер перевода в данном словаре, в символах
*
* @param[out]	aWordMaxSize	- указатель, по которому будет записан результат
*
* Устаревшее название, нужно удалить
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetHeadwordMaxSize(Int32* aWordMaxSize) const
{
	if (!aWordMaxSize)
		return eMemoryNullPointer;

	*aWordMaxSize = 0;
	UInt32 currentWordSize = 0;

	Int32 listCount = 0;
	ESldError error = GetNumberOfLists(&listCount);
	if (error != eOK)
		return error;

	for (UInt32 currentList = 0; currentList < listCount; currentList++)
	{
		const CSldListInfo* listInfo = NULL;
		ESldError error = GetWordListInfo(currentList, &listInfo);
		if (error != eOK)
			return error;

		currentWordSize = listInfo->GetMaximumWordSize();
		if (currentWordSize + 1 > *aWordMaxSize)
		{
			*aWordMaxSize = currentWordSize + 1;
		}
	}

	return eOK;
}

/** *********************************************************************
* Получает основной язык словаря
*
* @param[out]	aLanguage	- указатель на переменную, в которую будет сохранен код языка
*
* @return код ошибки
************************************************************************/
ESldLanguage CSldDictionary::GetLanguageFrom() const
{
	if (GetDictionaryHeader() == NULL)
		return SldLanguage::Unknown;
	
	return SldLanguage::fromCode(GetDictionaryHeader()->LanguageCodeFrom);
}

/** *********************************************************************
* Получает основной язык текущего списка слов
*
* @param[out]	aLanguage	- указатель на переменную, в которую будет сохранен код языка
*
* @return код ошибки
************************************************************************/
ESldLanguage CSldDictionary::GetCurrentLanguageFrom() const
{
	return GetListLanguageFrom(GetCurrentListIndex());
}

/** *********************************************************************
* Получает основной язык указанного списка слов
*
* @param[in]	aListIndex	- номер списка слов
* @param[out]	aLanguage	- указатель на переменную, в которую будет сохранен код языка
*
* @return код ошибки
************************************************************************/
ESldLanguage CSldDictionary::GetListLanguageFrom(Int32 aListIndex) const
{
	const CSldListInfo* listInfo = NULL;
	ESldError error = GetWordListInfo(aListIndex, &listInfo);
	if (error != eOK)
		return SldLanguage::Unknown;

	return listInfo->GetLanguageFrom();
}

/** *********************************************************************
* Получает язык перевода словаря
*
* @param[out]	aLanguage	- указатель на переменную, в которую будет сохранен код языка
*
* @return код ошибки
************************************************************************/
ESldLanguage CSldDictionary::GetLanguageTo() const
{
	if (GetDictionaryHeader() == NULL)
		return SldLanguage::Unknown;

	return SldLanguage::fromCode(GetDictionaryHeader()->LanguageCodeTo);
}

/** *********************************************************************
* Получает язык перевода текущего списка слов
*
* @param[out]	aLanguage	- указатель на переменную, в которую будет сохранен код языка
*
* @return код ошибки
************************************************************************/
ESldLanguage CSldDictionary::GetCurrentLanguageTo() const
{
	return GetListLanguageTo(GetCurrentListIndex());
}

/** *********************************************************************
* Получает язык перевода указанного списка слов
*
* @param[in]	aListIndex	- номер списка слов
* @param[out]	aLanguage	- указатель на переменную, в которую будет сохранен код языка
*
* @return код ошибки
************************************************************************/
ESldLanguage CSldDictionary::GetListLanguageTo(Int32 aListIndex) const
{
	const CSldListInfo* listInfo = NULL;
	ESldError error = GetWordListInfo(aListIndex, &listInfo);
	if (error != eOK)
		return SldLanguage::Unknown;

	return listInfo->GetLanguageTo();
}

/** *********************************************************************
* Получает количество кодов использования текущего списка слов
*
* @param[out]	aCount	- указатель на переменную, в которую будет сохранен результат
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetCurrentListUsageCount(Int32* aCount)
{
	return GetListUsageCount(GetCurrentListIndex(), aCount);
}

/** *********************************************************************
* Получает количество кодов использования указанного списка слов
*
* @param[in]	aListIndex	- номер списка слов
* @param[out]	aCount		- указатель на переменную, в которую будет сохранен результат
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetListUsageCount(Int32 aListIndex, Int32* aCount)
{
	if (!aCount)
		return eMemoryNullPointer;
	
	ISldList* pList = 0;
	ESldError error = GetWordList(aListIndex, &pList);
	if (error != eOK)
		return error;
	
	if (!pList)
		return eMemoryNullPointer;
	
	error = pList->GetUsageCount(aCount);
	if (error != eOK)
		return error;

	return eOK;
}

/** *********************************************************************
* Получает количество кодов использования указанного списка слов
*
* @param[in]	aListIndex	- номер списка слов
* @param[out]	aCount		- указатель на переменную, в которую будет сохранен результат
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetUsageCountByListIndex(Int32 aListIndex, Int32* aCount)
{
	return GetListUsageCount(aListIndex, aCount);
}

/** *********************************************************************
* Получает код использования текущего списка слов по индексу кода (см #EWordListTypeEnum)
*
* @param[in]	aIndex	- индекс кода использования
* @param[out]	aUsage	- указатель на переменную, в которую будет сохранен код
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetCurrentListUsage(Int32 aIndex, UInt32* aUsage)
{
	return GetListUsage(GetCurrentListIndex(), aIndex, aUsage);
}

/** *********************************************************************
* Получает код использования указанного списка слов по индексу кода (см #EWordListTypeEnum)
*
* @param[in]	aListIndex	- номер списка слов
* @param[in]	aUsageIndex	- индекс кода использования
* @param[out]	aUsage		- указатель на переменную, в которую будет сохранен код
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetListUsage(Int32 aListIndex, Int32 aUsageIndex, UInt32* aUsage)
{
	if (!aUsage)
		return eMemoryNullPointer;

	ISldList* pList = 0;
	ESldError error = GetWordList(aListIndex, &pList);
	if (error != eOK)
		return error;
	
	if (!pList)
		return eMemoryNullPointer;

	error = pList->GetListUsage(aUsageIndex, aUsage);
	if (error != eOK)
		return error;
	
	return eOK;
}

/** *********************************************************************
* Получает код использования указанного списка слов по индексу кода (см #EWordListTypeEnum)
*
* @param[in]	aListIndex	- номер списка слов
* @param[in]	aUsageIndex	- индекс кода использования
* @param[out]	aUsage		- указатель на переменную, в которую будет сохранен код
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetUsageByListIndex(Int32 aListIndex, Int32 aUsageIndex, UInt32* aUsage)
{
	return GetListUsage(aListIndex, aUsageIndex, aUsage);
}

/** *********************************************************************
* Производит перевод указанной статьи по локальному номеру слова в текущем списке слов и номеру перевода
* В результате текст статьи будет поблочно передаваться в функцию сборки перевода класса прослойки
*
* @param[in]	aIndex			- локальный номер слова в текущем списке слов, для которого требуется получить перевод
* @param[in]	aVariantIndex	- номер перевода
* @param[in]	aFullness		- полнота получаемого перевода
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::Translate(Int32 aIndex, Int32 aVariantIndex, ESldTranslationFullnesType aFullness, UInt32 aStartBlock, UInt32 aEndBlock)
{
	return Translate(GetCurrentListIndex(), aIndex, aVariantIndex, aFullness);
}

/** *********************************************************************
* Производит перевод указанной статьи по локальному номеру слова в указанном списке слов и номеру перевода
* В результате текст статьи будет поблочно передаваться в функцию сборки перевода класса прослойки
*
* @param[in]	aListIndex		- номер списка слов
* @param[in]	aIndex			- локальный номер слова в текущем списке слов, для которого требуется получить перевод
* @param[in]	aVariantIndex	- номер перевода
* @param[in]	aFullness		- полнота получаемого перевода
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::Translate(Int32 aListIndex, Int32 aIndex, Int32 aVariantIndex, ESldTranslationFullnesType aFullness, UInt32 aStartBlock, UInt32 aEndBlock)
{
	Int32 globalIndex = aIndex;
	ESldError error = LocalIndex2GlobalIndex(aListIndex, aIndex, &globalIndex);
	if (error != eOK)
		return error;

	error = GoToRealIndexes(aListIndex, globalIndex);
	if (error != eOK)
		return error;

	if (aListIndex == SLD_DEFAULT_LIST_INDEX)
		return eOK;

	ISldList* pList = NULL;
	error = GetWordList(aListIndex, &pList);
	if (error != eOK)
		return error;

	if (!pList)
		return eMemoryNullPointer;

	Int32 NumberOfWords = 0;
    error = pList->GetTotalWordCount(&NumberOfWords);
	if (error != eOK)
		return error;

	if (globalIndex >= NumberOfWords || globalIndex < 0)
		return eCommonWrongIndex;

	Int32 NumberOfTranslation = 0;
	error = pList->GetTranslationCount(globalIndex, &NumberOfTranslation);
	if (error != eOK)
		return error;

	if (aVariantIndex >= NumberOfTranslation || aVariantIndex < 0)
		return eCommonWrongIndex;

	UInt32 listType = 0;
	error = pList->GetListUsage(0, &listType);
	if (error != eOK)
		return error;

	Int32 ArticleIndex = 0;
	if (listType == eWordListType_Atomic && m_AtomicInfo)
	{
		ArticleIndex = m_AtomicInfo[globalIndex].TranslateIndex;
		aStartBlock = m_AtomicInfo[globalIndex].FirstBlockIndex;
		aEndBlock = m_AtomicInfo[globalIndex].FirstBlockIndex + m_AtomicInfo[globalIndex].BlockCount;
	}
	else
	{
		error = pList->GetTranslationIndex(globalIndex, aVariantIndex, &ArticleIndex);
		if (error != eOK)
			return error;
	}

	error = TranslateByArticleIndex(ArticleIndex, aFullness, aStartBlock, aEndBlock);
	if (error != eOK)
		return error;

	// Сохраняем данные из системы защиты.
	error = SldSaveRegistrationData(GetDictionaryHeader()->DictID, &m_RegistrationData, &m_RandomSeed, GetLayerAccess());
	if (error != eOK)
		return error;

	return eOK;
}


/** *********************************************************************
* Производит перевод указанной статьи по глобальному номеру статьи
*
* @param[in]	aIndex			- глобальный индекс статьи
* @param[in]	aFullness		- полнота получаемого перевода
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::TranslateByArticleIndex(Int32 aIndex, ESldTranslationFullnesType aFullness, UInt32 aStartBlock, UInt32 aEndBlock)
{
	if (m_Articles == NULL)
		return eCommonDictionaryHasNoTranslations;
	return m_Articles->Translate(aIndex, aFullness, aStartBlock, aEndBlock);
}

/** *********************************************************************
* Производит получение остатков перевода (если он в какой-то момент был приостановлен)
*
* @param[in]	aFullness	- степень полноты полученного перевода
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::TranslateContinue(ESldTranslationFullnesType aFullness)
{
	if (m_Articles == NULL)
		return eCommonDictionaryHasNoTranslations;
	return m_Articles->TranslateContinue(aFullness);
}


/** *********************************************************************
* Производит обновление состояния перевода (завершает предыдущий прерванный
* перевод и производит реинициализацию
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::TranslateClear()
{
	if (m_Articles == NULL)
		return eCommonDictionaryHasNoTranslations;

	return m_Articles->TranslateClear();
}


/** *********************************************************************
* Возвращает количество переводов слова по локальному номеру слова в текущем списке слов
*
* @param[in]	aIndex					- локальный номер слова, для которого нужно получить информацию
* @param[out]	aNumberOfTraslations	- указатель на переменную, в которую сохраняется количество переводов
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetNumberOfTranslations(Int32 aIndex, Int32* aNumberOfTraslations)
{
	return GetNumberOfTranslations(GetCurrentListIndex(), aIndex, aNumberOfTraslations);
}

/** *********************************************************************
* Возвращает количество переводов слова по локальному номеру слова в указанном списке слов
*
* @param[in]	aListIndex				- номер списка слов
* @param[in]	aIndex					- локальный номер слова, для которого нужно получить информацию
* @param[out]	aNumberOfTraslations	- указатель на переменную, в которую сохраняется количество переводов
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetNumberOfTranslations(Int32 aListIndex, Int32 aIndex, Int32* aNumberOfTraslations)
{
	if (!aNumberOfTraslations)
		return eMemoryNullPointer;

	*aNumberOfTraslations = 0;

	const CSldListInfo * listInfo = NULL;
	ESldError error = GetWordListInfo(aListIndex, &listInfo);
	if (error != eOK)
		return error;

	if (listInfo->IsFullTextSearchListType())
		return eOK;

	Int32 globalIndex = SLD_DEFAULT_WORD_INDEX;
	error = LocalIndex2GlobalIndex(aIndex, &globalIndex);
	if (error != eOK)
		return error;

	error = GoToRealIndexes(aListIndex, globalIndex);
	if (error != eOK)
		return error;

	if (aListIndex == SLD_DEFAULT_LIST_INDEX)
		return eOK;

	error = GetWordListInfo(aListIndex, &listInfo);
	if (error != eOK)
		return error;

	if (listInfo->GetUsage() == eWordListType_MorphologyBaseForm || listInfo->GetUsage() == eWordListType_MorphologyInflectionForm)
		return eOK;
	
	Int32 numberOfWords;
	error = GetTotalWordCount(aListIndex, &numberOfWords);
	if (error != eOK)
		return error;

	if (globalIndex < 0 || (globalIndex && globalIndex >= numberOfWords))
		return eCommonWrongIndex;

	ISldList* pList = 0;
	error = GetWordList(aListIndex, &pList);
	if (error != eOK)
		return error;

	if (!pList)
		return eMemoryNullPointer;

	return pList->GetTranslationCount(globalIndex, aNumberOfTraslations);
}

/** *********************************************************************
* Возвращает путь к текущему положению в каталоге в текущем списке слов
*
* @param[out]	aPath	- указатель на структуру, в которую будет помещен путь к 
*						  текущему положению в каталоге
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetCurrentPath(TCatalogPath *aPath)
{
	return GetCurrentPath(GetCurrentListIndex(), aPath);
}

/** *********************************************************************
* Возвращает путь к текущему положению в каталоге в указанном списке слов
*
* @param[in]	aListIndex	- номер списка слов
* @param[out]	aPath		- указатель на структуру, в которую будет помещен путь к 
*							  текущему положению в каталоге
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetCurrentPath(Int32 aListIndex, TCatalogPath *aPath)
{
	ISldList* pList = 0;
	ESldError error = GetWordList(aListIndex, &pList);
	if (error != eOK)
		return error;
	
	if (!pList)
		return eMemoryNullPointer;
		
	error = pList->GetCurrentPath(aPath);
	if (error != eOK)
		return error;
	
	aPath->ListIndex.Set(aListIndex);
	
	return eOK;
}

/** *********************************************************************
* Переходит по указанному пути
*
* @param[in]	aPath				- указатель на структуру, в которой содержится путь к месту 
*									  в каталоге, куда нужно попасть
* @param[in]	aNavigationType		- тип перехода
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GoToByPath(TCatalogPath* aPath, ESldNavigationTypeEnum aNavigationType)
{
	if (!aPath)
		return eMemoryNullPointer;
		
	ESldError error;
	UInt32 NewListIndex = aPath->ListIndex;
	
	if ((Int32)NewListIndex != GetCurrentListIndex())
	{
		error = SetCurrentWordlist(NewListIndex);
		if (error != eOK)
			return error;
	}

	ISldList* pList = 0;
	error = GetWordList(GetCurrentListIndex(), &pList);
	if (error != eOK)
		return error;

	if (!pList)
		return eMemoryNullPointer;

	error = pList->GoToByPath(aPath, aNavigationType);
	if (error != eOK)
		return error;
	
	return eOK;
}

/** *********************************************************************
* Поднимаемся в каталоге в текущем списке слов на уровень выше текущего
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GoToLevelUp(void)
{
	return GoToLevelUp(GetCurrentListIndex());
}

/** *********************************************************************
* Поднимаемся в каталоге в указанном списке слов на уровень выше текущего
* 
* @param[in]	aListIndex	- номер списка слов
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GoToLevelUp(Int32 aListIndex)
{
	ISldList* pList = 0;
	ESldError error = GetWordList(aListIndex, &pList);
	if (error != eOK)
		return error;
	
	if (!pList)
		return eMemoryNullPointer;
		
	return pList->GoToLevelUp();
}

/** *********************************************************************
* Возвращает название родительской категории для текущего слова в текущем списке слов
* Если у текущего слова нет родительской категории, возвращает пустую строку
*
* ВНИМАНИЕ! Память для результирующей строки выделяется в этом методе
* и должна быть освобождена в вызывающем методе вызовом функции sldMemFree()
*
* @param[out]	aText	- указатель, по которому будет записан указатель на строку-результат
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetCurrentParentWordName(UInt16** aText)
{	
	return GetCurrentParentWordName(GetCurrentListIndex(), aText);
}

/** *********************************************************************
* Возвращает название родительской категории для текущего слова в указанном списке слов
* Если у текущего слова в указанном списке слов нет родительской категории, возвращает пустую строку
*
* ВНИМАНИЕ! Память для результирующей строки выделяется в этом методе
* и должна быть освобождена в вызывающем методе вызовом функции sldMemFree()
*
* @param[in]	aListIndex	- номер списка слов
* @param[out]	aText		- указатель, по которому будет записан указатель на строку-результат
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetCurrentParentWordName(Int32 aListIndex, UInt16** aText)
{
	ISldList* pList = 0;
	ESldError error = GetWordList(aListIndex, &pList);
	if (error != eOK)
		return error;
	
	if (!pList)
		return eMemoryNullPointer;
		
	Int32 CurrGlobalIndex = 0;
	error = pList->GetCurrentGlobalIndex(&CurrGlobalIndex);
	if (error != eOK)
		return error;
		
	error = pList->GetParentWordNamesByGlobalIndex(CurrGlobalIndex, aText, NULL, 1);
	if (error != eOK)
		return error;
	
	return eOK;
}

/** *********************************************************************
* По глобальному номеру слова в текущем списке слов
* возвращает список названий родительских категорий, разделенных строкой-разделителем;
* если у слова нет родительских категорий, возвращает пустую строку
*
* ВНИМАНИЕ! Память для результирующей строки выделяется в этом методе
* и должна быть освобождена в вызывающем методе вызовом функции sldMemFree()
*
* @param[in]	aGlobalWordIndex	- глобальный номер слова
* @param[out]	aText				- указатель, по которому будет записан указатель на строку-результат
* @param[in]	aSeparatorText		- указатель на строку-разделитель, может быть NULL
* @param[in]	aMaxLevelCount		- максимальное количество родительских категорий (уровней иерархии),
*									  названия которых нужно узнать. Значение -1 означает все уровни
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetParentWordNamesByGlobalIndex(Int32 aGlobalWordIndex, UInt16** aText, const UInt16* aSeparatorText, Int32 aMaxLevelCount)
{	
	return GetParentWordNamesByGlobalIndex(GetCurrentListIndex(), aGlobalWordIndex, aText, aSeparatorText, aMaxLevelCount);
}

/** *********************************************************************
* Возвращает список названий родительских категорий, разделенных строкой-разделителем, по глобальному номеру слова в указанном списке слов
* Если у слова нет родительских категорий, возвращает пустую строку
*
* ВНИМАНИЕ! Память для результирующей строки выделяется в этом методе
* и должна быть освобождена в вызывающем методе вызовом функции sldMemFree()
*
* @param[in]	aListIndex			- номер списка слов
* @param[in]	aGlobalWordIndex	- глобальный номер слова
* @param[out]	aText				- указатель, по которому будет записан указатель на строку-результат
* @param[in]	aSeparatorText		- указатель на строку-разделитель, может быть NULL
* @param[in]	aMaxLevelCount		- максимальное количество родительских категорий (уровней иерархии),
*									  названия которых нужно узнать. Значение -1 означает все уровни
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetParentWordNamesByGlobalIndex(Int32 aListIndex, Int32 aGlobalWordIndex, UInt16** aText, const UInt16* aSeparatorText, Int32 aMaxLevelCount)
{
	ISldList* pList = 0;
	ESldError error = GetWordList(aListIndex, &pList);
	if (error != eOK)
		return error;
	
	if (!pList)
		return eMemoryNullPointer;
		
	error = pList->GetParentWordNamesByGlobalIndex(aGlobalWordIndex, aText, aSeparatorText, aMaxLevelCount);
	if (error != eOK)
		return error;
	
	return eOK;
}

/** *********************************************************************
* Возвращает глобальный номер слова-родителя по глобальному номеру слова в текущем списке слов
* Если у слова нет родительских категорий, глобальный номер слова-родителя будет равен -1
*
* @param[in]	aGlobalWordIndex		- глобальный номер слова
* @param[out]	aParentWordGlobalIndex	- указатель, по которому записывается глобальный номер слова-родителя
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetParentWordGlobalIndex(Int32 aGlobalWordIndex, Int32* aParentWordGlobalIndex)
{
	return GetParentWordGlobalIndex(GetCurrentListIndex(), aGlobalWordIndex, aParentWordGlobalIndex);
}

/** *********************************************************************
* Возвращает глобальный номер слова-родителя по глобальному номеру слова в указанном списке слов
* Если у слова нет родительских категорий, глобальный номер слова-родителя будет равен -1
*
* @param[in]	aListIndex				- номер списка слов
* @param[in]	aGlobalWordIndex		- глобальный номер слова
* @param[out]	aParentWordGlobalIndex	- указатель, по которому записывается глобальный номер слова-родителя
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetParentWordGlobalIndex(Int32 aListIndex, Int32 aGlobalWordIndex, Int32* aParentWordGlobalIndex)
{
	if (!aParentWordGlobalIndex)
		return eMemoryNullPointer;
	
	*aParentWordGlobalIndex = -1;

	ISldList* pList = 0;
	ESldError error = GetWordList(aListIndex, &pList);
	if (error != eOK)
		return error;
	
	if (!pList)
		return eMemoryNullPointer;

	if (aGlobalWordIndex < 0)
		return eOK;
	
	error = pList->SaveCurrentState();
	if (error != eOK)
		return error;

	TCatalogPath Path;
	error = pList->GetPathByGlobalIndex(aGlobalWordIndex, &Path);
	if (error != eOK)
		return error;
	
	if (Path.isRoot())
	{
		error = pList->RestoreState();
		if (error != eOK)
			return error;
		
		return eOK;
	}
	
	Int32 ParentLocalIndex = Path.GetParentWordLocalIndex();
	
	error = pList->GoToByPath(&Path, eGoToWord);
	if (error != eOK)
		return error;
	
	error = pList->GoToLevelUp();
	if (error != eOK)
		return error;
	
	error = pList->LocalIndex2GlobalIndex(ParentLocalIndex, aParentWordGlobalIndex);
	if (error != eOK)
		return error;
	
	error = pList->RestoreState();
	if (error != eOK)
		return error;
	
	return eOK;
}

/** *********************************************************************
* Возвращает границы глобальных номеров слов-потомков по глобальному номеру слова-родителя в текущем списке слов
* Получается диапазон глобальных индексов вида [aFirstGlobalIndex, aLastGlobalIndex) - левая граница включается, правая - нет
* Если у слова-родителя нет потомков, оба глобальных индекса будут равны -1
*
* @param[in]	aParentGlobalWordIndex	- глобальный номер слова-родителя
* @param[out]	aFirstGlobalIndex		- указатель, по которому записывается индекс левой границы
* @param[out]	aLastGlobalIndex		- указатель, по которому записывается индекс правой границы
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetChildrenWordsGlobalIndexes(Int32 aParentGlobalWordIndex, Int32* aFirstGlobalIndex, Int32* aLastGlobalIndex)
{
	return GetChildrenWordsGlobalIndexes(GetCurrentListIndex(), aParentGlobalWordIndex, aFirstGlobalIndex, aLastGlobalIndex);
}

/** *********************************************************************
* Возвращает границы глобальных номеров слов-потомков по глобальному номеру слова-родителя в указанном списке слов
* Получается диапазон глобальных индексов вида [aFirstGlobalIndex, aLastGlobalIndex) - левая граница включается, правая - нет
* Если у слова-родителя нет потомков, оба глобальных индекса будут равны -1
*
* @param[in]	aListIndex				- номер списка слов
* @param[in]	aParentGlobalWordIndex	- глобальный номер слова-родителя
* @param[out]	aFirstGlobalIndex		- указатель, по которому записывается индекс левой границы
* @param[out]	aLastGlobalIndex		- указатель, по которому записывается индекс правой границы
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetChildrenWordsGlobalIndexes(Int32 aListIndex, Int32 aParentGlobalWordIndex, Int32* aFirstGlobalIndex, Int32* aLastGlobalIndex)
{
	if (!aFirstGlobalIndex || !aLastGlobalIndex)
		return eMemoryNullPointer;
	
	*aFirstGlobalIndex = -1;
	*aLastGlobalIndex = -1;

	ISldList* pList = 0;
	ESldError error = GetWordList(aListIndex, &pList);
	if (error != eOK)
		return error;
	
	if (!pList)
		return eMemoryNullPointer;

	if (aParentGlobalWordIndex < 0)
		return eOK;
	
	error = pList->SaveCurrentState();
	if (error != eOK)
		return error;

	TCatalogPath Path;
	error = pList->GetPathByGlobalIndex(aParentGlobalWordIndex, &Path);
	if (error != eOK)
		return error;
	
	error = pList->GoToByPath(&Path, eGoToWord);
	if (error != eOK)
		return error;
	
	Int32 CurrLocalIndex = 0;
	error = pList->GetCurrentIndex(&CurrLocalIndex);
	if (error != eOK)
		return error;
	
	UInt32 hasChildren = 0;
	error = pList->isWordHasHierarchy(CurrLocalIndex, &hasChildren, NULL);
	if (error != eOK)
		return error;
	
	if (!hasChildren)
	{
		error = pList->RestoreState();
		if (error != eOK)
			return error;
		
		return eOK;
	}
	
	error = pList->SetBase(CurrLocalIndex);
	if (error != eOK)
		return error;
	
	error = pList->GetHierarchyLevelBounds(aFirstGlobalIndex, aLastGlobalIndex);
	if (error != eOK)
		return error;
	
	error = pList->RestoreState();
	if (error != eOK)
		return error;
		
	return eOK;
}
	
/** *********************************************************************
* Возвращает путь к слову по глобальному номеру слова (без учета иерархии) 
* в текущем списке слов (любом - в каталоге, словаре и т.д.)
* Последний элемент в списке элементов пути содержит локальный номер слова 
* на конечном уровне вложенности
*
* @param[in]	aGlobalWordIndex	- номер слова без учета иерархии (глобальный номер слова)
* @param[out]	aPath				- указатель на структуру, в которую будет сохранен путь к 
*									  указанному элементу списка слов
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetPathByGlobalIndex(Int32 aGlobalWordIndex, TCatalogPath* aPath)
{	
	return GetPathByGlobalIndex(GetCurrentListIndex(), aGlobalWordIndex, aPath);
}

/** *********************************************************************
* Возвращает путь к слову по глобальному номеру слова (без учета иерархии) 
* в указанном списке слов (любом - в каталоге, словаре и т.д.)
* Последний элемент в списке элементов пути содержит локальный номер слова 
* на конечном уровне вложенности
*
* @param[in]	aListIndex			- номер списка слов
* @param[in]	aGlobalWordIndex	- номер слова без учета иерархии (глобальный номер слова)
* @param[out]	aPath				- указатель на структуру, в которую будет сохранен путь к 
*									  указанному элементу списка слов
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetPathByGlobalIndex(Int32 aListIndex, Int32 aGlobalWordIndex, TCatalogPath* aPath)
{
	ISldList* pList = 0;
	ESldError error = GetWordList(aListIndex, &pList);
	if (error != eOK)
		return error;
	
	if (!pList)
		return eMemoryNullPointer;
	
	if (aGlobalWordIndex < 0)
		return eCommonWrongIndex;
		
	error = pList->GetPathByGlobalIndex(aGlobalWordIndex, aPath);
	if (error != eOK)
		return error;
	
	aPath->ListIndex.Set(aListIndex);
	
	return eOK;
}

/** *********************************************************************
* Возвращает класс, хранящий информацию о свойствах списка слов
*
* @param[in]	aListIndex	- номер списка слов
* @param[out]	aListInfo	- по данному указателю будет записан указатель на класс, 
*							  который хранит данные о списке слов
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetWordListInfo(Int32 aListIndex, const CSldListInfo **aListInfo) const
{
	ESldError error;
	Int32 ListCount = 0;
	
	if (!aListInfo)
		return eMemoryNullPointer;

	error = GetNumberOfLists(&ListCount);
	if (error != eOK)
		return error;
		
	if (aListIndex >= ListCount || aListIndex < 0)
		return eCommonWrongList;
	
	*aListInfo = m_ListInfo[aListIndex];
	
	return eOK;
}

/** *********************************************************************
* Возвращает класс, хранящий информацию о свойствах текущего списка слов
*
* @param[out]	aListInfo	- по данному указателю будет записан указатель на класс,
*							  который хранит данные о списке слов
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetWordListInfo(const CSldListInfo **aListInfo) const
{
	return GetWordListInfo(GetCurrentListIndex(), aListInfo);
}

/** ********************************************************************
* Возвращает класс, хранящий информацию об указанном стиле
*
* @param[in]	aIndex		- номер стиля
* @param[out]	aStyleInfo	- по данному указателю будет записан адрес класса, 
*							  который хранит информацию о стиле
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetStyleInfo(Int32 aIndex, const CSldStyleInfo **aStyleInfo) const
{
	if (!aStyleInfo)
		return eMemoryNullPointer;
	if (m_Articles == NULL)
		return eCommonDictionaryHasNoTranslations;

	return m_Articles->GetStyleInfo(aIndex, aStyleInfo);
}

/** *********************************************************************
* Возвращает количество стилей
*
* @return количество стилей
************************************************************************/
UInt32 CSldDictionary::GetNumberOfStyles() const
{
	return m_Articles ? m_Articles->GetNumberOfStyles() : 0;
}

/** *********************************************************************
* Устанавливает для стиля вариант отображения по умолчанию
*
* @param[in]	aStyleIndex		- номер стиля
* @param[in]	aVariantIndex	- номер варианта отображения
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::SetDefaultStyleVariant(Int32 aStyleIndex, Int32 aVariantIndex)
{
	if (m_Articles == NULL)
		return eCommonDictionaryHasNoTranslations;
	
	ESldError error = m_Articles->SetDefaultStyleVariant(aStyleIndex, aVariantIndex);
	if (error != eOK)
		return error;
	
	return eOK;
}


/** *********************************************************************
* Устанавливает для всех стилей вариант отображения по умолчанию
*
* @param[in]	aVariantIndex	- номер варианта отображения
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::SetDefaultStyleVariant(Int32 aVariantIndex)
{
	if (m_Articles == NULL)
		return eCommonDictionaryHasNoTranslations;

	Int32 numberOfStyles;

	ESldError error = GetStylesWithStressVariants(NULL, &numberOfStyles);
	if (error != eOK)
		return error;
	
	Int32* stylesArray = (Int32*)sldMemNewZero(numberOfStyles*sizeof(stylesArray[0]));
	if (stylesArray)
	{
		error = GetStylesWithStressVariants(stylesArray, &numberOfStyles);
		if (error != eOK)
		{
			sldMemFree(stylesArray);
			return error;
		}

		for (Int32 i=0;i<numberOfStyles;i++)
		{
			error = m_Articles->SetDefaultStyleVariant(stylesArray[i], aVariantIndex);
			if (error != eOK)
			{
				sldMemFree(stylesArray);
				return error;
			}
		}

		sldMemFree(stylesArray);
	}

	return eOK;
}

/** *********************************************************************
* Возвращает класс, хранящий информацию о строчках для всего словаря
*
* @param[out] aStrings			- указатель по которому будет записан указатель на
*								  объект хранящий строчки с переводами
* @param[in] aDictionaryIndex	- индекс словаря, для которого мы хотим получить информацию
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetLocalizedStrings(const CSldLocalizedString** aStrings, const Int32 aDictionaryIndex) const
{
	if (!aStrings)
		return eMemoryNullPointer;

	*aStrings = m_Strings.get();

	return eOK;
}

/** *********************************************************************
* Возвращает класс, хранящий информацию о строчках для конкретного списка слов
*
* @param[in]	aListIndex	- индекс списка слов
* @param[out]	aStrings	- указатель, по которому будет записан указатель на нужный объект,
							  или NULL, если у списка слов нет локализованных строк
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetLocalizedStringsByListIndex(Int32 aListIndex, const CSldListLocalizedString** aStrings) const
{
	if (!aStrings)
		return eMemoryNullPointer;
	
	ESldError error;
	UInt32 isStrings = 0;
	
	error = IsListHasLocalizedStrings(aListIndex, &isStrings);
	if (error != eOK)
		return error;
	
	const CSldListInfo * listInfo;
	error = GetWordListInfo(aListIndex, &listInfo);
	if (error != eOK)
		return error;

	*aStrings = listInfo->GetLocalizedStrings();
	return eOK;
}

/** *********************************************************************
* Возвращает класс, занимающийся сравнением строк
*
* @param[out]	aCMP	- указатель, по которому будет записан указатель на объект, занимающийся сравнением строк
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetCompare(const CSldCompare** aCMP) const
{
	if (!aCMP)
		return eMemoryNullPointer;
	
	*aCMP = &m_CMP;

	return eOK;
}

/** *********************************************************************
* Производит поиск слов по шаблону в текущем списке слов,
* начиная с текущего уровня (и во всех вложенных уровнях)
*
* @param[in]	aText			- шаблон поиска
* @param[in]	aMaximumWords	- максимальное количество слов, которые могут быть найдены
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::DoWildCardSearch(TExpressionBox* aExpressionBox, Int32 aMaximumWords)
{
	return DoWildCardSearch(GetCurrentListIndex(), aExpressionBox, aMaximumWords);
}

/** *********************************************************************
* Производит поиск слов по шаблону в указанном списке слов,
* начиная с текущего уровня (и во всех вложенных уровнях)
*
* @param[in]	aListIndex		- номер списка слов
* @param[in]	aText			- шаблон поиска
* @param[in]	aMaximumWords	- максимальное количество слов, которые могут быть найдены
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::DoWildCardSearch(Int32 aListIndex, TExpressionBox* aExpressionBox, Int32 aMaximumWords)
{
	if (!aExpressionBox)
		return eMemoryNullPointer;

	ISldList* pList = NULL;
	CSldList* realList = NULL;
	ESldError error = GetWordList(aListIndex, &pList);
	if (error != eOK)
		return error;

	if (!pList)
		return eMemoryNullPointer;

	const CSldListInfo * listInfo;
	error = GetWordListInfo(aListIndex, &listInfo);
	if (error != eOK)
		return error;

	Int32 simpleSortedListIndex = SLD_DEFAULT_LIST_INDEX;
	if (pList->HasSimpleSortedList())
		simpleSortedListIndex = listInfo->GetSimpleSortedListIndex();

	if (simpleSortedListIndex != SLD_DEFAULT_LIST_INDEX && !listInfo->IsHierarchy())
	{
		realList = (CSldList*)pList;
		pList = NULL;
		error = GetWordList(simpleSortedListIndex, &pList);
		if (error != eOK)
			return error;

		if (!pList)
		{
			error = GetWordList(aListIndex, &pList);
			if (error != eOK)
				return error;

			if (!pList)
				return eMemoryNullPointer;
		}
	}
	else if (listInfo->GetUsage() == eWordListType_SimpleSearch)
	{
		return eCommonWrongList;
	}

	Int32 ListCount = 0;
	// Количество списков в словаре
	error = GetNumberOfLists(&ListCount);
	if (error != eOK)
		return error;

	// Сохраняем текущее состояние списка слов
	error = pList->SaveCurrentState();
	if (error != eOK)
		return error;

	// Создаем новый поисковый список
	auto pSearchList = sld2::make_unique<CSldSearchList>();
	if (!pSearchList)
		return eMemoryNotEnoughMemory;

	// Инициализируем поисковый список
	error = pSearchList->Init(m_data, GetLayerAccess(), listInfo, NULL, 0);
	if (error != eOK)
		return error;

	// Устанавливаем максимальное количество списков, в которых мы можем производить поиск
	error = pSearchList->SetMaximumLists(ListCount);
	if (error != eOK)
		return error;

	// Устанавливаем максимальное количество слов, которое может содержать поисковый список
	error = pSearchList->SetMaximumWords(aMaximumWords);
	if (error != eOK)
		return error;

	// Производим поиск
	ESldError searchError = pSearchList->DoWildCardSearch(aExpressionBox, aMaximumWords, pList, aListIndex, realList);
	// Восстанавливаем ранее сохраненное состояние списков слов (независимо от того, успешно выполнился поиск или нет)
	error = pList->RestoreState();
	if (error != eOK)
		return error;

	// Проверяем, как выполнился поиск
	if (searchError != eOK)
		return searchError;

	// Добавляем поисковый список в общий массив списков словаря
	error = AddList(sld2::move(pSearchList), ListCount);
	if (error != eOK)
		return error;

	// Устанавливаем список с результатами поиска в качестве текущего
	error = SetCurrentWordlist(ListCount);
	if (error != eOK)
		return error;

	return eOK;
}

/** **********************************************************************
* Производит поиск похожих слов в текущем списке слов,
* начиная с текущего уровня (и во всех вложенных уровнях)
*
* @param[in]	aText				- шаблон поиска
* @param[in]	aMaximumWords		- максимальное количество слов которые могут быть найдены
* @param[in]	aMaximumDifference	- максимальная разница между искомыми словами
*									  Если aMaximumDifference == 0, то этот параметр выбирается автоматически
*									  в зависимости от длины слова
* @param[in]	aSearchMode			- тип поиска (см. #EFuzzySearchMode)
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::DoFuzzySearch(const UInt16 *aText, Int32 aMaximumWords, Int32 aMaximumDifference, EFuzzySearchMode aSearchMode)
{
	return DoFuzzySearch(GetCurrentListIndex(), aText, aMaximumWords, aMaximumDifference, aSearchMode);
}

/** **********************************************************************
* Производит поиск похожих слов в указанном списке слов,
* начиная с текущего уровня (и во всех вложенных уровнях)
*
* @param[in]	aListIndex			- номер списка слов
* @param[in]	aText				- шаблон поиска
* @param[in]	aMaximumWords		- максимальное количество слов которые могут быть найдены
* @param[in]	aMaximumDifference	- максимальная разница между искомыми словами.
*									  Если aMaximumDifference == 0, то этот параметр выбирается автоматически
*									  в зависимости от длины слова
* @param[in]	aSearchMode			- тип поиска (см. #EFuzzySearchMode)
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::DoFuzzySearch(Int32 aListIndex, const UInt16 *aText, Int32 aMaximumWords, Int32 aMaximumDifference, EFuzzySearchMode aSearchMode)
{
	if (!aText)
		return eMemoryNullPointer;
	
	ISldList* pList = 0;
	ESldError error = GetWordList(aListIndex, &pList);
	if (error != eOK)
		return error;
	
	if (!pList)
		return eMemoryNullPointer;
	
	// Устанавливаем параметр в зависимости от длины слова
	if (!aMaximumDifference)
	{
		Int32 Len = CSldCompare::StrLen(aText);
		switch (Len)
		{
			case 1:
			case 2:
			{
				aMaximumDifference = 1;
				break;
			}
			case 3:
			case 4:
			case 5:
			{
				aMaximumDifference = 2;
				break;
			}
			default:
			{
				aMaximumDifference = 3;
				break;
			}
		}
	}
	
	Int32 ListCount = 0;
	// Количество списков в словаре
	error = GetNumberOfLists(&ListCount);
	if (error != eOK)
		return error;

	// Сохраняем текущее состояние списка слов
	error = pList->SaveCurrentState();
	if (error != eOK)
		return error;
	
	// Создаем новый поисковый список
	auto pSearchList = sld2::make_unique<CSldSearchList>();
	if (!pSearchList)
		return eMemoryNotEnoughMemory;
	
	const CSldListInfo * listInfo;
	error = GetWordListInfo(aListIndex, &listInfo);
	if (error != eOK)
		return error;

	// Инициализируем поисковый список
	error = pSearchList->Init(m_data, GetLayerAccess(), listInfo, NULL, 0);
	if (error != eOK)
		return error;
	
	// Устанавливаем максимальное количество списков, в которых мы можем производить поиск
	error = pSearchList->SetMaximumLists(ListCount);
	if (error != eOK)
		return error;

	// Устанавливаем максимальное количество слов, которое может содержать поисковый список
	error = pSearchList->SetMaximumWords(aMaximumWords);
	if (error != eOK)
		return error;
	
	// Производим поиск
	ESldError searchError = pSearchList->DoFuzzySearch(aText, aMaximumWords, aMaximumDifference, pList, aListIndex, aSearchMode);
	// Восстанавливаем ранее сохраненное состояние списков слов (независимо от того, успешно выполнился поиск или нет)
	error = pList->RestoreState();
	if (error != eOK)
		return error;

	// Проверяем, как выполнился поиск
	if (searchError != eOK)
		return searchError;

	// Добавляем поисковый список в общий массив списков словаря
	error = AddList(sld2::move(pSearchList), ListCount);
	if (error != eOK)
		return error;

	// Устанавливаем список с результатами поиска в качестве текущего
	error = SetCurrentWordlist(ListCount);
	if (error != eOK)
		return error;
	
	return eOK;
}

/** *********************************************************************
* Производит поиск анаграмм в текущем списке слов,
* начиная с текущего уровня (и во всех вложенных уровнях)
*
* @param[in]	aText	- текст (набор символов), по которым будет производиться поиск слов
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::DoAnagramSearch(const UInt16 *aText, Int32 aMaximumWords)
{
	return DoAnagramSearch(GetCurrentListIndex(), aText, aMaximumWords);
}

/** *********************************************************************
* Производит поиск анаграмм в указанном списке слов,
* начиная с текущего уровня (и во всех вложенных уровнях)
*
* @param[in]	aListIndex	- номер списка слов
* @param[in]	aText		- текст (набор символов), по которым будет производиться поиск слов
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::DoAnagramSearch(Int32 aListIndex, const UInt16 *aText, Int32 aMaximumWords)
{
	if (!aText)
		return eMemoryNullPointer;
	
	Int32 TextLen = CSldCompare::StrLen(aText);
	if (TextLen < 1)
		return eOK;
	
	ISldList* pList = 0;
	ESldError error = GetWordList(aListIndex, &pList);
	if (error != eOK)
		return error;
	
	if (!pList)
		return eMemoryNullPointer;
	
	Int32 ListCount = 0;
	// Количество списков в словаре
	error = GetNumberOfLists(&ListCount);
	if (error != eOK)
		return error;
	
	// Сохраняем текущее состояние списка слов
	error = pList->SaveCurrentState();
	if (error != eOK)
		return error;

	// Создаем новый поисковый список
	auto pSearchList = sld2::make_unique<CSldSearchList>();
	if (!pSearchList)
		return eMemoryNotEnoughMemory;

	const CSldListInfo * listInfo;
	error = GetWordListInfo(aListIndex, &listInfo);
	if (error != eOK)
		return error;

	// Инициализируем поисковый список
	error = pSearchList->Init(m_data, GetLayerAccess(), listInfo, NULL, 0);
	if (error != eOK)
		return error;

	// Устанавливаем максимальное количество списков, в которых мы можем производить поиск
	error = pSearchList->SetMaximumLists(ListCount);
	if (error != eOK)
		return error;

	// Если переданное максимально возможное количество слов не определено, то устанавливаем это значение в общее количество слов с списке
	if (aMaximumWords == MAX_NUMBER_OF_WORDS_IS_UNDEFINED)
	{
		// Общее количество слов с списке
		Int32 TotalWordsCount = listInfo->GetNumberOfGlobalWords();
		// Устанавливаем максимальное количество слов, которое может содержать поисковый список
		error = pSearchList->SetMaximumWords(TotalWordsCount);
		if (error != eOK)
			return error;
	}
	else
	{
		// Устанавливаем переданное максимально возможное количество слов
		error = pSearchList->SetMaximumWords(aMaximumWords);
		if (error != eOK)
			return error;
	}

	// Производим поиск
	ESldError searchError = pSearchList->DoAnagramSearch(aText, TextLen, pList, aListIndex);
	// Восстанавливаем ранее сохраненное состояние списков слов (независимо от того, успешно выполнился поиск или нет)
	error = pList->RestoreState();
	if (error != eOK)
		return error;

	// Проверяем, как выполнился поиск
	if (searchError != eOK)
		return searchError;

	// Добавляем поисковый список в общий массив списков словаря
	error = AddList(sld2::move(pSearchList), ListCount);
	if (error != eOK)
		return error;

	// Устанавливаем список с результатами поиска в качестве текущего
	error = SetCurrentWordlist(ListCount);
	if (error != eOK)
		return error;
		
	return eOK;
}

/** *********************************************************************
* Производит поиск слов в текущем списке слов при вводе с учетом возможных опечаток,
* начиная с текущего уровня (и во всех вложенных уровнях)
*
* @param[in]	aText	- текст (набор символов), по которым будет производиться поиск слов
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::DoSpellingSearch(const UInt16 *aText, Int32 aMaximumWords)
{
	return DoSpellingSearch(GetCurrentListIndex(), aText, aMaximumWords);
}

/** *********************************************************************
* Производит поиск слов в указанном списке слов при вводе с учетом возможных опечаток,
* начиная с текущего уровня (и во всех вложенных уровнях)
*
* @param[in]	aListIndex	- номер списка слов
* @param[in]	aText		- текст (набор символов), по которым будет производиться поиск слов
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::DoSpellingSearch(Int32 aListIndex, const UInt16 *aText, Int32 aMaximumWords)
{
	if (!aText)
		return eMemoryNullPointer;
	
	ISldList* pList = 0;
	ESldError error = GetWordList(aListIndex, &pList);
	if (error != eOK)
		return error;
	
	if (!pList)
		return eMemoryNullPointer;
	
	Int32 TextLen = CSldCompare::StrLen(aText);
	if (TextLen < 1)
		return eOK;
	
	Int32 ListCount = 0;
	// Количество списков в словаре
	error = GetNumberOfLists(&ListCount);
	if (error != eOK)
		return error;
	
	// Сохраняем текущее состояние списка слов
	error = pList->SaveCurrentState();
	if (error != eOK)
		return error;

	// Создаем новый поисковый список
	auto pSearchList = sld2::make_unique<CSldSearchList>();
	if (!pSearchList)
		return eMemoryNotEnoughMemory;

	const CSldListInfo * listInfo;
	error = GetWordListInfo(aListIndex, &listInfo);
	if (error != eOK)
		return error;

	// Инициализируем поисковый список
	error = pSearchList->Init(m_data, GetLayerAccess(), listInfo, NULL, 0);
	if (error != eOK)
		return error;

	// Устанавливаем максимальное количество списков, в которых мы можем производить поиск
	error = pSearchList->SetMaximumLists(ListCount);
	if (error != eOK)
		return error;

	// Если переданное максимально возможное количество слов не определено, то устанавливаем это значение в общее количество слов с списке
	if (aMaximumWords == MAX_NUMBER_OF_WORDS_IS_UNDEFINED)
	{
		// Общее количество слов с списке
		Int32 TotalWordsCount = listInfo->GetNumberOfGlobalWords();
		// Устанавливаем максимальное количество слов, которое может содержать поисковый список
		error = pSearchList->SetMaximumWords(TotalWordsCount);
		if (error != eOK)
			return error;
	}
	else
	{
		// Устанавливаем переданное максимально возможное количество слов
		error = pSearchList->SetMaximumWords(aMaximumWords);
		if (error != eOK)
			return error;
	}

	// Производим поиск
	ESldError searchError = pSearchList->DoSpellingSearch(aText, pList, aListIndex);
	// Восстанавливаем ранее сохраненное состояние списков слов (независимо от того, успешно выполнился поиск или нет)
	error = pList->RestoreState();
	if (error != eOK)
		return error;

	// Проверяем, как выполнился поиск
	if (searchError != eOK)
		return searchError;

	// Добавляем поисковый список в общий массив списков словаря
	error = AddList(sld2::move(pSearchList), ListCount);
	if (error != eOK)
		return error;

	// Устанавливаем список с результатами поиска в качестве текущего
	error = SetCurrentWordlist(ListCount);
	if (error != eOK)
		return error;
		
	return eOK;
}

/** *********************************************************************
* Производит полнотекстовый поиск в текущем списке слов
*
* @param[in]	aText			- текст, по которому будет производиться поиск
* @param[in]	aMaximumWords	- максимальное количество слов, которое мы хотим получить
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::DoFullTextSearch(const UInt16* aText, Int32 aMaximumWords)
{
	return DoFullTextSearch(GetCurrentListIndex(), aText, aMaximumWords);
}

/** *********************************************************************
* Производит полнотекстовый поиск в указанном списке слов
*
* @param[in]	aListIndex		- номер списка слов
* @param[in]	aText			- текст, по которому будет производиться поиск
* @param[in]	aMaximumWords	- максимальное количество слов, которое мы хотим получить
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::DoFullTextSearch(Int32 aListIndex, const UInt16* aText, Int32 aMaximumWords)
{
	if (!aText)
		return eMemoryNullPointer;
	
	ISldList* pList = 0;
	ESldError error = GetWordList(aListIndex, &pList);
	if (error != eOK)
		return error;
	
	if (!pList)
		return eMemoryNullPointer;
	
	Int32 ListCount = 0;
	// Количество списков в словаре
	error = GetNumberOfLists(&ListCount);
	if (error != eOK)
		return error;
	
	const CSldListInfo * listInfo;
	error = GetWordListInfo(aListIndex, &listInfo);
	if (error != eOK)
		return error;

	if (!listInfo->IsFullTextSearchList())
		return eCommonWrongList;
	
	// Сохраняем текущее состояние всех списков слов, т.к. полнотекстовый поиск теоретически может обращаться к любому списку
	// А также все списки слов в словаре должны быть проинициализированы
	Int32 currListIndex = GetCurrentListIndex();
	for (Int32 i=0;i<ListCount;i++)
	{
		ISldList* pList = 0;
		ESldError error = GetWordList(i, &pList);
		if (error != eOK)
			return error;

		if (!(pList && pList->isInit()))
		{
			error = SetCurrentWordlist(i);
			if (error != eOK)
				return error;
		}
		error = pList->SaveCurrentState();
		if (error != eOK)
			return error;
	}
	error = SetCurrentWordlist(currListIndex);
	if (error != eOK)
		return error;
	
	// Создаем новый поисковый список
	auto pSearchList = sld2::make_unique<CSldSearchList>();
	if (!pSearchList)
		return eMemoryNotEnoughMemory;
	
	// Инициализируем поисковый список
	error = pSearchList->Init(m_data, GetLayerAccess(), listInfo, NULL, 0);
	if (error != eOK)
		return error;
	
	// Производим поиск
	ESldError searchError = pSearchList->DoFullTextSearch(aText, aMaximumWords, GetLists(), GetDictionaryHeader()->NumberOfLists, aListIndex);
	// Восстанавливаем ранее сохраненное состояние списков слов (независимо от того, успешно выполнился поиск или нет)
	for (Int32 i=0;i<ListCount;i++)
	{
		ISldList* pList = 0;
		ESldError error = GetWordList(i, &pList);
		if (error != eOK)
			return error;

		error = pList->RestoreState();
		if (error != eOK)
			return error;
	}
	// Проверяем, как выполнился поиск
	if (searchError != eOK)
		return searchError;

	// Добавляем поисковый список в общий массив списков словаря
	error = AddList(sld2::move(pSearchList), ListCount);
	if (error != eOK)
		return error;

	// Устанавливаем список с результатами поиска в качестве текущего
	error = SetCurrentWordlist(ListCount);
	if (error != eOK)
		return error;

	return eOK;
}

/** *********************************************************************
* Производит сортировку поискового списка слов по релевантности шаблону поиска
*
* @param[in]	aListIndex	- номер поикового списка слов, который нужно отсортировать
* @param[in]	aText		- шаблон поиска
* @param[in]	aSimpleMode	- true: сортировать по упрощенной схеме если возможно - более быстро, но менее качественно
*							  false: полная сортировка - более медленно, но более качественно
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::SortSearchListRelevant(Int32 aListIndex, const UInt16* aText, const CSldVector<TSldMorphologyWordStruct>& aMorphologyForms, ESldFTSSortingTypeEnum aMode)
{
	if (!aText)
		return eMemoryNullPointer;
	
	ESldError error;
	Int32 ListCount = 0;

	// Количество списков в словаре
	error = GetNumberOfLists(&ListCount);
	if (error != eOK)
		return error;
	
	if (aListIndex >= ListCount || aListIndex < 0)
		return eCommonWrongList;

	const CSldListInfo * listInfo;
	error = GetWordListInfo(aListIndex, &listInfo);
	if (error != eOK)
		return error;

	if (!(listInfo->IsFullTextSearchList() || listInfo->GetUsage() == eWordListType_RegularSearch))
		return eCommonWrongList;
	
	ISldList* pList = 0;
	error = GetWordList(aListIndex, &pList);
	if (error != eOK)
		return error;

	error = pList->SortListRelevant(aText, aMorphologyForms, aMode);
	if (error != eOK)
		return error;
	
	return eOK;
}

/** *********************************************************************
* Удаляет все поисковые списки (очистка результатов поиска)
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::ClearSearch(void)
{
	return RemoveList(eWordListType_RegularSearch);
}


/** **********************************************************************
* Регистрируем данный словарь с помощью серийного номера
*
* @param[in]	aSerialNumber	- строчка с серийным номером
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::RegisterDictionary(const UInt16 *aSerialNumber)
{
	ESldError error;
	UInt16 PlatformID[4];
	UInt32 DictID, Number, HASH, Date, IsBundle;
	if (!aSerialNumber)
		return eMemoryNullPointer;

	// Перед проверкой серийного номера сохраняем текущие регистрационные данные, т.к.
	// потом мы будем их считывать и количество кликов может потеряться.
	error = SldSaveRegistrationData(GetDictionaryHeader()->DictID, &m_RegistrationData, &m_RandomSeed, GetLayerAccess());
	if (error != eOK)
		return error;

	error = CSldSerialNumber::ParseSerialNumber(aSerialNumber, PlatformID, &DictID, &Number, &HASH, &Date, &IsBundle);
	if (error != eOK)
		return error;

	if (CSldCompare::StrCmp(PlatformID, GetLayerAccess()->GetPlatformID()) != 0)
		return eSNWrongSerialNumber;
	
	TRegistrationData tmpReg;
	tmpReg.Clicks = 0;
	tmpReg.Date = Date;
	tmpReg.HASH = HASH;
	tmpReg.Number = Number;
	error = SldSaveRegistrationData(DictID, &tmpReg, &m_RandomSeed, GetLayerAccess());
	if (error != eOK)
		return error;

	// Портим данные перед получением сохраненного серийного номера.
	// Это нужно для того, чтобы если нас захотят взломать путем отключения 
	// записи мы не стали бы корректно работать.
	Number = 0;
	HASH -= DictID;
	Date = 1;

	// Не зависимо от того, что мы только что регистрировали считываем данные о регистрации текущего словаря.
	error = SldLoadRegistrationData(GetDictionaryHeader()->DictID, &m_RegistrationData, GetLayerAccess());
	if (error != eOK)
		return error;
	
	return eOK;
}

/** *********************************************************************
* Возвращает флаг того, зарегистрирован словарь или нет
*
* @param[out]	aIsRegistered	- указатель на переменную, куда будет сохранен флаг
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::IsDictionaryRegistered(UInt32* aIsRegistered) const
{
	if (!aIsRegistered)
		return eMemoryNullPointer;
		
	*aIsRegistered = (m_RegistrationData.Number >> 24) ? 0 : 1;
		
	return eOK;
}

/** *********************************************************************
* Возвращает флаг того, есть ли в словаре флаг фичи
*
* @param[in]	aFeature	- проверяемая фича
*
* @return наличие в словаре фичи
************************************************************************/
bool CSldDictionary::HasFeature(const ESldFeatures aFeature) const
{
	const auto header = GetDictionaryHeader();
	if (!header)
		return false;

	return (header->DictionaryFeatures & (0x01 << aFeature)) != 0;
}

/** *********************************************************************
* Возвращает флаг того, есть ли в словаре SkeletalMode (скрываемые блоки)
*
* @return наличие в словаре SkeletalMode
************************************************************************/
bool CSldDictionary::IsDictionaryHasSkeletalMode() const
{
	return HasFeature(eFeature_SkeletalMode);
}

/** *********************************************************************
* Возвращает флаг того, есть ли в словаре Taboo (ненормативная лексика)
*
* @return наличие в словаре Taboo
************************************************************************/
bool CSldDictionary::IsDictionaryHasTaboo() const
{
	return HasFeature(eFeature_Taboo);
}

/** *********************************************************************
* Возвращает флаг того, есть ли в словаре Stress (ударения)
*
* @return наличие в словаре Stress
************************************************************************/
bool CSldDictionary::IsDictionaryHasStress() const
{
	return HasFeature(eFeature_Stress);
}

/** *********************************************************************
* Возвращает флаг того, есть ли в словаре темная тема
*
* @return наличие в словаре Stress
************************************************************************/
bool CSldDictionary:: IsDictionaryHasDarkTheme() const
{
    return HasFeature(eFeature_DarkTheme);
}

/** *********************************************************************
* Возвращает флаг того, есть ли в словаре блоки switch с тематиками
*
* @return наличие в словаре switch-блоков с тематиками
************************************************************************/
bool CSldDictionary::IsDictionaryHasSwitchThematics() const
{
	const auto header = GetDictionaryHeader();
	if (!header)
		return false;

	return header->HasSwitchThematics != 0;
}

/** *********************************************************************
* Возвращает флаг того, есть ли в словаре в таблицах сравнения
* пары соответствий символов верхнего и нижнего регистров определенного типа
*
* @param[out]	aFlag		- указатель на переменную, куда будет сохранен флаг
*							  0 - нет, 1 - есть
* @param[in]	aTableType	- тип таблицы пар соответствий
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::IsDictionaryHasCmpSymbolPairTable(UInt32* aFlag, ESymbolPairTableTypeEnum aTableType) const
{
	if (!aFlag)
		return eMemoryNullPointer;
	*aFlag = 0;
	
	ESldError error;
	UInt32 Count = 0;

	const CSldCompare* CMP;
	error = GetCompare(&CMP);
	if (error != eOK)
		return error;

	error = CMP->GetTablesCount(&Count);
	if (error != eOK)
		return error;
	
	for (UInt32 i=0;i<Count;i++)
	{
		error = CMP->IsTableHasSymbolPairTable(i, aTableType, aFlag);
		if (error != eOK)
			return error;
		
		if (!(*aFlag))
			break;
	}

	return eOK;
}

/** *********************************************************************
* Возвращает версию ядра, с которым был собран словарь
*
* @param[out]	aVersion	- указатель на переменную, куда будет сохранена версия ядра
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetEngineVersion(UInt32* aVersion) const
{
	if (!aVersion)
		return eMemoryNullPointer;
	
	if (GetDictionaryHeader() == NULL)
		*aVersion = 0;
	else
		*aVersion = GetDictionaryHeader()->Version;
	
	return eOK;
}

/** *********************************************************************
* Возвращает версию ядра, с которым был собран словарь для закрытой базы
*
* @param[in]	aFile			- указатель на открытый файл контейнера с sdc базой
* @param[out]	aVersion		- указатель на переменную, куда будет сохранена версия ядра
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetEngineVersionForced(ISDCFile *aFile, UInt32* aVersion)
{
	ESldError error = LoadBaseHeader(aFile);
	if (error != eOK)
	{
		Close();
		return error;
	}

	error = GetEngineVersion(aVersion);
	Close();
	if (error != eOK)
	{
		return error;
	}

	return eOK;
}

/** *********************************************************************
* Загружает заголовок словарной базы
*
* @param[in]	aFile - указатель на открытый файл контейнера с sdc базой
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::LoadBaseHeader(ISDCFile *aFile)
{
	// XXX: TODO do something about the random seed init
	// Инициализируем случайное число.
	SldInitRandom(&m_RandomSeed, (UInt32)((UInt4Ptr)aFile) + 1);

	// Открываем словарь
	ESldError error = m_data.Open(aFile);
	if (error != eOK)
		return error;

	// Загружаем заголовок словаря
	auto res = m_data.GetResource(SLD_RESOURCE_HEADER, 0);
	if (error != eOK)
	{
		Close();
		return error;
	}

	UInt32 dictHeaderSize = res.size() > sizeof(*m_Header) ? sizeof(*m_Header) : res.size();
	// Простая проверка на корректность ресурса заголовка
	TDictionaryHeader* tmpDictHeader = (TDictionaryHeader*)res.ptr();
	if (tmpDictHeader->HeaderSize != res.size())
	{
		Close();
		return eInputWrongStructSize;
	}

	// Выделяем память для заголовка
	m_Header = sldMemNewZero<TDictionaryHeader>();
	if (!m_Header)
	{
		Close();
		return eMemoryNotEnoughMemory;
	}

	// Копируем данные
	sldMemMove(m_Header, tmpDictHeader, dictHeaderSize);

	//////////////////////////////////////////////////////////////////////////
	// Восстанавливаем HASH.
	m_Header->HASH ^= m_Header->DictID;
	m_Header->HASH ^= m_Header->NumberOfArticles;

	return eOK;
}

/** *********************************************************************
* Инициализируем систему защиты.
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::InitRegistration(void)
{
	TRegistrationData tmpReg;
	// Если это первый запуск, тогда мы еще не смотрели статьи
	tmpReg.Clicks = 0;
	// По умолчанию срок действия серийного номера не ограничен
	tmpReg.Date = 0;
	// Записываем правильный HASH
	tmpReg.HASH = GetDictionaryHeader()->HASH;
	// Создаем такой серийный номер, в котором старший байт не пуст - признак того, что этот словарь не зарегистрирован.
	tmpReg.Number = 0xCA000000;

	m_RegistrationData.Clicks = 0;
	m_RegistrationData.Date = 1;
	m_RegistrationData.HASH = SldGetRandom(&m_RandomSeed);
	m_RegistrationData.Number = SldGetRandom(&m_RandomSeed)&GetDictionaryHeader()->DictID;

	ESldError error;
	// Получаем сохраненные ранее данные о регистрации.
	error = SldLoadRegistrationData(GetDictionaryHeader()->DictID, &m_RegistrationData, GetLayerAccess());
	if (error != eOK)
	{
		error = SldSaveRegistrationData(GetDictionaryHeader()->DictID, &tmpReg, &m_RandomSeed, GetLayerAccess());
		if (error != eOK)
			return error;

		error = SldLoadRegistrationData(GetDictionaryHeader()->DictID, &m_RegistrationData, GetLayerAccess());
		if (error != eOK)
			return error;
	}

	// В данный момент у нас должны быть корректные данные о регистрации.
	// Передаем регистрационные данные в ответственные классы.
	if (m_Articles)
	{
		error = m_Articles->SetRegisterData(&m_RegistrationData);
		if (error != eOK)
			return error;
	}

	for (UInt32 i = 0; i < GetDictionaryHeader()->NumberOfLists; i++)
	{
		ISldList* pList = 0;
		ESldError error = GetWordList(i, &pList);
		if (error != eOK)
			return error;

		if (pList)
		{
			error = pList->SetHASH(m_RegistrationData.HASH);
			if (error != eOK)
				return error;
		}
	}

	return eOK;
}

/** *********************************************************************
* Проверяет, принадлежит ли символ определеленному языку,
* либо к общим символам-разделителям для всех языков словарной базы (aLang == SldLanguage::Delimiters)
*
* @param[in]	aSymbolCode	- код проверяемого символа
* @param[in]	aLang		- код языка или константа SldLanguage::Delimiters - код языка для общих символов-разделителей
* @param[out]	aFlag		- указатель на переменную, куда сохраняется результат:
*							  1 - символ принадлежит языку,
*							  0 - символ не принадлежит языку
* @param[out]	aResultFlag	- указатель на переменную, куда будет сохранен флаг результата:
*							  1 - результат однозначно определен,
*							  0 - результат не может быть определен (соответствующая таблица символов либо сам символ отсутствует)
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::IsSymbolBelongToLanguage(UInt16 aSymbolCode, ESldLanguage aLang, UInt32* aFlag, UInt32* aResultFlag)
{
	return GetCMP().IsSymbolBelongToLanguage(aSymbolCode, aLang, aFlag, aResultFlag);
}

/** *********************************************************************
* Проверяет, является ли символ разделителем в конкретном языке
*
* @param[in]	aSymbolCode	- код проверяемого символа
* @param[in]	aLangCode	- код проверяемого языка
* @param[out]	aFlag		- указатель на переменную, куда сохраняется результат:
*							  1 - является,
*							  0 - не является
* @param[out]	aResultFlag	- указатель на переменную, куда будет сохранен флаг результата:
*							  1 - результат однозначно определен,
*							  0 - результат не может быть определен (соответствующая таблица символов либо сам символ отсутствует)
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::IsSymbolBelongToLanguageDelimiters(UInt16 aSymbolCode, ESldLanguage aLang, UInt32* aFlag, UInt32* aResultFlag)
{
	return GetCMP().IsSymbolBelongToLanguageDelimiters(aSymbolCode, aLang, aFlag, aResultFlag);
}

/** *********************************************************************
* Определение языка по строке текста
*
* @param[in]	aText		- указатель на строку с текстом, язык которого нужно определить
* @param[out]	aLang		- переменная, куда будет сохранен код языка
* @param[out]	aResultFlag	- переменная, куда будет сохранен флаг результата:
*							  1 - язык распознался успешно,
*							  0 - язык нельзя определить однозначно
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::RecognizeLanguage(const UInt16* aText, ESldLanguage& aLang, UInt32& aResultFlag)
{	
	if (!aText)
		return eMemoryNullPointer;
	
	aLang = SldLanguage::Unknown;
	aResultFlag = 0;
	
	Int32 TextLen = CSldCompare::StrLen(aText);
	if (!TextLen)
		return eOK;
	
	UInt32 tablesCount = 0;
	ESldError error = GetCMP().GetTablesCount(&tablesCount);
	if (error != eOK)
		return error;

	// Формируем список языков для проверки
	CSldVector<ESldLanguage> langCodes;
	for (UInt32 tableIdx = 0; tableIdx < tablesCount; tableIdx++)
	{
		if(GetCMP().GetTableLanguage(tableIdx) != SldLanguage::Default)
			sld2::sorted_insert(langCodes, GetCMP().GetTableLanguage(tableIdx));
	}
	
	UInt32 Flag = 0;
	UInt32 ResultFlag = 0;
	for (Int32 i = 0; i < TextLen; i++)
	{
		if (langCodes.empty())
			break;

		/// Если символ не является значимым ни в одной из таблиц сортировки - нет смысла его учитывать
		if (GetCMP().IsMarginalSymbol(aText[i]))
			continue;

		auto langIt = langCodes.begin();
		while (langIt != langCodes.end())
		{
			error = IsSymbolBelongToLanguage(aText[i], *langIt, &Flag, &ResultFlag);
			if (error != eOK)
				return error;
			
			/// В одной из таблиц нет нативных символов, значит дальнейший поиск языка будет некорректным
			if (!ResultFlag)
				return eOK;

			if (!Flag)
				langIt = langCodes.erase(langIt);
			else
				langIt++;
		}
	}

	if (langCodes.size() == 1)
	{
		aLang = langCodes.front();
		aResultFlag = 1;
	}

	return eOK;
}

/** *********************************************************************
* Переключение текущего направления словаря на обратное
*
* @param[out]	aResultFlag	- указатель на переменную, куда будет сохранен флаг результата:
*							  1 - направление переключилось,
*							  0 - направление не переключилось (текущее состояние не изменилось)
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::SwitchCurrentDirection(UInt32* aResultFlag)
{
	ESldError error;

	if (!aResultFlag)
		return eMemoryNullPointer;

	*aResultFlag = 0;

	if (GetCurrentLanguageFrom() == GetCurrentLanguageTo())
		return eOK;
	
	UInt32 Usage;
	error = GetCurrentListUsage(0, &Usage);
	if (error != eOK)
		return error;
	
	error = SwitchDirectionTo(GetCurrentLanguageTo(), (EWordListTypeEnum)Usage, aResultFlag);
	if (error != eOK)
		return error;

	return eOK;
}

/** *********************************************************************
* Переключение словаря на направление, определяемое по строке
*
* @param[in]	aText		- строка, по которой определяется нужное направление
* @param[out]	aResultFlag	- указатель на переменную, куда будет сохранен флаг результата:
*							  1 - направление переключилось,
*							  0 - направление не переключилось (текущее состояние не изменилось)
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::SwitchDirection(const UInt16* aText, UInt32* aResultFlag)
{
	ESldError error;
	
	if (!aText)
		return eMemoryNullPointer;
	if (!aResultFlag)
		return eMemoryNullPointer;
	
	*aResultFlag = 0;

	const auto langFrom = GetCurrentLanguageFrom();	

	UInt32 Usage;
	error = GetCurrentListUsage(0, &Usage);
	if (error != eOK)
		return error;

	const CSldListInfo *info = 0;
	error = GetWordListInfo(GetCurrentListIndex(), &info);
	if (error != eOK)
		return error;

	UInt32 curAlphabetType = info->GetAlphabetType();
	if (curAlphabetType != EAlphabet_Standard &&
		(langFrom == SldLanguage::Chinese || langFrom == SldLanguage::Japanese || langFrom == SldLanguage::Korean))
	{
		
		UInt32 alphabetType = GetCMP().GetAlphabetTypeByText(aText);
		if (alphabetType != EAlphabet_Unknown)
		{

			if (alphabetType != curAlphabetType)
				return SwitchDirectionTo(langFrom, (EWordListTypeEnum)Usage, aResultFlag, alphabetType);
			
			return eOK;
		}
	}

	ESldLanguage textLangFrom;
	UInt32 resultFlag;
	error = RecognizeLanguage(aText, textLangFrom, resultFlag);
	if (error != eOK)
		return error;
	
	if (!resultFlag)
		return eOK;
	
	if (textLangFrom == langFrom)
		return eOK;
	
	error = SwitchDirectionTo(textLangFrom, (EWordListTypeEnum)Usage, aResultFlag);
	if (error != eOK)
		return error;

	if (!aResultFlag)
		return eOK;
	
	if (textLangFrom == SldLanguage::Chinese || textLangFrom == SldLanguage::Japanese || textLangFrom == SldLanguage::Korean) {
		info = 0;
		error = GetWordListInfo(GetCurrentListIndex(), &info);
		if (error != eOK)
			return error;

		curAlphabetType = info->GetAlphabetType();
		UInt32 alphabetType = GetCMP().GetAlphabetTypeByText(aText);
		if (alphabetType != EAlphabet_Unknown) {
			if (alphabetType != curAlphabetType)
				return SwitchDirectionTo(textLangFrom, (EWordListTypeEnum)Usage, aResultFlag, alphabetType);
		}
	}
	return eOK;
}

/** *********************************************************************
* Переключение словаря на определенное направление
*
* @param[in]	aLangFrom	- основной код языка направления, на которое нужно переключиться
* @param[in]	aUsage		- код использования списка слов, на который нужно переключиться
* @param[out]	aResultFlag	- указатель на переменную, куда будет сохранен флаг результата:
*							  1 - направление переключилось,
*							  0 - направление не переключилось (текущее состояние не изменилось)
* @param[in]	aAlphabetType - тип алфавита основного языка списка. По умолчанию равен EAlphabet_Standard, если направление словаря представлено одним списком. 
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::SwitchDirectionTo(ESldLanguage aLangFrom, EWordListTypeEnum aUsage, UInt32* aResultFlag, UInt32 aAlphabetType)
{
	ESldError error;
	
	if (!aResultFlag)
		return eMemoryNullPointer;
	
	*aResultFlag = 0;
	
	Int32 ListCount;
	error = GetNumberOfLists(&ListCount);
	if (error != eOK)
		return error;

	const CSldListInfo* pListInfo = NULL;
	for (Int32 i=0;i<ListCount;i++)
	{
		if (i == GetCurrentListIndex())
			continue;

		error = GetWordListInfo(i, &pListInfo);
		if (error != eOK)
			return error;

		const auto LangFrom = pListInfo->GetLanguageFrom();
		const auto Usage = pListInfo->GetUsage();
		const auto alphabetType = pListInfo->GetAlphabetType();
		if (LangFrom == aLangFrom && Usage == aUsage && (aAlphabetType == EAlphabet_Standard || alphabetType == aAlphabetType))
		{
			error = SetCurrentWordlist(i);
			if (error != eOK)
				return error;
			
			*aResultFlag = 1;
			break;
		}
	}

	return eOK;
}

/** *********************************************************************
* Возвращает указатель на экземпляр объекта текущего списка слов
*
* @param[out]	aListPtr	- по данному указателю будет записан указатель на 
							  текущий список слов

* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetCurrentWordlistPtr(ISldList **aListPtr)
{
	return GetWordList(GetCurrentListIndex(), aListPtr);
}

/** *********************************************************************
* Возвращает указатель на экземпляр объекта списка слов по номеру списка
*
* @param[in]	aIndex		- номер списка слов
* @param[out]	aListPtr	- указатель, по которому будет записан указатель на список слов
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetWordList(Int32 aIndex, ISldList** aListPtr)
{
	if (!aListPtr)
		return eMemoryNullPointer;
	*aListPtr = 0;
	
	ESldError error;
	Int32 ListCount = 0;

	error = GetNumberOfLists(&ListCount);
	if (error != eOK)
		return error;

	if (aIndex >= ListCount || aIndex < 0)
		return eCommonWrongIndex;

	if (!m_List)
		return eMemoryNullPointer;

	if (!m_List[aIndex] || !m_List[aIndex]->isInit())
	{
		Int32 currListIdx = GetCurrentListIndex();
		// Инициализируем список
		error = SetCurrentWordlist(aIndex);
		if (error != eOK)
			return error;

		error = SetCurrentWordlist(currListIdx);
		if (error != eOK)
			return error;
	}

	*aListPtr = m_List[aIndex];

	return eOK;
}


/** *********************************************************************
* Возвращает массив номеров стилей словаря, в которых есть различные варианты отображения ударений
*
* @param[out]	aStyles	- указатель на массив, куда будут сохранены номера стилей
*						  если равен NULL, то возвращается только количество таких стилей (в переменной aCount)
* @param[out]	aCount	- указатель на переменную, в которую сохраняется количество стилей в массиве
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetStylesWithStressVariants(Int32* aStyles, Int32* aCount)
{
	if (!aCount)
		return eMemoryNullPointer;
	
	ESldError error;
	const CSldStyleInfo* pStyle = NULL;
	Int32 UsageCount = 0;
	ESldStyleUsageEnum Usage;
	Int32 StyleVariantsCount = 0;
	Int32 NumberOfStylesWithStressVariants = 0;
	
	UInt32 NumberOfStyles = GetNumberOfStyles();
	
	for (UInt32 styleIndex=1;styleIndex<NumberOfStyles;styleIndex++)
	{
		error = GetStyleInfo(styleIndex, &pStyle);
		if (error != eOK)
			return error;
		
		Int8 IsStress = 0;
		UsageCount = pStyle->GetNumberOfUsages();
		for (Int32 j=0;j<UsageCount;j++)
		{
			Usage = pStyle->GetStyleUsage(j);
			if (Usage == eStyleUsage_Unknown)
				continue;
			
			if (Usage == eStyleUsage_ThisStress || Usage == eStyleUsage_PairStress)
			{
				IsStress = 1;
				break;
			}
		}
		
		if (!IsStress)
			continue;
		
		StyleVariantsCount = pStyle->GetNumberOfVariants();
		if (StyleVariantsCount > 1)
		{
			if (aStyles)
			{
				aStyles[NumberOfStylesWithStressVariants] = styleIndex;
			}
			NumberOfStylesWithStressVariants++;
		}
	}
	
	*aCount = NumberOfStylesWithStressVariants;
	
	return eOK;
}

/** *********************************************************************
* Копирует строку без игнорируемых символов и в нижнем регистре.
*
* @param[out]	aDest - указатель на буфер, куда будем копировать
* @param[in]	aSrc - указатель на строку, которую будем копировать
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::StrEffectiveCopyToLowerStr(UInt16* aDest, const UInt16* aSrc)
{
	if (!aSrc || !aDest)
		return eMemoryNullPointer;

	GetCMP().StrEffectiveCopy(aDest, aSrc);
	return GetCMP().ToLowerStr(aDest, aDest);
}


/** *********************************************************************
* Устанавливает нужную локализацию записей по переданному языковому коду
*
* @param[in]	aLanguageCode	- код языка на который нужно локализовать записи в списке;
*								  если такой локализации нет, то локализация не меняется
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::SetLocalization(ESldLanguage aLocalizationLanguageCode)
{
	ISldList* pList = 0;
	ESldError error = GetWordList(GetCurrentListIndex(), &pList);
	if (error != eOK)
		return error;

	if (!pList)
		return eMemoryNullPointer;
		
	return pList->SetLocalization(aLocalizationLanguageCode);
}

/** *********************************************************************
* Устанавливает нужную локализацию записей по переданному языковому коду и набору других настроек
*
* @param[in]	aLanguageCode			- код языка на который нужно локализовать записи в списке;
*										  если такой локализации нет, то локализация не меняется
* @param[in]	aLocalizationDetails	- дополнительные настройки локализации (например, название платформы)
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::SetLocalization(ESldLanguage aLocalizationLanguageCode, const UInt16* aLocalizationDetails)
{
	ISldList* pList = 0;
	ESldError error = GetWordList(GetCurrentListIndex(), &pList);
	if (error != eOK)
		return error;

	if (!pList)
		return eMemoryNullPointer;
		
	return pList->SetLocalization(aLocalizationLanguageCode, aLocalizationDetails);
}


/** *********************************************************************
* Получает вектор индексов озвучек по введенному тексту. Поиск озвучки по имени осуществляется в текущем списке.
*
* @param[in]	aText - указатель на строку, по которой ищем озвучку
* @param[out]	aSoundIndex - указатель на переменную в которую будет помещен индекс звука
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetSoundIndexByText(const UInt16 *aText, CSldVector<Int32> & aSoundIndexes)
{
	ISldList* pList = 0;
	ESldError error = GetWordList(GetCurrentListIndex(), &pList);
	if (error != eOK)
		return error;

	if (!pList)
		return eMemoryNullPointer;

	return pList->GetSoundIndexByText(aText, aSoundIndexes);
}


/** *********************************************************************
* Разбивает китайское слово на слоги и возвращает вектор с индексами озвучки для конкретного слога, если способ разбиения найден.
*
* @param[in]	aText			- указатель на строку с китайским словом
* @param[in]	aCount			- глубина вхождения в рекурсию
* @param[out]	aSoundIndexes	- вектор в который должны помещаться компоненты найденого разбиения на слоги.
* @param[out]	aResultFlag		- указатель на переменную в которую запишется результат операции. 
*								  0 - способ разбиения не найден, 1 - способ разбиения найден.
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::SearchChineseSound(const UInt16 *aText, Int32 aCount, CSldVector<Int32> & aSoundIndexes, UInt32 *aResultFlag)
{
	if (!aText || !aResultFlag)
		return eMemoryNullPointer;
	
	if (aCount >= MAX_SEARCH_DEPTH)
	{
		*aResultFlag = 0;
		return eOK;
	}
	UInt32 textLen = GetCMP().StrLen(aText);
	if (textLen == 0)
	{
		*aResultFlag = 1;
		return eOK;
	}
	
	UInt16 syllab[MAX_SEARCH_CHAR+1] = {'\0'};
	ESldError error;
	CSldVector<Int32> soundIndexes;
	// пытаемся выделить слог и проверяем, имеется ли такое сочетание символов(слог)
	// в базе озвученых слов.
	for (UInt32 i = 1; i < MAX_SEARCH_CHAR && i <= textLen; ++i)
	{
		
		// Проверяем является ли символ согласным.
		UInt32 k = 0;
		for (; k < i; ++k)
		{
			if (!isConsonant(aText[k]))
				break;
		}
		// Если оказалось, что в строчке все согласные - ее проверять не 
		// нужно - это слогом быть не может.
		if (k == i)
			continue;

		for (k = 0; k < i; ++k) 
		{
			syllab[k] = aText[k];
		}

		syllab[i] = 0;

		error = GetSoundIndexByText(syllab, soundIndexes);
		if (error != eOK)
			return error;
		
		if (soundIndexes.empty())
			continue;

		aSoundIndexes.push_back(soundIndexes.front());
		// Если такой слог имеется, тогда рассматриваем остаток текста еще раз
		error = SearchChineseSound(aText + i, aCount + 1, aSoundIndexes, aResultFlag);
		if (error != eOK)
			return error;

		if (*aResultFlag == 1)
			return eOK;
	}

	*aResultFlag = 0;
	return eOK;
}

/** *********************************************************************
Проверяет есть ли у слова озвучка.
*
* @param[in]	aText - указатель на слово
* @param[out]	aResultFlag - указатель на переменную в которую запишется результат операции. 
*					0 - озвучки нет, 1 - озвучка есть.
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::IsWordHasSound(const UInt16 *aText, UInt32 *aResultFlag)
{
	ESldError error;
	if (!aText || !aResultFlag)
		return eMemoryNullPointer;

	CSldVector<Int32> soundIndexes;
	if (GetCurrentLanguageFrom() == SldLanguage::Chinese)
	{
		const UInt16 *tempText = aText;
		while (*tempText)
		{
			if (!isChinSoundCh(*tempText))
			{
				*aResultFlag = 0;
				return eOK;
			}
			++tempText;
		}
		
		return SearchChineseSound(aText, 0, soundIndexes, aResultFlag);
	}
	else
	{
		error = GetSoundIndexByText(aText, soundIndexes);
		if (error != eOK)
			return error;

		*aResultFlag = soundIndexes.empty() ? 0 : 1;
		return eOK;
	}
}

/** *********************************************************************
* Ищет ближайшее слово в текущем списке слов, которое больше или равно заданному
* Работает в несортированных списках, для сортированных по сорткею списках
* ищет по Show-варианту(начиная с баз версии 112+)
*
* @param[in]	aText	- искомое слово
* @param[out]	aResultFlag	- Флаг результата
*							0 - найти слово не удалось
*							1 - мы нашли само слово
*							2 - мы нашли альтернативный заголовок слова (запрос и Show-вариант не совпадают)
* @param[in]	aActionsOnFailFlag	- флаг, определяющий нужно ли подматываться к наиболее подходящем слову,
*							  если поиск завершился неудачно, в иерархических списках ищет только в корне списка
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetWordByTextExtended(const UInt16* aText, UInt32* aResultFlag, UInt32 aActionsOnFailFlag)
{
	return GetWordByTextExtended(GetCurrentListIndex(), aText, aResultFlag, aActionsOnFailFlag);
}

/** *********************************************************************
* Ищет ближайшее слово в указанном списке слов, которое больше или равно заданному
* Работает в несортированных списках, для сортированных по сорткею списках
* ищет по Show-варианту(начиная с баз версии 112+)
*
* @param[in]	aListIndex	- номер списка слов
* @param[in]	aText		- искомое слово
* @param[out]	aResultFlag	- Флаг результата
*							0 - найти слово не удалось
*							1 - мы нашли само слово
*							2 - мы нашли альтернативный заголовок слова (запрос и Show-вариант не совпадают)
* @param[in]	aActionsOnFailFlag	- флаг, определяющий нужно ли подматываться к наиболее подходящем слову,
*							  если поиск завершился неудачно, в иерархических списках ищет только в корне списка
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetWordByTextExtended(Int32 aListIndex, const UInt16* aText, UInt32* aResultFlag, UInt32 aActionsOnFailFlag)
{
	*aResultFlag = 0;
	Int32 listCount;
	ESldError error = GetNumberOfLists(&listCount);
	if (error != eOK)
		return error;
		
	if (aListIndex >= listCount || listCount < 0)
		return eCommonWrongList;

	ISldList* pList = 0;
	error = GetWordList(aListIndex, &pList);
	if (error != eOK)
		return error;

	error = pList->SaveCurrentState();
	if (error != eOK)
		return error;

	error = pList->GetWordByTextExtended(aText, aResultFlag, aActionsOnFailFlag);
	if (error != eOK)
		return error;

	// если поиск завершился неудачей, то восстанавливаем исходное состояние списка
	if(*aResultFlag == 0 && !aActionsOnFailFlag)
		return pList->RestoreState();

	// если список иерархический, то необходимо перейти по пути, полученному по найденному глобальному индексу
	if (pList->HasHierarchy() && !aActionsOnFailFlag)
	{
		Int32 globalIndex = SLD_DEFAULT_WORD_INDEX;
		error = pList->GetCurrentGlobalIndex(&globalIndex);
		if (error != eOK)
			return error;

		error = pList->GoToByGlobalIndex(globalIndex);
		if (error != eOK)
			return error;
	}

	return eOK;
}

/** *********************************************************************
* Ищет слова, совпадающие по массе с заданным в текущем списке слов
* Работает в несортированных списках, для сортированных по сорткею списках
* ищет по Show-варианту(начиная с баз версии 112+)
*
* @param[in]	aText		- искомое слово
* @param[out]	aWordSet	- вектор с индексами найденных слов
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetWordSetByTextExtended(const UInt16* aText, CSldVector<Int32>& aWordSet)
{
	return GetWordSetByTextExtended(GetCurrentListIndex(), aText, aWordSet);
}

/** *********************************************************************
* Ищет слова, совпадающие по массе с заданным в указанном списке слов
* Работает в несортированных списках, для сортированных по сорткею списках
* ищет по Show-варианту(начиная с баз версии 112+)
*
* @param[in]	aListIndex	- номер списка слов
* @param[in]	aText		- искомое слово
* @param[out]	aWordSet	- вектор с индексами найденных слов
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetWordSetByTextExtended(Int32 aListIndex, const UInt16* aText, CSldVector<Int32>& aWordSet)
{
	aWordSet.clear();
	Int32 listCount;
	ESldError error = GetNumberOfLists(&listCount);
	if (error != eOK)
		return error;

	if (aListIndex >= listCount || listCount < 0)
		return eCommonWrongList;

	ISldList* pList = 0;
	error = GetWordList(aListIndex, &pList);
	if (error != eOK)
		return error;

	UInt32 result = 0;
	error = pList->GetWordBySortedText(aText, &result);
	if (error != eOK)
		return error;

	if (!result)
		return eOK;

	const CSldListInfo* listInfo = nullptr;
	error = pList->GetWordListInfo(&listInfo);
	if (error != eOK)
		return error;

	UInt16* word = nullptr;
	Int8 hasNextWord = 1;
	while (hasNextWord != 0)
	{
		error = pList->GetCurrentWord(listInfo->GetShowVariantIndex(), &word);
		if (error != eOK)
			return error;

		if (m_CMP.StrICmp(word, aText) != 0)
			return eOK;

		aWordSet.push_back(SLD_DEFAULT_WORD_INDEX);
		error = pList->GetCurrentGlobalIndex(&aWordSet.back());
		if (error != eOK)
			return error;

		error = pList->GetNextRealSortedWord(&hasNextWord);
		if (error != eOK)
			return error;
	}

	return eOK;
}

/** *********************************************************************
* Проверяет сортированность текущего уровня иерархии
*
* @param[out]	aIsSorted	- указатель, по которому будет записан результат
*							  0 - уровень не сортирован
*							  1 - уровень сортирван
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::IsCurrentLevelSorted(UInt32* aIsSorted)
{
	ISldList* pList = 0;
	ESldError error = GetWordList(GetCurrentListIndex(), &pList);
	if (error != eOK)
		return error;

	if (!pList)
		return eMemoryNullPointer;

	return pList->IsCurrentLevelSorted(aIsSorted);
}

/** *********************************************************************
* Получает элемент истории для заданного слова в текущем списке слов
*
* @param[in]  aGlobalIndex		- глобальный индекс слова, информацию о котором мы хотим получить
* @param[out] aHistoryElements	- вектор CSldHistoryElement-ов, содержащих информацию, необходимую для восстановления слова.
*									  В случае одиночного словаря содержит один элемент.
*									  Поля времени и даты заполняются самостоятельно.
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetHistoryElement(Int32 aGlobalIndex, CSldVector<CSldHistoryElement> & aHistoryElements)
{
	aHistoryElements.clear();

	Int32 currentWordIndex = aGlobalIndex;
	Int32 currentListIndex = GetCurrentListIndex();

	const CSldListInfo* listInfo = NULL;
	ESldError error = GetWordListInfo(GetCurrentListIndex(), &listInfo);
	if (error != eOK)
		return error;

	EWordListTypeEnum listUsage = listInfo->GetUsage();
	if (listUsage == eWordListType_CustomList || listUsage == eWordListType_RegularSearch)
	{
		error = GetRealListIndex(aGlobalIndex, &currentListIndex);
		if (error != eOK)
			return error;

		error = GetRealGlobalIndex(aGlobalIndex, &currentWordIndex);
		if (error != eOK)
			return error;

		error = GetWordListInfo(currentListIndex, &listInfo);
		if (error != eOK)
			return error;

		listUsage = listInfo->GetUsage();
	}

	ISldList* pList = NULL;
	error = GetWordList(currentListIndex, &pList);
	if (error != eOK)
		return error;

	if (!pList)
		return eMemoryNullPointer;

	error = pList->SaveCurrentState();
	if (error != eOK)
		return error;

	error = pList->GoToByGlobalIndex(currentWordIndex);
	if (error != eOK)
		return error;

	Int32 translationCount = 0;
	error = pList->GetTranslationCount(currentWordIndex, &translationCount);
	if (error != eOK)
		return error;

	// Если переводов нет - все ранвно запоминаем
	if (translationCount == 0)
		translationCount++;

	CSldVector<Int32> dictIndexes;
	Int32 singleWordIndex = currentWordIndex;
	Int32 singleListIndex = currentListIndex;
	const TMergedMetaInfo* mergeInfo = GetMergeInfo();

	error = GetDictionaryIndexesByGlobalIndex(currentWordIndex, dictIndexes);
	if (error != eOK)
		return error;

	for (Int32 translationIndex = 0; translationIndex < dictIndexes.size(); translationIndex++)
	{	
		if (mergeInfo)
		{
			const auto & listVector = mergeInfo->ListIndexes[dictIndexes[translationIndex]];
			const auto findResult = sld2::find(listVector, currentListIndex);
			if (findResult == listVector.npos)
				return eMergeWrongListIndex;

			singleListIndex = listVector[findResult];
		}
		else if (translationIndex > 0)
			break;	

		if (pList->GetHereditaryListType() == eHereditaryListTypeMerged)
		{
			CSldMergedList* mergedList = (CSldMergedList*)pList;
			error = mergedList->GetSingleIndex(currentWordIndex, translationIndex, singleWordIndex);
			if (error != eOK)
				return error;
		}

		auto &historyElement = aHistoryElements.emplace_back();

		historyElement.SetWordIndex(singleWordIndex);
		historyElement.SetListIndex(singleListIndex);
		historyElement.SetLanguageCode(listInfo->GetLanguageFrom());
		historyElement.SetListType((EWordListTypeEnum)listUsage);

		UInt32 dictId = 0;
		error = GetDictionaryID(&dictId, dictIndexes[translationIndex]);
		if (error != eOK)
			return error;
		historyElement.SetDictID(dictId);

		error = pList->GetWordListInfo(&listInfo, translationIndex);
		if (error != eOK)
			return error;

		const UInt32 VariantCount = listInfo->GetNumberOfVariants();

		historyElement.SetWordsCount(VariantCount);

		for (UInt32 currentVariantIndex = 0; currentVariantIndex < VariantCount; currentVariantIndex++)
		{
			EListVariantTypeEnum variantType = listInfo->GetVariantType(currentVariantIndex);

			error = historyElement.SetVariantType(currentVariantIndex, variantType);
			if (error != eOK)
				return error;

			UInt16* currentWord = NULL;
			error = pList->GetCurrentWord(currentVariantIndex, &currentWord);
			if (error != eOK)
				return error;

			error = historyElement.SetCurrentWord(currentVariantIndex, currentWord);
			if (error != eOK)
				return error;
		}
	}

	error = pList->RestoreState();
	if (error != eOK)
		return error;

	return eOK;
}

/** *********************************************************************
* Получает слово по элементу истории, при необходимости переключает списки
*
* @param[in]	aHistoryElement		- структура, содержащая информацию, необходимую для восстановления слова.
* @param[out]	aResultFlag			- указатель, по которому будет записан результат выполнения функции	(см. #ESldHistoryResult)
* @param[out]	aListIndex			- возможно в процессе был переключен список, тогда по этому указателю будет записан
*									  индекс нового списка, иначе SLD_DEFAULT_LIST_INDEX
* @param[out]	aGlobalIndex		- указатель, по которому будет записан индекс полученного слова
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetWordByHistoryElement(CSldHistoryElement* aHistoryElement, ESldHistoryResult* aResultFlag, Int32* aListIndex, Int32* aGlobalIndex)
{
	if (!aHistoryElement || !aResultFlag || !aListIndex || !aGlobalIndex)
		return eMemoryNullPointer;

	*aResultFlag = eHistoryNotCompare;
	*aListIndex = SLD_DEFAULT_LIST_INDEX;
	*aGlobalIndex = SLD_DEFAULT_WORD_INDEX;

	if (aHistoryElement->GetDictID() != GetDictionaryHeader()->DictID || aHistoryElement->GetListIndex() >= GetDictionaryHeader()->NumberOfLists)
		return eOK;

	ESldError error = UpdateHistoryElement(aHistoryElement, aResultFlag);
	if (error != eOK)
		return error;

	if (*aResultFlag != eHistoryNotCompare)
	{
		*aListIndex = aHistoryElement->GetListIndex();
		if (*aListIndex != GetCurrentListIndex())
		{
			error = SetCurrentWordlist(*aListIndex);
			if (error != eOK)
				return error;
		}

		*aGlobalIndex = aHistoryElement->GetWordIndex();
		error = GoToByGlobalIndex(aHistoryElement->GetWordIndex());
		if (error != eOK)
			return error;
	}

	return eOK;
}

/** *********************************************************************
* Обновляет элемент истории до текущего состояния базы в заданном списке слов 
*
* @param[in]	aHistoryElement		- структура, содержащая информацию, необходимую для восстановления слова.
* @param[in]	aListIndex			- индекс списка, в котором мы хотим восстановить элемент истории
* @param[out]	aResultFlag			- указатель, по которому будет записан результат выполнения функции	(см. #ESldHistoryResult)
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::UpdateHistoryElementInList(CSldHistoryElement* aHistoryElement, const Int32 aListIndex, ESldHistoryResult* aResultFlag)
{
	if (!aHistoryElement || !aResultFlag)
		return eMemoryNullPointer;

	ISldList* pList = NULL;
	ESldError error = GetWordList(aListIndex, &pList);
	if (error != eOK)
		return error;

	if (!pList)
		return eMemoryNullPointer;

	Int32 globalIndex = SLD_DEFAULT_WORD_INDEX;
	error = pList->GetWordByHistoryElement(aHistoryElement, aResultFlag, &globalIndex);
	if (error != eOK)
		return error;

	if (*aResultFlag == eHistoryNotCompare)
		return eOK;

	if (aHistoryElement->GetListIndex() != aListIndex)
	{
		aHistoryElement->SetListIndex(aListIndex);
	}

	if (aHistoryElement->GetWordIndex() != globalIndex)
	{
		UInt32 date = 0;
		UInt32 time = 0;

		error = aHistoryElement->GetDate(&date);
		if (error != eOK)
			return error;

		error = aHistoryElement->GetTime(&time);
		if (error != eOK)
			return error;

		Int8* userData = NULL;
		CSldVector<Int8> userDataBuff;
		const UInt32 userDataSize = aHistoryElement->GetUserData(&userData);
		if (userDataSize != 0)
		{
			userDataBuff.resize(userDataSize);
			sldMemMove(userDataBuff.data(), userData, userDataSize);
		}

		const Int32 currentListIndex = GetCurrentListIndex();
		error = SetCurrentWordlist(aListIndex);
		if (error != eOK)
			return error;

		CSldVector<CSldHistoryElement> hElement;
		error = GetHistoryElement(globalIndex, hElement);
		if (error != eOK)
			return error;

		if (!hElement.empty())
			*aHistoryElement = hElement.front();

		error = SetCurrentWordlist(currentListIndex);
		if (error != eOK)
			return error;

		aHistoryElement->SetDate(date);
		aHistoryElement->SetTime(time);
		if (!userDataBuff.empty())
		{
			aHistoryElement->SetUserData(userDataBuff.data(), userDataBuff.size());
		}
	}

	return eOK;
}

bool CheckType(const UInt32 aType1, const UInt32 aType2)
{
	return aType1 == aType2 || (aType1 == eWordListType_InApp && aType2 == eWordListType_Dictionary) || (aType2 == eWordListType_InApp && aType1 == eWordListType_Dictionary);
}

/** *********************************************************************
* Обновляет элемент истории до текущего состояния базы
* Функция может изменить состояние списка, поэтому послее ее выполнения необходимо самостоятельно восстановить
* состояние списка. Сама функция не восстанавливает состояние в рамках оптимизации при восстановлении
* большого колличества элеметов истории
*
* @param[in]	aHistoryElement		- структура, содержащая информацию, необходимую для восстановления слова.
* @param[out]	aResultFlag			- указатель, по которому будет записан результат выполнения функции (см. #ESldHistoryResult)
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::UpdateHistoryElement(CSldHistoryElement* aHistoryElement, ESldHistoryResult* aResultFlag)
{
	if (!aHistoryElement || !aResultFlag)
		return eMemoryNullPointer;

	*aResultFlag = eHistoryNotCompare;
	if (aHistoryElement->GetDictID() != GetDictionaryHeader()->DictID)
		return eOK;

	ESldError error = eOK;
	CSldVector<Int32> listIndexes;
	for (Int32 listIndex = 0; listIndex < GetDictionaryHeader()->NumberOfLists; listIndex++)
	{
		const CSldListInfo* listInfo = NULL;
		error = GetWordListInfo(listIndex, &listInfo);
		if (error != eOK)
			return error;

		if (listInfo->GetLanguageFrom() == aHistoryElement->GetLanguageCode() && CheckType(aHistoryElement->GetListType(), listInfo->GetUsage()))
		{
			if (listIndex == aHistoryElement->GetListIndex())
				listIndexes.insert(0, listIndex);
			else
				listIndexes.push_back(listIndex);
		}
	}

	for (auto listIndex : listIndexes)
	{
		error = UpdateHistoryElementInList(aHistoryElement, listIndex, aResultFlag);
		if (error != eOK)
			return error;

		if (*aResultFlag != eHistoryNotCompare)
			return eOK;
	}

	return eOK;
}

/** *********************************************************************
* Ищет слово в текущем списке слов, совпадающее по всем переданным вариантам написания, всеми возможными способами
*
* @param[in]	aWordVariantsSet	- набор вариантов написания слова
* @param[out]	aResultFlag			- флаг результата
*										0 - подмотаться не удалось
*										1 - мы подмотались к совпадающему слову
*										2 - мы подмотались к ближайшему слову по виртуальному 
*										идентификатору (запрос и Show-вариант не совпадают)
* @param[in]	aWordSearchType		- тип (правила) поиска слова (см. ESldWordSearchTypeEnum)
* @param[in]	aActionsOnFailFlag	- флаг, определяющий нужно ли подматываться к наиболее подходящем слову, если поиск завершился неудачно
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::FindWordBySpecificRule(TWordVariantsSet* aWordVariantsSet, UInt32* aResultFlag, UInt32 aWordSearchType, UInt32 aActionsOnFailFlag)
{
	return FindWordBySpecificRule(GetCurrentListIndex(), aWordVariantsSet, aResultFlag, aWordSearchType, aActionsOnFailFlag);
}

/** *********************************************************************
* Ищет слово в указанном списке слов, совпадающее по всем переданным вариантам написания, всеми возможными способами
*
* @param[in]	aListIndex			- номер списка слов
* @param[in]	aWordVariantsSet	- набор вариантов написания слова
* @param[out]	aResultFlag			- флаг результата
*										0 - подмотаться не удалось
*										1 - мы подмотались к совпадающему слову
*										2 - мы подмотались к ближайшему слову по виртуальному 
*										идентификатору (запрос и Show-вариант не совпадают)
* @param[in]	aWordSearchType		- тип (правила) поиска слова (см. ESldWordSearchTypeEnum)
* @param[in]	aActionsOnFailFlag	- флаг, определяющий нужно ли подматываться к наиболее подходящем слову, если поиск завершился неудачно
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::FindWordBySpecificRule(Int32 aListIndex, TWordVariantsSet* aWordVariantsSet, UInt32* aResultFlag, UInt32 aWordSearchType, UInt32 aActionsOnFailFlag)
{
	*aResultFlag = 0;
	Int32 listCount;
	ESldError error = GetNumberOfLists(&listCount);
	if (error != eOK)
		return error;

	if (aListIndex >= listCount || listCount < 0)
		return eCommonWrongList;

	ISldList* pList = 0;
	error = GetWordList(aListIndex, &pList);
	if (error != eOK)
		return error;

	// если подматываться при неудаче не нужно, то нам надо будет восстановить начальное состояние списка, поэтому сохраняем его
	if (!aActionsOnFailFlag)
	{
		error = pList->SaveCurrentState();
		if (error != eOK)
			return error;
	}

	error = pList->GetWordBySetOfVariants(aWordVariantsSet, aWordSearchType, aResultFlag);
	if (error != eOK)
		return error;

	// если поиск завершился неудачно и подматываться при неудаче не нужно, то восстанавливаем начальное состояние списка
	if (!*aResultFlag && !aActionsOnFailFlag)
		return pList->RestoreState();

	// если список иерархический, то необходимо перейти по пути, полученному по найденному глобальному индексу
	if (pList->HasHierarchy())
	{
		TCatalogPath path;
		error = pList->GetCurrentPath(&path);
		if (error != eOK)
			return error;
		error = pList->GoToByPath(&path, eGoToWord);
		if (error != eOK)
			return error;
	}

	return eOK;
}

/** *********************************************************************
* Ищет слово в текущем списке слов всеми возможными способами
*
* @param[in]	aText			- искомое слово
* @param[out]	aResultFlag		- флаг результата
*									0 - подмотаться не удалось
*									1 - мы подмотались к совпадающему слову
*									2 - мы подмотались к ближайшему слову по виртуальному 
*										идентификатору (запрос и Show-вариант не совпадают)
* @param[in]	aActionsOnFailFlag	- флаг, определяющий нужно ли подматываться к наиболее подходящем слову, если поиск завершился неудачно
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::FindWordBySpecificRule(const UInt16* aText, UInt32* aResultFlag, UInt32 aActionsOnFailFlag)
{
	return FindWordBySpecificRule(GetCurrentListIndex(), aText, aResultFlag, aActionsOnFailFlag);
}

/** *********************************************************************
* Ищет слово в текущем списке слов всеми возможными способами
*
* @param[in]	aListIndex		- номер списка слов
* @param[in]	aText			- искомое слово
* @param[out]	aResultFlag		- флаг результата
*									0 - подмотаться не удалось
*									1 - мы подмотались к совпадающему слову
*									2 - мы подмотались к ближайшему слову по виртуальному 
*										идентификатору (запрос и Show-вариант не совпадают)
* @param[in]	aActionsOnFailFlag	- флаг, определяющий нужно ли подматываться к наиболее подходящем слову, если поиск завершился неудачно
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::FindWordBySpecificRule(Int32 aListIndex, const UInt16* aText, UInt32* aResultFlag, UInt32 aActionsOnFailFlag)
{
	if (!aResultFlag)
		return eMemoryNullPointer;

	*aResultFlag = 0;

	ISldList* pList = 0;
	ESldError error = GetWordList(aListIndex, &pList);
	if (error != eOK)
		return error;

	if (!pList)
		return eMemoryNullPointer;

	// сохраняем состояние списка
	error = pList->SaveCurrentState();
	if (error != eOK)
		return error;

	bool isRoot = true;
	if (pList->HasHierarchy() && aActionsOnFailFlag == 1)
	{
		Int32 localIndex = SLD_DEFAULT_WORD_INDEX;
		Int32 globalIndex = SLD_DEFAULT_WORD_INDEX;

		error = pList->GetCurrentIndex(&localIndex);
		if (error != eOK)
			return error;

		error = pList->GetCurrentGlobalIndex(&globalIndex);
		if (error != eOK)
			return error;

		isRoot = localIndex == globalIndex;
	}

	// при установленном aActionsOnFailFlag ищем только в корне
	if (aActionsOnFailFlag == 1 && !isRoot)
		return eOK;

	// ищем слово сначала по show-варианту
	error = pList->GetWordByTextExtended(aText, aResultFlag, aActionsOnFailFlag);
	if (error != eOK)
		return error;

	// если слово найдено по реальному show-варианту, то завершаем поиск
	if (*aResultFlag == 1)
		return eOK;

	Int32 extendedIndex = SLD_DEFAULT_WORD_INDEX;
	if (*aResultFlag == 2)
	{
		// если слово найдено по альтернативному show-варианту, то сохраняем индекс этого слова, чтобы, если GetMostSimilarWordByText ничего не найдет, восстановить этот путь
		error = pList->GetCurrentGlobalIndex(&extendedIndex);
		if (error != eOK)
			return error;
	}

	// ищем наиболее похожее слово
	error = pList->GetMostSimilarWordByText(aText, aResultFlag);
	if (error != eOK)
		return error;

	// если слово найдено, то завершаем поиск
	if (*aResultFlag == 1 && aActionsOnFailFlag == 1)
		return eOK;
	else if (extendedIndex != SLD_DEFAULT_WORD_INDEX)
	{
		// если слово было найдено по альтернативному show-варианту, то восстанавливаем сохраненный путь и завершаем поиск 
		*aResultFlag = 2;
		return pList->GoToByGlobalIndex(extendedIndex);
	}
	else if (aActionsOnFailFlag != 1)
	{
		error = pList->RestoreState();
		if (error != eOK)
			return error;
	}

	// если слово не было найдено, то завершаем поиск. Cписок в данном случае в состоянии подмотки к переданному слову с помощью GetWordByText.
	return eOK;
}

/** *********************************************************************
* Получает морфологию для заданного кода языка и опционально id базы морфологии
*
* @param[in]	aLanguageCode	- код языка, для которого нужно получить морфологию
* @param[out]	aMorphoData		- указатель на переменную, в которую будет сохранена морфология (память выделяется и освобождается внутри)
* @param[in]	aDictId			- id базы морфологии
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetMorphology(UInt32 aLanguageCode, MorphoData **aMorphoData, UInt32 aDictId /*= 0*/)
{
	if (!m_MorphologyManager)
		return eOK;

	*aMorphoData = NULL;

	// получаем индекс базы по коду языка и id
	Int32 resourceIndex = m_MorphologyManager->GetResourceIndexByLangCode(aLanguageCode, aDictId);
	if (resourceIndex == -1)
		return eOK;

	// если уже база уже иницализирована, то получаем указатель на нее и завершаем работу
	if (m_MorphologyManager->IsMorphologyInit(resourceIndex))
	{
		*aMorphoData = m_MorphologyManager->GetMorphologyByIndex(resourceIndex);
		return eOK;
	}

	// иначе нужно сначала базу проинициализировать

	// получаем смещение в файле до необходимой морфологии 
	UInt32 resourceShift, resourceSize;
	ESldError error = m_data.GetResourceShiftAndSize(&resourceShift, &resourceSize, RESOURCE_TYPE_MORPHOLOGY_DATA, resourceIndex);
	if (error != eOK)
		return error;

	// получаем открытый поток 
	ISDCFile *fileData = m_data.GetFileData();
	if (!fileData)
		return eMemoryNullPointer;

	// инициализируем морфологию 
	error = m_MorphologyManager->InitMorphology(fileData, m_LayerAccess, resourceIndex, resourceShift, resourceSize);
	if (error != eOK)
		return error;

	// получаем указатель на нее и завершаем работу
	*aMorphoData = m_MorphologyManager->GetMorphologyByIndex(resourceIndex);
	return eOK;
}

/***********************************************************************
* Возвращает тип содержимого в контейнере
*
* @return тип содержимого в контейнере
************************************************************************/
ESlovoEdContainerDatabaseTypeEnum CSldDictionary::GetDatabaseType(void) const
{
	return (ESlovoEdContainerDatabaseTypeEnum)m_data.GetDatabaseType();
}


/***********************************************************************
* Проверяет полная это база или нет
*
* @return 0 - это полная база
*		  1 - это демо база
************************************************************************/
UInt32 CSldDictionary::IsInApp(void) const
{
	return m_data.GetDatabaseType() == eDatabaseType_InApp ? 1 : m_data.IsInApp();
}

/** *********************************************************************
* Возвращает количество результатов полнотекстового поиска
* Функция проверяет количество вхождений для данного запроса без использования логики
*
* @param[in]	aListIndex	- номер списка слов
* @param[in]	aRequest	- текст, по которому будет производиться поиск
* @param[out]	aCount		- указатель на переменную, в которую будет сохранено количество результатов
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetFullTextResultCount(const UInt32 aListIndex, const UInt16* aRequest, Int32* aCount)
{
	if (!aCount)
		return eMemoryNullPointer;

	ISldList* pList = 0;
	ESldError error = GetWordList(aListIndex, &pList);
	if (error != eOK)
		return error;

	if (!pList)
		return eMemoryNullPointer;

	return pList->GetFullTextResultCount(aRequest, aCount);
}

/** *********************************************************************
* Проверяет наличие стилизованного текста для текущего варианта написания
*
* @param[in]	aVariantIndex	- номер варианта написания
*
* @return результат выполнения функции	1 - у данного варианта есть дополнительная разметка
										0 - у данного варианта нет дополнительной разметки
************************************************************************/
UInt32 CSldDictionary::HasStylizedVariantForCurrentWord(Int32 aVariantIndex)
{
	ISldList* pList = 0;
	ESldError error = GetWordList(GetCurrentListIndex(), &pList);
	if (error != eOK)
		return 0;

	if (!pList)
		return 0;

	if (pList->GetStylizedVariantArticleIndex(aVariantIndex) != SLD_INDEX_STYLIZED_VARIANT_NO || pList->GetStyleVariantIndex(aVariantIndex) != 0)
		return 1;

	return 0;
}

/** *********************************************************************
* Возвращает форматированный вариант написания для текущего слова
*
* @param[in]	aVariantIndex	- номер варианта написания
* @param[out]	aArticle		- разобранная на блоки статья, содержащая форматированный вариант написания для текущего слова
*								  В случае отсутсвия форматированного текста в aArticle ничего записано не будет
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetCurrentWordStylizedVariant(Int32 aVariantIndex, SplittedArticle & aArticle)
{
	ISldList* pList = 0;
	ESldError error = GetWordList(GetCurrentListIndex(), &pList);
	if (error != eOK)
		return error;

	if (!pList)
		return eMemoryNullPointer;

	const Int32 articleIndex = pList->GetStylizedVariantArticleIndex(aVariantIndex);
	if (articleIndex != SLD_INDEX_STYLIZED_VARIANT_NO && m_Articles)
	{
		return m_Articles->GetArticle(articleIndex, aArticle);
	}

	const Int32 styleVariant = pList->GetStyleVariantIndex(aVariantIndex);
	if (styleVariant)
	{
		UInt16 * word;
		error = GetCurrentWord(aVariantIndex, &word);
		if (error != eOK)
			return error;

		aArticle.emplace_back(styleVariant, word);
	}

	return eOK;
}

/** *********************************************************************
* Добавляет пользовательский список
*
* @param[out]	aListControl		- указатель по которому будет записан объект, для управления созданным списком
* @param[out]	aCustomListIndex	- указатель по которому будет записан индекс нового списка, среди всех загруженных
* @param[in]	aBaseListIndex		- индекс списка, на основе которого будет создан пользовательский список
*									  по умолчанию передается SLD_DEFAULT_LIST_INDEX, при котором список создается пустым
* @param[in]	aUncoverHierarchy	- флаг, указывающий на то, что всю иерархию в списке нужно будет раскрыть
*									  иначе в список попадет лишь верхний уровень иерархии (включен по умолчанию)
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::AddCustomWordList(CSldCustomListControl* aListControl, Int32* aCustomListIndex, Int32 aBaseListIndex, UInt32 aUncoverHierarchy)
{
	if(!aCustomListIndex || !aListControl)
		return eMemoryNullPointer;

	Int32 currentListIndex = GetCurrentListIndex();

	auto pCustomList = sld2::make_unique<CSldCustomList>();
	if (!pCustomList)
		return eMemoryNotEnoughMemory;

	ESldError error = eOK;
	if(aBaseListIndex != SLD_DEFAULT_LIST_INDEX)
	{
		const CSldListInfo* currentListInfo = NULL;
		error = GetWordListInfo(aBaseListIndex, &currentListInfo);
		if (error != eOK)
			return error;

		if (!currentListInfo)
			return eMemoryNullPointer;

		error = pCustomList->Init(GetReadData(), GetLayerAccess(), currentListInfo, &GetCMP(), 0);
		if (error != eOK)
			return error;

		error = SetCurrentWordlist(aBaseListIndex);
		if (error != eOK)
			return error;
	}
	else
	{
		const CSldListInfo* listInfo = NULL;
		ESldError error = GetWordListInfo(0, &listInfo);
		if (error != eOK)
			return error;

		error = pCustomList->Init(NULL, GetLayerAccess(), listInfo, &GetCMP(), 0);
		if (error != eOK)
			return error;
	}

	Int32 listCount = 0;
	error = GetNumberOfLists(&listCount);
	if (error != eOK)
		return error;

	error = pCustomList->SetRealLists(GetLists(), listCount);
	if (error != eOK)
		return error;

	error = AddList(sld2::move(pCustomList), listCount);
	if (error != eOK)
		return error;

	*aCustomListIndex = listCount;

	error = aListControl->Init(GetLists(), *aCustomListIndex, listCount + 1);
	if(error != eOK)
		return error;

	if(aBaseListIndex == SLD_DEFAULT_LIST_INDEX)
		return eOK;

	error = SetBase(MAX_UINT_VALUE);
	if(error != eOK)
		return error;

	Int32 wordsCount = 0;
	error = GetNumberOfWords(&wordsCount);
	if(error != eOK)
		return error;

	for(UInt32 wordIndex = 0; wordIndex < wordsCount; wordIndex++)
	{
		error = aListControl->AddWord(aBaseListIndex, wordIndex, aUncoverHierarchy);
		if (error != eOK)
			return error;
	}

	error = SetCurrentWordlist(currentListIndex);
	if (error != eOK)
		return error;

	return eOK;
}

/** *********************************************************************
* Удаляет определенный пользовательский список
*
* @param[in]	aListIndex	- индекс списка, который нужно удалить
*							  по умолчанию передается -1 и удаляет все списки
*							  определенного типа
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::RemoveCustomList(Int32 aListIndex)
{	
	return RemoveList(eWordListType_CustomList, aListIndex);
}

/** *********************************************************************
* Удаляет списки определенного типа
*
* @param[in]	aListType	- типа списка, который нужно удалить
* @param[in]	aListIndex	- индекс списка, который нужно удалить
*							  по умолчанию передается -1 и удаляет все списки
*							  определенного типа
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::RemoveList(EWordListTypeEnum aListType, Int32 aListIndex)
{
	Int32 ListCount = 0;
	ESldError error = GetNumberOfLists(&ListCount);
	if (error != eOK)
		return error;

	if (!ListCount)
		return eOK;

	while (ListCount)
	{
		ListCount--;

		const CSldListInfo* ListInfo = 0;
		error = GetWordListInfo(ListCount, &ListInfo);
		if (error != eOK)
			return error;

		if (ListInfo->GetUsage() == aListType)
		{
			if(aListIndex == -1)
			{
				error = RemoveList(ListCount);
				if (error != eOK)
					return error;

				error = GetNumberOfLists(&ListCount);
				if (error != eOK)
					return error;
			}
			else if(aListIndex == ListCount)
			{
				error = RemoveList(ListCount);
				if (error != eOK)
					return error;
				break;
			}
		}
	}

	error = GetNumberOfLists(&ListCount);
	if (error != eOK)
		return error;

	if (!ListCount)
		return eCommonWrongList;

	if (GetPrevListIndex() >= ListCount)
		return eCommonWrongList;

	// После удаления всех поисковых списков текущим становится непоисковый список, который использовался последним.
	// Проверка на текущий список является небольшой оптимизацией + избавляет нас от ошибок, в случае если
	// мы хотим закрыть экземпляр CSldDictionary, а валидной базы у нас по каким то причинам уже нет
	if (GetCurrentListIndex() != GetPrevListIndex())
	{
		error = SetCurrentWordlist(GetPrevListIndex());
		if (error != eOK)
			return error;
	}

	return eOK;
}

/** *********************************************************************
* Создает пользовательский список со словами, привязанными к текущему слову
* Для полнотекстового списка - список слинкованных индексов, для каталога - содержимое папки
*
* @param[out]	aListControl		- указатель по которому будет записан объект, для управления созданным списком
* @param[out]	aCustomListIndex	- указатель по которому будет записан индекс нового списка, среди всех загруженных
* @param[in]	aUncoverHierarchy	- флаг, указывающий на то, что всю иерархию в списке нужно будет раскрыть
*									  иначе в список попадет лишь верхний уровень иерархии (выключен по умолчанию)
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetSubwordsList(CSldCustomListControl* aListControl, Int32* aCustomListIndex, const bool aUncoverHierarchy)
{
	if (!aListControl || !aCustomListIndex)
		return eMemoryNullPointer;

	const CSldListInfo *listInfo = NULL;
	ESldError error = GetWordListInfo(&listInfo);
	if (error != eOK)
		return error;

	Int32 currentGlobalIndex = SLD_DEFAULT_WORD_INDEX;
	error = GetCurrentGlobalIndex(&currentGlobalIndex);
	if (error != eOK)
		return error;

	Int32 translationCount = 0;
	error = GetRealIndexesCount(currentGlobalIndex, &translationCount);
	if (error != eOK)
		return error;

	if (!translationCount)
		return eOK;

	error = AddCustomWordList(aListControl, aCustomListIndex);
	if (error != eOK)
		return error;

	Int32 listIndex = 0;
	Int32 wordGlobalIndex = 0;
	Int32 tIndex = 0;
	Int32 shiftIndex = 0;

	Int32 listCount = 0;
	error = GetNumberOfLists(&listCount);
	if (error != eOK)
		return error;

	CSldSearchWordResult resultWords;
	error = resultWords.Init(GetLists(), listCount);
	if (error != eOK)
		return error;

	for (Int32 translationIndex = 0; translationIndex < translationCount; translationIndex++)
	{
		error = GetRealIndexes(currentGlobalIndex, translationIndex, &listIndex, &wordGlobalIndex);
		if (error != eOK)
			return error;

		const CSldListInfo *resultListInfo = NULL;
		error = GetWordListInfo(listIndex, &resultListInfo);
		if (error != eOK)
			return error;

		error = resultWords.AddWord(listIndex, wordGlobalIndex, tIndex, shiftIndex);
		if (error != eOK)
			return error;
	}

	Int32 numberOfWords = resultWords.GetWordCount();
	sld2::DynArray<TSldSearchWordStruct> wordVector(numberOfWords);
	if (numberOfWords && wordVector.empty())
		return eMemoryNotEnoughMemory;

	Int32 addedWordCount = resultWords.FillWordVector(wordVector, wordVector.size());

	UInt32 localWordCount = 0;
	Int8 collapseThisList = 0;
	for (UInt32 currentListIndex = 0; currentListIndex < listCount; currentListIndex++)
	{
		if (!resultWords.IsListPresent(currentListIndex))
			continue;

		collapseThisList = 0;
		error = GetWordListInfo(currentListIndex, &listInfo);
		if (error != eOK)
			return error;

		if (listInfo->GetUsage() == eWordListType_Hidden)
		{
			collapseThisList = 1;
			error = aListControl->AddWord(currentListIndex);
			if (error != eOK)
				return error;

			localWordCount++;

			const CSldListLocalizedString* strings = listInfo->GetLocalizedStrings();

			const UInt16* listName = NULL;
			error = strings->GetListName(SldLanguage::Default, &listName);
			if (error != eOK)
				return error;

			error = aListControl->SetCurrentWord(localWordCount - 1, listName);
			if (error != eOK)
				return error;
		}

		error = UpdateVariantPropertyInCustomList(currentListIndex, *aCustomListIndex, aListControl);
		if (error != eOK)
			return error;

		for (Int32 currentWordIndex = 0; currentWordIndex < addedWordCount; currentWordIndex++)
		{
			if (currentListIndex != wordVector[currentWordIndex].ListIndex)
				continue;

			if (collapseThisList)
			{
				error = aListControl->AddSubWord(localWordCount - 1, wordVector[currentWordIndex].WordIndex);
			}
			else
			{
				error = aListControl->AddWord(currentListIndex, wordVector[currentWordIndex].WordIndex);
				localWordCount++;
			}
			if (error != eOK)
				return error;
		}

		if (collapseThisList && aUncoverHierarchy)
		{
			UInt32 subwordsCount = 0;
			error = aListControl->GetSubWordsCount(localWordCount - 1, &subwordsCount);
			if (error != eOK)
				return error;

			EUncoverFolderResult uncoverResult = eFolderNoActions;
			error = aListControl->UncoverHierarchy(localWordCount - 1, &uncoverResult);
			if (error != eOK)
				return error;

			localWordCount += subwordsCount;
		}
	}

	return eOK;
}

/** *********************************************************************
* Добавляет в пользовательский список варианты написания из заданного списка
*
* @param[in]	aCheckedListIndex	- индекс списка, из которого мы хотим добавить варианты написания
* @param[in]	aCustomListIndex	- индекс пользовательского списка, в который будут добавлены варианты написания
* @param[in]	aListControl		- контрол пользовательского списка
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::UpdateVariantPropertyInCustomList(const Int32 aCheckedListIndex, const Int32 aCustomListIndex, CSldCustomListControl* aListControl)
{
	ESldError error = eOK;
	UInt32 variantType = 0;

	const CSldListInfo* listInfo = NULL;
	error = GetWordListInfo(aCheckedListIndex, &listInfo);
	if (error != eOK)
		return error;

	const CSldListInfo* customListInfo = NULL;
	error = GetWordListInfo(aCustomListIndex, &customListInfo);
	if (error != eOK)
		return error;

	for (Int32 variantIndex = 0; variantIndex < listInfo->GetNumberOfVariants(); variantIndex++)
	{
		EListVariantTypeEnum variantType = listInfo->GetVariantType(variantIndex);
		if (customListInfo->GetVariantIndexByType(variantType) != SLD_DEFAULT_VARIANT_INDEX)
			continue;

		TListVariantProperty variantProperty;
		variantProperty.Type = variantType;
		variantProperty.LangCode = listInfo->GetVariantLangCode(variantIndex);
		error = aListControl->AddVariantProperty(variantProperty);
		if (error != eOK)
			return error;
	}

	return eOK;
}

/** *********************************************************************
* Возвращает структуру с информацией об использовании внешних баз
*
* @param[out]	aCount	- число ресурсов
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetExternContentInfo(TExternContentInfo (&aExtInf)[eExternContentTypeMax]) const
{
	sldMemZero(aExtInf, sizeof(TExternContentInfo) * eExternContentTypeMax);

	UInt32 count = 0;
	ESldError error = GetExternContentInfoCount(&count);
	if (error != eOK)
		return error;

	if (count == 0)
		return eOK;

	for (Int32 i = 0; i < count; i++)
	{
		auto res = ((CSDCReadMy&)(m_data)).GetResource(SLD_EXTERN_CONTENT_INFO, i + 1);
		if (res != eOK)
			return res.error();

		// Тип контента
		const UInt8* pointer = res.ptr();
		UInt32 contentType = *(UInt32*)pointer;
		pointer += sizeof(UInt32);

		// Число баз с внешним контентом этого типа
		UInt32 dictCount = *(UInt32*)pointer;
		pointer += sizeof(UInt32);

		aExtInf[contentType].Size = dictCount;
		aExtInf[contentType].Info = (TExternContentInfo::InfoPair*)sldMemNew(sizeof(TExternContentInfo::InfoPair) * dictCount);
		if (!aExtInf[contentType].Info)
			return eMemoryNotEnoughMemory;
		
		for (UInt32 k = 0; k < dictCount; k++)
		{
			// Длина названия базы
			UInt32 length = *(UInt32*)pointer;
			pointer += sizeof(UInt32);

			// Название базы
			aExtInf[contentType].Info[k].DictId = (UInt16*)sldMemNew(sizeof(UInt16) * (length + 1));
			if (!aExtInf[contentType].Info[k].DictId)
				return eMemoryNotEnoughMemory;
			sldMemMove(aExtInf[contentType].Info[k].DictId, pointer, sizeof(UInt16) * length);
			aExtInf[contentType].Info[k].DictId[length] = 0;
			pointer += sizeof(UInt16) * length;

			// Число использований данного ресурса из этой внешней базы
			aExtInf[contentType].Info[k].Count = *(UInt32*)pointer;
			pointer += sizeof(UInt32);
		}
	}

	return eOK;
}


/** *********************************************************************
* Возвращает число ресурсов с информацией о внешних базах
*
* @param[out]	aCount	- число ресурсов
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetExternContentInfoCount(UInt32* aCount) const
{
	if (!aCount)
		return eMemoryNullPointer;

	auto res = ((CSDCReadMy&)(m_data)).GetResource(SLD_EXTERN_CONTENT_INFO, 0);
	if (res != eOK)
		*aCount = 0;
	else
		*aCount = *res.ptr();

	return eOK;
}


/** *********************************************************************
* Возвращает номер статьи
*
* @param[in]	aListIndex		- номер списка слов
* @param[in]	aIndex			- локальный номер слова в текущем списке слов
* @param[in]	aVariantIndex	- номер варианта написания в списке слов
* @param[out]	aArticleIndex	- глобальный номер статьи
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetTranslationIndex(Int32 aListIndex, Int32 aIndex, Int32 aVariantIndex, Int32* aArticleIndex)
{
	if (!aArticleIndex)
		return eMemoryNullPointer;

	ESldError error;
	ISldList* pList = 0;
	error = GetWordList(aListIndex, &pList);
	if (error != eOK)
		return error;
	
	if (!pList)
		return eMemoryNullPointer;
	
	Int32 globalIndex = SLD_DEFAULT_WORD_INDEX;
	error = pList->LocalIndex2GlobalIndex(aIndex, &globalIndex);
	if (error != eOK)
		return error;

	Int32 NumberOfWords = 0;
	error = pList->GetTotalWordCount(&NumberOfWords);
	if (error != eOK)
		return error;

	if (globalIndex >= NumberOfWords || globalIndex < 0)
		return eCommonWrongIndex;

	Int32 NumberOfTranslation = 0;
	error = pList->GetTranslationCount(globalIndex, &NumberOfTranslation);
	if (error != eOK)
		return error;
	if (aVariantIndex >= NumberOfTranslation || aVariantIndex < 0)
		return eCommonWrongIndex;

	error = pList->GetTranslationIndex(globalIndex, aVariantIndex, aArticleIndex);
	if (error != eOK)
		return error;

	return eOK;
}


/** *********************************************************************
* Возвращает номер статьи
*
* @param[in]	aListIndex		- номер списка слов
* @param[in]	aIndex			- локальный номер слова в текущем списке слов
* @param[out]	aArticleIndex	- глобальный номер статьи
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetTranslationIndex(Int32 aIndex, Int32 aVariantIndex, Int32* aArticleIndex)
{
	return GetTranslationIndex(GetCurrentListIndex(), aIndex, aVariantIndex, aArticleIndex);
}
/** *********************************************************************
* Возвращает структуру с дополнительной информацией для таблицы сортировки
*
* @param[in]	aResId			- номер ресурса
* @param[out]	aResInfo		- заполняемая структура с информацией
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetCMPTableResourceInfo(UInt32 aResId, TResourceInfo* aResInfo)
{
	return GetResourceInfo(SLD_RESOURCE_COMPARE_TABLE, aResId, aResInfo);
}


/** *********************************************************************
* Возвращает структуру с дополнительной информацией для изображения
*
* @param[in]	aResId			- номер ресурса
* @param[out]	aResInfo		- заполняемая структура с информацией
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetPictureResourceInfo(UInt32 aResId, TResourceInfo* aResInfo)
{
	return GetResourceInfo(RESOURCE_TYPE_IMAGE, aResId, aResInfo);
}


/** *********************************************************************
* Возвращает структуру с дополнительной информацией для изображения
*
* @param[in]	aResId			- номер ресурса
* @param[out]	aResInfo		- заполняемая структура с информацией
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetAbstractResourceInfo(UInt32 aResId, TResourceInfo* aResInfo)
{
	return GetResourceInfo(RESOURCE_TYPE_ABSTRACT, aResId, aResInfo);
}

/** *********************************************************************
* Возвращает структуру с дополнительной информацией
*
* @param[in]	aResType		- тип ресурса
* @param[in]	aResId			- номер ресурса
* @param[out]	aResInfo		- заполняемая структура с информацией
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetResourceInfo(UInt32 aResType, UInt32 aResId, TResourceInfo* aResInfo)
{
	if (!aResInfo)
		return eMemoryNullPointer;

	sldMemZero(aResInfo, sizeof(aResInfo));

	// Получаем число типов ресурсов в таблице смещений
	auto res = m_data.GetResource(RESOURCE_TYPE_RESOURCE_INFO, 0);
	if (res != eOK)
		return res.error();

	UInt16 count = *((const UInt16*)res.ptr());
	UInt32 shiftTable[2];
	UInt32 shift = 0;

	// Получаем смещения для данного типа ресурса
	for (Int32 i = 0; i < count; i++)
	{
		auto res = m_data.GetResource(RESOURCE_TYPE_RESOURCE_INFO, i + 1);
		if (res != eOK)
			return res.error();
		sldMemCopy(shiftTable, res.ptr(), sizeof(shiftTable));
		if (shiftTable[0] == aResType)
		{
			shift  = shiftTable[1];
			break;
		}
	}

	// Получаем саму информацию
	if (shift)
	{
		auto res = m_data.GetResource(RESOURCE_TYPE_RESOURCE_INFO, shift + aResId);
		if (res != eOK)
			return res.error();

		TResourceInfoIndexes ResIndexes;
		sldMemZero(&ResIndexes, sizeof(ResIndexes));
		sldMemCopy(&ResIndexes, res.ptr(), res.size());

		auto resName = m_data.GetResource(RESOURCE_TYPE_RESOURCE_NAMES, ResIndexes.NameIndex);
		if (resName != eOK)
			return resName.error();

		auto resPath = m_data.GetResource(RESOURCE_TYPE_RESOURCE_PATHS, ResIndexes.NameIndex);
		if (resPath != eOK)
			return resPath.error();

		aResInfo->ResName = (UInt16*)sldMemNew(resName.size());
		aResInfo->ResPath = (UInt16*)sldMemNew(resPath.size());
		aResInfo->ResNameLength = resName.size() / 2;
		aResInfo->ResPathLength = resPath.size() / 2;
		sldMemCopy(aResInfo->ResName, resName.ptr(), resName.size());
		sldMemCopy(aResInfo->ResPath, resPath.ptr(), resPath.size());
	}

	return eOK;
}

/** *********************************************************************
* Проверяет принадлежность базы к ЛЭУ
*
* @param[out]	aFlag		- флаг того, что это .datf база
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::IsETBDatabase(UInt8* aFlag) const
{
	if (!aFlag)
		return eMemoryNullPointer;

	UInt32 databaseType = m_data.GetDatabaseType();
	*aFlag = (databaseType == eDatabaseType_TextBook || databaseType == eDatabaseType_Book);
	return eOK;
}

/** *********************************************************************
* Переходит по пути, соответсвующему заданному глобальному индексу
*
* @param[in]	aGlobalIndex	- глобальный индекс, к которому мы хотим перейти
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GoToByGlobalIndex(const Int32 aGlobalIndex)
{
	return GoToByGlobalIndex(aGlobalIndex, GetCurrentListIndex());
}

/** *********************************************************************
* Переходит по пути, соответсвующему заданному глобальному индексу
*
* @param[in]	aGlobalIndex	- глобальный индекс, к которому мы хотим перейти
* @param[in]	aListIndex		- номер списка слов
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GoToByGlobalIndex(const Int32 aGlobalIndex, const Int32 aListIndex)
{
	ISldList* pList = NULL;
	ESldError error = GetWordList(aListIndex, &pList);
	if (error != eOK)
		return error;

	if (!pList)
		return eMemoryNullPointer;

	return pList->GoToByGlobalIndex(aGlobalIndex);
}

/** *********************************************************************
* Индексирует все слова, содержащиеся в списке в соответсвии с заданным вариантом написания
*
* @param[in]	aListIndex		- номер списка, который мы хотим индексировать
* @param[in]	aVariantType	- тип варианта написания, по которому будет индексирован список
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::SortListByVariant(const Int32 aListIndex, const EListVariantTypeEnum aVariantType)
{
	ISldList* pList = NULL;
	ESldError error = GetWordList(aListIndex, &pList);
	if (error != eOK)
		return error;

	if (!pList)
		return eMemoryNullPointer;

	const CSldListInfo* listInfo = NULL;
	error = GetWordListInfo(aListIndex, &listInfo);
	if (error != eOK)
		return error;

	const Int32 variantIndex = listInfo->GetVariantIndexByType(aVariantType);
	if (variantIndex == SLD_DEFAULT_VARIANT_INDEX)
		return eCommonWrongVariantIndex;

	error = pList->SortListByVariant(variantIndex);
	if (error != eOK)
		return error;

	return eOK;
}

/** *********************************************************************
* Возвращает информацию об элементарном объекте по его индексу
*
* @param[in]	aIndex			- индекс атомарного объекта
* @param[out]	aAtomicObject	- указатель, по которому будет записан указатель на структуру
*								  с информацией об атомарном объекте
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetAtomicObjectInfo(const Int32 aIndex, const TAtomicObjectInfo **aAtomicObject) const
{
	if (!aAtomicObject)
		return eMemoryNullPointer;

	Int32 listCount = 0;
	ESldError error = GetNumberOfLists(&listCount);
	if (error != eOK)
		return error;

	UInt32 listIndex = 0;
	const CSldListInfo* listInfo = NULL;
	for (; listIndex < listCount; listIndex++)
	{
		error = GetWordListInfo(listIndex, &listInfo);
		if (error != eOK)
			return error;

		if (listInfo->GetUsage() == eWordListType_Atomic)
			break;
	}

	if (listIndex == listCount)
		return eCommonWrongIndex;

	if (aIndex < 0 || aIndex >= listInfo->GetNumberOfGlobalWords())
		return eCommonWrongIndex;

	*aAtomicObject = &m_AtomicInfo[aIndex];

	return eOK;
}

/** *********************************************************************
* Получает изначальные индексы для полнотекстового поиска
* Т.е. вовращаются индексы для слова в листе, которое изначально ссылается на статью перевод, которую нашел поиск.
* Например пользователь искал "клетка ДНК", нашлась фраза "В клетке есть ДНК...", это фраза-результат поиска ссылается на статью с номером 15015 в списке статей (где есть такой текст)
* Эта функция позволяет получить список и слово, к которым изначально относится, например Список 2 (Оглавление), Слово - "Глава 3. Строение бактерий".
* Это необходимо для восстановления правильного пути к результатам поиска
*
* @param[in]	aLocalIndex			- номер слова в текущем списке слов, для которого мы хотим получить InitialIndexes
* @param[out]	aInitialListIndex	- номер списка слов, который является изначальным для переданной во входных аргументах статьи-перевода (результата поиска)
* @param[out]	aInitialWordIndex	- номер слова внутри списка слов (вестимо, глобальный)
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetInitialWordIndexes(Int32 aLocalIndex, Int32* aInitialListIndex, Int32* aInitialWordIndex)
{
	return GetInitialWordIndexes(GetCurrentListIndex(), aLocalIndex, aInitialListIndex, aInitialWordIndex);
}

/** *********************************************************************
* Получает изначальные индексы для полнотекстового поиска
* Т.е. вовращаются индексы для слова в листе, которое изначально ссылается на статью перевод, которую нашел поиск.
* Например пользователь искал "клетка ДНК", нашлась фраза "В клетке есть ДНК...", это фраза-результат поиска ссылается на статью с номером 15015 в списке статей (где есть такой текст)
* Эта функция позволяет получить список и слово, к которым изначально относится, например Список 2 (Оглавление), Слово - "Глава 3. Строение бактерий".
* Это необходимо для восстановления правильного пути к результатам поиска
*
* @param[in]	aListIndex			- номер списка слов
* @param[in]	aLocalIndex			- номер слова в текущем списке слов, для которого мы хотим получить InitialIndexes
* @param[out]	aInitialListIndex	- номер списка слов, который является изначальным для переданной во входных аргументах статьи-перевода (результата поиска)
* @param[out]	aInitialWordIndex	- номер слова внутри списка слов (вестимо, глобальный)
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetInitialWordIndexes(Int32 aListIndex, Int32 aLocalIndex, Int32* aInitialListIndex, Int32* aInitialWordIndex)
{
	if (!aInitialListIndex || !aInitialWordIndex)
		return eMemoryNullPointer;

	*aInitialListIndex = SLD_DEFAULT_LIST_INDEX;
	*aInitialWordIndex = SLD_DEFAULT_WORD_INDEX;

	Int32 realIndexesCount = 0;
	ESldError error = GetRealIndexesCount(aListIndex, aLocalIndex, &realIndexesCount);
	if (error != eOK)
		return error;

	if (!realIndexesCount)
		return eOK;

	error = GetRealIndexes(aListIndex, aLocalIndex, 0, aInitialListIndex, aInitialWordIndex);
	if (error != eOK)
		return error;

	const CSldListInfo* listInfo = NULL;
	error = GetWordListInfo(*aInitialListIndex, &listInfo);
	if (error != eOK)
		return error;

	if (listInfo->GetUsage() != eWordListType_FullTextAuxiliary)
		return eOK;

	error = GetRealIndexesCount(*aInitialListIndex, *aInitialWordIndex, &realIndexesCount);
	if (error != eOK)
		return error;

	if (!realIndexesCount)
		return eOK;

	error = GetRealIndexes(*aInitialListIndex, *aInitialWordIndex, 0, aInitialListIndex, aInitialWordIndex);
	if (error != eOK)
		return error;

	return eOK;
}

/**
 * Возвращает индекс первого листа заданного типа
 *
 * @param[in] aListIndex - тип искомого списка слов
 *
 * @return индекс списка слов или SLD_DEFAULT_LIST_INDEX если списка с таким типом нет в базе
 */
Int32 CSldDictionary::FindListIndex(UInt32 aListType) const
{
	Int32 listCount = 0;
	ESldError error = GetNumberOfLists(&listCount);
	if (error != eOK)
		return error;

	for (Int32 listIndex = 0; listIndex < listCount; listIndex++)
	{
		const CSldListInfo* listInfo = NULL;
		ESldError error = GetWordListInfo(listIndex, &listInfo);
		if (error != eOK)
			return error;

		if (listInfo->GetUsage() == aListType)
			return listIndex;
	}
	return SLD_DEFAULT_LIST_INDEX;
}

/** *********************************************************************
* Производит сортировку поискового списка слов по заданному варианту написания
*
* @param[in]	aListIndex	- номер поикового списка слов, который нужно отсортировать
* @param[in]	aType		- тип варианта написания, по которому нужно отсортировать
*							  приоритетными считаются варианты из Auxiliary списка
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::SortResultByVariantType(const Int32 aListIndex, const EListVariantTypeEnum aType)
{
	Int32 wordsCount = 0;
	ESldError error = GetNumberOfWords(aListIndex, &wordsCount);
	if (error != eOK)
		return error;

	if (wordsCount < 2)
		return eOK;

	CSldVector<Int32> realWordIndexes;
	realWordIndexes.resize(wordsCount);

	Int32 realListIndex = SLD_DEFAULT_LIST_INDEX;
	Int32 realGlobalIndex = SLD_DEFAULT_WORD_INDEX;

	error = GetRealListIndex(aListIndex, 0, &realListIndex);
	if (error != eOK)
		return error;

	const CSldListInfo * listInfo;
	error = GetWordListInfo(realListIndex, &listInfo);
	if (error != eOK)
		return error;

	Int32 variantIndex = listInfo->GetVariantIndexByType(aType);

	if (variantIndex != SLD_DEFAULT_VARIANT_INDEX)
	{
		for (Int32 wordIndex = 0; wordIndex < wordsCount; wordIndex++)
		{
			error = GetRealIndexes(aListIndex, wordIndex, 0, &realListIndex, &realGlobalIndex);
			if (error != eOK)
				return error;

			realWordIndexes[wordIndex] = realGlobalIndex;
		}
	}
	else
	{
		Int32 initialListIndex = SLD_DEFAULT_LIST_INDEX;
		error = GetInitialWordIndexes(aListIndex, 0, &initialListIndex, &realGlobalIndex);
		if (error != eOK)
			return error;

		if (realListIndex != initialListIndex)
		{
			realListIndex = initialListIndex;
			error = GetWordListInfo(realListIndex, &listInfo);
			if (error != eOK)
				return error;

			variantIndex = listInfo->GetVariantIndexByType(aType);
			if (error != eOK)
				return error;

			if (variantIndex != SLD_DEFAULT_VARIANT_INDEX)
			{
				for (Int32 wordIndex = 0; wordIndex < wordsCount; wordIndex++)
				{
					error = GetInitialWordIndexes(aListIndex, wordIndex, &initialListIndex, &realGlobalIndex);
					if (error != eOK)
						return error;

					realWordIndexes[wordIndex] = realGlobalIndex;
				}
			}
		}
	}

	if (variantIndex == SLD_DEFAULT_VARIANT_INDEX)
		return eOK;

	ISldList * realWordList;
	error = GetWordList(realListIndex, &realWordList);
	if (error != eOK)
		return error;

	SldU16WordsArray allWordsStrings;
	CSldVector<TSldPair<Int32, Int32>> usedWords;
	CSldVector<TSldPair<const SldU16String &, CSldVector<Int32>>> sortedWords;

	allWordsStrings.reserve(realWordIndexes.size());
	usedWords.reserve(realWordIndexes.size());
	sortedWords.reserve(realWordIndexes.size());

	UInt32 sortedWordPos = 0;

	UInt16 * currentWord = NULL;
	for (UInt32 wordIndex = 0; wordIndex < realWordIndexes.size(); wordIndex++)
	{
		UInt32 usedWordsSize = usedWords.size();
		auto it = sld2::sorted_insert(usedWords, TSldPair<Int32, Int32>(realWordIndexes[wordIndex], allWordsStrings.size()));

		if (usedWordsSize != usedWords.size())
		{
			error = realWordList->GetWordByGlobalIndex(realWordIndexes[wordIndex]);
			if (error != eOK)
				return error;

			error = realWordList->GetCurrentWord(variantIndex, &currentWord);
			if (error != eOK)
				return error;

			allWordsStrings.push_back(currentWord);
		}

		error = GetRealIndexes(aListIndex, wordIndex, 0, &realListIndex, &realGlobalIndex);
		if (error != eOK)
			return error;

		if (sld2::binary_search(sortedWords.data(), sortedWords.size(), allWordsStrings[(*it).second], &sortedWordPos))
		{
			sortedWords[sortedWordPos].second.push_back(realGlobalIndex);
		}
		else
		{
			auto currentWordString = TSldPair<const SldU16String &, CSldVector<Int32>>(allWordsStrings[(*it).second], CSldVector<Int32>());
			currentWordString.second.push_back(realGlobalIndex);
			sortedWords.insert(sortedWordPos, sld2::move(currentWordString));
		}
	}

	CSldVector<Int32> sortedIndexes;
	sortedIndexes.reserve(realWordIndexes.size());
	for (const auto &wordInfo : sortedWords)
	{
		for (Int32 index : wordInfo.second)
			sortedIndexes.push_back(index);
	}

	CSldSearchList * currentList;
	error = GetWordList(aListIndex, (ISldList **)&currentList);
	if (error != eOK)
		return error;

	error = currentList->SortByVector(sortedIndexes);
	if (error != eOK)
		return error;

	return eOK;
}

/** *********************************************************************
* Получает количество локализаций записей в текущем списке слов
*
* @param[out]	aNumberOfLocalization	- количество локализаций
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetNumberOfLocalization(Int32 & aNumberOfLocalization)
{
	ISldList* pList = 0;
	ESldError error = GetWordList(GetCurrentListIndex(), &pList);
	if (error != eOK)
		return error;

	return pList->GetNumberOfLocalization(aNumberOfLocalization);
}

/** *********************************************************************
* Устанавливает нужную локализацию записей по номеру локализации в текущем списке слов
*
* @param[in]	aIndex	- номер локализации, которую нужно установить
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::SetLocalizationByIndex(const Int32 aIndex)
{
	ISldList* pList = 0;
	ESldError error = GetWordList(GetCurrentListIndex(), &pList);
	if (error != eOK)
		return error;

	return pList->SetLocalizationByIndex(aIndex);
}

/** *********************************************************************
* Получает информацию о локализации по ее номеру в текущем списке слов
*
* @param[in]	aIndex				- номер локализации, о которой мы хотим получить информацию
* @param[out]	aLocalizationCode	- код языка локализации
* @param[out]	aLocalizationName	- строчка с названием локализации
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetLocalizationInfoByIndex(const Int32 aIndex, UInt32 & aLocalizationCode, SldU16String & aLocalizationName)
{
	ISldList* pList = 0;
	ESldError error = GetWordList(GetCurrentListIndex(), &pList);
	if (error != eOK)
		return error;

	return pList->GetLocalizationInfoByIndex(aIndex, aLocalizationCode, aLocalizationName);
}

/** *********************************************************************
* Возвращает индекс текущей локализации
*
* @param[out]	aIndex		- номер текущей локализации
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetCurrentLocalizationIndex(Int32 & aIndex)
{
	ISldList* pList = 0;
	ESldError error = GetWordList(GetCurrentListIndex(), &pList);
	if (error != eOK)
		return error;

	return pList->GetCurrentLocalizationIndex(aIndex);
}

/**
 * Возвращает строку с javascript'ом ассоциированным со статьями
 *
 * @param[out]  aString - ссылка на строку в которую будет записан javascript
 *
 * ВАЖНО:
 *  eOK говорит *только* об отсутствии ошибки!
 *  есть или нет js для статей нужно проверять по самой строке (длине)
 *  т.е. ситуация когда функция возвращает eOK и пустую строку - нормальна, просто
 *  в словаре нету js для статей
 *
 * @return код ошибки
 */
ESldError CSldDictionary::GetArticlesJavaScript(SldU16String &aString)
{
	aString.clear();

	CSldSingleStringStore stringStore(m_data);
	ESldError error = stringStore.Load(SLD_RESOURCE_ARTICLES_JAVA_SCRIPT);
	if (error != eOK) // fudge the return in case there is no js resource
		return error == eResourceCantGetResource ? eOK : error;

	UInt32 length = stringStore.Size();
	if (length == 0)
		return eOK;

	// length includes a nul-terminator, SldString tracks it on its own, so substract it
	aString.resize(length - 1);

	error = stringStore.Decode(aString.data(), &length);
	if (error != eOK)
		return error;

	aString.resize(length);
	return eOK;
}

/** *********************************************************************
* Возвращает вектор индексов словаря для заданного слова в заданном списке слов
*
* @param[in]	aListIndex		- номер списка
* @param[in]	aGlobalIndex	- номер слова в списке слияния
* @param[out]	aDictIndexes	- вектор с индексами словаря
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetDictionaryIndexesByGlobalIndex(const Int32 aListIndex, const Int32 aGlobalIndex, CSldVector<Int32> & aDictIndexes)
{
	ISldList* listPtr = NULL;
	ESldError error = GetWordList(aListIndex, &listPtr);
	if (error != eOK)
		return error;

	error = listPtr->GetDictionaryIndexesByGlobalIndex(aGlobalIndex, aDictIndexes);
	if (error != eOK)
		return error;

	return eOK;
}

/** *********************************************************************
* Возвращает вектор индексов словаря для заданного слова в текущем списке слов
*
* @param[in]	aGlobalIndex	- номер слова в списке слияния
* @param[out]	aDictIndex		- вектор с индексами словаря
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetDictionaryIndexesByGlobalIndex(const Int32 aGlobalIndex, CSldVector<Int32> & aDictIndexes)
{ 
	return GetDictionaryIndexesByGlobalIndex(GetCurrentListIndex(), aGlobalIndex, aDictIndexes);
}

/** *********************************************************************
* Приводит индексы списка и слова к реальным записям, имеющим переводы
*
* @param[in/out]	aRealListIndex		- номер списка
* @param[in/out]	aRealWordIndex		- номер слова
*										  если вместо одного из индексов возвращается (-1), значит запись имеет количество ссылок,
*										  отличных от 1 и однозначно перейти не удалось
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GoToRealIndexes(Int32 & aRealListIndex, Int32 & aRealWordIndex)
{
	ESldError error;

	const CSldListInfo * listInfo = NULL;
	error = GetWordListInfo(aRealListIndex, &listInfo);
	if (error != eOK)
		return error;

	EWordListTypeEnum listType = listInfo->GetUsage();
	bool isFullTextSearchList = listInfo->IsFullTextSearchList();
	while (isFullTextSearchList != 0 || listType == eWordListType_RegularSearch)
	{
		if (listType != eWordListType_CustomList && listType != eWordListType_Atomic && listType != eWordListType_FullTextAuxiliary && listType != eWordListType_RegularSearch)
			return eOK;

		Int32 refCount = 0;
		error = GetRealIndexesCount(aRealListIndex, aRealWordIndex, &refCount);
		if (error != eOK)
			return error;

		// @TODO Если переводов несколько - нужно выдавать ошибку, однако сейчас много баз с подобными Auxiliary-списками
		//if (refCount != 1)
		//{
		//	aRealListIndex = SLD_DEFAULT_LIST_INDEX;
		//	aRealWordIndex = SLD_DEFAULT_WORD_INDEX;
		//	return eCommonWrongNumberOfTranslation;
		//}

		Int32 realListIndex = SLD_DEFAULT_LIST_INDEX;
		Int32 realWordIndex = SLD_DEFAULT_WORD_INDEX;

		error = GetRealIndexes(aRealListIndex, aRealWordIndex, 0, &realListIndex, &realWordIndex);
		if (error != eOK)
			return error;

		aRealListIndex = realListIndex;
		aRealWordIndex = realWordIndex;

		error = GetWordListInfo(realListIndex, &listInfo);
		if (error != eOK)
			return error;

		listType = listInfo->GetUsage();
		isFullTextSearchList = listInfo->IsFullTextSearchList();
	}

	return eOK;
}

/** *********************************************************************
* Устанавливает данные о слиянии словарей
*
* @param[in]	aInfo		- указатель на дополнительные данные о слиянии
* @param[in]	aDictIdx	- номер текущего словаря среди всех слитых словарей
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::SetMergeInfo(const TMergedMetaInfo *aInfo, UInt32 aDictIdx)
{
	if (aInfo == nullptr)
		return eOK;

	m_MergedDictInfo.MetaInfo = aInfo;
	m_MergedDictInfo.DictIndex = aDictIdx;

	for (Int32 listIndex = 0; listIndex < m_ListCount; listIndex++)
	{
		ISldList* pList = 0;
		ESldError error = GetWordList(listIndex, &pList);
		if (error != eOK)
			return error;

		if (!pList)
			return eMemoryNullPointer;

		if (pList->GetHereditaryListType() == eHereditaryListTypeNormal)
		{
			((CSldList*)pList)->SetMergeInfo(&m_MergedDictInfo);
		}
	}

	return eOK;
}

/** *********************************************************************
* Возвращает коллекцию ссылок, идущих в начале статьи
* Декодирует статью до первого текстового блока
*
* @param[in]	aListIndex		- номер списка слов
* @param[in]	aGlobalIndex	- глобальный индекс статьи, для которой мы хотим получить коллекцию ссылок
* @param[out]	aLinks			- коллекция ссылок
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionary::GetArticleLinks(const Int32 aListIndex, const Int32 aGlobalIndex, SldLinksCollection & aLinks)
{
	if (m_Articles == NULL)
		return eCommonDictionaryHasNoTranslations;

	aLinks.clear();

	ISldList* pList = NULL;
	ESldError error = GetWordList(aListIndex, &pList);
	if (error != eOK)
		return error;

	if (!pList)
		return eMemoryNullPointer;

	Int32 NumberOfWords = 0;
	error = pList->GetTotalWordCount(&NumberOfWords);
	if (error != eOK)
		return error;

	if (aGlobalIndex >= NumberOfWords || aGlobalIndex < 0)
		return eCommonWrongIndex;

	Int32 translationCount = 0;
	error = pList->GetTranslationCount(aGlobalIndex, &translationCount);
	if (error != eOK)
		return error;

	SplittedArticle article;
	for (UInt32 translationIndex = 0; translationIndex < translationCount; translationIndex++)
	{
		Int32 articleIndex;
		error = pList->GetTranslationIndex(aGlobalIndex, translationIndex, &articleIndex);
		if (error != eOK)
			return error;

		error = m_Articles->GetArticle(articleIndex, article, eMetaText);
		if (error != eOK)
			return error;
	}

	CSldMetadataParser parser(this, nullptr);
	for (const auto & block : article)
	{
		const CSldStyleInfo* styleInfo;
		error = GetStyleInfo(block.StyleIndex, &styleInfo);
		if (error != eOK)
			return error;

		if (styleInfo->GetStyleMetaType() == eMetaLink)
		{
			const auto link = parser.GetMetadata<eMetaLink>(block.Text.c_str());
			if(!link.isClosing())
				aLinks.emplace_back(link->ListIndex, link->EntryIndex, link.c_str(link->ExtKey.DictId), link.c_str(link->ExtKey.Key));
		}
	}

	return eOK;
}
