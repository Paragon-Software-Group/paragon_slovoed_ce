#ifndef IMAGE_ITEM_H
#define IMAGE_ITEM_H

#include "SldDefines.h"
#include "SldError.h"
#include "SldTypes.h"

/// Объект, описывающий одно изображение
class CSldImageItem
{
public:
	/// Конструктор класса
	CSldImageItem();
	/// Деструктор класса
	~CSldImageItem();

	/// Конструктор копирования
	CSldImageItem(const CSldImageItem &aImageItem);
	/// Оператор присваивания
	CSldImageItem &operator=(const CSldImageItem &aImageItem);

	/// Очищает объект от хранимой информации
	void Clear(const bool isPureClear = true);

	/// Возвращает идентификатор изображения
	UInt32 GetPictureIndex() const;
	/// Устанавливает идентификатор изображения
	void SetPictureIndex(const UInt32 aiPictureIndex);
	/// Устанавливает идентификатор изображения из строки
	ESldError SetPictureIndex(const UInt16 *const asPictureIndex);

	/// Возвращает имя внешнего словаря
	const UInt16* GetExtDict() const;
	/// Устанавливает имя в списке внешнего словаря
	ESldError SetExtDict(const UInt16 *const asExtDict);

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

	/// Возвращает желаемую ширину изображения при его отображении
	TSizeValue GetShowWidth() const;
	/// Устанавливает желаемую ширину изображения при его отображении
	void SetShowWidth(const TSizeValue aiShowWidth);
	/// Возвращает желаемую ширину изображения при его отображении из стоки
	ESldError SetShowWidth(const UInt16 *const asShowWidth);

	/// Возвращает желаемую высоту изображения при его отображении
	TSizeValue GetShowHeight() const;
	/// Устанавливает желаемую высоту изображения при его отображении
	void SetShowHeight(const TSizeValue aiShowHeight);
	/// Устанавливает желаемую высоту изображения при его отображении из строки
	ESldError SetShowHeight(const UInt16 *const asiShowHeight);

	/// Устанавливает желаемую высоту изображения при его отображении
	void SetScalability(bool aScalability);

	/// Возвращает желаемую высоту изображения при его отображении
	bool GetScalability() const;

	/// Устанавливает флаг "системности" изображения
	void SetSystemFlag(bool aScalability);

	/// Возвращает флаг "системности" изображения
	bool GetSystemFlag() const;

private:
	/// Заменяет старое значение строки на новое (через удаление, при необходимости, старой)
	ESldError ReplaceString(UInt16 **const asDst, const UInt16 *const asSrc);
	/// Производит проверку формата и установку идентификатора внешнего словаря
	ESldError StrToDictId(UInt32 *const apiDictIdPtr, const UInt16 *const asDictId);

private:
	/// идентификатор изображения
	UInt32	m_iPictureIndex;
	/// имя внешнего словаря
	UInt16	*m_sExtDict;
	/// идентификатор списка во внешнем словаре
	Int32	m_iExtListIdx;
	/// ключ (слово) в списке внешнего словаря
	UInt16	*m_sExtKey;
	/// желаемая ширина изображения при его отображении
	TSizeValue	m_iShowWidth;
	/// желаемая высота изображения при его отображении
	TSizeValue	m_iShowHeight;
	/// флаг масштабируемости изображения
	bool	m_isScalable;
	/// флаг масштабируемости изображения
	bool	m_isSystem;
};

# endif // IMAGE_ITEM_H