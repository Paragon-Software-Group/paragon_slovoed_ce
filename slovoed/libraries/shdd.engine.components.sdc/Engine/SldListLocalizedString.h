#ifndef _SLD_LIST_LOCALIZED_STRINGS_H_
#define _SLD_LIST_LOCALIZED_STRINGS_H_

#include "SldError.h"
#include "SldSDCReadMy.h"


///  Класс, содержащий локализованные строки списка слов
class CSldListLocalizedString
{
public:

	/// Конструктор
	CSldListLocalizedString(CSDCReadMy &aData, UInt32 aResourceIndex, UInt32 aNumberOfStrings, ESldError &aError);

public:

	/// Возвращает количество языков, для которых имеется перевод
	UInt32 GetNumberOfLanguages() const;

	/// Возвращает код языка по индексу
	ESldError GetLanguagesCode(UInt32 aIndex, UInt32* aLanguageCode) const;

	/// Возвращает полное название списка слов
	ESldError GetListName(UInt32 aLanguageCode, const UInt16** aText) const;

	/// Возвращает cокращенное название списка слов
	ESldError GetListNameShort(UInt32 aLanguageCode, const UInt16** aText) const;

	/// Возвращает название класса списка слов
	ESldError GetListClass(UInt32 aLanguageCode, const UInt16** aText) const;

	/// Возвращает название прямой языковой пары
	ESldError GetListLanguagePair(UInt32 aLanguageCode, const UInt16** aText) const;
	
	/// Возвращает сокращенное название прямой языковой пары
	ESldError GetListLanguagePairShort(UInt32 aLanguageCode, const UInt16** aText) const;
	
	/// Возвращает название языка, с которого производится перевод
	ESldError GetLanguageNameFrom(UInt32 aLanguageCode, const UInt16** aText) const;
	
	/// Возвращает название языка, на который производится перевод
	ESldError GetLanguageNameTo(UInt32 aLanguageCode, const UInt16** aText) const;	

	
	/// Возвращает полное название списка слов
	ESldError GetListNameByIndex(UInt32 aIndex, const UInt16** aText) const;

	/// Возвращает cокращенное название списка слов
	ESldError GetListNameShortByIndex(UInt32 aIndex, const UInt16** aText) const;

	/// Возвращает название класса списка слов
	ESldError GetListClassByIndex(UInt32 aIndex, const UInt16** aText) const;
	
	/// Возвращает название прямой языковой пары
	ESldError GetListLanguagePairByIndex(UInt32 aIndex, const UInt16** aText) const;
	
	/// Возвращает сокращенное название прямой языковой пары
	ESldError GetListLanguagePairShortByIndex(UInt32 aIndex, const UInt16** aText) const;

	/// Возвращает название языка, с которого производится перевод
	ESldError GetLanguageNameFromByIndex(UInt32 aIndex, const UInt16** aText) const;

	/// Возвращает название языка, на который производится перевод
	ESldError GetLanguageNameToByIndex(UInt32 aIndex, const UInt16** aText) const;

private:

	/// Производит поиск номера набора локализованных строк на указанном языке
	Int32 FindLanguage(UInt32 aLanguageCode) const;

private:

	/// Массив локализованных строк
	sld2::DynArray<TListLocalizedNames> m_Strings;

	/// Номер набора строк для языка по умолчанию
	Int32 m_DefaultLanguageIndex;
};

#endif //_SLD_LIST_LOCALIZED_STRINGS_H_
