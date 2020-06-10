#include "AttributeConverter.h"
#include "StringFunctions.h"
#include "Log.h"


/***********************************************************************
*  онструктор
*
* @param aLocation	- им€ вызывающего метода (используетс€ дл€ ведени€ логов)
*
* @param aAttribute	- им€ разбираемого атрибута (используетс€ дл€ ведени€ логов)
************************************************************************/
AttributeConverter::AttributeConverter(const std::wstring &aLocation, const std::wstring &aAttribute)
	: m_location(aLocation)
	, m_attribute(aAttribute)
{
}

/***********************************************************************
* ƒобавление новой ассоциации стоки числу
*
* @param aNewPair	- пара, состо€ща€ из строки и ассоциированного ей значени€
*
* @return ссылка на объект 'this'. “ребуетс€ дл€ возможности последовательного использовани€
************************************************************************/
AttributeConverter& AttributeConverter::operator<<(std::pair<std::wstring, Int32> aNewPair)
{
	(*this)[TagNormilize(aNewPair.first)] = aNewPair.second;
	return *this;
}

/***********************************************************************
* »звлечение значени€, ассоциированного со строкой
*
* @param aValueStr	- строка, дл€ которой требуетс€ получить ассоциированное ей значение
*
* @param aValueInt	- переменна€ (ссылка), куда необходимо записать найденное значение
*
* @return флаг успешности операции
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
