#include "AttributeConverter.h"
#include "StringFunctions.h"
#include "Log.h"


/***********************************************************************
* �����������
*
* @param aLocation	- ��� ����������� ������ (������������ ��� ������� �����)
*
* @param aAttribute	- ��� ������������ �������� (������������ ��� ������� �����)
************************************************************************/
AttributeConverter::AttributeConverter(const std::wstring &aLocation, const std::wstring &aAttribute)
	: m_location(aLocation)
	, m_attribute(aAttribute)
{
}

/***********************************************************************
* ���������� ����� ���������� ����� �����
*
* @param aNewPair	- ����, ��������� �� ������ � ���������������� �� ��������
*
* @return ������ �� ������ 'this'. ��������� ��� ����������� ����������������� �������������
************************************************************************/
AttributeConverter& AttributeConverter::operator<<(std::pair<std::wstring, Int32> aNewPair)
{
	(*this)[TagNormilize(aNewPair.first)] = aNewPair.second;
	return *this;
}

/***********************************************************************
* ���������� ��������, ���������������� �� �������
*
* @param aValueStr	- ������, ��� ������� ��������� �������� ��������������� �� ��������
*
* @param aValueInt	- ���������� (������), ���� ���������� �������� ��������� ��������
*
* @return ���� ���������� ��������
************************************************************************/
bool AttributeConverter::GetValue(std::wstring aValueStr, Int32 &aValueInt) const
{
	auto it(this->find(TagNormilize(aValueStr)));
	const bool isMappingExists(it != this->end());
	if (isMappingExists)
		aValueInt = it->second;
	else
		CLogW::InstanceXmlLog().Log(
			L"Error! " + m_location + L" : "
			L"Wrong attribute value for attribute \"" + m_attribute + L"\" : " + aValueStr);

	return isMappingExists;
}
