#include "SldLocalizedString.h"


/**
 * Конструктор
 *
 * @param[in]  aData            - ссылка на объект отвечающий за получение данных из контейнера
 * @param[in]  aNumberOfStrings - количество строк в словаре
 * @param[out] aError           - статус инициализации объекта
 *
 * Если aError != eOK объект находится в "неопределенном" состоянии с единственным
 * возможноым (определенным) действием - деструкцией
 */
CSldLocalizedString::CSldLocalizedString(CSDCReadMy &aData, UInt32 aNumberOfStrings, ESldError &aError)
	: m_DefaultLanguageIndex(-1)
{
#define error_out(_err) do { aError = _err; return; } while(0)

	if (!aNumberOfStrings)
		error_out(eCommonWrongNumberOfLanguages);

	auto strings = decltype(m_Strings)(aNumberOfStrings);
	if (aNumberOfStrings && strings.empty())
		error_out(eMemoryNotEnoughMemory);

	for (UInt32 i = 0; i < strings.size(); i++)
	{
		auto res = aData.GetResource(SLD_RESOURCE_STRINGS, i);
		if (res != eOK)
			error_out(res.error());

		sldMemCopy(&strings[i], res.ptr(), (sld2::min)(res.size(), (UInt32)sizeof(strings[0])));

		if (strings[i].LanguageCode == SldLanguage::Default)
		{
			m_DefaultLanguageIndex = i;
		}
	}

	m_Strings = sld2::move(strings);
	aError = eOK;
#undef error_out
}

/** *********************************************************************
* Возвращает количество языков для которых имеется перевод.
*
* @return количество языков
************************************************************************/
UInt32 CSldLocalizedString::GetNumberOfLanguages() const
{
	return m_Strings.size();
}

/** *********************************************************************
* Возвращает код языка по индексу
*
* @param[in]	aIndex				- индекс языка, для которого нужно получить код
* @param[out]	aNumberOfLanguages	- указатель на переменную, в которую нужно
*									  будет записать код языка
*
* @return код ошибки
************************************************************************/
ESldError CSldLocalizedString::GetLanguagesCode(UInt32 aIndex, UInt32 *aLanguageCode) const
{
	if (aIndex >= m_Strings.size())
		return eCommonWrongIndex;
	if (!aLanguageCode)
		return eMemoryNullPointer;

	*aLanguageCode = m_Strings[aIndex].LanguageCode;
	return eOK;
}

/** *********************************************************************
* Производит поиск номера набора локализованных строк на указанном языке
*
* Если для указанного языка строки не найдены, возвращается номер набора строк
* для языка по умолчанию
*
* @param[in]	aLanguageCode	- код языка, для которого нужно найти строки
*
* @return номер языка
************************************************************************/
Int32 CSldLocalizedString::FindLanguage(UInt32 aLanguageCode) const
{
	for (UInt32 i = 0; i < m_Strings.size(); i++)
	{
		if (m_Strings[i].LanguageCode == aLanguageCode)
		{
			return i;
		}
	}

	return m_DefaultLanguageIndex;
}

/** *********************************************************************
* Возвращает название продукта
*
* @param[in]	aLanguageCode	- код языка для которого требуется найти строчку
* @param[out]	aText			- указатель, по которому будет записан указатель на строчку.
*
* @return код ошибки
************************************************************************/
ESldError CSldLocalizedString::GetProductName(UInt32 aLanguageCode, const UInt16 **aText) const
{
	if (!aText)
		return eMemoryNullPointer;

	Int32 i = FindLanguage(aLanguageCode);

	return GetProductNameByIndex(i, aText);
}

/** *********************************************************************
* Возвращает название словаря
*
* @param[in]	aLanguageCode	- код языка для которого требуется найти строчку
* @param[out]	aText			- указатель, по которому будет записан указатель на строчку.
*
* @return код ошибки
************************************************************************/
ESldError CSldLocalizedString::GetDictionaryName(UInt32 aLanguageCode, const UInt16 **aText) const
{
	if (!aText)
		return eMemoryNullPointer;

	Int32 i = FindLanguage(aLanguageCode);

	return GetDictionaryNameByIndex(i, aText);
}

/** *********************************************************************
* Возвращает сокращенное название словаря
*
* @param[in]	aLanguageCode	- код языка для которого требуется найти строчку
* @param[out]	aText			- указатель, по которому будет записан указатель на строчку.
*
* @return код ошибки
************************************************************************/
ESldError CSldLocalizedString::GetDictionaryNameShort(UInt32 aLanguageCode, const UInt16 **aText) const
{
	if (!aText)
		return eMemoryNullPointer;

	Int32 i = FindLanguage(aLanguageCode);

	return GetDictionaryNameShortByIndex(i, aText);
}

/** *********************************************************************
* Возвращает тип словаря(Compact, Classic, Deluxe)
*
* @param[in]	aLanguageCode	- код языка для которого требуется найти строчку
* @param[out]	aText			- указатель, по которому будет записан указатель на строчку.
*
* @return код ошибки
************************************************************************/
ESldError CSldLocalizedString::GetDictionaryClass(UInt32 aLanguageCode, const UInt16 **aText) const
{
	if (!aText)
		return eMemoryNullPointer;

	Int32 i = FindLanguage(aLanguageCode);

	return GetDictionaryClassByIndex(i, aText);
}

/** *********************************************************************
* Возвращает название прямой языковой пары
*
* @param[in]	aLanguageCode	- код языка для которого требуется найти строчку
* @param[out]	aText			- указатель, по которому будет записан указатель на строчку.
*
* @return код ошибки
************************************************************************/
ESldError CSldLocalizedString::GetDictionaryLanguagePair(UInt32 aLanguageCode, const UInt16 **aText) const
{
	if (!aText)
		return eMemoryNullPointer;

	Int32 i = FindLanguage(aLanguageCode);

	return GetDictionaryLanguagePairByIndex(i, aText);
}

/** *********************************************************************
* Возвращает название обратной языковой пары
*
* @param[in]	aLanguageCode	- код языка для которого требуется найти строчку
* @param[out]	aText			- указатель, по которому будет записан указатель на строчку.
*
* @return код ошибки
************************************************************************/
ESldError CSldLocalizedString::GetDictionaryLanguagePairReverse(UInt32 aLanguageCode, const UInt16 **aText) const
{
	if (!aText)
		return eMemoryNullPointer;

	Int32 i = FindLanguage(aLanguageCode);

	return GetDictionaryLanguagePairReverseByIndex(i, aText);
}

/** *********************************************************************
* Возвращает короткое название прямой языковой пары
*
* @param[in]	aLanguageCode	- код языка для которого требуется найти строчку
* @param[out]	aText			- указатель, по которому будет записан указатель на строчку.
*
* @return код ошибки
************************************************************************/
ESldError CSldLocalizedString::GetDictionaryLanguagePairShort(UInt32 aLanguageCode, const UInt16 **aText) const
{
	if (!aText)
		return eMemoryNullPointer;

	Int32 i = FindLanguage(aLanguageCode);

	return GetDictionaryLanguagePairShortByIndex(i, aText);
}

/** *********************************************************************
* Возвращает короткое название обратной языковой пары
*
* @param[in]	aLanguageCode	- код языка для которого требуется найти строчку
* @param[out]	aText			- указатель, по которому будет записан указатель на строчку.
*
* @return код ошибки
************************************************************************/
ESldError CSldLocalizedString::GetDictionaryLanguagePairShortReverse(UInt32 aLanguageCode, const UInt16 **aText) const
{
	if (!aText)
		return eMemoryNullPointer;

	Int32 i = FindLanguage(aLanguageCode);

	return GetDictionaryLanguagePairShortReverseByIndex(i, aText);
}

/** *********************************************************************
* Возвращает название языка с которого производится перевод
*
* @param[in]	aLanguageCode	- код языка для которого требуется найти строчку
* @param[out]	aText			- указатель, по которому будет записан указатель на строчку.
*
* @return код ошибки
************************************************************************/
ESldError CSldLocalizedString::GetLanguageNameFrom(UInt32 aLanguageCode, const UInt16 **aText) const
{
	if (!aText)
		return eMemoryNullPointer;

	Int32 i = FindLanguage(aLanguageCode);

	return GetLanguageNameFromByIndex(i, aText);
}

/** *********************************************************************
* Возвращает название языка на который производится перевод
*
* @param[in]	aLanguageCode	- код языка для которого требуется найти строчку
* @param[out]	aText			- указатель, по которому будет записан указатель на строчку.
*
* @return код ошибки
************************************************************************/
ESldError CSldLocalizedString::GetLanguageNameTo(UInt32 aLanguageCode, const UInt16 **aText) const
{
	if (!aText)
		return eMemoryNullPointer;

	Int32 i = FindLanguage(aLanguageCode);

	return GetLanguageNameToByIndex(i, aText);
}

/** *********************************************************************
* Возвращает название автора словаря
*
* @param[in]	aLanguageCode	- код языка для которого требуется найти строчку
* @param[out]	aText			- указатель, по которому будет записан указатель на строчку.
*
* @return код ошибки
************************************************************************/
ESldError CSldLocalizedString::GetAuthorName(UInt32 aLanguageCode, const UInt16 **aText) const
{
	if (!aText)
		return eMemoryNullPointer;

	Int32 i = FindLanguage(aLanguageCode);

	return GetAuthorNameByIndex(i, aText);
}

/** *********************************************************************
* Возвращает адрес сайта автора словаря
*
* @param[in]	aLanguageCode	- код языка для которого требуется найти строчку
* @param[out]	aText			- указатель, по которому будет записан указатель на строчку.
*
* @return код ошибки
************************************************************************/
ESldError CSldLocalizedString::GetAuthorWeb(UInt32 aLanguageCode, const UInt16 **aText) const
{
	if (!aText)
		return eMemoryNullPointer;

	Int32 i = FindLanguage(aLanguageCode);

	return GetAuthorWebByIndex(i, aText);
}

/** *********************************************************************
* Возвращает название продукта
*
* @param[in]	aIndex	- номер языковых данных (не код языка)
* @param[out]	aText	- указатель, по которому будет записан указатель на строчку.
*
* @return код ошибки
************************************************************************/
ESldError CSldLocalizedString::GetProductNameByIndex(UInt32 aIndex, const UInt16 **aText) const
{
	if (aIndex >= m_Strings.size())
		return eCommonWrongIndex;
	if (!aText)
		return eMemoryNullPointer;

	*aText = m_Strings[aIndex].ProductName;

	return eOK;
}

/** *********************************************************************
* Возвращает название словаря
*
* @param[in]	aIndex	- номер языковых данных (не код языка)
* @param[out]	aText	- указатель, по которому будет записан указатель на строчку.
*
* @return код ошибки
************************************************************************/
ESldError CSldLocalizedString::GetDictionaryNameByIndex(UInt32 aIndex, const UInt16 **aText) const
{
	if (aIndex >= m_Strings.size())
		return eCommonWrongIndex;
	if (!aText)
		return eMemoryNullPointer;

	*aText = m_Strings[aIndex].DictionaryName;

	return eOK;
}

/** *********************************************************************
* Возвращает сокращенное название словаря
*
* @param[in]	aIndex	- номер языковых данных (не код языка)
* @param[out]	aText	- указатель, по которому будет записан указатель на строчку.
*
* @return код ошибки
************************************************************************/
ESldError CSldLocalizedString::GetDictionaryNameShortByIndex(UInt32 aIndex, const UInt16 **aText) const
{
	if (aIndex >= m_Strings.size())
		return eCommonWrongIndex;
	if (!aText)
		return eMemoryNullPointer;

	*aText = m_Strings[aIndex].DictionaryNameShort;

	return eOK;
}

/** *********************************************************************
* Возвращает тип словаря(Compact, Classic, Deluxe)
*
* @param[in]	aIndex	- номер языковых данных (не код языка)
* @param[out]	aText	- указатель, по которому будет записан указатель на строчку.
*
* @return код ошибки
************************************************************************/
ESldError CSldLocalizedString::GetDictionaryClassByIndex(UInt32 aIndex, const UInt16 **aText) const
{
	if (aIndex >= m_Strings.size())
		return eCommonWrongIndex;
	if (!aText)
		return eMemoryNullPointer;

	*aText = m_Strings[aIndex].DictionaryClass;

	return eOK;
}

/** *********************************************************************
* Возвращает название прямой языковой пары
*
* @param[in]	aIndex	- номер языковых данных (не код языка)
* @param[out]	aText	- указатель, по которому будет записан указатель на строчку.
*
* @return код ошибки
************************************************************************/
ESldError CSldLocalizedString::GetDictionaryLanguagePairByIndex(UInt32 aIndex, const UInt16 **aText) const
{
	if (aIndex >= m_Strings.size())
		return eCommonWrongIndex;
	if (!aText)
		return eMemoryNullPointer;

	*aText = m_Strings[aIndex].DictionaryLanguagePair;

	return eOK;
}

/** *********************************************************************
* Возвращает название обратной языковой пары
*
* @param[in]	aIndex	- номер языковых данных (не код языка)
* @param[out]	aText	- указатель, по которому будет записан указатель на строчку.
*
* @return код ошибки
************************************************************************/
ESldError CSldLocalizedString::GetDictionaryLanguagePairReverseByIndex(UInt32 aIndex, const UInt16 **aText) const
{
	if (aIndex >= m_Strings.size())
		return eCommonWrongIndex;
	if (!aText)
		return eMemoryNullPointer;

	*aText = m_Strings[aIndex].DictionaryLanguagePairReverse;

	return eOK;
}

/** *********************************************************************
* Возвращает короткое название прямой языковой пары
*
* @param[in]	aIndex	- номер языковых данных (не код языка)
* @param[out]	aText	- указатель, по которому будет записан указатель на строчку.
*
* @return код ошибки
************************************************************************/
ESldError CSldLocalizedString::GetDictionaryLanguagePairShortByIndex(UInt32 aIndex, const UInt16 **aText) const
{
	if (aIndex >= m_Strings.size())
		return eCommonWrongIndex;
	if (!aText)
		return eMemoryNullPointer;

	*aText = m_Strings[aIndex].DictionaryLanguagePairShort;

	return eOK;
}

/** *********************************************************************
* Возвращает короткое название обратной языковой пары
*
* @param[in]	aIndex	- номер языковых данных (не код языка)
* @param[out]	aText	- указатель, по которому будет записан указатель на строчку.
*
* @return код ошибки
************************************************************************/
ESldError CSldLocalizedString::GetDictionaryLanguagePairShortReverseByIndex(UInt32 aIndex, const UInt16 **aText) const
{
	if (aIndex >= m_Strings.size())
		return eCommonWrongIndex;
	if (!aText)
		return eMemoryNullPointer;

	*aText = m_Strings[aIndex].DictionaryLanguagePairShortReverse;

	return eOK;
}

/** *********************************************************************
* Возвращает название языка с которого производится перевод
*
* @param[in]	aIndex	- номер языковых данных (не код языка)
* @param[out]	aText	- указатель, по которому будет записан указатель на строчку.
*
* @return код ошибки
************************************************************************/
ESldError CSldLocalizedString::GetLanguageNameFromByIndex(UInt32 aIndex, const UInt16 **aText) const
{
	if (aIndex >= m_Strings.size())
		return eCommonWrongIndex;
	if (!aText)
		return eMemoryNullPointer;

	*aText = m_Strings[aIndex].LanguageNameFrom;

	return eOK;
}

/** *********************************************************************
* Возвращает название языка на который производится перевод
*
* @param[in]	aIndex	- номер языковых данных (не код языка)
* @param[out]	aText	- указатель, по которому будет записан указатель на строчку.
*
* @return код ошибки
************************************************************************/
ESldError CSldLocalizedString::GetLanguageNameToByIndex(UInt32 aIndex, const UInt16 **aText) const
{
	if (aIndex >= m_Strings.size())
		return eCommonWrongIndex;
	if (!aText)
		return eMemoryNullPointer;

	*aText = m_Strings[aIndex].LanguageNameTo;

	return eOK;
}

/** *********************************************************************
* Возвращает название автора словаря
*
* @param[in]	aIndex	- номер языковых данных (не код языка)
* @param[out]	aText	- указатель, по которому будет записан указатель на строчку.
*
* @return код ошибки
************************************************************************/
ESldError CSldLocalizedString::GetAuthorNameByIndex(UInt32 aIndex, const UInt16 **aText) const
{
	if (aIndex >= m_Strings.size())
		return eCommonWrongIndex;
	if (!aText)
		return eMemoryNullPointer;

	*aText = m_Strings[aIndex].AuthorName;

	return eOK;
}

/** *********************************************************************
* Возвращает адрес сайта автора словаря
*
* @param[in]	aIndex	- номер языковых данных (не код языка)
* @param[out]	aText	- указатель, по которому будет записан указатель на строчку.
*
* @return код ошибки
************************************************************************/
ESldError CSldLocalizedString::GetAuthorWebByIndex(UInt32 aIndex, const UInt16 **aText) const
{
	if (aIndex >= m_Strings.size())
		return eCommonWrongIndex;
	if (!aText)
		return eMemoryNullPointer;

	*aText = m_Strings[aIndex].AuthorWeb;

	return eOK;
}
