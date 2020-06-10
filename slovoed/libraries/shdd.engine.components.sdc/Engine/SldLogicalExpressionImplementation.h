#ifndef _I_SLD_LOGICAL_EXPRESSION_IMPLEMENTATION_H_
#define _I_SLD_LOGICAL_EXPRESSION_IMPLEMENTATION_H_

#include "SldLogicalExpression.h"
#include "SldSearchWordResult.h"
#include "SldCompare.h"

namespace sld2 {

/// Базовый класс операнда для поиска в словаре
template <typename SearchResult>
class SearchOperandBase : public CSldLogicalExpression::Operand
{
public:
	/// Конструктор
	SearchOperandBase() : m_IsComputed(false) {}

	SearchOperandBase(const SearchOperandBase&) = delete;
	SearchOperandBase& operator=(const SearchOperandBase&) = delete;

	/// Устанавливает исходное содержимое операнда
	bool SetSourceData(const UInt16* aText)
	{
		if (!aText)
			return false;

		const UInt32 textLen = CSldCompare::StrLen(aText);
		if (textLen == 0)
			return false;

		return m_SearchPattern.assign(aText, textLen + 1);
	}

	/// Возвращает исходное содержимое операнда
	const UInt16* GetSourceData() const { return m_SearchPattern.data(); }

	/// Возвращает указатель на результат полнотекстового поиска
	SearchResult* GetSearchWordResult() const { return (SearchResult*)&m_SearchResult; }

	/// Возвращает был ли произведен поиск
	bool IsComputed() const { return m_IsComputed; }

	/// Устанавливает влаг того что поиск был произведен
	void SetComputed() { m_IsComputed = true; }

	/// Очищает все результаты поиска
	void Clear()
	{
		m_SearchResult.Clear();
		m_IsComputed = false;
	}

private:
	/// Исходная строку для поиска
	sld2::DynArray<UInt16> m_SearchPattern;

	/// Результаты поиска
	SearchResult m_SearchResult;

	/// Флаг был ли произведен поиск
	bool m_IsComputed;
};

template <typename Impl, typename SearchResultT>
class SearchImplementationBase : public CSldLogicalExpression
{
public:
	using SearchResult = SearchResultT;
	using Operand = sld2::SearchOperandBase<SearchResult>;

protected:
	/**
	 * Создает логический операнд
	 *
	 * @param[in] aText - текст операнда
	 *
	 * @return указатель на созданный операнд
	 */
	sld2::UniquePtr<CSldLogicalExpression::Operand> CreateLogicalOperand(const UInt16 *aText) override
	{
		sld2::UniquePtr<Operand> operand;
		if (!m_FreeOperands.empty())
		{
			operand.reset((Operand*)m_FreeOperands.popv().release());
			operand->Clear();
		}
		else
		{
			operand = sld2::make_unique<Operand>();
			if (!operand)
				return nullptr;
		}

		if (!operand->SetSourceData(aText))
			return nullptr;

		return sld2::move(operand);
	}

	/**
	 * Применяет логическую операцию и сохраняет результат в aOperand1
	 *
	 * @param[in][out]	aOperand1	- первый операнд операции
	 * @param[in]		aOperand2	- второй операнд операции
	 * @param[in]		aOperation	- логическая операция
	 *
	 * @return true в случае успеха, false в случае ошибки
	 */
	bool MakeOperation(CSldLogicalExpression::Operand* aOperand1, CSldLogicalExpression::Operand* aOperand2, const Operation *aOperation) override
	{
#define DISPATCH(call) static_cast<Impl*>(this)->call

		// Вычислять значение самих операндов нужно тут, а не при создании операнда, чтобы не пожирать память:
		// в каждый момент времени вычислено должно быть не больше двух операндов. Если же вычислять при создании операндов -
		// может отъесть много памяти, так как операндов в стеке может накопиться достаточно.

		// Нужно просто выполнить поиск
		if (aOperand1 && !aOperand2 && !aOperation)
			return DISPATCH(CalculateResult(aOperand1)) != nullptr;

		// Должен быть хотя бы один операнд и операция
		if (!aOperand1 || !aOperation)
			return false;

		if (aOperation->GetType() == eTokenType_Operation_NOT)
		{
			if (aOperand2)
				return false;

			auto result = DISPATCH(CalculateResult(aOperand1));
			if (result == nullptr)
				return false;

			result->ResultsNOT();
			return true;
		}
		else if (aOperation->GetType() == eTokenType_Operation_AND ||
				 aOperation->GetType() == eTokenType_Operation_OR)
		{
			if (!aOperand2)
				return false;

			auto result1 = DISPATCH(CalculateResult(aOperand1));
			if (result1 == nullptr)
				return false;

			auto result2 = DISPATCH(CalculateResult(aOperand2));
			if (result2 == nullptr)
				return false;

			if (aOperation->GetType() == eTokenType_Operation_AND)
				return result1->ResultsAND(result2) == eOK;
			else
				return result1->ResultsOR(result2) == eOK;
		}

		return false;

#undef DISPATCH
	}
};

} // namespace sld2

/// Класс реализации для поиска по шаблону в словаре
class CWildCardSearchImplementation : public sld2::SearchImplementationBase<CWildCardSearchImplementation, CSldSimpleSearchWordResult>
{
	template <typename, typename> friend class sld2::SearchImplementationBase;
public:
	/// Конструктор
	CWildCardSearchImplementation(void);

	/// Инициализация
	bool Init(ISldList* aSearchList, ISldList* aRealList, Int32 aMaximumNumberOfWordsInRealList, Int32 aRealListIndex);

private:

	/// Производит поиск (если он еще не выполнялся) по переданному операнду
	SearchResult* CalculateResult(CSldLogicalExpression::Operand *aOperand);

	/// Указатель на поисковый список слов, который будет осуществлять поиск
	ISldList* m_SearchList;

	/// Указатель на реальный список слов, в котором будем осуществлять поиск
	ISldList* m_RealList;

	/// Максимальное количество слов в реальном списке слов
	Int32 m_MaximumNumberOfWordsInRealList;

	/// Реальный номер реального списка слов
	Int32 m_RealListIndex;
};


/// Класс реализации для полнотекстового поиска в словаре
class CFullTextSearchImplementation : public sld2::SearchImplementationBase<CFullTextSearchImplementation, CSldSearchWordResult>
{
	template <typename, typename> friend class sld2::SearchImplementationBase;
public:
	/// Конструктор
	CFullTextSearchImplementation(void);

	/// Инициализация
	bool Init(ISldList* aSearchList, ISldList** aRealLists, Int32 aRealListCount, Int32 aRealListIndex, Int32 aMaximumWords);

private:

	/// Производит поиск (если он еще не выполнялся) по переданному операнду
	SearchResult* CalculateResult(CSldLogicalExpression::Operand *aOperand);

	/// Указатель на поисковый список слов, который будет осуществлять поиск
	ISldList* m_SearchList;

	/// Указатель на массив списков словаря
	ISldList** m_RealLists;

	/// Количество списков слов в массиве
	Int32 m_RealListCount;

	/// Реальный номер списка слов полнотекстового поиска
	Int32 m_RealListIndex;

	/// Максимальное количество слов, которое может быть найдено
	Int32 m_MaximumWords;
};

#endif //_I_SLD_LOGICAL_EXPRESSION_IMPLEMENTATION_H_
