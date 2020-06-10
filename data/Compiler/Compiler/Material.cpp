#include "Material.h"

#include <sstream>
#include <assert.h>

#include "sld2_Error.h"
#include "Log.h"

using namespace std;

/***********************************************************************
* Конструктор
************************************************************************/
CMaterial::CMaterial(void) 
	: m_DiffuseMap(0), m_SpecularMap(0), m_NormalMap(0)
{
}

/***********************************************************************
* Загрузка файла из xml файла
************************************************************************/
int CMaterial::LoadFromXML(pugi::xml_node aNode, CImageContainer &aImageContainer, const std::wstring &aImagePath)
{
	const struct {
		const wchar_t *name;
		CColour *ptr;
	} colors[] = {
		{ L"ambient", &m_Ambient },
		{ L"diffuse", &m_Diffuse },
		{ L"specular", &m_Specular },
	};
	for (auto&& color : colors)
	{
		pugi::xml_node node = aNode.child(color.name);
		if (node)
		{
			color.ptr->setRed(node.attribute(L"r").as_uint());
			color.ptr->setGreen(node.attribute(L"g").as_uint());
			color.ptr->setBlue(node.attribute(L"b").as_uint());
			color.ptr->setAlpha(node.attribute(L"a").as_uint());
		}
	}

	pugi::xml_node nodeTextures = aNode.child(L"texture");
	if(nodeTextures.empty())
		return ERROR_NO;

	const struct {
		const wchar_t *name;
		UInt32 *ptr;
	} textures[] = {
		{ L"diffuse", &m_DiffuseMap },
		{ L"specular", &m_SpecularMap },
		{ L"normal", &m_NormalMap },
	};
	for (auto&& texture : textures)
	{
		pugi::xml_node node = nodeTextures.child(texture.name);
		if (node)
		{
			std::wstring name = aImagePath + node.attribute(L"name").value();
			*(texture.ptr) = aImageContainer.AddImage(name);
		}
	}

	return ERROR_NO;
}

/***********************************************************************
* Замена имен текстур на глобальные индексы из CImageManager
************************************************************************/
int CMaterial::MapNameToGlobalIndex(const AddedFileResourceMap &aImageMap)
{
	m_DiffuseMap = aImageMap[m_DiffuseMap];
	m_SpecularMap = aImageMap[m_SpecularMap];
	m_NormalMap = aImageMap[m_NormalMap];
	return ERROR_NO;
}

/***********************************************************************
* Сериализует объект
************************************************************************/
MemoryBuffer CMaterial::Serialize() const
{
	MemoryBuffer buf(3 * sizeof(CColour) + 3 * sizeof(UInt32));
	uint8_t *buffer = buf.data();

	memcpy(buffer, &m_Diffuse, sizeof(CColour));
	buffer += sizeof(CColour);
	memcpy(buffer, &m_Specular, sizeof(CColour));
	buffer += sizeof(CColour);
	memcpy(buffer, &m_Ambient, sizeof(CColour));
	buffer += sizeof(CColour);

	memcpy(buffer, &m_DiffuseMap, sizeof(UInt32));
	buffer += sizeof(UInt32);
	memcpy(buffer, &m_SpecularMap, sizeof(UInt32));
	buffer += sizeof(UInt32);
	memcpy(buffer, &m_NormalMap, sizeof(UInt32));
	buffer += sizeof(UInt32);

	return buf;
}
