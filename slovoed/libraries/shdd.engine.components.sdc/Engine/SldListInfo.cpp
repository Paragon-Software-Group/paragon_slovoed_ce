#include "SldListInfo.h"

/** ********************************************************************
* Производит инициализацию класса
*
* @param[in]	aData	- ссылка на объект, отвечающий за получение данных из контейнера
* @param[in]	aHeader	- указатель на заголовок списка слов
*
* @return код ошибки
************************************************************************/
ESldError CSldListInfo::Init(CSDCReadMy &aData, const TListHeader* aHeader)
{
	ESldError error;

	if (!aHeader)
		return eMemoryNullPointer;

	// Заголовок
	sldMemCopy(&m_Header, aHeader, aHeader->HeaderSize);

	// Локализованные строки списка слов
	if (m_Header.LocalizedStringsRecordsCount)
	{
		auto strings = sld2::make_unique<CSldListLocalizedString>(aData, m_Header.LocalizedStringsResourceIndex,
																  m_Header.LocalizedStringsRecordsCount, error);
		if (!strings || error != eOK)
			return strings ? error : eMemoryNotEnoughMemory;

		m_ListStrings = sld2::move(strings);
	}
	
	// Таблица свойств вариантов написания
	if (m_Header.NumberOfVariants)
	{
		// Загружаем данные из ресурса
		auto res = aData.GetResource(SLD_RESOURCE_VARIANTS_PROPERTY_TABLE, m_Header.VariantsPropertyResourceIndex);
		// TODO: Возвращать ошибку (пока в большинстве баз этих данных нет)
		//return res.error();
		if (res != eOK)
			return eOK;

		const UInt32 propertySize = sizeof(m_VariantPropertyTable[0]);
		m_VariantPropertyTable.resize(m_Header.NumberOfVariants);
		
		// Проверка на корректность данных
		const UInt32 propertyCountInRes = res.size() / propertySize;
		if (res.size() % propertySize || propertyCountInRes != m_Header.NumberOfVariants)
			return eInputWrongStructSize;

		auto propertyTable = (TListVariantProperty *)res.ptr();
		for (UInt32 variantIndex = 0; variantIndex < propertyCountInRes; variantIndex++)
		{
			m_VariantPropertyTable[variantIndex] = propertyTable[variantIndex];

			if (m_VariantPropertyTable[variantIndex].Type == eVariantShow)
				m_ShowVariantIndex = variantIndex;
		}
	}
	
	return eOK;
}

/** ********************************************************************
* Производит инициализацию класса
*
* @param[in]	aHeader	- указатель на заголовок списка слов
*
* @return код ошибки
************************************************************************/
ESldError CSldListInfo::Init(const TListHeader & aHeader)
{
	m_Header = aHeader;
	m_Header.NumberOfVariants = 0;

	TListVariantProperty newProperty;
	newProperty.LangCode = aHeader.LanguageCodeFrom;
	newProperty.Type = eVariantShow;
	AddVariantProperty(newProperty);
	m_ShowVariantIndex = 0;

	return eOK;
}

/** ********************************************************************
* Производит инициализацию класса
*
* @param[in]	aListInfo	- информация о списке
* @param[in]	aHeader	    - указатель на заголовок списка слов
*
* @return код ошибки
************************************************************************/
ESldError CSldListInfo::Init(const CSldListInfo & aListInfo, const TListHeader* aHeader)
{
	if (aHeader)
		sldMemCopy(&m_Header, aHeader, aHeader->HeaderSize);
	else
		m_Header = aListInfo.m_Header;

	m_ShowVariantIndex = aListInfo.GetShowVariantIndex();
	m_VariantPropertyTable = aListInfo.m_VariantPropertyTable;
	m_ListStrings = sld2::make_unique<CSldListLocalizedString>(*aListInfo.m_ListStrings);
	return eOK;
}

/** *********************************************************************
* Возвращает тип варианта написания слова по индексу варианта
*
* @param[in]	aIndex			- номер запрашиваемого варианта написания
*
* @return тип варианта написания или eVariantUnknown в случае ошибки
************************************************************************/
EListVariantTypeEnum CSldListInfo::GetVariantType(UInt32 aIndex) const
{
	if (aIndex >= m_VariantPropertyTable.size())
		return eVariantUnknown;

	return static_cast<EListVariantTypeEnum>(m_VariantPropertyTable[aIndex].Type);
}

/** *********************************************************************
* Возвращает код языка варианта написания слова по индексу варианта
*
* @param[in]	aIndex		- номер запрашиваемого варианта написания
* @param[out]	aLangCode	- указатель на переменную, куда будет сохранен результат
*
* @return код языка или 0 в случае ошибки
************************************************************************/
UInt32 CSldListInfo::GetVariantLangCode(UInt32 aIndex) const
{
	if (aIndex >= m_VariantPropertyTable.size())
		return 0;

	return m_VariantPropertyTable[aIndex].LangCode;
}

/** *********************************************************************
* Возвращает код использования данного списка слов, т.е. для каких действий
* он предназначен. Если код использования принадлежит одному из диапазонов
* списков полнотекстового поиска, то фунция вернет начало диапазона
*
* @return тип списка (возвращает значения из #EWordListTypeEnum)
************************************************************************/
EWordListTypeEnum CSldListInfo::GetUsage() const
{
	UInt32 usage = m_Header.WordListUsage;

	if (usage >= eWordListType_FullTextSearchBase && usage < eWordListType_FullTextSearchHeadword)
		usage = eWordListType_FullTextSearchBase;
	else if (usage >= eWordListType_FullTextSearchHeadword && usage < eWordListType_FullTextSearchContent)
		usage = eWordListType_FullTextSearchHeadword;
	else if (usage >= eWordListType_FullTextSearchContent && usage < eWordListType_FullTextSearchTranslation)
		usage = eWordListType_FullTextSearchContent;
	else if (usage >= eWordListType_FullTextSearchTranslation && usage < eWordListType_FullTextSearchExample)
		usage = eWordListType_FullTextSearchTranslation;
	else if (usage >= eWordListType_FullTextSearchExample && usage < eWordListType_FullTextSearchDefinition)
		usage = eWordListType_FullTextSearchExample;
	else if (usage >= eWordListType_FullTextSearchDefinition && usage < eWordListType_FullTextSearchPhrase)
		usage = eWordListType_FullTextSearchDefinition;
	else if (usage >= eWordListType_FullTextSearchPhrase && usage < eWordListType_FullTextSearchIdiom)
		usage = eWordListType_FullTextSearchPhrase;
	else if (usage >= eWordListType_FullTextSearchIdiom && usage < eWordListType_FullTextSearchLast)
		usage = eWordListType_FullTextSearchIdiom;

	return static_cast<EWordListTypeEnum>(usage);
}

/** *********************************************************************
* Добавляет вариант написания
*
* @param[in]	aVariantProperty	- указатель на добавляемый вариант
* @param[in]	aUnique				- флаг проверки уникальности нового варианта написания
*									  при aUnique=true и наличии добавляемого варианта написания
*									  в listInfo новый вариант не будет добавлен
*									  дефолтное значение - false
*
* @return код ошибки
************************************************************************/
ESldError CSldListInfo::AddVariantProperty(const TListVariantProperty & aVariantProperty, bool aUnique)
{
	auto propType = static_cast<EListVariantTypeEnum>(aVariantProperty.Type);
	if (aUnique && GetVariantIndexByType(propType) != SLD_DEFAULT_VARIANT_INDEX)
		return eOK;

	m_VariantPropertyTable.emplace_back(aVariantProperty);
	m_VariantPropertyTable.back().Number = m_Header.NumberOfVariants++;

	return eOK;
}

/** *********************************************************************
* Получает индекс варианта написания по его типу
*
* @param[in] aType	- тип варианта написания
*
* @return индекс варианта написания,
*		  если вариант не найден возвращает SLD_DEFAULT_VARIANT_INDEX
************************************************************************/
Int32 CSldListInfo::GetVariantIndexByType(const EListVariantTypeEnum aType) const
{
	Int32 variantIndex = 0;
	for (auto && variant : m_VariantPropertyTable)
	{
		if (variant.Type == aType)
			return variantIndex;
		variantIndex++;
	}

	return SLD_DEFAULT_VARIANT_INDEX;
}
