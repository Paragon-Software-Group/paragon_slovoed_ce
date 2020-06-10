#include "SldSceneWrapper.h"

/// Конструктор
CSldSceneWrapper::CSldSceneWrapper(void) : m_BeginData(NULL), m_CurrentData(NULL), m_CurrenNode(0), m_NodeCount(0)
{
}

/// Деструктор
CSldSceneWrapper::~CSldSceneWrapper(void)
{
}

/** *********************************************************************
* Проверка поддержки версии
*
* @param[in]	aVersion	- номер версии
*
* @return код ошибки
************************************************************************/
ESldError CSldSceneWrapper::CheckVersion(UInt32 aVersion)
{
	return eOK;
}

/** *********************************************************************
* Получаем цвет сцены
*
* @param[out]	aAmbient	- освещенность сцены
*
* @param[out]	aBackground	- цвет сцены
*
* @return код ошибки
************************************************************************/
ESldError CSldSceneWrapper::GetColour(CColour* aAmbient, CColour* aBackground)
{
	if (!aAmbient || !aBackground)
		return eMemoryNullPointer;

	if(!m_BeginData)
		return eCommonErrorBase;

	*aAmbient		= (*((UInt32 *)m_BeginData));
	*aBackground	= (*((UInt32 *)m_BeginData + 1));

	return eOK;
}

/** *********************************************************************
* Инициализация данными
*
* @param[in]	aElement	- данные ресурса
*
* @return код ошибки
************************************************************************/
ESldError CSldSceneWrapper::Init(TElement* aElement)
{
	ESldError error = eOK;

	m_CurrentData = aElement->Data;

	// Считываем заголовок TSceneHeader
	m_header.sizeHeader = *((UInt32 *)m_CurrentData);
	m_CurrentData += sizeof(UInt32);

	m_header.version = *((UInt32 *)m_CurrentData);
	m_CurrentData += sizeof(UInt32);

	// Проверка версии
	error = CheckVersion(m_header.version);
	if(error != eOK)
		return error;

	m_header.sizeScene = *((UInt32 *)m_CurrentData);
	m_CurrentData += sizeof(UInt32);

	m_header.hasAnimation = *((UInt32 *)m_CurrentData);
	m_CurrentData += sizeof(UInt32);

	m_CurrentData += (m_header.sizeHeader - 4 * sizeof(UInt32));
	m_BeginData = m_CurrentData;

	m_CurrentData += sizeof(UInt32);
	m_CurrentData += sizeof(UInt32);

	m_NodeCount = *((UInt32 *)m_CurrentData);
	m_CurrentData += sizeof(UInt32);

	return eOK;
}

/// Сбрасываем на начало
ESldError CSldSceneWrapper::Reset()
{
	if(!m_BeginData)
		return eCommonErrorBase;

	m_CurrentData = m_BeginData;

	return eOK;
}

/** *********************************************************************
* Получение количества нодов
*
* @param[out]	aCount	- количество нодов
*
* @return код ошибки
************************************************************************/
ESldError CSldSceneWrapper::GetCountNode(UInt32* aCount)
{
	if(!aCount)
		return eMemoryNullPointer;

	*aCount = m_NodeCount;
	return eOK;
}

/** *********************************************************************
* Получение следующего нода
*
* @param[out]	aNode	- нода
*
* @return код ошибки
************************************************************************/
ESldError CSldSceneWrapper::GetNextNode(TSldSceneNode* aNode)
{
	if(!aNode)
		return eMemoryNullPointer;

	sldMemZero(aNode, sizeof(TSldSceneNode));

	if(m_CurrenNode == m_NodeCount)
		return eMetadataErrorBase;

	aNode->Position			=	*((TPosition*)m_CurrentData);
	m_CurrentData			+=	sizeof(TPosition);

	aNode->Rotation			=	*((TQuaternion*)m_CurrentData);
	m_CurrentData			+=	sizeof(TQuaternion);

	aNode->Scale			=	*((TScale*)m_CurrentData);
	m_CurrentData			+=	sizeof(TScale);

	aNode->MeshIndex		=	*((UInt32*)m_CurrentData);
	m_CurrentData			+=	sizeof(UInt32);

	aNode->CountSubEntity	=	*((UInt32*)m_CurrentData);
	m_CurrentData			+=	sizeof(UInt32);

	aNode->SubEntities		=	(TSubentity*)sldMemNew(aNode->CountSubEntity * sizeof(TSubentity));
	if (!aNode->SubEntities)
		return eMemoryNotEnoughMemory;
	sldMemMove(aNode->SubEntities, m_CurrentData, aNode->CountSubEntity * sizeof(TSubentity));
	m_CurrentData			+=	aNode->CountSubEntity * sizeof(TSubentity);

	if (m_header.hasAnimation)
	{
		aNode->CountAnimations	=	*((UInt32*)m_CurrentData);
		m_CurrentData			+=	sizeof(UInt32);

		aNode->Animations		=	(TAnimation*)sldMemNewZero(aNode->CountAnimations * sizeof(TAnimation));
		if (!aNode->Animations)
			return eMemoryNotEnoughMemory;

		for (Int32 i = 0; i < aNode->CountAnimations; i++)
		{
			// Общая длина анимации
			aNode->Animations[i].length =	*((UInt32*)m_CurrentData);
			m_CurrentData += sizeof(UInt32);

			// Название анимации
			UInt32 name_length = *((UInt32*)m_CurrentData);
			m_CurrentData += sizeof(UInt32);

			sldMemMove(aNode->Animations[i].name, m_CurrentData, name_length * sizeof(UInt16));
			m_CurrentData += name_length * sizeof(UInt16);

			// Число ключевых кадров
			aNode->Animations[i].keyframesCount = *((UInt32*)m_CurrentData);
			m_CurrentData += sizeof(UInt32);

			aNode->Animations[i].keyframes = (TKeyframe*)sldMemNewZero(aNode->Animations[i].keyframesCount * sizeof(TKeyframe));
			if (!aNode->Animations[i].keyframes)
				return eMemoryNotEnoughMemory;

			for (Int32 j = 0; j < aNode->Animations->keyframesCount; j++)
			{
				// Временная отметка кадра
				aNode->Animations[i].keyframes[j].time = *((double*)m_CurrentData);
				m_CurrentData += sizeof(double);

				// Position
				aNode->Animations[i].keyframes[j].pos = *((TPosition*)m_CurrentData);
				m_CurrentData += sizeof(TPosition);

				// Rotation
				aNode->Animations[i].keyframes[j].rotation = *((TQuaternion*)m_CurrentData);
				m_CurrentData += sizeof(TQuaternion);

				// Size
				aNode->Animations[i].keyframes[j].size = *((TScale*)m_CurrentData);
				m_CurrentData += sizeof(TScale);
			}
		}
	}

	m_CurrenNode++;
	return eOK;
}
