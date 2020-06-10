#ifndef IMAGE_AREA_ITEM_H
#define IMAGE_AREA_ITEM_H

#include "SldDefines.h"
#include "SldError.h"
#include "SldTypes.h"

/// Элемент, описывающий одну область выделения
class CSldImageAreaItem
{
public:
	/// Конструктор класса
	CSldImageAreaItem();
	/// Деструктор класса
	~CSldImageAreaItem();

	/// Конструктор копирования
	CSldImageAreaItem(const CSldImageAreaItem &aImageAreaItem);
	/// Оператор присваивания
	CSldImageAreaItem &operator=(const CSldImageAreaItem &aImageAreaItem);

	/// Возвращает флаг корректности содержимого
	UInt8 IsValid() const;

	/// Возвращает строку, идентификатор области
	UInt16 *GetId() const;
	/// Устанавливает идентификатор области
	ESldError SetId(const UInt16 *const asId);

	/// Возвращает X-координату области (ее левой границы)
	UInt32 GetLeft() const;
	/// Устанавливает X-координату области (ее левой границы)
	void SetLeft(const UInt32 aiLeft);
	/// Устанавливает X-координату области (ее левой границы) из строки
	ESldError SetLeft(const UInt16 *const asLeft);

	/// Возвращает Y-координату области (ее верхней границы)
	UInt32 GetTop() const;
	/// Устанавливает Y-координату области (ее верхней границы)
	void SetTop(const UInt32 aiTop);
	/// Устанавливает Y-координату области (ее верхней границы) из строки
	ESldError SetTop(const UInt16 *const asTop);

	/// Возвращает ширину области
	UInt32 GetWidth() const;
	/// Устанавливает ширину области
	void SetWidth(const UInt32 aiWidth);
	/// Устанавливает ширину области из строки
	ESldError SetWidth(const UInt16 *const asWidth);

	/// Возвращает высоту области
	UInt32 GetHeight() const;
	/// Устанавливает высоту области
	void SetHeight(const UInt32 aiHeight);
	/// Устанавливает высоту области из строки
	ESldError SetHeight(const UInt16 *const asHeight);

	/// Возвращает тип события, ассоциированного с областью
	ESldImageAreaType GetType() const;
	/// Устанавливает тип события, ассоциированного с областью
	void SetType(const ESldImageAreaType aeType);
	/// Устанавливает тип события, ассоциированного с областью из строки
	ESldError SetType(const UInt16 *const asTypeToken);

	/// Возвращает строку-сценарий, необходимую для обработки события, ассоциированного с областью
	const UInt16 *GetActionString() const;
	/// Устанавливает строку-сценарий, необходимую для обработки события, ассоциированного с областью
	ESldError SetActionString(const UInt16 *const asActionString);

	/// Возвращает строку координат, необходимаая для задания области произвольной формы
	const UInt16 *GetCoordsString() const;
	/// Устанавливает строку координат, необходимаая для задания области произвольной формы
	ESldError SetCoordsString(const UInt16 *const asCoordsString);

	/// Возвращает флаг указаны ли размеры в процентах
	UInt32 IsPercent() const;
	/// устанавливает флаг указаны ли размеры в процентах
	ESldError SetPercent(const UInt16 *const asPercent);
	/// устанавливает флаг указаны ли размеры в процентах
	void SetPercent(const UInt32 asPercent);

	/// Возвращает тип области
	ESldAreaShape GetShapeType() const;

	/// Возвращает индекс маски
	UInt32 GetMask() const;
	/// Устанавливает индекс маски
	void SetMask(const UInt32 aMask);
	/// Устанавливает индекс маски из строки
	ESldError SetMask(const UInt16 *const asMask);

	ESldError TransformToPixel(Int32 aWidth, Int32 aHeight, Int32 aUnit = 0);

private:
	/// Заменяет старое значение строки на новое (через удаление, при необходимости, старой)
	ESldError ReplaceString(UInt16 **const asDst, const UInt16 *const asSrc);

private:
	/// Идентификатор области
	UInt16 *m_sId;

	/// X-координата области (ее левой границы)
	UInt32 m_iLeft;
	/// Y-координата области (ее левой границы)
	UInt32 m_iTop;
	/// Ширина области
	UInt32 m_iWidth;
	/// Высота области
	UInt32 m_iHeight;
	/// Индекс маски
	UInt32 m_iMask;

	/// В процентах ли размеры
	UInt32 m_Percent;

	/// Тип события, ассоциированного с областью (см. ESldImageAreaType)
	ESldImageAreaType m_eType;
	/// Строка-сценарий, необходимая для обработки события, ассоциированного с областью
	UInt16 *m_sActionScript;

	/// Строка координат, необходимаая для задания области произвольной формы
	UInt16* m_Coords;

	/// Шаблон строки, обозначающей тип события - переход по ссылке
	static const UInt16 AREA_ITEM_TYPE_LINK[];
	/// Шаблон строки, обозначающей тип события - перетаскивание какого-либо объекта на область
	static const UInt16 AREA_ITEM_TYPE_DROP[];
};

#endif // IMAGE_AREA_ITEM_H
