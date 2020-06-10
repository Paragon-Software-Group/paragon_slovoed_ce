#ifndef _VIDEO_ITEM_H_
#define _VIDEO_ITEM_H_

#include "SldError.h"
#include "SldTypes.h"

/// Класс, представляющий ресурс видео
class CSldVideoItem
{
public:
	/// Конструктор объекта
	CSldVideoItem();
	/// Деструктор объекта
	~CSldVideoItem();

	/// Очистка объекта
	void Clear(const bool isPureClear = true);
	/// Метод, определяющий корректность хранимых данных
	UInt8 IsValid() const;

	/// Конструктор копирования
	CSldVideoItem(const CSldVideoItem&);
	/// Оператор присваивания
	CSldVideoItem& operator=(const CSldVideoItem&);

	/// Возвращает идентификатор видео
	Int32 GetVideoId() const { return m_videoId; }
	/// Установка идентификатора видео
	void SetVideoId(const Int32 aVideoId) { m_videoId = aVideoId; }
	/// Установка идентификатора видео из строки
	ESldError SetVideoId(const UInt16 *const asVideoId);

	/// Возвращает идентификатор внешнего словаря
	const UInt16* GetExtDict() const;
	/// Устанавливает идентификатор внешнего словаря
	ESldError SetExtDict(const UInt16* aiExtDict);

	/// Возвращает идентификатор списка во внешнем словаре
	Int32 GetExtListIdx() const;
	/// Устанавливает идентификатор списка во внешнем словаре
	void SetExtListIdx(const Int32 aiExtListIdx);
	/// Устанавливает идентификатор списка во внешнем словаре из строки
	ESldError SetExtListIdx(const UInt16 *const asExtListIdx);

	/// Возвращает ключ (слово) в списке внешнего словаря
	const UInt16 *GetExtKey() const;
	/// Устанавливает ключ (слово) в списке внешнего словаря
	ESldError SetExtKey(const UInt16 *const asExtKey);

	/// Возвращает тип видеокодека
	ESldVideoType GetVideoCodec() const { return m_videoCodec; }
	/// Установка кодека видеоконтента
	void SetVideoCodec(const ESldVideoType aVideoCodec) { m_videoCodec = aVideoCodec; }
	/// Установка кодека видеоконтента из строки
	ESldError SetVideoCodec(const UInt16 *const aVideoCodecStr);

	/// Возвращает разрешение видеоконтента
	Int32 GetVideoResolution() const { return m_videoResolution; }
	/// Устанавливает разрешение видеоконтента
	void SetVideoResolution(const Int32 aVideoResolution) { m_videoResolution = aVideoResolution; }
	/// Определение размера видеоконтена из стоки
	ESldError SetVideoResolution(const UInt16 *const asVideoResolution);

private:
	/// Заменяет старое значение строки на новое (через удаление, при необходимости, старой)
	ESldError ReplaceString(UInt16 **const asDst, const UInt16 *const asSrc);
	/// Производит проверку формата и установку идентификатора внешнего словаря
	ESldError StrToDictId(UInt32 *const apiDictIdPtr, const UInt16 *const asDictId);

private:
	/// Идентификатор видео
	Int32  m_videoId;
	/// идентификатор внешнего словаря
	UInt16* m_sExtDict;
	/// идентификатор списка во внешнем словаре
	Int32   m_iExtListIdx;
	/// ключ (слово) в списке внешнего словаря
	UInt16 *m_sExtKey;
	/// Размер видеоконтента
	Int32  m_videoResolution;
	/// Тип кодека видеоконтента
	ESldVideoType m_videoCodec;
};

#endif // _VIDEO_ITEM_H_
