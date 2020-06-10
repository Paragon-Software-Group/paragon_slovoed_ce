#ifndef _SLD_SCENE_WRAPPER_H_
#define _SLD_SCENE_WRAPPER_H_

#include "SldSceneManager.h"
#include "Colour.h"
#include "SldError.h"

/// Класс для упрощеной работой с ресурсом типа RESOURCE_TYPE_SCENE
class CSldSceneWrapper
{
public:
	/// Конструктор
	CSldSceneWrapper(void);
	/// Деструктор
	~CSldSceneWrapper(void);

	/// Инициализация данными
	ESldError Init(TElement* aElement);

	/// Получаем цвет сцены
	ESldError GetColour(CColour* aAmbient, CColour* aBackground);

	/// Сбрасываем на начало
	ESldError Reset();

	/// Получение следующего нода
	ESldError GetNextNode(TSldSceneNode* aNode);

	/// Получение количества нодов
	ESldError GetCountNode(UInt32* aCount);

protected:
	/// Проверка поддержки версии
	ESldError CheckVersion(UInt32 aVersion);

protected:
	/// Указатель на начало данных
	UInt8* m_BeginData;
	/// Указатель на текущее положение в памяти
	UInt8* m_CurrentData;
	/// Номер текущего нода
	UInt32 m_CurrenNode;
	/// Количество нодов
	UInt32 m_NodeCount;
	/// Заголовк сцены
	TSceneHeader m_header;
};

#endif // _SLD_SCENE_WRAPPER_H_