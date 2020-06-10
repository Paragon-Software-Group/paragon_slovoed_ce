#include "SldLogicalExpressionImplementation.h"

#include "SldSearchList.h"

/// Конструктор
CWildCardSearchImplementation::CWildCardSearchImplementation(void)
	: m_SearchList(nullptr), m_RealList(nullptr),
	  m_MaximumNumberOfWordsInRealList(0), m_RealListIndex(0)
{}

/** *********************************************************************
* Инициализация
*
* @param[in]	aSearchList						- указатель на список, куда будут помещаться результаты
* @param[in]	aRealList						- указатель на список, в котором производится поиск
* @param[in]	aMaximumNumberOfWordsInRealList	- максимальное количество результатов поиска
* @param[in]	aRealListIndex					- номер списка, в котором производим поиск (среди всех списков словаря)
*
* @return true в случае успеха, false в случае ошибки
************************************************************************/
bool CWildCardSearchImplementation::Init(ISldList* aSearchList, ISldList* aRealList, Int32 aMaximumNumberOfWordsInRealList, Int32 aRealListIndex)
{
	if (!aSearchList || !aRealList)
		return false;

	m_SearchList = aSearchList;
	m_RealList = aRealList;
	m_MaximumNumberOfWordsInRealList = aMaximumNumberOfWordsInRealList;
	m_RealListIndex = aRealListIndex;

	return true;
}

// Производит поиск (если он еще не выполнялся) по переданному операнду
// Возвращает результат поиска или nullptr в случае ошибки
CSldSimpleSearchWordResult* CWildCardSearchImplementation::CalculateResult(CSldLogicalExpression::Operand *aOperand)
{
	auto operand = (Operand*)aOperand;
	auto result = operand->GetSearchWordResult();
	// если результат операнда еще не вычислен (поиск не производился)
	if (!operand->IsComputed())
	{
		if (!result->IsInit())
		{
			if (result->Init(m_MaximumNumberOfWordsInRealList, m_RealListIndex) != eOK)
				return nullptr;
		}

		ESldError error = ((CSldSearchList*)m_SearchList)->DoWordWildCardSearch(operand->GetSourceData(), m_RealList, m_RealListIndex, result);
		if (error != eOK)
			return nullptr;

		operand->SetComputed();
	}
	return result;
};

/// Конструктор
CFullTextSearchImplementation::CFullTextSearchImplementation(void)
	: m_SearchList(nullptr), m_RealLists(nullptr), m_RealListCount(0),
	  m_RealListIndex(0), m_MaximumWords(0)
{}

/** *********************************************************************
* Инициализация
*
* @param[in]	aSearchList		- указатель на список, куда будут помещаться результаты
* @param[in]	aRealLists		- указатель на массив списков словаря
* @param[in]	aRealListCount	- количество списков в словаре
* @param[in]	aRealListIndex	- номер списка, в котором производим поиск (среди всех списков словаря)
* @param[in]	aMaximumWords	- максимальное количество результатов поиска
*
* @return true в случае успеха, false в случае ошибки
************************************************************************/
bool CFullTextSearchImplementation::Init(ISldList* aSearchList, ISldList** aRealLists, Int32 aRealListCount, Int32 aRealListIndex, Int32 aMaximumWords)
{
	if (!aSearchList || !aRealLists)
		return false;

	m_SearchList = aSearchList;
	m_RealLists = aRealLists;
	m_RealListCount = aRealListCount;
	m_RealListIndex = aRealListIndex;
	m_MaximumWords = aMaximumWords;

	return true;
}

// Производит поиск (если он еще не выполнялся) по переданному операнду
// Возвращает результат поиска или nullptr в случае ошибки
CSldSearchWordResult* CFullTextSearchImplementation::CalculateResult(CSldLogicalExpression::Operand *aOperand)
{
	auto operand = (Operand*)aOperand;
	auto result = operand->GetSearchWordResult();
	// если результат операнда еще не вычислен (поиск не производился)
	if (!operand->IsComputed())
	{
		if (!result->IsInit())
		{
			if (result->Init(m_RealLists, m_RealListCount) != eOK)
				return nullptr;
		}

		ESldError error = ((CSldSearchList*)m_SearchList)->DoWordFullTextSearch(operand->GetSourceData(), m_MaximumWords, m_RealLists, m_RealListCount, m_RealListIndex, result);
		if (error != eOK)
			return nullptr;

		operand->SetComputed();
	}
	return result;
};
