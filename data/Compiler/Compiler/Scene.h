#pragma once

#include <string>
#include <vector>

#include "MeshManager.h"
#include "MaterialManager.h"

struct TQuaternion
{
	Float32 x;
	Float32 y;
	Float32 z;
	Float32 w;

	TQuaternion() : x(0), y(0), z(0), w(0) {};
	TQuaternion(Float32 _x, Float32 _y, Float32 _z, Float32 _w) : x(_x), y(_y), z(_z), w(_w) {};

	TQuaternion operator*(TQuaternion& q2) const
	{
		return TQuaternion( w * q2.w - x * q2.x - y * q2.y - z * q2.z,
							w * q2.x + x * q2.w + y * q2.y - z * q2.y,
							w * q2.y - x * q2.z + y * q2.w + z * q2.x,
							w * q2.z + x * q2.y - y * q2.x + z * q2.w);
	}
};

struct TPosition
{
	Float32 x;
	Float32 y;
	Float32 z;

	TPosition() : x(0), y(0), z(0) {};
	TPosition(Float32 _x, Float32 _y, Float32 _z) : x(_x), y(_y), z(_z) {};

	TPosition operator+(TPosition& p2) const
	{
		return TPosition(x + p2.x, y + p2.y, z + p2.z);
	}
};

struct TScale
{
	Float32 x;
	Float32 y;
	Float32 z;

	TScale() : x(1), y(1), z(1) {};
	TScale(Float32 _x, Float32 _y, Float32 _z) : x(_x), y(_y), z(_z) {};

	TScale operator*(TScale& p2) const
	{
		return TScale(x * p2.x, y * p2.y, z * p2.z);
	}
};

struct TSubentity
{
	TSubentity() : index(0), Material(0) {}
	/// Индекс подсущности
	UInt32 index;
	/// Глобальный индекс материала
	UInt32 Material;
};

struct TKeyframe
{
	TKeyframe() : time(0) {}
	/// Время кадра
	double time;
	/// Параметры кадра
	TPosition pos;
	TQuaternion rotation;
	TScale size;
};

struct TAnimation
{
	TAnimation() : length(0) {}
	/// Время анимации
	double length;
	/// Название анимации
	std::wstring name;
	/// Кадры
	std::vector<TKeyframe> keyframes;
};


struct TSceneEntity
{
	TSceneEntity() : mesh(0), numberOfSubentities(0) {};

	UInt32 mesh;
	/// Количество подсущностей у объекта
	UInt32 numberOfSubentities;
	/// Вектор с подсущностями
	std::vector<TSubentity> sceneNodes;
};

struct TSceneNode
{
	TPosition point;
	TQuaternion rotation;
	TScale nodeScale;
	TSceneEntity entity;
	std::vector<TAnimation> animations;

	TSceneNode() {};
	TSceneNode(TPosition p, TQuaternion q, TScale s) : point(p), rotation(q), nodeScale(s) {};
	TSceneNode(const TSceneNode& aRef)
	{
		point = aRef.point;
		rotation = aRef.rotation;
		nodeScale = aRef.nodeScale;
		entity = aRef.entity;
		animations = aRef.animations;
	};

	TSceneNode operator*(TSceneNode& child) const
	{
		return TSceneNode(point + child.point, rotation * child.rotation, nodeScale * child.nodeScale);
	}
};

struct TSceneHeader
{
	/// Размер структуры
	UInt32 sizeHeader;
	/// Версия заголовка
	UInt32 version;
	/// Размер бинаризованной сцены
	UInt32 sizeScene;

	UInt32 hasAnimation;

	/// Зарезервировано
	UInt32 Reserved[4];
};

struct TSceneEnviroment
{
	CColour colourAmbient;

	CColour colourBackground;

};

class CScene
{
public:
	CScene(void);

	/// Сериализует объект
	MemoryBuffer Serialize() const;

	/// Загружаем объект из файла xml
	void LoadFromXML(std::wstring aFullSceneName, CMeshManager& aMeshManager, CMaterialManager& aMaterialManager);
protected:
	/// Заголовок класса сцены
	TSceneHeader m_header;
	/// Описание окружения сцены
	TSceneEnviroment m_enviroment;
	/// Количество нодов в сцене
	UInt32	m_numberOfNodes;
	/// Вектор с самими нодами
	std::vector<TSceneNode> m_sceneNodes;

};

