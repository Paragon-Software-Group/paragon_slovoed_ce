#ifndef _SLD_MORPHOLOGY_H_
#define _SLD_MORPHOLOGY_H_

#include "SldError.h"
#include "SldDefines.h"
#include "SldPlatform.h"
#include "SldTypes.h"
#include "SldSDCReadMy.h"
#include "SldDynArray.h"
#include "Morphology/MorphoData.h"

class CSldMorphology
{
public:
	/// Инициализация
	ESldError Init(CSDCReadMy &aData);

	/// Инициализация базы морфологии
	ESldError InitMorphology(ISDCFile *aFileData, ISldLayerAccess * aLayerAcces, UInt32 aIndex, UInt32 aShift, UInt32 aSize);

	/// Получает указатель на базу морфологии
	MorphoData* GetMorphologyByIndex(UInt32 aIndex);

	/// Проверяет инициализирована ли база морфологии
	bool IsMorphologyInit(UInt32 aIndex) const;

	/// Получает индекс (порядковый номер) ресурса морфологии в словаре, которая соответствует заданным коду языка и id базы
	Int32 GetResourceIndexByLangCode(UInt32 aLanguageCode, UInt32 aDictId) const;

private:

	/// Структура со всеми данными базы морфолгии
	struct Morpho {
		/// указатель на базу
		sld2::UniquePtr<MorphoData> data;
		/// указатель на класс чтения данных
		sld2::UniquePtr<ISDCFile> file;
		/// информация о базе
		TMorphoBaseElement info;
	};

	/// Массив баз морфологии
	sld2::DynArray<Morpho> m_Morphology;

};

#endif //_SLD_MORPHOLOGY_H_