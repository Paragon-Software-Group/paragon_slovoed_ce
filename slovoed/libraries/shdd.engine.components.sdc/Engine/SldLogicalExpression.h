#ifndef _SLD_LOGICAL_EXPRESSION_H_
#define _SLD_LOGICAL_EXPRESSION_H_

#include "SldTypes.h"
#include "SldDynArray.h"
#include "SldStack.h"

/// Класс, позволяющий вычислять логические выражения (методом двух стеков)
class CSldLogicalExpression
{
public:

	/// Базовый класс для операндов логического выражения
	class Operand
	{
	public:
		virtual ~Operand() {}
	};

	/// Класс для операций над операндами логического выражения
	class Operation
	{
	public:
		/// Конструктор
		explicit Operation(ELogicalExpressionTokenTypes aType)
			: m_Type(aType), m_Arity(aType == eTokenType_Operation_NOT ? 1 : 2)
		{}

		/// Возвращает тип операции
		ELogicalExpressionTokenTypes GetType(void) const { return m_Type; }

		/// Возвращает арность операции
		UInt32 GetArity(void) const { return m_Arity; }

		/// Возвращает приоритет операции
		UInt32 GetPriority(void) const { return m_Type; }

	private:

		/// Тип логической операции
		ELogicalExpressionTokenTypes m_Type;

		/// Арность логической операции
		UInt32 m_Arity;
	};

	/// Конструктор
	CSldLogicalExpression(void);
	/// Деструктор
	virtual ~CSldLogicalExpression() {}

public:

	/// Устанавливает исходное логическое выражение
	bool SetExpression(const UInt16* aText);

	/// Вычисляет выражение
	bool Calculate(void);

	/// Возвращает результат вычислений
	const Operand* GetResult(void);

protected:
	/// Создает логический операнд
	virtual sld2::UniquePtr<CSldLogicalExpression::Operand> CreateLogicalOperand(const UInt16 *aText) = 0;

	/// Применяет логическую операцию и сохраняет результат в aOperand1
	virtual bool MakeOperation(CSldLogicalExpression::Operand* aOperand1, CSldLogicalExpression::Operand* aOperand2, const CSldLogicalExpression::Operation *aOperation) = 0;

private:

	/// Подготовка выражения для обработки
	bool PrepareExpression(void);

	/// Проверяет корректность логического выражения
	bool IsValidExpression(void);

	/// Подготовка к вычислению выражения
	bool InitCalculation(void);

	/// Получает следующий токен из выражения
	bool GetNextToken(UInt16* aOutToken, ELogicalExpressionTokenTypes* aOutTokenType);

	/// Добавляет очередную операцию
	bool AddOperation(const Operation &aOperation);

	/// Добавляет очередной операнд
	bool AddOperand(sld2::UniquePtr<Operand> aOperand);

	/// Выполняет очередную операцию
	bool DoOperation(const Operation &aOperation);

private:

	/// Строка с исходным логическим выражением
	sld2::DynArray<UInt16> m_Expression;

	/// Текущая позиция в выражении для чтения
	UInt32 m_ExpressionCurrPos;

	/// Стек операндов логического выражения
	sld2::Stack<sld2::UniquePtr<Operand>> m_Operands;

	/// Стек операций логического выражения
	sld2::Stack<Operation> m_Operations;

protected:
	/// Стек "свободных" операндов логического выражения
	sld2::Stack<sld2::UniquePtr<Operand>> m_FreeOperands;
};

#endif //_SLD_LOGICAL_EXPRESSION_H_
