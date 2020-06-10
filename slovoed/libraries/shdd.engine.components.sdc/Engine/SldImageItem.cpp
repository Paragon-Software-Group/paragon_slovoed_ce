#include "SldImageItem.h"
#include "SldCompare.h"
#include "SldMetadataParser.h"

/** *********************************************************************
* Конструктор класса
************************************************************************/
CSldImageItem::CSldImageItem()
	: m_iPictureIndex(SLD_INDEX_PICTURE_NO)
	, m_sExtDict	(NULL)
	, m_iExtListIdx	(0)
	, m_sExtKey		(NULL)
	, m_iShowWidth	(0, eMetadataUnitType_px)
	, m_iShowHeight	(0, eMetadataUnitType_px)
	, m_isScalable  (0)
	, m_isSystem    (0) 
{
}

/** *********************************************************************
* Деструктор класса
************************************************************************/
CSldImageItem::~CSldImageItem()
{
	Clear(false);
}

/** *********************************************************************
* Конструктор копирования
*
* @param[in] aImageItem	- объект, чье содержимое необходимо продублировать
************************************************************************/
CSldImageItem::CSldImageItem(const CSldImageItem &aImageItem)
	: m_sExtDict(NULL), m_sExtKey(NULL)
{
	(*this) = aImageItem;
}

/** *********************************************************************
* Оператор присваивания
*
* @param[in] aImageItem	- объект, чье содержимое необходимо продублировать
*
* @return текущий объект
************************************************************************/
CSldImageItem &CSldImageItem::operator=(const CSldImageItem &aImageItem)
{
	m_iPictureIndex	= aImageItem.GetPictureIndex();
	m_iExtListIdx	= aImageItem.GetExtListIdx();
	m_iShowWidth	= aImageItem.GetShowWidth();
	m_iShowHeight	= aImageItem.GetShowHeight();
	m_isScalable	= aImageItem.GetScalability();
	m_isSystem		= aImageItem.GetSystemFlag();

	ReplaceString(&m_sExtKey, aImageItem.GetExtKey());

	ReplaceString(&m_sExtDict, aImageItem.GetExtDict());

	return *this;
}

/** *********************************************************************
* Очищает объект от хранимой информации
************************************************************************/
void CSldImageItem::Clear(const bool isPureClear)
{
	if (m_sExtKey)
		sldMemFree(m_sExtKey);
	if (m_sExtDict)
		sldMemFree(m_sExtDict);

	if (isPureClear)
		(*this) = CSldImageItem();
}

/** *********************************************************************
* Возвращает идентификатор изображения
*
* @return идентификатор изображения
************************************************************************/
UInt32 CSldImageItem::GetPictureIndex() const
{
	return m_iPictureIndex;
}

/** *********************************************************************
* Устанавливает идентификатор изображения
*
* @param[in] aiPictureIndex	- идентификатор изображения
************************************************************************/
void CSldImageItem::SetPictureIndex(const UInt32 aiPictureIndex)
{
	m_iPictureIndex = aiPictureIndex;
}

/** *********************************************************************
* Устанавливает идентификатор изображения из строки
*
* @param[in] asPictureIndex	- идентификатор изображения в виде строки
*
* @return код ошибки
************************************************************************/
ESldError CSldImageItem::SetPictureIndex(const UInt16 *const asPictureIndex)
{
	return CSldCompare::StrToUInt32(asPictureIndex, 16, &m_iPictureIndex);
}

/** *********************************************************************
* Возвращает идентификатор внешнего словаря
*
* @return идентификатор внешнего словаря
************************************************************************/
const UInt16* CSldImageItem::GetExtDict() const
{
	return m_sExtDict;
}

/** *********************************************************************
* Устанавливает идентификатор внешнего словаря из строки
*
* @param[in] asExtDictId	- идентификатор внешнего словаря в виде строки
*
* @return код ошибки
************************************************************************/
ESldError CSldImageItem::SetExtDict(const UInt16 *const asExtDictId)
{
	return ReplaceString(&m_sExtDict, asExtDictId);
}

/** *********************************************************************
* Возвращает идентификатор списка во внешнем словаре
*
* @return идентификатор списка во внешнем словаре
************************************************************************/
Int32 CSldImageItem::GetExtListIdx() const
{
	return m_iExtListIdx;
}

/** *********************************************************************
* Устанавливает идентификатор списка во внешнем словаре
*
* @param[in] aiExtListIdx	- идентификатор списка во внешнем словаре
************************************************************************/
void CSldImageItem::SetExtListIdx(const Int32 aiExtListIdx)
{
	m_iExtListIdx = aiExtListIdx;
}

/** *********************************************************************
* Устанавливает идентификатор списка во внешнем словаре из строки
*
* @param[in] asExtListIdx	- идентификатор списка во внешнем словаре в виде строки
*
* @return код ошибки
************************************************************************/
ESldError CSldImageItem::SetExtListIdx(const UInt16 *const asExtListIdx)
{
	return CSldCompare::StrToInt32(asExtListIdx, 10, &m_iExtListIdx);
}

/** *********************************************************************
* Возвращает ключ (слово) в списке внешнего словаря
*
* @return ключ (слово) в списке внешнего словаря
************************************************************************/
const UInt16 *CSldImageItem::GetExtKey() const
{
	return m_sExtKey;
}

/** *********************************************************************
* Устанавливает ключ (слово) в списке внешнего словаря
*
* @param[in] asExtKey	- ключ (слово) в списке внешнего словаря
*
* @return код ошибки
************************************************************************/
ESldError CSldImageItem::SetExtKey(const UInt16 *const asExtKey)
{
	return ReplaceString(&m_sExtKey, asExtKey);
}

/** *********************************************************************
* Возвращает желаемую ширину изображения при его отображении
*
* @return желаемая ширина изображения при его отображении
************************************************************************/
TSizeValue CSldImageItem::GetShowWidth() const
{
	return m_iShowWidth;
}

/** *********************************************************************
* Возвращает желаемую ширину изображения при его отображении
*
* @param[in] aiShowWidth	- желаемая ширина изображения при его отображении
************************************************************************/
void CSldImageItem::SetShowWidth(const TSizeValue aiShowWidth)
{
	m_iShowWidth = aiShowWidth;
}

/** *********************************************************************
* Устанавливает желаемую ширину изображения при его отображении из строки
*
* @param[in] asShowWidth	- желаемая ширина изображения при его отображении в виде строки
*
* @return код ошибки
************************************************************************/
ESldError CSldImageItem::SetShowWidth(const UInt16 *const asShowWidth)
{
	Int32 value;
	ESldError error = CSldCompare::StrToInt32(asShowWidth, 10, &value);
	if (error == eOK)
		m_iShowWidth.Set(value * SLD_SIZE_VALUE_SCALE, eMetadataUnitType_px);
	return error;
}

/** *********************************************************************
* Возвращает желаемую высоту изображения при его отображении
*
* @return желаемая высота изображения при его отображении
************************************************************************/
TSizeValue CSldImageItem::GetShowHeight() const
{
	return m_iShowHeight;
}

/** *********************************************************************
* Устанавливает желаемую высоту изображения при его отображении
*
* @param[in] aiShowHeight	- изображения при его отображении
************************************************************************/
void CSldImageItem::SetShowHeight(const TSizeValue aiShowHeight)
{
	m_iShowHeight = aiShowHeight;
}

/** *********************************************************************
* Устанавливает желаемую высоту изображения при его отображении из строки
*
* @param[in] asiShowHeight	- изображения при его отображении в виде строки
*
* @return код ошибки
************************************************************************/
ESldError CSldImageItem::SetShowHeight(const UInt16 *const asiShowHeight)
{
	Int32 value;
	ESldError error = CSldCompare::StrToInt32(asiShowHeight, 10, &value);
	if (error == eOK)
		m_iShowHeight.Set(value * SLD_SIZE_VALUE_SCALE, eMetadataUnitType_px);
	return error;
}


/** *********************************************************************
* Устанавливает флаг масштабируемости изображения
*
* @param[in] aiShowHeight	- изображения при его отображении
************************************************************************/
void CSldImageItem::SetScalability(bool aScalability)
{
	m_isScalable = aScalability;
}

/** *********************************************************************
* Возвращает флаг масштабируемости изображения
*
* @return желаемая высота изображения при его отображении
************************************************************************/
bool CSldImageItem::GetScalability() const
{
	return m_isScalable;
}

/** *********************************************************************
* Устанавливает флаг "системности" изображения
*
* @param[in] aiShowHeight	- изображения при его отображении
************************************************************************/
void CSldImageItem::SetSystemFlag(bool aSystem)
{
	m_isSystem = aSystem;
}

/** *********************************************************************
* Возвращает флаг "системности" изображения
*
* @return желаемая высота изображения при его отображении
************************************************************************/
bool CSldImageItem::GetSystemFlag() const
{
	return m_isSystem;
}


/** *********************************************************************
* Заменяет старое значение строки на новое (через удаление, при необходимости, старой)
*
* @param[in] asDst	- указатель на строку, на место которой надо поместить новую строчку
* @param[in] asSrc	- строчка, которую необходимо сохранить
*
* @return код ошибки
************************************************************************/
ESldError CSldImageItem::ReplaceString(UInt16 **const asDst, const UInt16 *const asSrc)
{
	const Int32 iSrcStringLenght(CSldCompare::StrLen(asSrc));

	if (iSrcStringLenght == 0) 
		*asDst = NULL;

	if (CSldCompare::StrLen(*asDst) < iSrcStringLenght)
	{
		if (*asDst)
			sldMemFree(*asDst);

		*asDst = (UInt16*)sldMemNew((iSrcStringLenght + 1)*sizeof(UInt16));
		if (!*asDst)
			return eMemoryNotEnoughMemory;
	}

	if (iSrcStringLenght != CSldCompare::StrCopy(*asDst, asSrc))
		return eCommonWrongSizeOfData;

	return eOK;
}