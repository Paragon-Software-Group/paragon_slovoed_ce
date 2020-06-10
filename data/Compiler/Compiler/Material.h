#pragma once

#include "PugiXML/pugixml.hpp"

#include "Colour.h"
#include "ImageManager.h"
#include "util.h"

class CMaterial
{
public:

	/// �����������
	CMaterial(void);

public:

	/// �������� ����� �� xml �����
	int LoadFromXML(pugi::xml_node aNode, CImageContainer &aImageContainer, const std::wstring &aImagePath);
	/// ������ ���� ������� �� ���������� ������� �� CImageManager
	int MapNameToGlobalIndex(const AddedFileResourceMap &aImageMap);
	/// ����������� ������
	MemoryBuffer Serialize() const;

protected:

	CColour m_Diffuse, m_Specular, m_Ambient;
	UInt32 m_DiffuseMap, m_SpecularMap, m_NormalMap;
};

