#include "SldLogicalExpression.h"

#include "SldCompare.h"

namespace {

/// Символы, представляющие логические операции в выражении
enum EOperationTokens
{
	eOperationToken_AND = '&',
	eOperationToken_OR = '|',
	eOperationToken_NOT = '!',
	eOperationToken_OPEN_BR = '(',
	eOperationToken_CLOSE_BR = ')'
};

} // anon namespace

/// Конструктор
CSldLogicalExpression::CSldLogicalExpression(void)
	: m_ExpressionCurrPos(0)
{}

/** *********************************************************************
* Устанавливает исходное логическое выражение
*
* @param[in]	aText	- текст логического выражения
*
* @return true в случае успеха, иначе false
************************************************************************/
bool CSldLogicalExpression::SetExpression(const UInt16* aText)
{
	if (!aText)
		return false;

	m_ExpressionCurrPos = 0;

	const UInt32 len = CSldCompare::StrLen(aText);
	if (!len)
		return false;

	if (!m_Expression.resize(len + 3))
		return false;

	sld2::memcopy_n(m_Expression.data(), aText, len);

	return true;
}

/** *********************************************************************
* Подготовка выражения для обработки.
* Исправляет возможные опечатки и добавляет скобки по краям выражения - смысл не меняет, но облегчает разбор.
* Считается, что выражение уже является логическим (то есть, например, замен пробелов на & и добавления * не происходит).
* Пытается исправить только возможные опечатки в логических операциях.
*
* @return true в случае успешного исправления, иначе false
************************************************************************/
bool CSldLogicalExpression::PrepareExpression(void)
{
	auto preparedExpression = m_Expression;
	if (preparedExpression.empty())
		return false;

	const UInt16* src = m_Expression.data();
	UInt16* dst = preparedExpression.data() + 1;

	// Корректируем начало выражения
	while (*src && (*src==' ' || *src==eOperationToken_AND || *src==eOperationToken_OR || *src==eOperationToken_CLOSE_BR))
		src++;

	CSldCompare::StrCopy(dst, src);

	// Корректируем конец выражения
	Int32 destLen = CSldCompare::StrLen(dst);
	while (destLen)
	{
		if (dst[destLen-1] && (dst[destLen-1]==' ' || dst[destLen-1]==eOperationToken_AND || dst[destLen-1]==eOperationToken_OR || dst[destLen-1]==eOperationToken_NOT || dst[destLen-1]==eOperationToken_OPEN_BR))
		{
			dst[destLen-1] = 0;
			destLen--;
			continue;
		}
		break;
	}

	// Удаляем пробелы между операциями
	Int32 outpos = 0;
	Int32 currpos = 0;
	destLen = CSldCompare::StrLen(dst);
	while (outpos < destLen-1)
	{
		if (dst[outpos] && (dst[outpos]==eOperationToken_AND || dst[outpos]==eOperationToken_OR || dst[outpos]==eOperationToken_NOT || dst[outpos]==eOperationToken_OPEN_BR || dst[outpos]==eOperationToken_CLOSE_BR))
		{
			outpos++;
			currpos = outpos;
			while (dst[currpos] && dst[currpos]==' ')
				currpos++;

			if (!dst[currpos])
			{
				dst[outpos] = 0;
				break;
			}
			else if (dst[currpos]==eOperationToken_AND || dst[currpos]==eOperationToken_OR ||
					dst[currpos]==eOperationToken_NOT || dst[currpos]==eOperationToken_OPEN_BR ||
					dst[currpos]==eOperationToken_CLOSE_BR)
			{
				if (currpos==outpos)
					continue;

				// move
				sld2::memmove_n(&dst[outpos], &dst[currpos], destLen - currpos + 1);
				destLen -= (currpos-outpos);
				continue;
			}
			else
				outpos = currpos;
		}
		outpos++;
	}

	// Добавляем открывающую скобку
	preparedExpression[0] = eOperationToken_OPEN_BR;
	destLen = CSldCompare::StrLen(preparedExpression.data());
	// Добавляем закрывающую скобку
	preparedExpression[destLen] = eOperationToken_CLOSE_BR;
	preparedExpression[destLen+1] = 0;

	// Исправляем дублирование операций
	Int32 i = 0;
	destLen = CSldCompare::StrLen(preparedExpression.data());
	while (i < destLen-1)
	{
		while (preparedExpression[i] && (preparedExpression[i]==eOperationToken_AND || preparedExpression[i]==eOperationToken_OR || preparedExpression[i]==eOperationToken_NOT))
		{
			while (preparedExpression[i+1] && (preparedExpression[i+1]==eOperationToken_AND || preparedExpression[i+1]==eOperationToken_OR))
			{
				sld2::memmove_n(&preparedExpression[i + 1], &preparedExpression[i + 2], destLen - i - 1);
				destLen--;
			}
			if (preparedExpression[i+1]==eOperationToken_CLOSE_BR)
			{
				sld2::memmove_n(&preparedExpression[i], &preparedExpression[i + 1], destLen - i);
				destLen--;
				i--;
				continue;
			}
			i++;
		}
		i++;
	}

	m_Expression = sld2::move(preparedExpression);

	return IsValidExpression();
}


/** *********************************************************************
* Проверяет корректность логического выражения
*
* @return true, если выражение корректно, иначе false
************************************************************************/
bool CSldLogicalExpression::IsValidExpression(void)
{
	if (m_Expression.empty())
		return false;

	Int32 exprLen = CSldCompare::StrLen(m_Expression.data());
	if (!exprLen)
		return false;

	Int32 i = 0;

	// Проверяем сбалансированность открывающих и закрывающих скобок
	Int32 openBrCount = 0;
	Int32 closeBrCount = 0;
	for (i=0;i<exprLen;i++)
	{
		if (m_Expression[i] == eOperationToken_OPEN_BR)
			openBrCount++;
		else if (m_Expression[i] == eOperationToken_CLOSE_BR)
			closeBrCount++;
	}

	return openBrCount == closeBrCount;
}

/** *********************************************************************
* Подготовка к вычислению выражения (инициализация параметров)
*
* @return true в случае успешной инициализации, иначе false
************************************************************************/
bool CSldLogicalExpression::InitCalculation(void)
{
	m_Operands.clear();
	m_Operations.clear();

	m_ExpressionCurrPos = 0;

	return PrepareExpression();
}

static ELogicalExpressionTokenTypes to_token_type(EOperationTokens aToken)
{
	switch (aToken)
	{
	case eOperationToken_AND: return eTokenType_Operation_AND;
	case eOperationToken_OR:  return eTokenType_Operation_OR;
	case eOperationToken_NOT: return eTokenType_Operation_NOT;
	case eOperationToken_OPEN_BR: return eTokenType_Operation_OPEN_BR;
	case eOperationToken_CLOSE_BR: return eTokenType_Operation_CLOSE_BR;
	default: break;
	}
	return eTokenType_Unknown;
}

/** *********************************************************************
* Получает следующий токен из выражения
*
* @param[out]	aOutToken		- указатель, по которому сохраняется токен
* @param[out]	aOutTokenType	- указатель, по которому сохраняется тип токена
*
* @return true в случае успешного получения токена, иначе 0
************************************************************************/
bool CSldLogicalExpression::GetNextToken(UInt16* aOutToken, ELogicalExpressionTokenTypes* aOutTokenType)
{
	if (!aOutToken)
		return false;

	if (!aOutTokenType)
		return false;

	*aOutToken = 0;
	*aOutTokenType = eTokenType_Operand;

	/*
	while (m_Expression[m_ExpressionCurrPos] && m_Expression[m_ExpressionCurrPos]==' ')
		m_ExpressionCurrPos++;
	*/

	if (!m_Expression[m_ExpressionCurrPos])
		return false;

	UInt32 outTokenPos = 0;
	while (m_Expression[m_ExpressionCurrPos])
	{
		const UInt16 token = m_Expression[m_ExpressionCurrPos];
		const auto tokenType = to_token_type((EOperationTokens)token);
		if (tokenType != eTokenType_Unknown)
		{
			if (!outTokenPos)
			{
				aOutToken[outTokenPos++] = token;
				*aOutTokenType = tokenType;
				m_ExpressionCurrPos++;
			}
			break;
		}

		aOutToken[outTokenPos++] = token;
		m_ExpressionCurrPos++;
	}

	aOutToken[outTokenPos] = 0;

	return true;
}


/** *********************************************************************
* Добавляет очередную операцию.
* При этом, если достаточно операндов, происходит вычисление результата операции
* (отдельной части логического выражения)
*
* @param[in]	aOperation		- указатель на добавляемую операцию
*
* @return true в случае успешного добавления, иначе false
************************************************************************/
bool CSldLogicalExpression::AddOperation(const Operation &aOperation)
{
	// Открывающая скобка - просто добавляем
	if (aOperation.GetType() == eTokenType_Operation_OPEN_BR)
	{
		m_Operations.push(aOperation);
		return true;
	}

	// Закрывающая скобка - выталкиваем все операции до первой открывающей скобки включительно
	if (aOperation.GetType() == eTokenType_Operation_CLOSE_BR)
	{
		while (m_Operations.size())
		{
			Operation topOperation = m_Operations.popv();
			if (topOperation.GetType() == eTokenType_Operation_OPEN_BR)
				break;

			if (!DoOperation(topOperation))
				return false;
		}

		// Если мы заканчиваем обработку подвыражения такого вида: !(a+b), то после выталкивания открывающей скобки
		// нужно еще раз добавить последний добавленный операнд, чтобы обработалась унарная операция отрицания (!).
		// В остальных случаях это не повлияет на результат.
		if (m_Operands.size())
		{
			if (!AddOperand(m_Operands.popv()))
				return false;
		}

		return true;
	}

	// Если арность операции равна 1 (операция требует операнд справа) - просто добавляем ее
	if (aOperation.GetArity() == 1)
	{
		m_Operations.push(aOperation);
		return true;
	}

	// Если для новой операции не хватает операндов или нет операций для выталкивания - просто добавляем новую операцию
	if (m_Operands.size() < aOperation.GetArity() || m_Operations.empty())
	{
		m_Operations.push(aOperation);
		return true;
	}

	// Выталкиваем все операции с низшим или равным приоритетом
	while (m_Operations.size())
	{
		if (m_Operations.top().GetPriority() > aOperation.GetPriority())
			break;

		if (!DoOperation(m_Operations.popv()))
			return false;
	}

	m_Operations.push(aOperation);
	return true;
}

/** *********************************************************************
* Добавляет очередной операнд
*
* @param[in]	aOperand		- указатель на добавляемый операнд
*
* @return true в случае успешного добавления, иначе false
************************************************************************/
bool CSldLogicalExpression::AddOperand(sld2::UniquePtr<Operand> aOperand)
{
	if (!aOperand)
		return false;

	m_Operands.push(sld2::move(aOperand));

	if (m_Operations.empty())
		return true;

	if (m_Operations.top().GetArity() == 2)
		return true;

	const Operation operation = m_Operations.popv();
	if (operation.GetArity() != 1)
		return false;

	auto operand = m_Operands.popv();
	if (!MakeOperation(operand.get(), NULL, &operation))
		return false;

	return AddOperand(sld2::move(operand));
}

/**
 * Выполняет очередную операцию
 *
 * @param[in]  aOperation  - указатель на добавляемый операнд
 *
 * @return true в случае успешного выполнения, иначе false
 */
bool CSldLogicalExpression::DoOperation(const Operation &aOperation)
{
	sld2::UniquePtr<Operand> operand1, operand2;
	if (m_Operands.size())
		operand1 = m_Operands.popv();

	if (aOperation.GetArity() == 2 && m_Operands.size())
		operand2 = m_Operands.popv();

	if (!MakeOperation(operand1.get(), operand2.get(), &aOperation))
		return false;

	m_FreeOperands.push(sld2::move(operand2));

	return AddOperand(sld2::move(operand1));
}


/** *********************************************************************
* Вычисляет выражение (в итоге в стеке остается 1 операнд с результатом выражения)
*
* @return true в случае успеха, иначе false (ошибка разбора/ошибка синтаксиса/неизвестная ошибка)
************************************************************************/
bool CSldLogicalExpression::Calculate(void)
{
	if (!InitCalculation())
		return false;

	// Строка для хранения очередного токена
	sld2::DynArray<UInt16> exprToken(m_Expression.size());
	if (m_Expression.size() && exprToken.empty())
		return false;

	// Тип очередного токена
	ELogicalExpressionTokenTypes tokenType = eTokenType_Unknown;

	while (GetNextToken(exprToken.data(), &tokenType))
	{
		switch (tokenType)
		{
			case eTokenType_Operand:
			{
				auto newOperand = CreateLogicalOperand(exprToken.data());
				if (!newOperand)
					return false;

				if (!AddOperand(sld2::move(newOperand)))
					return false;

				break;
			}
			case eTokenType_Operation_AND:
			case eTokenType_Operation_OR:
			case eTokenType_Operation_NOT:
			case eTokenType_Operation_OPEN_BR:
			case eTokenType_Operation_CLOSE_BR:
			{
				if (!AddOperation(Operation(tokenType)))
					return false;

				break;
			}
			default:
				return false;
		}
	}

	// Если все ок, то стек операций должен быть пустой, а в стеке операндов должен быть один операнд, содержащий конечный результат вычислений.
	// Иначе - ошибка разбора или ошибка синтаксиса выражения, либо еще чего похуже.
	return m_Operations.empty() && m_Operands.size() == 1;
}

/** *********************************************************************
* Возвращает результат вычислений
*
* @return указатель на операнд с результатом вычисления, либо NULL (если вычислить не удалось)
************************************************************************/
const CSldLogicalExpression::Operand* CSldLogicalExpression::GetResult(void)
{
	if (m_Operands.empty())
		return nullptr;

	auto operand = m_Operands.popv();
	if (!operand)
		return nullptr;

	if (!MakeOperation(operand.get(), NULL, NULL))
		return NULL;

	m_Operands.push(sld2::move(operand));
	return m_Operands.top().get();
}
