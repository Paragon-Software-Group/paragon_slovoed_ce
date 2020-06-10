#ifndef _SLD_LIST_INFO_H_
#define _SLD_LIST_INFO_H_

#include "SldPlatform.h"
#include "SldTypes.h"
#include "SldError.h"
#include "SldListLocalizedString.h"


/// Класс, хранящий информацию о списке слов
class CSldListInfo
{
	friend class CSldList;
	friend class CSldSearchList;
	friend class CSldMergedList;
	
public:
	
	/// Конструктор
	CSldListInfo()
		:m_ShowVariantIndex(0)
	{
		sld2::memzero(m_Header);
	}

	 /// Деструктор
	~CSldListInfo() {}

	/// Производит инициализацию класса.
	ESldError Init(CSDCReadMy &aData, const TListHeader* aHeader);
	ESldError Init(const TListHeader & aHeader);
	ESldError Init(const CSldListInfo & aListInfo, const TListHeader* aHeader = NULL);

	/// Возвращает количество вариантов написания слова
	UInt32 GetNumberOfVariants() const { return m_Header.NumberOfVariants; };
	/// Возвращает тип варианта написания слова по индексу варианта
	EListVariantTypeEnum GetVariantType(UInt32 aIndex) const;
	/// Возвращает код языка варианта написания слова по индексу варианта
	UInt32 GetVariantLangCode(UInt32 aIndex) const;

	/// Возвращает количество слов в списке слов. Имеется в виду полное 
	/// количество, а не только количество слов текущего уровня.
	UInt32 GetNumberOfGlobalWords() const { return m_Header.NumberOfWords; }

	/// Возвращает общее количество картинок в списке слов
	UInt32 GetTotalPictureCount() const { return m_Header.TotalPictureCount; }

	/// Возвращает общее количество видео в списке слов
	UInt32 GetTotalVideoCount() const { return m_Header.TotalVideoCount; }

	/// Возвращает общее количество озвучек в списке слов
	UInt32 GetTotalSoundCount() const { return m_Header.TotalSoundCount; }

	/// Возвращает максимальный размер слова в списке в символах(для UTF16 чтобы
	/// узнать количество байт - нужно умножить на 2). Включая терминирующий 0.
	UInt32 GetMaximumWordSize() const { return m_Header.MaximumWordSize; }

	/// Возвращает код языка с которого производится перевод
	ESldLanguage GetLanguageFrom() const { return SldLanguage::fromCode(m_Header.LanguageCodeFrom); }

	/// Возвращает код языка на который производится перевод
	ESldLanguage GetLanguageTo() const { return SldLanguage::fromCode(m_Header.LanguageCodeTo); }

	/// Возвращает код использования данного списка слов, т.е. для каких действий
	/// он предназначен
	EWordListTypeEnum GetUsage() const;

	/// Возвращает тип алфавита основного язка списка
	EAlphabetType GetAlphabetType() const { return static_cast<EAlphabetType>(m_Header.AlphabetType); }

	/// Возвращает дефолтные уровни каталога, на которых будет производиться поиск
	ESldSearchRange GetSearchRange() const { return static_cast<ESldSearchRange>(m_Header.SearchRange); }

	/// Возвращает флаг того, является ли данный список слов списком слов полнотекстового поиска
	bool IsFullTextSearchList() const { return IsFullTextSearchListType() || !!m_Header.IsFullTextSearchList; }

	/// Возвращает флаг того, является ли тип данного списка слов типом списка слов полнотекстового поиска
	bool IsFullTextSearchListType() const
	{
		return m_Header.WordListUsage >= eWordListType_FullTextSearchBase &&
			   m_Header.WordListUsage <= eWordListType_FullTextSearchLast;
	}

	/// Возвращает флаг того, является ли данный список слов полнотекстового поиска суффиксным
	bool IsSuffixFullTextSearchList() const { return !!m_Header.IsSuffixFullTextSearchList; }

	/// Возвращает флаг того, cодержит или нет данный список для каждого слова все словоформы
	bool IsFullMorphologyWordFormsList() const { return !!m_Header.IsFullMorphologyWordFormsList; }

	/// Возвращает ID морфологической базы, с учетом которой собирался данный список
	UInt32 GetMorphologyId() const { return m_Header.MorphologyId; }

	/// Возвращает флаг того, является ли список слов простым списком или 
	/// содержит в себе иерархию.
	bool IsHierarchy() const { return !!m_Header.IsHierarchy; }

	/// Возвращает флаг того, что некоторым словам из списка слов сопоставлены картинки
	bool IsPicture() const { return !!m_Header.IsPicture; }

	/// Возвращает флаг того, что некоторым словам из списка слов сопоставлено видео
	bool IsVideo() const { return !!m_Header.IsVideo; }

	/// Возвращает флаг того, что некоторым словам из списка слов сопоставлена озвучка
	bool IsSound() const { return !!m_Header.IsSound; }

	/// Возвращает флаг того, что некоторым словам из списка слов сопоставлена 3d сцена
	bool IsScene() const { return !!m_Header.IsScene; }

	/// Возвращает флаг того, имеет или нет список слов локализованные строки
	bool IsListHasLocalizedStrings() const { return m_Header.LocalizedStringsRecordsCount != 0; }

	/// Возвращает флаг того, является ли список слов отсортированным (по
	/// таблице сравнения). Наличие этого флага влияет на возможность поиска
	/// по тексту.
	bool IsSortedList() const { return !!m_Header.IsSortedList; }

	/// Возвращает указатель на заголовок списка слов.
	TListHeader const* GetHeader() const { return &m_Header; }

	/// Возвращает флаг того, ассоциирован ли список слов впрямую с переводами
	bool IsDirectList() const { return !!m_Header.IsDirectList; }

	/// Возвращает указатель на локализованные строки списка слов
	const CSldListLocalizedString* GetLocalizedStrings() const { return m_ListStrings.get(); }

	/// Возвращает флаг того, содержит или нет данный список локализацию слов
	bool GetLocalizedWordsFlag() const { return !!m_Header.IsLocalizedList; }

	/// Возвращает индекс списка, для которого данный список является вспомогательным
	Int32 GetSimpleSortedListIndex() const { return m_Header.SimpleSortedListIndex; }

	/// Возвращает индекс Show-варианта
	UInt32 GetShowVariantIndex() const { return m_ShowVariantIndex; }

	/// Проверяет, есть ли у списка форматированные варианты написания
	UInt32 IsStylizedVariant() const { return m_Header.IsStylizedVariant || GetVariantIndexByType(eVariantStylePreferences) != SLD_DEFAULT_VARIANT_INDEX; }

	/// Добавляет вариант написания
	ESldError AddVariantProperty(const TListVariantProperty & aVariantProperty, bool aUnique = false);

	/// Получает индекс варианта написания по его типу
	Int32 GetVariantIndexByType(const EListVariantTypeEnum aType) const;

private:

	/// Заголовок списка слов
	TListHeader m_Header;

	/// Класс, содержащий локализованные строки списка слов
	sld2::UniquePtr<CSldListLocalizedString> m_ListStrings;

	/// Таблица свойств вариантов написания
	CSldVector<TListVariantProperty> m_VariantPropertyTable;

	/// Индекс Show-варианта (самый запрашиваемый индекс)
	UInt32 m_ShowVariantIndex;
};

#endif
