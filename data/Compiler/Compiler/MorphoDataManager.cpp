#include "MorphoDataManager.h"

#include <algorithm>
#include <iterator>

#include "Log.h"
#include "Tools.h"

/***********************************************************************
* ��������� ���� ����������
*
* @param aLangCode	- ��� ����� ���� ����������
* @param aFilename	- ��� ����� � ����� ����������
*
* @return ��� ������
***********************************************************************/
int CMorphoDataManager::AddMorphoBase(UInt32 aLangCode, const wchar_t* aFilename)
{
	if (!aFilename)
		return ERROR_NULL_POINTER;

	// ��������� ���� ����������
	auto fileData = sld::read_file(aFilename);
	if (fileData.empty())
	{
		sldILog("Error! Can't read morphology base file \"%s\"\n", sld::as_ref(aFilename));
		return ERROR_CANT_OPEN_FILE;
	}

	// �������� Id ���� �� ����� �����
	char morphoId[5] = {0};
	wcstombs(morphoId, aFilename, 4);
	UInt32 morphoIdCode = *((UInt32*)morphoId);

	if (none_of(m_MorphoBases.begin(), m_MorphoBases.end(), [morphoIdCode](const TMorphoData &data){ return (morphoIdCode == data.DictId); }))
	{
		// ��������� ���� ����������
		m_MorphoBases.emplace_back(aLangCode, morphoIdCode, std::move(fileData));
	}
	else
	{
		sldILog("Warning! Duplicated morphology base specified: filename=\"%s\". Ignored.\n", sld::as_ref(aFilename));
	}

	return ERROR_NO;
}

/***********************************************************************
* ���������� ���������� ����������� ��� ����������
*
* @return ���������� ����������� ��� ����������
***********************************************************************/
UInt32 CMorphoDataManager::GetMorphoBasesCount(void) const
{
	return (UInt32)m_MorphoBases.size();
}
