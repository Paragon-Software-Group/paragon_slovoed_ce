#include "SldMergedDictionary.h"

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
ESldError CSldMergedDictionary::Open(ISDCFile *aFile, ISldLayerAccess *aLayerAccess)
{
	m_LayerAccess = aLayerAccess;
	m_Dictionaries.push_back(sld2::make_unique<CSldDictionary>());
	return m_Dictionaries.back()->Open(aFile, aLayerAccess);
}

/** *********************************************************************
* Закрывает словарь и освобождает все ресурсы и память
*
* @return код ошибки
************************************************************************/
ESldError CSldMergedDictionary::Close()
{
	for (auto list : m_MergedWordlists)
	{
		if (list->GetHereditaryListType() == eHereditaryListTypeMerged)
			sldDelete(list);
	}
	m_MergedWordlists.clear();

	CSldDictionary::Clear();
	m_Dictionaries.clear();

	Clear();
	return eOK;
}

/** *********************************************************************
* Инициализация членов по умолчанию
************************************************************************/
void CSldMergedDictionary::Clear()
{
	m_LayerAccess = NULL;
	m_CurrentListIndex = 0;
	m_PrevListIndex = 0;
}

/** *********************************************************************
* Добавляет новый словарь в ядро слияния
*
* @param[in]	aFile	- указатель на открытый файл контейнера с sdc базой
*
* @return код ошибки
************************************************************************/
ESldError CSldMergedDictionary::AddDictionary(ISDCFile & aFile)
{
	auto newDict = sld2::make_unique<CSldDictionary>();
	ESldError error = newDict->Open(&aFile, m_LayerAccess);
	if (error != eOK)
		return error;

	UInt32 dictId = 0;
	error = newDict->GetDictionaryID(&dictId);
	if (error != eOK)
		return error;

	UInt32 hash = 0;
	error = newDict->GetDictionaryHash(&hash);
	if (error != eOK)
		return error;

	for (auto&& dict : m_Dictionaries)
	{
		UInt32 someDictId = 0;
		error = dict->GetDictionaryID(&someDictId);
		if (error != eOK)
			return error;

		UInt32 someHash = 0;
		error = dict->GetDictionaryHash(&someHash);
		if (error != eOK)
			return error;

		if (dictId == someDictId || hash == someHash)
			return newDict->Close();
	}

	m_Dictionaries.push_back(sld2::move(newDict));
	return eOK;
}

/** *********************************************************************
* Производит слияние словарей
*
* @param[in]	aMergedDictionary	- если у нас уже есть полностью инициализированное ядро слияния
*									  мы можем воспользоваться им и ускорить загрузку
*
* @return код ошибки
************************************************************************/
ESldError CSldMergedDictionary::Merge(const CSldMergedDictionary * aMergedDictionary)
{
	if (m_Dictionaries.size() < 1)
		return eOK;

	ESldError error = CheckWordLists();
	if (error != eOK)
		return error;

	m_MergedMetaInfo.WordIndexes.resize(m_ListsBuffer.size());
	Int32 mergedListIndex = 0;
	for (auto & list : m_ListsBuffer)
	{
		if (list.size() == 1)
		{
			m_MergedWordlists.push_back(list.back().first);
		}
		else if (list.size() > 1)
		{
			auto mergedList = sld2::make_unique<CSldMergedList>(list, m_MergedMetaInfo);

			auto & wordIndexes = m_MergedMetaInfo.WordIndexes[mergedListIndex];
			wordIndexes.resize(m_Dictionaries.size());

			CSldMergedList * sourceMergedList = NULL;
			if (aMergedDictionary != NULL)
				sourceMergedList = (CSldMergedList*)aMergedDictionary->m_MergedWordlists[m_MergedWordlists.size()];

			error = mergedList->InitMergedList(wordIndexes, sourceMergedList);
			if (error != eOK)
				return error;

			m_MergedWordlists.push_back(mergedList.release());
		}
		mergedListIndex++;
	}

	m_ListsBuffer.clear();
	m_CurrentListIndex = 0;

	if (aMergedDictionary != NULL)
	{
		m_MergedMetaInfo = aMergedDictionary->m_MergedMetaInfo;
	}
	else
	{
		m_MergedMetaInfo.StylesCount.push_back(0);
		m_MergedMetaInfo.ArticlesCount.push_back(0);
		m_MergedMetaInfo.SoundsCount.push_back(0);
		m_MergedMetaInfo.PicturiesCount.push_back(0);
		for (auto & dict : m_Dictionaries)
		{
			m_MergedMetaInfo.StylesCount.push_back(m_MergedMetaInfo.StylesCount.back() + dict->GetDictionaryHeader()->NumberOfStyles);
			m_MergedMetaInfo.ArticlesCount.push_back(m_MergedMetaInfo.ArticlesCount.back() + dict->GetDictionaryHeader()->NumberOfArticles);
			m_MergedMetaInfo.SoundsCount.push_back(m_MergedMetaInfo.SoundsCount.back() + dict->GetDictionaryHeader()->TotalSoundCount);
			m_MergedMetaInfo.PicturiesCount.push_back(m_MergedMetaInfo.PicturiesCount.back() + dict->GetDictionaryHeader()->TotalPictureCount);
		}
	}

	for (UInt32 i = 0; i < m_Dictionaries.size(); i++)
	{
		error = m_Dictionaries[i]->SetMergeInfo(&m_MergedMetaInfo, i);
		if (error != eOK)
			return error;
	}

	for (Int32 styleIndex = 0; styleIndex < m_MergedMetaInfo.StylesCount.back(); styleIndex++)
	{
		CSldStyleInfo* style;
		error = GetStyleInfo(styleIndex, (const CSldStyleInfo**)&style);
		if (error != eOK)
			return error;

		style->SetGlobalIndex(styleIndex);
	}

	return eOK;
}

/** *********************************************************************
* Подготавливает списки для слияния
*
* @return код ошибки
************************************************************************/
ESldError CSldMergedDictionary::CheckWordLists()
{
	Int32 dictIndex = 0;
	for (auto&& dict : m_Dictionaries)
	{
		m_MergedMetaInfo.ListIndexes.emplace_back();
		for (UInt32 listIndex = 0; listIndex < dict->m_ListCount; listIndex++)
		{
			ISldList* listPtr = NULL;
			ESldError error = dict->GetWordList(listIndex, &listPtr);
			if (error != eOK)
				return error;

			UInt32 resultListIndex = SLD_DEFAULT_LIST_INDEX;
			error = CheckList(dictIndex, listPtr, resultListIndex);
			if (error != eOK)
				return error;

			if (resultListIndex == m_ListsBuffer.size())
			{
				m_ListsBuffer.emplace_back().emplace_back(listPtr, dictIndex);
				m_MergedMetaInfo.ListIndexes.back().push_back(m_ListsBuffer.size() - 1);
				m_MergedMetaInfo.DictIndexes.emplace_back();
				m_MergedMetaInfo.DictIndexes.back().push_back(dictIndex);
			}
			else if (resultListIndex != SLD_DEFAULT_LIST_INDEX)
			{
				m_ListsBuffer[resultListIndex].emplace_back(listPtr, dictIndex);
				m_MergedMetaInfo.ListIndexes.back().push_back(resultListIndex);
				m_MergedMetaInfo.DictIndexes[resultListIndex].push_back(dictIndex);
			}
		}
		dictIndex++;
	}

	return eOK;
}

/** *********************************************************************
* Ищет для заданного списка одиночного словаря подходящий список слияния
*
* @param[in]	aDictIndex		- индекс словаря
* @param[in]	aListPtr		- указатель на одиночный список
* @param[out]	resultListIndex	- индекс подходящего списка слияния
*								  SLD_DEFAULT_LIST_INDEX - если такого списка нет
*
* @return код ошибки
************************************************************************/
ESldError CSldMergedDictionary::CheckList(const Int32 aDictIndex, ISldList* aListPtr, UInt32 & resultListIndex)
{
	if (!aListPtr)
		return eMemoryNullPointer;

	resultListIndex = SLD_DEFAULT_LIST_INDEX;

	const CSldListInfo *listInfo = NULL;
	ESldError error = aListPtr->GetWordListInfo(&listInfo);
	if (error != eOK)
		return error;

	Int32 baseListIndex = SLD_DEFAULT_LIST_INDEX;

	const EWordListTypeEnum listUsage = listInfo->GetUsage();
	if (listUsage == eWordListType_SimpleSearch || listUsage == eWordListType_StructuredMetadataStrings || listUsage == eWordListType_CSSDataStrings)
		return eOK;

	if (listInfo->IsFullTextSearchListType())
	{
		error = GetBaseListIndex(aDictIndex, aListPtr, baseListIndex);
		if (error != eOK)
			return error;
	}
	else if (listUsage != eWordListType_Dictionary)
	{
		resultListIndex = m_ListsBuffer.size();
		return eOK;
	}

	const UInt32 languageFrom = listInfo->GetLanguageFrom();
	const UInt32 languageTo = listInfo->GetLanguageTo();

	Int32 checkedListIndex = 0;
	for (auto & checkedList : m_ListsBuffer)
	{
		const CSldListInfo * checkedListInfo = NULL;
		error = checkedList.back().first->GetWordListInfo(&checkedListInfo);
		if (error != eOK)
			return error;

		Int32 checkedBaseListIndex = SLD_DEFAULT_LIST_INDEX;

		if (listInfo->IsFullTextSearchListType())
		{
			error = GetBaseListIndex(aDictIndex, aListPtr, checkedBaseListIndex);
			if (error != eOK)
				return error;
		}

		const EWordListTypeEnum checkedListUsage = checkedListInfo->GetUsage();
		const UInt32 checkedLanguageFrom = checkedListInfo->GetLanguageFrom();
		const UInt32 checkedLanguageTo = checkedListInfo->GetLanguageTo();
		if (listUsage == checkedListUsage && languageFrom == checkedLanguageFrom && languageTo == checkedLanguageTo && baseListIndex == checkedBaseListIndex && aDictIndex != checkedList.back().second)
		{
			resultListIndex = checkedListIndex;
			return eOK;
		}
		checkedListIndex++;
	}

	resultListIndex = m_ListsBuffer.size();
	return eOK;
}

/** *********************************************************************
* Для полнотекстовых списков возвращает индекс базового списка
*
* @param[in]	aDictIndex		- индекс словаря
* @param[in]	aListPtr		- указатель на одиночный список
* @param[out]	aBaseListIndex	- индекс базового списка
*
* @return код ошибки
************************************************************************/
ESldError CSldMergedDictionary::GetBaseListIndex(const Int32 aDictIndex, ISldList * aListPtr, Int32 & aBaseListIndex)
{
	if (!aListPtr)
		return eMemoryNullPointer;

	Int32 listEntryIdx = 0;
	Int32 transIdx = 0;
	Int32 shiftIdx = 0;

	ESldError error = ((CSldList*)aListPtr)->GetFullTextTranslationData(0, 0, &aBaseListIndex, &listEntryIdx, &transIdx, &shiftIdx);
	if (error != eOK)
		return error;

	ISldList* listPtr = NULL;
	error = m_Dictionaries[aDictIndex]->GetWordList(aBaseListIndex, &listPtr);
	if (error != eOK)
		return error;

	const CSldListInfo *listInfo = NULL;
	error = listPtr->GetWordListInfo(&listInfo);
	if (error != eOK)
		return error;

	if (listInfo->IsFullTextSearchList())
	{
		error = GetBaseListIndex(aDictIndex, listPtr, aBaseListIndex);
		if (error != eOK)
			return error;
	}

	return eOK;
}

/** *********************************************************************
* Возвращает количество списков слов
*
* @param[out]	aNumberOfLists	- указатель на буфер для количества слов.
*
* @return код ошибки
************************************************************************/
ESldError CSldMergedDictionary::GetNumberOfLists(Int32* aNumberOfLists) const
{
	if (!aNumberOfLists)
		return eMemoryNullPointer;

	*aNumberOfLists = m_MergedWordlists.size();
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
ESldError CSldMergedDictionary::GetWordListInfo(Int32 aListIndex, const CSldListInfo **aListInfo) const
{
	if (!aListInfo)
		return eMemoryNullPointer;

	if (aListIndex >= m_MergedWordlists.size() || aListIndex < 0)
		return eCommonWrongList;

	return m_MergedWordlists[aListIndex]->GetWordListInfo(aListInfo);
}

/** *********************************************************************
* Устанавливает список слов в качестве текущего; таблица сравнения также
* переключается на язык нового списка
*
* @param[in]	aIndex	- номер списка слов
*
* @return код ошибки
************************************************************************/
ESldError CSldMergedDictionary::SetCurrentWordlist(Int32 aIndex)
{
	if (aIndex >= m_MergedWordlists.size() || aIndex < 0)
		return eCommonWrongIndex;

	UInt32 listUsage = 0;
	ESldError error = m_MergedWordlists[m_CurrentListIndex]->GetListUsage(0, &listUsage);
	if (error != eOK)
		return error;

	if (listUsage != eWordListType_Unknown && listUsage != eWordListType_RegularSearch && listUsage != eWordListType_CustomList)
		m_PrevListIndex = m_CurrentListIndex;

	m_CurrentListIndex = aIndex;

	return eOK;
}
/** *********************************************************************
* Получает индекс текущего списка слов
*
* @param[out]	aIndex	- указатель на переменную, куда сохраняется индекс
*
* @return код ошибки
************************************************************************/
ESldError CSldMergedDictionary::GetCurrentWordList(Int32* aIndex) const
{
	if (!aIndex)
		return eMemoryNullPointer;

	*aIndex = m_CurrentListIndex;

	return eOK;
}


/** *********************************************************************
* Возвращает указатель на экземпляр объекта списка слов по номеру списка
*
* @param[in]	aIndex		- номер списка слов
* @param[out]	aListPtr	- указатель, по которому будет записан указатель на список слов
*
* @return код ошибки
************************************************************************/
ESldError CSldMergedDictionary::GetWordList(Int32 aIndex, ISldList** aListPtr)
{
	if (!aListPtr)
		return eMemoryNullPointer;
	*aListPtr = 0;

	Int32 listCount = 0;
	ESldError error = GetNumberOfLists(&listCount);
	if (error != eOK)
		return error;

	if (aIndex >= listCount || aIndex < 0)
		return eCommonWrongIndex;

	*aListPtr = m_MergedWordlists[aIndex];
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
ESldError CSldMergedDictionary::AddList(sld2::UniquePtr<ISldList> aList, Int32 aListIndex)
{
	m_MergedWordlists.insert(aListIndex, aList.release());
	return eOK;
}

/** *********************************************************************
* Удаляет список слов
*
* @param[in]	aListIndex	- индекс списка, который нужно удалить из словаря
*
* @return код ошибки
************************************************************************/
ESldError CSldMergedDictionary::RemoveList(Int32 aListIndex)
{
	m_MergedWordlists.erase(aListIndex);
	return eOK;
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
ESldError CSldMergedDictionary::GetStyleInfo(Int32 aIndex, const CSldStyleInfo **aStyleInfo) const
{
	UInt32 dictIndex;
	aIndex = m_MergedMetaInfo.ToLocalIndex<TMergedMetaInfo::Style>(aIndex, &dictIndex);
	return m_Dictionaries[dictIndex]->GetStyleInfo(aIndex, aStyleInfo);
}

/** *********************************************************************
* Возвращает количество стилей
*
* @return количество стилей
************************************************************************/
UInt32 CSldMergedDictionary::GetNumberOfStyles() const
{
	return m_MergedMetaInfo.StylesCount.empty() ? 0 : m_MergedMetaInfo.StylesCount.back();
}

/** *********************************************************************
* Производит перевод указанной статьи по глобальному номеру статьи
*
* @param[in]	aIndex			- глобальный индекс статьи
* @param[in]	aFullness		- полнота получаемого перевода
*
* @return код ошибки
************************************************************************/
ESldError CSldMergedDictionary::TranslateByArticleIndex(Int32 aIndex, ESldTranslationFullnesType aFullness, UInt32 aStartBlock, UInt32 aEndBlock)
{
	aIndex = m_MergedMetaInfo.ToLocalIndex<TMergedMetaInfo::Article>(aIndex, (UInt32*)&m_TranslateDictionaryIndex);
	return m_Dictionaries[m_TranslateDictionaryIndex]->TranslateByArticleIndex(aIndex, aFullness, aStartBlock, aEndBlock);
}

/** *********************************************************************
* Производит получение остатков перевода (если он в какой-то момент был приостановлен)
*
* @param[in]	aFullness	- степень полноты полученного перевода
*
* @return код ошибки
************************************************************************/
ESldError CSldMergedDictionary::TranslateContinue(ESldTranslationFullnesType aFullness)
{
	return m_Dictionaries[m_TranslateDictionaryIndex]->TranslateContinue(aFullness);
}


/** *********************************************************************
* Производит обновление состояния перевода (завершает предыдущий прерванный
* перевод и производит реинициализацию
*
* @return код ошибки
************************************************************************/
ESldError CSldMergedDictionary::TranslateClear()
{
	return m_Dictionaries[m_TranslateDictionaryIndex]->TranslateClear();
}


/** *********************************************************************
* Инициализируем систему защиты.
*
* @return код ошибки
************************************************************************/
ESldError CSldMergedDictionary::InitRegistration(void)
{
	ESldError error = eOK;
	for (auto & dict : m_Dictionaries)
	{
		error = dict->InitRegistration();
		if (error != eOK)
			return error;
	}

	return eOK;
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
ESldError CSldMergedDictionary::GetCurrentWordStylizedVariant(Int32 aVariantIndex, SplittedArticle & aArticle)
{
	return eOK;
}

/** *********************************************************************
* Возвращает общее количество картинок в словаре
*
* @param[out]	aCount	- указатель, по которому будет записан результат
*
* @return код ошибки
************************************************************************/
ESldError CSldMergedDictionary::GetTotalPictureCount(Int32* aCount) const
{
	if (!aCount)
		return eMemoryNullPointer;

	*aCount = m_MergedMetaInfo.PicturiesCount.back();

	return eOK;
}

/** *********************************************************************
* Возвращает общее количество озвучек в словаре
*
* @param[out]	aCount	- указатель, по которому будет записан результат
*
* @return код ошибки
************************************************************************/
ESldError CSldMergedDictionary::GetTotalSoundCount(Int32* aCount) const
{
	if (!aCount)
		return eMemoryNullPointer;

	*aCount = m_MergedMetaInfo.SoundsCount.back();

	return eOK;
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
ESldError CSldMergedDictionary::GetWordPicture(Int32 aPictureIndex, Int32 aMaxSideSize, TImageElement* aImageElement)
{
	ESldError error = eMergeWrongDictionaryIndex;
	UInt32 dictIndex;
	const Int32 localIndex = m_MergedMetaInfo.ToLocalIndex<TMergedMetaInfo::Picture>(aPictureIndex, &dictIndex);
	if (dictIndex < m_Dictionaries.size())
	{
		error = m_Dictionaries[dictIndex]->GetWordPicture(localIndex, aMaxSideSize, aImageElement);
		if (error == eOK)
			aImageElement->ImageIndex = aPictureIndex;
	}
	return error;
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
ESldError CSldMergedDictionary::PlaySoundByIndex(Int32 aSoundIndex, UInt8 aIsLast, UInt32* aStartPos, UInt32 aExternFlag, UInt8 aBeginPlay)
{
	UInt32 dictIndex;
	aSoundIndex = m_MergedMetaInfo.ToLocalIndex<TMergedMetaInfo::Sound>(aSoundIndex, &dictIndex);
	if (dictIndex < m_Dictionaries.size())
		return m_Dictionaries[dictIndex]->PlaySoundByIndex(aSoundIndex, aIsLast, aStartPos, aExternFlag, aBeginPlay);

	return eMergeWrongDictionaryIndex;
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
ESldError CSldMergedDictionary::GetLocalizedStrings(const CSldLocalizedString** aStrings, const Int32 aDictionaryIndex) const
{
	if (aDictionaryIndex == SLD_DEFAULT_DICTIONARY_INDEX)
		return eMergeWrongDictionaryIndex;

	return m_Dictionaries[aDictionaryIndex]->GetLocalizedStrings(aStrings);
}

/** *********************************************************************
* Возвращает номер старшей версии словарной базы
*
* @param[out]	aVersion			- указатель, по которому будет записан результат
* @param[in]	aDictionaryIndex	- индекс словаря, для которого мы хотим получить информацию
*
* @return код ошибки
************************************************************************/
ESldError CSldMergedDictionary::GetDictionaryMajorVersion(UInt32* aVersion, const Int32 aDictionaryIndex) const
{
	return m_Dictionaries[aDictionaryIndex]->GetDictionaryMajorVersion(aVersion);
}

/** *********************************************************************
* Возвращает номер младшей версии словарной базы
*
* @param[out]	aVersion			- указатель, по которому будет записан результат
* @param[in]	aDictionaryIndex	- индекс словаря, для которого мы хотим получить информацию
*
* @return код ошибки
************************************************************************/
ESldError CSldMergedDictionary::GetDictionaryMinorVersion(UInt32* aVersion, const Int32 aDictionaryIndex) const
{
	return m_Dictionaries[aDictionaryIndex]->GetDictionaryMinorVersion(aVersion);
}

/** *********************************************************************
* Возвращает константу, которая обозначает бренд словарной базы (см. EDictionaryBrandName)
*
* @param[out]	aBrand				- указатель, по которому будет записан результат
* @param[in]	aDictionaryIndex	- индекс словаря, для которого мы хотим получить информацию
*
* @return код ошибки
************************************************************************/
ESldError CSldMergedDictionary::GetDictionaryBrand(UInt32* aBrand, const Int32 aDictionaryIndex) const
{
	return m_Dictionaries[aDictionaryIndex]->GetDictionaryBrand(aBrand);
}

/** *********************************************************************
* Возвращает хэш словарной базы
*
* @param[out]	aHash				- указатель, по которому будет записан результат
* @param[in]	aDictionaryIndex	- индекс словаря, для которого мы хотим получить информацию
*
* @return код ошибки
************************************************************************/
ESldError CSldMergedDictionary::GetDictionaryHash(UInt32* aHash, const Int32 aDictionaryIndex) const
{
	return m_Dictionaries[aDictionaryIndex]->GetDictionaryHash(aHash);
}

/** *********************************************************************
* Возвращает ID словарной базы
*
* @param[out]	aDictID				- указатель, по которому будет записан результат
* @param[in]	aDictionaryIndex	- индекс словаря, для которого мы хотим получить информацию
*
* @return код ошибки
************************************************************************/
ESldError CSldMergedDictionary::GetDictionaryID(UInt32* aDictID, const Int32 aDictionaryIndex) const
{
	return m_Dictionaries[aDictionaryIndex]->GetDictionaryID(aDictID);
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
ESldError CSldMergedDictionary::DoFullTextSearch(Int32 aListIndex, const UInt16* aText, Int32 aMaximumWords)
{
	ESldError error;
	CSldVector<CSldVector<Int32>> ftsResult(m_MergedWordlists.size());

	Int32 dictIndex = 0;
	for (auto&& dict : m_Dictionaries)
	{
		Int32 listIndex = 0;
		for (auto mergedListIndex : m_MergedMetaInfo.ListIndexes[dictIndex])
		{
			if (mergedListIndex == aListIndex)
			{
				error = dict->DoFullTextSearch(listIndex, aText, aMaximumWords);
				if (error != eOK)
					return error;

				Int32 wordsCount = 0;
				error = dict->GetNumberOfWords(&wordsCount);
				if (error != eOK)
					return error;

				for (Int32 resultIndex = 0; resultIndex < wordsCount; resultIndex++)
				{
					Int32 realListIndex = SLD_DEFAULT_LIST_INDEX;
					Int32 realWordIndex = SLD_DEFAULT_WORD_INDEX;

					error = dict->GetRealIndexes(resultIndex, 0, &realListIndex, &realWordIndex);
					if (error != eOK)
						return error;

					const Int32 mergedListIndex = m_MergedMetaInfo.ListIndexes[dictIndex][realListIndex];
					const Int32 mergedWordIndex = m_MergedMetaInfo.WordIndexes[mergedListIndex].empty() ? realWordIndex : m_MergedMetaInfo.WordIndexes[mergedListIndex][dictIndex][realWordIndex];

					sld2::sorted_insert(ftsResult[mergedListIndex], mergedWordIndex);
				}

				error = dict->ClearSearch();
				if (error != eOK)
					return error;
			}
			listIndex++;
		}
		dictIndex++;
	}

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

	error = pSearchList->SetMaximumLists(m_MergedWordlists.size());
	if (error != eOK)
		return error;

	error = pSearchList->SetMaximumWords(aMaximumWords);
	if (error != eOK)
		return error;

	Int32 listIndex = 0;
	Int32 maxWordsCount = 0;
	Int32 firstResultListIndex = SLD_DEFAULT_LIST_INDEX;
	TSldSearchListStruct* wordList = NULL;

	for (auto & list : ftsResult)
	{
		error = pSearchList->MakeList((CSldList*)m_MergedWordlists[listIndex], listIndex, &wordList);
		if (error != eOK)
			return error;

		if (!list.empty())
		{
			if (firstResultListIndex == SLD_DEFAULT_LIST_INDEX)
				firstResultListIndex = listIndex;
		}

		for (auto wordIndex : list)
		{
			if (maxWordsCount >= aMaximumWords)
				break;

			TSldSearchWordStruct word;
			word.WordIndex = wordIndex;
			word.ListIndex = listIndex;

			error = pSearchList->AddWord(word);
			if (error != eOK)
				return error;

			maxWordsCount++;
		}
		listIndex++;
	}

	if (firstResultListIndex != SLD_DEFAULT_LIST_INDEX)
	{
		const CSldListInfo * sourceListInfo;
		error = GetWordListInfo(firstResultListIndex, &sourceListInfo);
		if (error != eOK)
			return error;

		error = pSearchList->UpdateListHeader(sourceListInfo);
		if (error != eOK)
			return error;
	}

	// Добавляем поисковый список в общий массив списков словаря
	error = AddList(sld2::move(pSearchList), m_MergedWordlists.size());
	if (error != eOK)
		return error;

	// Устанавливаем список с результатами поиска в качестве текущего
	error = SetCurrentWordlist(m_MergedWordlists.size() - 1);
	if (error != eOK)
		return error;

	return eOK;
}

/** *********************************************************************
* Получает реальные индексы списка и записи в реальном списке в по глобальному номеру слова
*
* @param[in]	aListIndex			- номер списка в ядре слияния
* @param[in]	aWordIndex			- глобальный номер слова
* @param[in]	aTrnslationIndex	- номер индексов которые мы хотим получить (можно получить через GetNumberOfTranslations() для списков типа IsFullTextSearchList())
* @param[out]	aRealListIndex		- указатель на переменную, в которую сохраняется индекс списка
* @param[out]	aGlobalWordIndex	- указатель на переменную, в которую сохраняется глобальный номер слова
*
* @return код ошибки
************************************************************************/
ESldError CSldMergedDictionary::GetRealIndexes(Int32 aListIndex, Int32 aLocalWordIndex, Int32 aTrnslationIndex, Int32* aRealListIndex, Int32* aGlobalWordIndex)
{
	ISldList* pList = 0;
	ESldError error = GetWordList(aListIndex, &pList);
	if (error != eOK)
		return error;

	if (!pList)
		return eMemoryNullPointer;

	UInt32 listType = eWordListType_Unknown;
	error = pList->GetListUsage(0, &listType);
	if (error != eOK)
		return error;

	error = pList->GetRealIndexes(aLocalWordIndex, aTrnslationIndex, aRealListIndex, aGlobalWordIndex);
	if (error != eOK)
		return error;

	if (listType != eWordListType_RegularSearch && listType != eWordListType_CustomList && *aRealListIndex != SLD_DEFAULT_LIST_INDEX && *aRealListIndex != aListIndex)
	{
		const Int32 dictIndex = m_MergedMetaInfo.DictIndexes[aListIndex].front();
		*aRealListIndex = m_MergedMetaInfo.ListIndexes[dictIndex][*aRealListIndex];
		*aGlobalWordIndex = m_MergedMetaInfo.WordIndexes[*aRealListIndex].empty() ? *aGlobalWordIndex : m_MergedMetaInfo.WordIndexes[*aRealListIndex][dictIndex][*aGlobalWordIndex];
	}

	return eOK;
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
ESldError CSldMergedDictionary::UpdateHistoryElement(CSldHistoryElement* aHistoryElement, ESldHistoryResult* aResultFlag)
{
	if (!aHistoryElement || !aResultFlag)
		return eMemoryNullPointer;

	ESldError error = eOK;
	UInt32 dictID = 0;
	*aResultFlag = eHistoryNotCompare;
	for (auto && dict : m_Dictionaries)
	{
		error = dict->GetDictionaryID(&dictID);
		if (error != eOK)
			return error;

		if (dictID == aHistoryElement->GetDictID())
			return dict->UpdateHistoryElement(aHistoryElement, aResultFlag);
	}

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
ESldError CSldMergedDictionary::GetWordByHistoryElement(CSldHistoryElement* aHistoryElement, ESldHistoryResult* aResultFlag, Int32* aListIndex, Int32* aGlobalIndex)
{
	if (!aHistoryElement || !aResultFlag || !aListIndex || !aGlobalIndex)
		return eMemoryNullPointer;

	ESldError error = eOK;
	UInt32 dictID = 0;
	*aResultFlag = eHistoryNotCompare;
	Int32 dictIndex = 0;

	for (auto && dict : m_Dictionaries)
	{
		error = dict->GetDictionaryID(&dictID);
		if (error != eOK)
			return error;

		if (dictID == aHistoryElement->GetDictID())
		{
			error = dict->UpdateHistoryElement(aHistoryElement, aResultFlag);
			if (error != eOK)
				return error;

			break;
		}
		dictIndex++;
	}

	if (*aResultFlag != eHistoryNotCompare)
	{
		const Int32 mergedListIndex = m_MergedMetaInfo.ListIndexes[dictIndex][aHistoryElement->GetListIndex()];
		const Int32 mergedWordIndex = m_MergedMetaInfo.WordIndexes[dictIndex][aHistoryElement->GetListIndex()][aHistoryElement->GetWordIndex()];

		if (mergedListIndex != GetCurrentListIndex())
		{
			*aListIndex = mergedListIndex;
			error = SetCurrentWordlist(aHistoryElement->GetListIndex());
			if (error != eOK)
				return error;
		}

		*aGlobalIndex = mergedWordIndex;
		error = GoToByGlobalIndex(mergedWordIndex);
		if (error != eOK)
			return error;
	}

	return eOK;
}
