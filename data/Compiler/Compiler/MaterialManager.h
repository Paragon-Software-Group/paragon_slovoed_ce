#pragma once

#include "FileResourceManagement.h"
#include "Material.h"

///  ласс дл€ управлени€ 3d материалами
class CMaterialManager
{
public:

	///  онструктор
	CMaterialManager() = default;

	/// ¬озвращает общее количество материалов в глобальном массиве
	UInt32 GetMaterialCount() const { return c.count(); }

	/// ƒобавл€ет материал, использу€ полное им€ материала
	UInt32 AddMaterial(sld::wstring_ref aMaterialName, CMaterial aMaterial);

	/// ¬озвращает глобальный индекс материала по полному имени файла
	UInt32 GetMaterialIndexByFullMaterialName(sld::wstring_ref aFullMaterialName) const { return c.getIndex(aFullMaterialName); }
	/// ¬озвращает полное им€ файла материала по глобальному индексу
	std::wstring GetFullMaterialNameByMaterialIndex(UInt32 aIndex) const { return to_string(c.getName(aIndex)); }

	/// ¬озвращаем сам материал по глобальному индексу
	const CMaterial* GetMaterialByIndex(UInt32 aIndex) const;
	/// «амена локальных индексов текстур на глобальные
	int MapNameToGlobalIndex(const AddedFileResourceMap &aImageMap);

private:
	FileResourceContainer c;

	/// √лобальный массив материалов
	/// —одержит сами материалы и их глобальные индексы
	std::unordered_map<UInt32, CMaterial> m_IndexToMaterial;
};
