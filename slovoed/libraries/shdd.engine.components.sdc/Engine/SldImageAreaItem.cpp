#include "SldImageAreaItem.h"
#include "SldCompare.h"
#include "SldMetadataParser.h"

const UInt16 CSldImageAreaItem::AREA_ITEM_TYPE_LINK[] = { 'l', 'i', 'n', 'k', 0 };
const UInt16 CSldImageAreaItem::AREA_ITEM_TYPE_DROP[] = { 'd', 'r', 'o', 'p', 0 };

const UInt16 DPI = 89;
const double InchInMm = 0.0393700787;

/** *********************************************************************
* Конструктор класса
************************************************************************/
CSldImageAreaItem::CSldImageAreaItem()
	: m_sId(0), m_iLeft(0), m_iTop(0)
	, m_iWidth(0), m_iHeight(0), m_iMask(SLD_INDEX_PICTURE_NO)
	, m_Percent(0), m_eType(eImageAreaType_Unknown)
	, m_sActionScript(NULL), m_Coords(NULL)
{
}

/** *********************************************************************
* Деструктор класса
************************************************************************/
CSldImageAreaItem::~CSldImageAreaItem()
{
	sldMemFree(m_sId);
	sldMemFree(m_sActionScript);
	sldMemFree(m_Coords);
}

/** *********************************************************************
* Конструктор копирования
*
* @param[in] aImageAreaItem	- объект, чье содержимое необходимо продублировать
************************************************************************/
CSldImageAreaItem::CSldImageAreaItem(const CSldImageAreaItem &aImageAreaItem)
	: m_sId(0)
	, m_sActionScript(0)
	, m_Coords(0)
{
	(*this) = aImageAreaItem;
}

/** *********************************************************************
* Оператор присваивания
*
* @param[in] aImageAreaItem	- объект, чье содержимое необходимо продублировать
*
* @return объект *this
************************************************************************/
CSldImageAreaItem &CSldImageAreaItem::operator=(const CSldImageAreaItem &aImageAreaItem)
{
	// Копируем standalone поля
	m_iLeft = aImageAreaItem.GetLeft();
	m_iTop = aImageAreaItem.GetTop();
	m_iWidth = aImageAreaItem.GetWidth();
	m_iHeight = aImageAreaItem.GetHeight();
	m_eType = aImageAreaItem.GetType();
	m_Percent = aImageAreaItem.IsPercent();
	m_iMask = aImageAreaItem.GetMask();

	ESldError error;
	// Копируем строки
	error = ReplaceString(&m_sId, aImageAreaItem.GetId());
	if(error != eOK)
		m_sId = 0;

	error = ReplaceString(&m_sActionScript, aImageAreaItem.GetActionString());
	if(error != eOK)
		m_sActionScript = 0;

	error = ReplaceString(&m_Coords, aImageAreaItem.GetCoordsString());
	if(error != eOK)
		m_Coords = 0;

	return *this;
}

/** *********************************************************************
* Возвращает флаг корректности содержимого
*
* @return 1/0
************************************************************************/
UInt8 CSldImageAreaItem::IsValid() const
{
	UInt8 validSideSize = (m_iHeight && m_iWidth);
	UInt8 validCoordinates = (m_Coords && CSldCompare::StrLen(m_Coords) != 0);
	if (m_sActionScript && (validSideSize || m_iMask != SLD_INDEX_PICTURE_NO || validCoordinates) && m_eType != eImageAreaType_Unknown)
		return 1;
	else
		return 0;
}

/** *********************************************************************
* Возвращает строку, идентификатор области
*
* @return идентификатор области
************************************************************************/
UInt16 *CSldImageAreaItem::GetId() const
{
	return m_sId;
}

/** *********************************************************************
* Устанавливает идентификатор области
*
* @param[in] asId	- идентификатор области
*
* @return код ошибки
************************************************************************/
ESldError CSldImageAreaItem::SetId(const UInt16 *const asId)
{
	return ReplaceString(&m_sId, asId);
}

/** *********************************************************************
* Возвращает X-координату области (ее левой границы)
*
* @return координаты левой границы области
************************************************************************/
UInt32 CSldImageAreaItem::GetLeft() const
{
	return m_iLeft;
}

/** *********************************************************************
* Устанавливает X-координату области (ее левой границы)
*
* @param[in] aiLeft	- координаты левой границы области
************************************************************************/
void CSldImageAreaItem::SetLeft(const UInt32 aiLeft)
{
	m_iLeft = aiLeft;
}

/** *********************************************************************
* Устанавливает X-координату области (ее левой границы) из строки
*
* @param[in] asLeft	- координаты левой границы области
*
* @return код ошибки
************************************************************************/
ESldError CSldImageAreaItem::SetLeft(const UInt16 *const asLeft)
{
	return CSldCompare::StrToUInt32(asLeft, 10, &m_iLeft);
}

/** *********************************************************************
* Возвращает Y-координату области (ее верхней границы)
*
* @return координаты верхней границы области
************************************************************************/
UInt32 CSldImageAreaItem::GetTop() const
{
	return m_iTop;
}

/** *********************************************************************
* Устанавливает Y-координату области (ее левой границы)
*
* @param[in] aiTop	- координаты верхней границы области
************************************************************************/
void CSldImageAreaItem::SetTop(const UInt32 aiTop)
{
	m_iTop = aiTop;
}

/** *********************************************************************
* Устанавливает Y-координату области (ее верхней границы) из строки
*
* @param[in] asTop	- координаты верхней границы области
*
* @return код ошибки
************************************************************************/
ESldError CSldImageAreaItem::SetTop(const UInt16 *const asTop)
{
	return CSldCompare::StrToUInt32(asTop, 10, &m_iTop);
}

/** *********************************************************************
* Возвращает ширину области
*
* @return ширина области
************************************************************************/
UInt32 CSldImageAreaItem::GetWidth() const
{
	return m_iWidth;
}

/** *********************************************************************
* Устанавливает ширину области
*
* @param[in] aiWidth	- ширина области
************************************************************************/
void CSldImageAreaItem::SetWidth(const UInt32 aiWidth)
{
	m_iWidth = aiWidth;
}

/** *********************************************************************
* Устанавливает ширину области из строки
*
* @param[in] asWidth	- ширина области
*
* @return код ошибки
************************************************************************/
ESldError CSldImageAreaItem::SetWidth(const UInt16 *const asWidth)
{
	return CSldCompare::StrToUInt32(asWidth, 10, &m_iWidth);
}

/** *********************************************************************
* Возвращает высоту области
*
* @return высота области
************************************************************************/
UInt32 CSldImageAreaItem::GetHeight() const
{
	return m_iHeight;
}

/** *********************************************************************
* Устанавливает высоту области
*
* @param[in] aiHeight	- высота области
************************************************************************/
void CSldImageAreaItem::SetHeight(const UInt32 aiHeight)
{
	m_iHeight = aiHeight;
}

/** *********************************************************************
* Устанавливает высоту области из строки
*
* @param[in] asHeight	- высота области
*
* @return код ошибки
************************************************************************/
ESldError CSldImageAreaItem::SetHeight(const UInt16 *const asHeight)
{
	return CSldCompare::StrToUInt32(asHeight, 10, &m_iHeight);
}

/** *********************************************************************
* Возвращает тип события, ассоциированного с областью
*
* @return тип события, ассоциированного с областью
************************************************************************/
ESldImageAreaType CSldImageAreaItem::GetType() const
{
	return m_eType;
}

/** *********************************************************************
* Устанавливает тип события, ассоциированного с областью
*
* @param[in] aeType	- тип события, ассоциированного с областью
************************************************************************/
void CSldImageAreaItem::SetType(const ESldImageAreaType aeType)
{
	m_eType = aeType;
}

/** *********************************************************************
* Устанавливает тип события, ассоциированного с областью из строки
*
* @param[in] asTypeToken	- тип события, ассоциированного с областью
*
* @return код ошибки
************************************************************************/
ESldError CSldImageAreaItem::SetType(const UInt16 *const asTypeToken)
{
	return CSldCompare::StrToInt32(asTypeToken, 10, (Int32*)&m_eType);
}

/** *********************************************************************
* Возвращает строку-сценарий, необходимую для обработки события, ассоциированного с областью
*
* @return строка-сценарий, необходимая для обработки события
************************************************************************/
const UInt16 *CSldImageAreaItem::GetActionString() const
{
	return m_sActionScript;
}

/** *********************************************************************
* Устанавливает строку-сценарий, необходимую для обработки события, ассоциированного с областью
*
* @param[in] asActionString	- строка-сценарий, необходимая для обработки события
*
* @return код ошибки
************************************************************************/
ESldError CSldImageAreaItem::SetActionString(const UInt16 *const asActionString)
{
	return ReplaceString(&m_sActionScript, asActionString);
}

/** *********************************************************************
* Возвращает строку координат, необходимаая для задания области произвольной формы
*
* @return строка координат, необходимаая для задания области произвольной формы
************************************************************************/
const UInt16 *CSldImageAreaItem::GetCoordsString() const
{
	return m_Coords;
}

/** *********************************************************************
* Возвращает тип области
*
* @return тип области из перечисления eShapeType
************************************************************************/
ESldAreaShape CSldImageAreaItem::GetShapeType() const
{
	if (m_Coords == 0)
		return eAreaShapeRect;
	else
		return eAreaShapePoly;
}

/** *********************************************************************
* Устанавливает строку координат, необходимаая для задания области произвольной формы
*
* @param[in] asCoordsString	- строка координат, необходимаая для задания области произвольной формы
*
* @return код ошибки
************************************************************************/
ESldError CSldImageAreaItem::SetCoordsString(const UInt16 *const asCoordsString)
{
	return ReplaceString(&m_Coords, asCoordsString);
}

/** *********************************************************************
* Заменяет старое значение строки на новое (через удаление, при необходимости, старой)
*
* @param[in] asDst	- указатель на строку, на место которой надо поместить новую строчку
* @param[in] asSrc	- строчка, которую необходимо сохранить
*
* @return код ошибки
************************************************************************/
ESldError CSldImageAreaItem::ReplaceString(UInt16 **const asDst, const UInt16 *const asSrc)
{
	if(!asSrc || !asDst)
		return eMemoryNullPointer;

	const UInt32 iSrcStringLength(CSldCompare::StrLen(asSrc));

	if (!*asDst || CSldCompare::StrLen(*asDst) < iSrcStringLength)
	{
		sldMemFree(*asDst);
		*asDst = (UInt16*)sldMemNew((iSrcStringLength + 1)*sizeof(UInt16));
		if (!*asDst)
			return eMemoryNotEnoughMemory;
	}

	if (iSrcStringLength != CSldCompare::StrCopy(*asDst, asSrc))
		return eCommonWrongSizeOfData;

	return eOK;
}

/** *********************************************************************
* Возвращает указаны ли размеры в процентах
*
* @return флаг
************************************************************************/
UInt32 CSldImageAreaItem::IsPercent() const
{
	return m_Percent;
}

/** *********************************************************************
* Устанавливает флаг того, что значения размеров указаны в процентах
*
* @param[in] asPercent	- флаг того, что значения размеров указаны в процентах
*
* @return код ошибки
************************************************************************/
ESldError CSldImageAreaItem::SetPercent(const UInt16 *const asPercent)
{
	return CSldCompare::StrToUInt32(asPercent, 10, &m_Percent);
}

/** *********************************************************************
* Устанавливает флаг того, что значения размеров указаны в процентах
*
* @param[in] asPercent	- флаг того, что значения размеров указаны в процентах
************************************************************************/
void CSldImageAreaItem::SetPercent(const UInt32 asPercent)
{
	 m_Percent = asPercent;
}

/** *********************************************************************
* Приводит процентные значения в пиксели, в зависимости от единиц измерения
*
* @param[in] aWidth	- ширина картинки
* @param[in] aHeight - высота картинки
* @param[in] aUnit	- единица измерения ширины и высоты
*
* @return код ошибки
************************************************************************/
ESldError CSldImageAreaItem::TransformToPixel(Int32 aWidth, Int32 aHeight, Int32 aUnit)
{
	if(!m_Percent || (m_Coords != NULL))
		return eOK;

	if (aUnit == eMetadataUnitType_mm)
	{
		aWidth = aWidth * DPI * InchInMm;
		aHeight = aHeight * DPI * InchInMm;
	}

	m_iLeft		= m_iLeft	* aWidth	/ 100;
	m_iWidth	= m_iWidth	* aWidth	/ 100;
	m_iTop		= m_iTop	* aHeight	/ 100;
	m_iHeight	= m_iHeight	* aHeight	/ 100;

	m_Percent = 0;

	return eOK;
}

/************************************************************************
* Возвращает индекс маски
*
* @return флаг
************************************************************************/
UInt32 CSldImageAreaItem::GetMask() const
{
	return m_iMask;
}

/** *********************************************************************
* Устанавливает индекс маски
*
* @param[in] aMask	- флаг того, что значения размеров указаны в процентах
*
* @return код ошибки
************************************************************************/
void CSldImageAreaItem::SetMask(const UInt32 aMask)
{
	m_iMask = aMask;
}

/// Устанавливает индекс маски из строки
ESldError CSldImageAreaItem::SetMask(const UInt16 *const asMask)
{
	return CSldCompare::StrToUInt32(asMask, 10, &m_iMask);
}