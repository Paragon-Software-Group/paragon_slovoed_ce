#include "SldVideoElement.h"

/** *********************************************************************
* Конструктор объекта по-умолчанию
************************************************************************/
CSldVideoElement::CSldVideoElement()
	: m_iIndex(SLD_INDEX_VIDEO_NO)
	, m_pData(0)
	, m_iSize(0)
{
}

/** *********************************************************************
* Конструктор объекта, позволяющий создать объект с корректными данными
*
*@param[in] aiIndex	- идентификатор видеоэлемента
*@param[in] apData	- данные, представляющие видеоэлемент
*@param[in] aiSize	- размер данных видеоэлемента
************************************************************************/
CSldVideoElement::CSldVideoElement(const UInt32 aiIndex, const UInt8 *const apData, const UInt32 aiSize)
{
	// Очищаемся перед заполнением
	Clear();

	// Копируем данные
	m_pData = (UInt8*)sldMemNew(aiSize);
	if (m_pData)
	{
		m_iIndex = aiIndex;
		sldMemMove(m_pData, apData, aiSize);
		m_iSize = aiSize;
	}
}

/** *********************************************************************
* Метод, определяющий корректность хранимых данных
*
* @return '0' в случае некорректных данных
************************************************************************/
UInt8 CSldVideoElement::IsValid() const
{
	return m_iIndex && m_pData && m_iSize;
}
