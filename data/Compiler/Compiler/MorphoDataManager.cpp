#include "MorphoDataManager.h"

#include <algorithm>
#include <iterator>

#include "Log.h"
#include "Tools.h"

/***********************************************************************
* Добавляет базу морфологии
*
* @param aLangCode	- код языка базы морфологии
* @param aFilename	- имя файла с базой морфологии
*
* @return код ошибки
***********************************************************************/
int CMorphoDataManager::AddMorphoBase(UInt32 aLangCode, const wchar_t* aFilename)
{
	if (!aFilename)
		return ERROR_NULL_POINTER;

	// Загружаем базу морфологии
	auto fileData = sld::read_file(aFilename);
	if (fileData.empty())
	{
		sldILog("Error! Can't read morphology base file \"%s\"\n", sld::as_ref(aFilename));
		return ERROR_CANT_OPEN_FILE;
	}

	// получаем Id базы из имени файла
	char morphoId[5] = {0};
	wcstombs(morphoId, aFilename, 4);
	UInt32 morphoIdCode = *((UInt32*)morphoId);

	if (none_of(m_MorphoBases.begin(), m_MorphoBases.end(), [morphoIdCode](const TMorphoData &data){ return (morphoIdCode == data.DictId); }))
	{
		// Добавляем базу морфологии
		m_MorphoBases.emplace_back(aLangCode, morphoIdCode, std::move(fileData));
	}
	else
	{
		sldILog("Warning! Duplicated morphology base specified: filename=\"%s\". Ignored.\n", sld::as_ref(aFilename));
	}

	return ERROR_NO;
}

/***********************************************************************
* Возвращает количество загруженных баз морфологии
*
* @return количество загруженных баз морфологии
***********************************************************************/
UInt32 CMorphoDataManager::GetMorphoBasesCount(void) const
{
	return (UInt32)m_MorphoBases.size();
}
