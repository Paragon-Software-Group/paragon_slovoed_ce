#pragma once

#include <map>
#include <string>
#include "sld_Platform.h"

/// Класс для разбора строковых аргументов
class AttributeConverter :
	protected std::map<std::wstring, Int32>
{
public:
	/// Конструктор
	AttributeConverter(const std::wstring &aLocation, const std::wstring &aAttribute);

	/// Добавление новой ассоциации стоки числу
	AttributeConverter& operator<<(std::pair<std::wstring, Int32> aNewPair);

	/// Извлечение значения, ассоциированного со строкой
	bool GetValue(std::wstring aAttribute, Int32 &aValue) const;

private:
	/// имя вызывающего метода
	const std::wstring m_location;

	/// имя разбираемого атрибута
	const std::wstring m_attribute;
};
