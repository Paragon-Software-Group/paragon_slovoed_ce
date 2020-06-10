#include "SldVideoItem.h"

#include "SldCompare.h"
#include "SldMetadataParser.h"

/** *********************************************************************
* Конструктор объекта
*************************************************************************/
CSldVideoItem::CSldVideoItem()
	: m_videoId(SLD_INDEX_VIDEO_NO)
	, m_sExtDict(NULL)
	, m_iExtListIdx(SLD_INDEX_VIDEO_NO)
	, m_sExtKey(NULL)
	, m_videoResolution(0)
	, m_videoCodec(eVideoType_Unknown)
{
}

/** *********************************************************************
* Деструктор объекта
************************************************************************/
CSldVideoItem::~CSldVideoItem()
{
	Clear(false);
}

/** *********************************************************************
* Конструктор копирования
*
* @param[in] aVideoType	- объект, который необходимо скопировать
************************************************************************/
CSldVideoItem::CSldVideoItem(const CSldVideoItem &aVideoType)
	: m_sExtDict(NULL), m_sExtKey(NULL)
{
	*this = aVideoType;
}

/** *********************************************************************
* Очистка объекта
*************************************************************************/
void CSldVideoItem::Clear(const bool isPureClear)
{
	sldMemFree(m_sExtKey);
	sldMemFree(m_sExtDict);

	if (isPureClear)
		(*this) = CSldVideoItem();
}

/** *********************************************************************
* Метод, определяющий корректность хранимых данных
*
* @return '0' в случае некорректных данных
************************************************************************/
UInt8 CSldVideoItem::IsValid() const
{
	UInt8 hasExternalLink = CSldCompare::StrLen(m_sExtDict) && m_iExtListIdx != SLD_DEFAULT_LIST_INDEX && CSldCompare::StrLen(m_sExtKey);

	return (m_videoCodec != eVideoType_Unknown && m_videoResolution > 0 && (m_videoId != SLD_INDEX_VIDEO_NO || hasExternalLink));
}

/** *********************************************************************
* Установка идентификатора видео из строки
*
* @param[in] asVideoId	- строка, содержащая идентификатор видео
*
* @return код ошибки
************************************************************************/
ESldError CSldVideoItem::SetVideoId(const UInt16 *const asVideoId)
{
	return CSldCompare::StrToInt32(asVideoId, 16, &m_videoId);
}

/** *********************************************************************
* Возвращает идентификатор внешнего словаря
*
* @return идентификатор внешнего словаря
************************************************************************/
const UInt16* CSldVideoItem::GetExtDict() const
{
	return m_sExtDict;
}

/** *********************************************************************
* Устанавливает идентификатор внешнего словаря
*
* @param[in] aiExtDictId	- идентификатор внешнего словаря
************************************************************************/
ESldError CSldVideoItem::SetExtDict(const UInt16* asExtDict)
{
	return ReplaceString(&m_sExtDict, asExtDict);
}


/** *********************************************************************
* Возвращает идентификатор списка во внешнем словаре
*
* @return идентификатор списка во внешнем словаре
************************************************************************/
Int32 CSldVideoItem::GetExtListIdx() const
{
	return m_iExtListIdx;
}

/** *********************************************************************
* Устанавливает идентификатор списка во внешнем словаре
*
* @param[in] aiExtListIdx	- идентификатор списка во внешнем словаре
************************************************************************/
void CSldVideoItem::SetExtListIdx(const Int32 aiExtListIdx)
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
ESldError CSldVideoItem::SetExtListIdx(const UInt16 *const asExtListIdx)
{
	return CSldCompare::StrToInt32(asExtListIdx, 10, &m_iExtListIdx);
}

/** *********************************************************************
* Возвращает ключ (слово) в списке внешнего словаря
*
* @return ключ (слово) в списке внешнего словаря
************************************************************************/
const UInt16 *CSldVideoItem::GetExtKey() const
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
ESldError CSldVideoItem::SetExtKey(const UInt16 *const asExtKey)
{
	return ReplaceString(&m_sExtKey, asExtKey);
}

/** *********************************************************************
* Установка кодека видеоконтента из строки
*
* @param[in] aVideoCodecStr	- строка, содержащая идентификатор кодека
*
* @return код ошибки
************************************************************************/
ESldError CSldVideoItem::SetVideoCodec(const UInt16 *const aVideoCodecStr)
{
	ESldError eError(eOK);
	Int32 iCodecType(eVideoType_Unknown);

	if (eOK == (eError = CSldCompare::
		StrToInt32(aVideoCodecStr, 10, &iCodecType)))
			m_videoCodec = (ESldVideoType)iCodecType;

	return eError;
}

/** *********************************************************************
* Определение размера видеоконтена из стоки
*
* @param[in] asVideoResolution	- строка, содержащая размер видеоконтента (в пикселах)
*
* @return код ошибки
************************************************************************/
ESldError CSldVideoItem::SetVideoResolution(const UInt16 *const asVideoResolution)
{
	return CSldCompare::StrToInt32(asVideoResolution, 10, &m_videoResolution);
}

/** *********************************************************************
* Оператор присваивания
*
* @param[in] videoType	- объект, который необходимо склонировать в текущем объекте
*
* @return ссылка на текущий объект
************************************************************************/
CSldVideoItem& CSldVideoItem::operator=(const CSldVideoItem &videoType)
{
	SetVideoId			(videoType.GetVideoId());
	SetVideoCodec		(videoType.GetVideoCodec());
	SetVideoResolution	(videoType.GetVideoResolution());
	SetExtDict			(videoType.GetExtDict());
	SetExtListIdx		(videoType.GetExtListIdx());
	SetExtKey			(videoType.GetExtKey());

	return *this;
}

/** *********************************************************************
* Заменяет старое значение строки на новое (через удаление, при необходимости, старой)
*
* @param[in] asDst	- указатель на строку, на место которой надо поместить новую строчку
* @param[in] asSrc	- строчка, которую необходимо сохранить
*
* @return код ошибки
************************************************************************/
ESldError CSldVideoItem::ReplaceString(UInt16 **const asDst, const UInt16 *const asSrc)
{
	const Int32 iSrcStringLenght(CSldCompare::StrLen(asSrc));

	if (CSldCompare::StrLen(*asDst) < iSrcStringLenght)
	{
		sldMemFree(*asDst);
		*asDst = (UInt16*)sldMemNew(sizeof(UInt16) * (iSrcStringLenght + 1));
		if (!*asDst)
			return eMemoryNullPointer;
	}

	if (iSrcStringLenght !=
		CSldCompare::StrCopy(*asDst, asSrc))
		return eCommonWrongSizeOfData;

	return eOK;
}