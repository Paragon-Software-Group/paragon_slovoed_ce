#include "SldListLocalizedString.h"


/**
* Конструктор
*
* @param[in]  aData             - ссылка на объект, отвечающий за получение данных из контейнера
* @param[in]  aResourceIndex    - индекс ресурса с необходимыми данными
* @param[in]  aNumberOfStrings  - количество блоков со строками в ресурсе
* @param[out] aError            - статус инициализации объекта
*
* Если aError != eOK объект находится в "неопределенном" состоянии с единственным
* возможноым (определенным) действием - деструкцией
*/
CSldListLocalizedString::CSldListLocalizedString(CSDCReadMy &aData, UInt32 aResourceIndex, UInt32 aNumberOfStrings, ESldError &aError)
	: m_DefaultLanguageIndex(-1)
{
#define error_out(_err) do { aError = _err; return; } while(0)

	if (!aNumberOfStrings)
		error_out(eCommonWrongNumberOfLanguages);

	// Загружаем данные из ресурса
	auto res = aData.GetResource(SLD_RESOURCE_LIST_STRINGS, aResourceIndex);
	if (res != eOK)
		error_out(res.error());

	// Простая проверка на корректность данных
	if (res.size() != aNumberOfStrings * sizeof(m_Strings[0]))
		error_out(eInputWrongStructSize);

	// Копируем данные
	if (!m_Strings.resize(aNumberOfStrings))
		error_out(eMemoryNotEnoughMemory);
	sldMemCopy(m_Strings.data(), res.ptr(), res.size());

	// Найдем номер набора строк для языка по умолчанию
	for (UInt32 i = 0; i < m_Strings.size(); i++)
	{
		if (m_Strings[i].LanguageCode == SldLanguage::Default)
		{
			m_DefaultLanguageIndex = i;
		}
	}

	aError = eOK;
#undef error_out
}

/** *********************************************************************
* Возвращает количество языков, для которых имеется перевод
*
* @return количество языков
************************************************************************/
UInt32 CSldListLocalizedString::GetNumberOfLanguages() const
{
	return m_Strings.size();
}

/** *********************************************************************
* Возвращает код языка по индексу
*
* @param[in]	aIndex				- индекс запрашиваемого кода языка
* @param[out]	aNumberOfLanguages	- указатель на переменную, в которую сохраняется код языка
*
* @return код ошибки
************************************************************************/
ESldError CSldListLocalizedString::GetLanguagesCode(UInt32 aIndex, UInt32* aLanguageCode) const
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
Int32 CSldListLocalizedString::FindLanguage(UInt32 aLanguageCode) const
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
* Возвращает полное название списка слов
*
* @param[in]	aLanguageCode	- код языка, для которого требуется найти строчку
* @param[out]	aText			- указатель, по которому будет записан указатель на строчку
*
* @return код ошибки
************************************************************************/
ESldError CSldListLocalizedString::GetListName(UInt32 aLanguageCode, const UInt16** aText) const
{
	if (!aText)
		return eMemoryNullPointer;

	Int32 i = FindLanguage(aLanguageCode);

	return GetListNameByIndex(i, aText);
}

/** *********************************************************************
* Возвращает cокращенное название списка слов
*
* @param[in]	aLanguageCode	- код языка, для которого требуется найти строчку
* @param[out]	aText			- указатель, по которому будет записан указатель на строчку
*
* @return код ошибки
************************************************************************/
ESldError CSldListLocalizedString::GetListNameShort(UInt32 aLanguageCode, const UInt16** aText) const
{
	if (!aText)
		return eMemoryNullPointer;

	Int32 i = FindLanguage(aLanguageCode);

	return GetListNameShortByIndex(i, aText);
}

/** *********************************************************************
* Возвращает название класса списка слов
*
* @param[in]	aLanguageCode	- код языка, для которого требуется найти строчку
* @param[out]	aText			- указатель, по которому будет записан указатель на строчку
*
* @return код ошибки
************************************************************************/
ESldError CSldListLocalizedString::GetListClass(UInt32 aLanguageCode, const UInt16** aText) const
{
	if (!aText)
		return eMemoryNullPointer;

	Int32 i = FindLanguage(aLanguageCode);

	return GetListClassByIndex(i, aText);
}

/** *********************************************************************
* Возвращает название прямой языковой пары
*
* @param[in]	aLanguageCode	- код языка, для которого требуется найти строчку
* @param[out]	aText			- указатель, по которому будет записан указатель на строчку
*
* @return код ошибки
************************************************************************/
ESldError CSldListLocalizedString::GetListLanguagePair(UInt32 aLanguageCode, const UInt16** aText) const
{
	if (!aText)
		return eMemoryNullPointer;

	Int32 i = FindLanguage(aLanguageCode);

	return GetListLanguagePairByIndex(i, aText);
}

/** *********************************************************************
* Возвращает сокращенное название прямой языковой пары
*
* @param[in]	aLanguageCode	- код языка, для которого требуется найти строчку
* @param[out]	aText			- указатель, по которому будет записан указатель на строчку
*
* @return код ошибки
************************************************************************/
ESldError CSldListLocalizedString::GetListLanguagePairShort(UInt32 aLanguageCode, const UInt16** aText) const
{
	if (!aText)
		return eMemoryNullPointer;

	Int32 i = FindLanguage(aLanguageCode);

	return GetListLanguagePairShortByIndex(i, aText);
}

/** *********************************************************************
* Возвращает название языка, с которого производится перевод
*
* @param[in]	aLanguageCode	- код языка, для которого требуется найти строчку
* @param[out]	aText			- указатель, по которому будет записан указатель на строчку
*
* @return код ошибки
************************************************************************/
ESldError CSldListLocalizedString::GetLanguageNameFrom(UInt32 aLanguageCode, const UInt16** aText) const
{
	if (!aText)
		return eMemoryNullPointer;

	Int32 i = FindLanguage(aLanguageCode);

	return GetLanguageNameFromByIndex(i, aText);
}

/** *********************************************************************
* Возвращает название языка, на который производится перевод
*
* @param[in]	aLanguageCode	- код языка, для которого требуется найти строчку
* @param[out]	aText			- указатель, по которому будет записан указатель на строчку
*
* @return код ошибки
************************************************************************/
ESldError CSldListLocalizedString::GetLanguageNameTo(UInt32 aLanguageCode, const UInt16** aText) const
{
	if (!aText)
		return eMemoryNullPointer;

	Int32 i = FindLanguage(aLanguageCode);

	return GetLanguageNameToByIndex(i, aText);
}

/** *********************************************************************
* Возвращает полное название списка слов
*
* @param[in]	aIndex	- номер языковых данных (не код языка)
* @param[out]	aText	- указатель, по которому будет записан указатель на строчку
*
* @return код ошибки
************************************************************************/
ESldError CSldListLocalizedString::GetListNameByIndex(UInt32 aIndex, const UInt16** aText) const
{
	if (aIndex >= m_Strings.size())
		return eCommonWrongIndex;
	if (!aText)
		return eMemoryNullPointer;

	*aText = m_Strings[aIndex].ListName;

	return eOK;
}

/** *********************************************************************
* Возвращает cокращенное название списка слов
*
* @param[in]	aIndex	- номер языковых данных (не код языка)
* @param[out]	aText	- указатель, по которому будет записан указатель на строчку
*
* @return код ошибки
************************************************************************/
ESldError CSldListLocalizedString::GetListNameShortByIndex(UInt32 aIndex, const UInt16** aText) const
{
	if (aIndex >= m_Strings.size())
		return eCommonWrongIndex;
	if (!aText)
		return eMemoryNullPointer;

	*aText = m_Strings[aIndex].ListNameShort;

	return eOK;
}

/** *********************************************************************
* Возвращает название класса списка слов
*
* @param[in]	aIndex	- номер языковых данных (не код языка)
* @param[out]	aText	- указатель, по которому будет записан указатель на строчку
*
* @return код ошибки
************************************************************************/
ESldError CSldListLocalizedString::GetListClassByIndex(UInt32 aIndex, const UInt16** aText) const
{
	if (aIndex >= m_Strings.size())
		return eCommonWrongIndex;
	if (!aText)
		return eMemoryNullPointer;

	*aText = m_Strings[aIndex].ListClass;

	return eOK;
}

/** *********************************************************************
* Возвращает название прямой языковой пары
*
* @param[in]	aIndex	- номер языковых данных (не код языка)
* @param[out]	aText	- указатель, по которому будет записан указатель на строчку
*
* @return код ошибки
************************************************************************/
ESldError CSldListLocalizedString::GetListLanguagePairByIndex(UInt32 aIndex, const UInt16** aText) const
{
	if (aIndex >= m_Strings.size())
		return eCommonWrongIndex;
	if (!aText)
		return eMemoryNullPointer;

	*aText = m_Strings[aIndex].ListLanguagePair;

	return eOK;
}

/***********************************************************************
* Возвращает сокращенное название прямой языковой пары
*
* @param[in]	aIndex	- номер языковых данных (не код языка)
* @param[out]	aText	- указатель, по которому будет записан указатель на строчку
*
* @return код ошибки
************************************************************************/
ESldError CSldListLocalizedString::GetListLanguagePairShortByIndex(UInt32 aIndex, const UInt16** aText) const
{
	if (aIndex >= m_Strings.size())
		return eCommonWrongIndex;
	if (!aText)
		return eMemoryNullPointer;

	*aText = m_Strings[aIndex].ListLanguagePairShort;

	return eOK;
}

/***********************************************************************
* Возвращает название языка, с которого производится перевод
*
* @param[in]	aIndex	- номер языковых данных (не код языка)
* @param[out]	aText	- указатель, по которому будет записан указатель на строчку
*
* @return код ошибки
************************************************************************/
ESldError CSldListLocalizedString::GetLanguageNameFromByIndex(UInt32 aIndex, const UInt16** aText) const
{
	if (aIndex >= m_Strings.size())
		return eCommonWrongIndex;
	if (!aText)
		return eMemoryNullPointer;

	*aText = m_Strings[aIndex].ListLanguageNameFrom;

	return eOK;
}

/** *********************************************************************
* Возвращает название языка, на который производится перевод
*
* @param[in]	aIndex	- номер языковых данных (не код языка)
* @param[out]	aText	- указатель, по которому будет записан указатель на строчку
*
* @return код ошибки
************************************************************************/
ESldError CSldListLocalizedString::GetLanguageNameToByIndex(UInt32 aIndex, const UInt16** aText) const
{
	if (aIndex >= m_Strings.size())
		return eCommonWrongIndex;
	if (!aText)
		return eMemoryNullPointer;

	*aText = m_Strings[aIndex].ListLanguageNameTo;

	return eOK;
}
