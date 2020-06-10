#ifndef _SLD_HISTORY_H_
#define _SLD_HISTORY_H_

#include "SldError.h"
#include "SldDefines.h"
#include "SldTypes.h"
#include "SldCompare.h"

/// флаг результата восстановления слова по элементу истории
enum ESldHistoryResult
{
	/// Слово не восстановлено
	eHistoryNotCompare = 0,
	/// Слово восстановлено точно
	eHistoryFullCompare,
	/// Слово восстановлено с точностью до Show-варианта (у слова есть дубли в списке слов и восстановить точное вхождение среди них не получилось)
	eHistoryDuplicateCompare,
	/// Слово восстановлено с точностью до Show-варианта, но список восстановлен не точно
	eHistoryShowCompare,
	/// Слово совпадает по массе, но точного совпадения найти не удалось
	eHistoryMassCompare
};

class CSldHistoryElement
{
public:
	/// Конструктор
	CSldHistoryElement();

	/// Деструктор
	~CSldHistoryElement() { ReleaseElement(); }

	/// move конструктор
	CSldHistoryElement(CSldHistoryElement&&);

	/// Конструктор копирования
	CSldHistoryElement(const CSldHistoryElement& aRef) : CSldHistoryElement() { *this = aRef; }

	/// Оператор присваивания
	CSldHistoryElement& operator=(const CSldHistoryElement & aRef);

	/// Оператор присваивания из rvalue
	CSldHistoryElement& operator=(CSldHistoryElement&&);

	/// Оператор сравнения
	bool operator==(const CSldHistoryElement & aRef)
	{
		return m_DictID == aRef.m_DictID && m_ListIndex == aRef.m_ListIndex && m_LanguageCode == aRef.m_LanguageCode && m_ListType == aRef.m_ListType && m_WordIndex == aRef.m_WordIndex;
	}

	/// Загружает элемент из бинарных данных
	ESldError	LoadElement(const Int8* aData, const UInt32 aDataSize);
	/// Возвращает бинарное представление текущего элемента
	ESldError	GetBinaryData(Int8** aData, UInt32* aDataSize) const;

	/// Устанавливает ID базы
	void		SetDictID(UInt32 aDictID) { m_DictID = aDictID; }
	/// Возвращает ID базы
	UInt32		GetDictID() const { return m_DictID; }

	/// Устанавливает индекс словарного списка
	void		SetListIndex(Int32 aListIndex) { m_ListIndex = aListIndex; }
	/// Возвращает индекс словарного списка
	Int32		GetListIndex() const { return m_ListIndex; }

	/// Устанавливает код языка словарного списка
	void		SetLanguageCode(UInt32 aLanguageCode) { m_LanguageCode = aLanguageCode; }
	/// Возвращает код языка словарного списка
	UInt32		GetLanguageCode() const { return m_LanguageCode; }

	/// Устанавливает тип словарного списка
	void		SetListType(EWordListTypeEnum aListType) { m_ListType = aListType; }
	/// Возвращает тип словарного списка
	UInt32		GetListType() const { return m_ListType; }

	/// Устанавливает индекс слова
	void		SetWordIndex(UInt32 aWordIndex) { m_WordIndex = aWordIndex; }
	/// Возвращает индекс слова
	UInt32		GetWordIndex() const { return m_WordIndex; }

	/// Устанавливает количество вариантов написания
	void		SetWordsCount(UInt32 aWordsCount) { m_WordsCount = aWordsCount; }
	/// Возвращает количество вариантов написания
	UInt32		GetWordsCount() const { return m_WordsCount; }

	/// Устанавливает тип варианта написания
	ESldError	SetVariantType(UInt32 aVariantIndex, EListVariantTypeEnum aVariantType);
	/// Возвращает тип варианта написания
	ESldError	GetVariantType(UInt32 aVariantIndex, EListVariantTypeEnum* aVariantType) const;

	/// Устанавливает слово для заданного варианта написания
	ESldError	SetCurrentWord(UInt32 aVariantIndex, UInt16* aWord);
	/// Возвращает слово для заданного варианта написания
	ESldError	GetCurrentWord(UInt32 aVariantIndex, UInt16** aWord) const;

	/// Возвращает Show-вариант для данного элемента
	ESldError	GetShowVariant(UInt16** aWord) const;

	/// Устанавливает дату
	void		SetDate(UInt32 aDay, UInt32 aMonth, UInt32 aYear);
	/// Устанавливает время
	void		SetTime(UInt32 aHour, UInt32 aMinute, UInt32 aSecond);

	/// Устанавливает дату
	void		SetDate(UInt32 aDate) { m_Date = aDate; }
	/// Устанавливает время
	void		SetTime(UInt32 aTime) { m_Time = aTime; }

	/// Возвращает дату
	ESldError	GetDate(UInt32* aDay, UInt32* aMonth, UInt32* aYear) const;
	/// Возвращает время
	ESldError	GetTime(UInt32* aHour, UInt32* aMinute, UInt32* aSecond) const;

	/// Возвращает дату
	ESldError	GetDate(UInt32* aDate) const;
	/// Возвращает время
	ESldError	GetTime(UInt32* aTime) const;

	/// Устанавливает пользовательские данные
	ESldError	SetUserData(const Int8* aUserData, UInt32 aDataSize);
	/// Возвращает пользовательские данные
	UInt32		GetUserData(Int8** aUserData) const;

	/// Возвращает хэш элемента (без учета даты, юзер даты и пр.)
	UInt32		GetHash() const;

private:

	/// Освобождает память
	void	ReleaseElement();

	/// Восстанавливает целостность загруженного из бинарных данных элемента
	ESldError ResolveElement(const Int8* aData, const UInt32 aDataSize, UInt32& aOldHistoryElementSize);

private:

	/// Дата создания элемента
	UInt32					m_Date;
	/// Время создания элемента
	UInt32					m_Time;

	/// ID словарной базы
	UInt32					m_DictID;

	/// Индекс словарного списка
	Int32					m_ListIndex;
	/// Код языка словарного списка
	UInt32					m_LanguageCode;
	/// Тип словарного списка
	EWordListTypeEnum		m_ListType;

	/// Индекс слова в словарном списке
	Int32					m_WordIndex;
	/// Количество вариантов написания
	UInt32					m_WordsCount;
	/// Массив слов с разными вариантами написания данного слова
	UInt16**				m_Words;
	/// Массив типов вариантов написания
	EListVariantTypeEnum*	m_VariantType;

	/// Размер массива с пользовательскими данными в байтах
	UInt32					m_UserDataSize;
	/// Указатель на бинарный массив пользовательских данных
	Int8*					m_UserData;

	/// Зарезеривировано
	UInt32					m_Reserved[7];

	/// Сигнатура конца структуры см. HISTORY_ELEMENT_SIGNATURE
	/// так как в элементы пишутся указатели - это единственный адекватный метод
	/// найти конец структуры в элементах записаных на системе с другой разрядностью
	UInt32					m_HistorySignature;
};

#endif