#ifndef _SLD_LOCALIZED_STRINGS_H_
#define _SLD_LOCALIZED_STRINGS_H_

#include "SldError.h"
#include "SldSDCReadMy.h"


/// Класс, содержащий локализованные строки словаря
class CSldLocalizedString
{
public:

	/// Конструктор
	CSldLocalizedString(CSDCReadMy &aData, UInt32 aNumberOfStrings, ESldError &aError);

public:

	/// Возвращает количество языков для которых имеется перевод.
	UInt32 GetNumberOfLanguages() const;

	/// Возвращает код языка по индексу
	ESldError GetLanguagesCode(UInt32 aIndex, UInt32 *aLanguageCode) const;

	/// Возвращает название продукта
	ESldError GetProductName(UInt32 aLanguageCode, const UInt16 **aText) const;

	/// Возвращает название словаря
	ESldError GetDictionaryName(UInt32 aLanguageCode, const UInt16 **aText) const;

	/// Возвращает сокращенное название словаря
	ESldError GetDictionaryNameShort(UInt32 aLanguageCode, const UInt16 **aText) const;

	/// Возвращает тип словаря(Compact, Classic, Deluxe)
	ESldError GetDictionaryClass(UInt32 aLanguageCode, const UInt16 **aText) const;

	/// Возвращает название прямой языковой пары
	ESldError GetDictionaryLanguagePair(UInt32 aLanguageCode, const UInt16 **aText) const;
	
	/// Возвращает название обратной языковой пары
	ESldError GetDictionaryLanguagePairReverse(UInt32 aLanguageCode, const UInt16 **aText) const;

	/// Возвращает короткое название прямой языковой пары
	ESldError GetDictionaryLanguagePairShort(UInt32 aLanguageCode, const UInt16 **aText) const;
	
	/// Возвращает короткое название обратной языковой пары
	ESldError GetDictionaryLanguagePairShortReverse(UInt32 aLanguageCode, const UInt16 **aText) const;

	/// Возвращает название языка с которого производится перевод
	ESldError GetLanguageNameFrom(UInt32 aLanguageCode, const UInt16 **aText) const;

	/// Возвращает название языка на который производится перевод
	ESldError GetLanguageNameTo(UInt32 aLanguageCode, const UInt16 **aText) const;

	/// Возвращает название автора словаря
	ESldError GetAuthorName(UInt32 aLanguageCode, const UInt16 **aText) const;

	/// Возвращает адрес сайта автора словаря
	ESldError GetAuthorWeb(UInt32 aLanguageCode, const UInt16 **aText) const;

	/// Возвращает название продукта
	ESldError GetProductNameByIndex(UInt32 aIndex, const UInt16 **aText) const;

	/// Возвращает название словаря
	ESldError GetDictionaryNameByIndex(UInt32 aIndex, const UInt16 **aText) const;

	/// Возвращает сокращенное название словаря
	ESldError GetDictionaryNameShortByIndex(UInt32 aIndex, const UInt16 **aText) const;

	/// Возвращает тип словаря(Compact, Classic, Deluxe)
	ESldError GetDictionaryClassByIndex(UInt32 aIndex, const UInt16 **aText) const;
	
	/// Возвращает название прямой языковой пары
	ESldError GetDictionaryLanguagePairByIndex(UInt32 aIndex, const UInt16 **aText) const;
	
	/// Возвращает название обратной языковой пары
	ESldError GetDictionaryLanguagePairReverseByIndex(UInt32 aIndex, const UInt16 **aText) const;

	/// Возвращает короткое название прямой языковой пары
	ESldError GetDictionaryLanguagePairShortByIndex(UInt32 aIndex, const UInt16 **aText) const;

	/// Возвращает короткое название обратной языковой пары
	ESldError GetDictionaryLanguagePairShortReverseByIndex(UInt32 aIndex, const UInt16 **aText) const;

	/// Возвращает название языка с которого производится перевод
	ESldError GetLanguageNameFromByIndex(UInt32 aIndex, const UInt16 **aText) const;

	/// Возвращает название языка на который производится перевод
	ESldError GetLanguageNameToByIndex(UInt32 aIndex, const UInt16 **aText) const;

	/// Возвращает название автора словаря
	ESldError GetAuthorNameByIndex(UInt32 aIndex, const UInt16 **aText) const;

	/// Возвращает адрес сайта автора словаря
	ESldError GetAuthorWebByIndex(UInt32 aIndex, const UInt16 **aText) const;

private:

	/// Производит поиск номера набора локализованных строк на указанном языке
	Int32 FindLanguage(UInt32 aLanguageCode) const;

private:

	/// Массив локализованных строчек
	sld2::DynArray<TLocalizedNames> m_Strings;

	/// Номер набора строк для языка по умолчанию
	Int32 m_DefaultLanguageIndex;
};

#endif
