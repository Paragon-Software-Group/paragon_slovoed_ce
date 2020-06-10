#pragma once

#include <map>
#include <string>
#include "sld_Platform.h"

/// ����� ��� ������� ��������� ����������
class AttributeConverter :
	protected std::map<std::wstring, Int32>
{
public:
	/// �����������
	AttributeConverter(const std::wstring &aLocation, const std::wstring &aAttribute);

	/// ���������� ����� ���������� ����� �����
	AttributeConverter& operator<<(std::pair<std::wstring, Int32> aNewPair);

	/// ���������� ��������, ���������������� �� �������
	bool GetValue(std::wstring aAttribute, Int32 &aValue) const;

private:
	/// ��� ����������� ������
	const std::wstring m_location;

	/// ��� ������������ ��������
	const std::wstring m_attribute;
};
