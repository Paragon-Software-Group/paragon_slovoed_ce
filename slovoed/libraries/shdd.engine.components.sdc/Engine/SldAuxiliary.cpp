#include "SldAuxiliary.h"

enum : Int32
{
	DefaultMorphologyFlag = M_BASEFORMS_SHOULD_MATCH_PRECONDITION | M_BASEFORMS_USE_DICTIONARY | M_BASEFORMS_ONLY_DICTIONARY_WORDS | M_IGNORE_BASEFORMS_OF_PARTS
};

/***********************************************************************
* Преобразует строку поиска, введенную пользователем, в расширенный запрос
* для поиска по шаблону, использую морфологию встроенную в базу. (например OALD)
*
* @param aText			- исходная строка запроса
* @param aDict			- указатель на класс открытого словаря
* @param aListIndex		- номер списка слов в словаре, в котором предполагаем осуществлять поиск
* @param aExpressionBox	- указатель, по которому будет записан разобранный запрос
*
* @return код ошибки
************************************************************************/
ESldError PrepareQueryForWildCardSearch(const UInt16* aText, CSldDictionary* aDict, Int32 aListIndex, TExpressionBox* aExpressionBox)
{
	if (!aText || !aDict || !aExpressionBox)
		return eMemoryNullPointer;
	
	ESldError ProcessError;

	const CSldListInfo* pListInfo = NULL;
	ProcessError = aDict->GetWordListInfo(aListIndex, &pListInfo);
	if (ProcessError != eOK)
		return ProcessError;
	
	const bool isFullMorphologyWordFormsList = pListInfo->IsFullMorphologyWordFormsList();

	// Длина строки исходного запроса
	Int32 TextLen = CSldCompare::StrLen(aText);
	if (!TextLen)
	{
		if (!aExpressionBox->AddExpression(SldU16StringLiteral("*"), eTokenType_Operation_AND))
			return eMemoryNotEnoughMemory;

		return eOK;
	}
	
	// Максимальное количество слов в исходном запросе
	Int32 TextWordsMaxCount = TextLen + 2;
	
	// Массив слов в исходном запросе
	UInt16** TextWords = (UInt16**)sldMemNewZero((TextWordsMaxCount)*sizeof(TextWords[0]));
	if (!TextWords)
		return eMemoryNotEnoughMemory;

	ELogicalExpressionTokenTypes* logicalOperation = (ELogicalExpressionTokenTypes*)sldMemNewZero((TextWordsMaxCount)*sizeof(logicalOperation[0]));
	if (!logicalOperation)
	{
		sldMemFree(TextWords);
		return eMemoryNotEnoughMemory;
	}
	
	// Количество слов в исходном запросе
	Int32 TextWordsCount = 0;
	
	// Разбиваем исходный запрос на слова
	const UInt16* SourcePtr = aText;
	UInt16* DestPtr = NULL;
	ProcessError = eOK;
	UInt32 isDelimiterSymbol = 0;
	UInt32 isValidResult = 0;
	Int32 Index = 0;
	UInt8 addNewWord = 1;
	UInt8 isLogicalOperation = 0;
	ELogicalExpressionTokenTypes bracketLogic = eTokenType_Operand;

	while (*SourcePtr)
	{
		if(addNewWord)
		{
			DestPtr = (UInt16*)sldMemNew((TextLen+1)*sizeof(DestPtr[0]));
			if (!DestPtr)
			{
				ProcessError = eMemoryNotEnoughMemory;
				break;
			}
			TextWords[TextWordsCount] = DestPtr;
			*DestPtr = 0;
			addNewWord = 0;
		}

		ProcessError = aDict->IsSymbolBelongToLanguage(*SourcePtr, SldLanguage::Delimiters, &isDelimiterSymbol, &isValidResult);
		if (ProcessError != eOK)
			return eOK;
				
		if(*SourcePtr == CMP_QUERY_SPECIAL_SYMBOL_ANY_CHARS || *SourcePtr == CMP_QUERY_SPECIAL_SYMBOL_ONE_CHAR)
		{
			*DestPtr++ = *SourcePtr++;
			*DestPtr = 0;
		}
		else if(*SourcePtr == CMP_QUERY_SPECIAL_SYMBOL_AND)
		{
			logicalOperation[TextWordsCount + 1] = eTokenType_Operation_AND;
			isLogicalOperation = 1;
		}
		else if(*SourcePtr == CMP_QUERY_SPECIAL_SYMBOL_OR)
		{
			if(CSldCompare::StrLen(TextWords[TextWordsCount]) == 0 || SourcePtr[1] == CMP_QUERY_SPECIAL_SYMBOL_OPEN_BR)
			{
				logicalOperation[TextWordsCount] = eTokenType_Operation_OR;
				isLogicalOperation = 1;
			}
			else
			{
				SourcePtr++;
			}
		}
		else if(*SourcePtr == CMP_QUERY_SPECIAL_SYMBOL_NOT)
		{
			if(CSldCompare::StrLen(TextWords[TextWordsCount]) == 0 || SourcePtr[1] == CMP_QUERY_SPECIAL_SYMBOL_OPEN_BR)
			{
				logicalOperation[TextWordsCount] = eTokenType_Operation_NOT;
				isLogicalOperation = 1;
			}
			else
			{
				SourcePtr++;
			}
		}
		else if(*SourcePtr == CMP_QUERY_SPECIAL_SYMBOL_OPEN_BR)
		{
			if(CSldCompare::StrLen(TextWords[TextWordsCount]) == 0)
			{
				const CSldCompare* compare;
				aDict->GetCompare(&compare);
				if(compare->CheckBracket(SourcePtr) == 1)
				{
					if(logicalOperation[TextWordsCount])
					{
						bracketLogic = logicalOperation[TextWordsCount];
					}
					else
					{
						bracketLogic = eTokenType_Operation_AND;
					}
				}
			}
			SourcePtr++;
		}
		else if(*SourcePtr == CMP_QUERY_SPECIAL_SYMBOL_CLOSE_BR)
		{
			logicalOperation[TextWordsCount] = bracketLogic;
			bracketLogic = eTokenType_Operand;

			const CSldCompare* compare;
			aDict->GetCompare(&compare);
			if(SourcePtr[1] && compare->IsZeroSymbol(SourcePtr[1]))
			{
				isLogicalOperation = 1;
			}
			else
			{
				SourcePtr++;
			}
		}
		else if (isValidResult && isDelimiterSymbol && SourcePtr[1])
		{	
			if(bracketLogic)
			{
				logicalOperation[TextWordsCount] = bracketLogic;
			}

			if(logicalOperation[TextWordsCount] == eTokenType_Operand)
			{
				logicalOperation[TextWordsCount] = eTokenType_Operation_AND;
			}

			isLogicalOperation = 1;
		}
		else
		{
			*DestPtr++ = *SourcePtr++;
			*DestPtr = 0;
		}

		if(isLogicalOperation)
		{
			if(CSldCompare::StrLen(TextWords[TextWordsCount]) && SourcePtr[1])
			{
				addNewWord = 1;
				TextWordsCount++;
			}

			isLogicalOperation = 0;
			SourcePtr++;
		}
	}

	*DestPtr = 0;

	if(logicalOperation[TextWordsCount] == eTokenType_Operand)
		logicalOperation[TextWordsCount] = eTokenType_Operation_AND;

	TextWordsCount++;
	
	if (!TextWordsCount || (TextWordsCount == 1 && CSldCompare::StrLen(TextWords[0]) == 0))
	{
		Index = 0;
		while (TextWords[Index])
		{
			sldMemFree(TextWords[Index]);
			Index++;
		}
		sldMemFree(TextWords);
		sldMemFree(logicalOperation);
		
		if (!aExpressionBox->AddExpression(SldU16StringLiteral("*"), eTokenType_Operation_AND))
			return eMemoryNotEnoughMemory;
		
		return eOK;
	}
	
	// Максимально поддерживаемое количество слов в исходном запросе
	const Int32 MaxTextWordsCount = 16;
	
	if (MaxTextWordsCount < TextWordsCount)
	{
		Index = 0;
		while (TextWords[Index])
		{
			sldMemFree(TextWords[Index]);
			Index++;
		}
		sldMemFree(TextWords);
		sldMemFree(logicalOperation);
		return eCommonTooLargeText;
	}

	// Создаем запрос
	for (Int32 wordIndex = 0; wordIndex < TextWordsCount; wordIndex++)
	{
		if(aExpressionBox->Count > SLD_DEFAULT_NUMBER_OF_REQUEST_WORD)
		{
			break;
		}
		if (CSldCompare::StrLen(TextWords[wordIndex]) == 0)
		{
			continue;
		}

		UInt16* TextWord = TextWords[wordIndex];
		UInt32 wordHasWildCardSymbols = CSldCompare::IsWordHasWildCardSymbols(TextWord);
			
		#ifndef NOT_USING_MORPHOLOGY_IN_AUXILIARY
		if (!isFullMorphologyWordFormsList && !wordHasWildCardSymbols)
		{
			
			const UInt32 langFrom = pListInfo->GetLanguageFrom();

			CSldVector<TSldMorphologyWordStruct> morfologyForms;
			ESldError error = GetMorphoForms(TextWord, aDict, langFrom, morfologyForms);
			if (error != eOK)
				return error;

			if (morfologyForms.size())
			{
				bool isPresent = false;
				for (const auto & form : morfologyForms)
				{
					if (CSldCompare::StrCmp(TextWord, form.MorphologyForm.c_str()) == 0)
					{
						isPresent = true;
						break;
					}
				}

				if (!isPresent)
				{
					if (!aExpressionBox->AddExpression(TextWord, eTokenType_Operation_OR))
						return eMemoryNotEnoughMemory;
				}

				bool isNotFirst = false;
				for (const auto & form : morfologyForms)
				{
					if (isNotFirst)
					{
						if (!aExpressionBox->AddExpression(form.MorphologyForm.c_str(), eTokenType_Operation_OR))
							return eMemoryNotEnoughMemory;
					}
					else
					{
						isNotFirst = true;
						if (!aExpressionBox->AddExpression(form.MorphologyForm.c_str(), logicalOperation[wordIndex]))
							return eMemoryNotEnoughMemory;
					}
				}
			}
			else
			{
				if (!aExpressionBox->AddExpression(TextWord, logicalOperation[wordIndex]))
					return eMemoryNotEnoughMemory;
			}						
		}
		else
		{
			if (!aExpressionBox->AddExpression(TextWord, logicalOperation[wordIndex]))
				return eMemoryNotEnoughMemory;
		}
		#else
		if (!aExpressionBox->AddExpression(TextWord, logicalOperation[wordIndex]))
			return eMemoryNotEnoughMemory;
		#endif
	}
	
	// Удаляем массив исходных слов
	Index = 0;
	while (TextWords[Index])
	{
		sldMemFree(TextWords[Index]);
		Index++;
	}
	sldMemFree(TextWords);
	sldMemFree(logicalOperation);
	
	return eOK;
}

/***********************************************************************
* Преобразует строку поиска, введенную пользователем, в расширенный запрос для поиска по шаблону
*
* @param aText			- исходная строка запроса
* @param aDict			- указатель на класс открытого словаря
* @param aListIndex		- номер списка слов в словаре, в котором предполагаем осуществлять поиск
* @param aMorpho		- указатель на класс морфологии для получения базовых форм слов
*						  может быть NULL, тогда морфология использоваться не будет
* @param aExpressionBox	- указатель, по которому будет записан разобранный запрос
*
* @return код ошибки
************************************************************************/
ESldError PrepareQueryForWildCardSearch(const UInt16* aText, CSldDictionary* aDict, Int32 aListIndex, MorphoDataType* aMorpho, TExpressionBox* aExpressionBox)
{
	if (!aText || !aDict || !aExpressionBox)
		return eMemoryNullPointer;
	
	ESldError ProcessError;
	const CSldListInfo* pListInfo = NULL;
	
	ProcessError = aDict->GetWordListInfo(aListIndex, &pListInfo);
	if (ProcessError != eOK)
		return ProcessError;
	
	// Длина строки исходного запроса
	Int32 TextLen = CSldCompare::StrLen(aText);
	if (!TextLen)
	{
		if (!aExpressionBox->AddExpression(SldU16StringLiteral("*"), eTokenType_Operation_AND))
			return eMemoryNotEnoughMemory;

		return eOK;
	}
	
	// Максимальное количество слов в исходном запросе
	Int32 TextWordsMaxCount = TextLen + 2;
	
	// Массив слов в исходном запросе
	UInt16** TextWords = (UInt16**)sldMemNewZero((TextWordsMaxCount)*sizeof(TextWords[0]));
	if (!TextWords)
		return eMemoryNotEnoughMemory;

	ELogicalExpressionTokenTypes* logicalOperation = (ELogicalExpressionTokenTypes*)sldMemNewZero((TextWordsMaxCount)*sizeof(logicalOperation[0]));
	if (!logicalOperation)
	{
		sldMemFree(TextWords);
		return eMemoryNotEnoughMemory;
	}
	
	// Количество слов в исходном запросе
	Int32 TextWordsCount = 0;
	
	// Разбиваем исходный запрос на слова
	const UInt16* SourcePtr = aText;
	UInt16* DestPtr = NULL;
	ProcessError = eOK;
	UInt32 isDelimiterSymbol = 0;
	UInt32 isValidResult = 0;
	Int32 Index = 0;
	UInt8 addNewWord = 1;
	UInt8 isLogicalOperation = 0;
	ELogicalExpressionTokenTypes bracketLogic = eTokenType_Operand;

	while (*SourcePtr)
	{
		if(addNewWord)
		{
			DestPtr = (UInt16*)sldMemNew((TextLen+1)*sizeof(DestPtr[0]));
			if (!DestPtr)
			{
				ProcessError = eMemoryNotEnoughMemory;
				break;
			}
			TextWords[TextWordsCount] = DestPtr;
			*DestPtr = 0;
			addNewWord = 0;
		}

		ProcessError = aDict->IsSymbolBelongToLanguage(*SourcePtr, SldLanguage::Delimiters, &isDelimiterSymbol, &isValidResult);
		if (ProcessError != eOK)
			return ProcessError;
				
		if(*SourcePtr == CMP_QUERY_SPECIAL_SYMBOL_ANY_CHARS || *SourcePtr == CMP_QUERY_SPECIAL_SYMBOL_ONE_CHAR)
		{
			*DestPtr++ = *SourcePtr++;
			*DestPtr = 0;
		}
		else if(*SourcePtr == CMP_QUERY_SPECIAL_SYMBOL_AND)
		{
			logicalOperation[TextWordsCount + 1] = eTokenType_Operation_AND;
			isLogicalOperation = 1;
		}
		else if(*SourcePtr == CMP_QUERY_SPECIAL_SYMBOL_OR)
		{
			if(CSldCompare::StrLen(TextWords[TextWordsCount]) == 0 || SourcePtr[1] == CMP_QUERY_SPECIAL_SYMBOL_OPEN_BR)
			{
				logicalOperation[TextWordsCount] = eTokenType_Operation_OR;
				isLogicalOperation = 1;
			}
			else
			{
				SourcePtr++;
			}
		}
		else if(*SourcePtr == CMP_QUERY_SPECIAL_SYMBOL_NOT)
		{
			if(CSldCompare::StrLen(TextWords[TextWordsCount]) == 0 || SourcePtr[1] == CMP_QUERY_SPECIAL_SYMBOL_OPEN_BR)
			{
				logicalOperation[TextWordsCount] = eTokenType_Operation_NOT;
				isLogicalOperation = 1;
			}
			else
			{
				SourcePtr++;
			}
		}
		else if(*SourcePtr == CMP_QUERY_SPECIAL_SYMBOL_OPEN_BR)
		{
			if(CSldCompare::StrLen(TextWords[TextWordsCount]) == 0)
			{
				const CSldCompare* compare;
				aDict->GetCompare(&compare);
				if(compare->CheckBracket(SourcePtr) == 1)
				{
					if(logicalOperation[TextWordsCount])
					{
						bracketLogic = logicalOperation[TextWordsCount];
					}
					else
					{
						bracketLogic = eTokenType_Operation_AND;
					}
				}
			}
			SourcePtr++;
		}
		else if(*SourcePtr == CMP_QUERY_SPECIAL_SYMBOL_CLOSE_BR)
		{
			logicalOperation[TextWordsCount] = bracketLogic;
			bracketLogic = eTokenType_Operand;

			const CSldCompare* compare;
			aDict->GetCompare(&compare);
			if(SourcePtr[1] && compare->IsZeroSymbol(SourcePtr[1]))
			{
				isLogicalOperation = 1;
			}
			else
			{
				SourcePtr++;
			}
		}
		else if (isValidResult && isDelimiterSymbol && SourcePtr[1])
		{	
			if(bracketLogic)
			{
				logicalOperation[TextWordsCount] = bracketLogic;
			}

			if(logicalOperation[TextWordsCount] == eTokenType_Operand)
			{
				logicalOperation[TextWordsCount] = eTokenType_Operation_AND;
			}

			isLogicalOperation = 1;
		}
		else
		{
			*DestPtr++ = *SourcePtr++;
			*DestPtr = 0;
		}

		if(isLogicalOperation)
		{
			if(CSldCompare::StrLen(TextWords[TextWordsCount]) && SourcePtr[1])
			{
				addNewWord = 1;
				TextWordsCount++;
			}

			isLogicalOperation = 0;
			SourcePtr++;
		}
	}

	*DestPtr = 0;

	if(logicalOperation[TextWordsCount] == eTokenType_Operand)
		logicalOperation[TextWordsCount] = eTokenType_Operation_AND;

	TextWordsCount++;
	
	if (!TextWordsCount || (TextWordsCount == 1 && CSldCompare::StrLen(TextWords[0]) == 0))
	{
		Index = 0;
		while (TextWords[Index])
		{
			sldMemFree(TextWords[Index]);
			Index++;
		}
		sldMemFree(TextWords);
		sldMemFree(logicalOperation);
		
		if (!aExpressionBox->AddExpression(SldU16StringLiteral("*"), eTokenType_Operation_AND))
			return eMemoryNotEnoughMemory;
		
		return eOK;
	}
	
	// Максимально поддерживаемое количество слов в исходном запросе
	const Int32 MaxTextWordsCount = 16;
	// Максимальное количество поддерживаемых уникальных словоформ для одного слова
	const Int32 MaxMorphoWordForms = 128;
	
	if (MaxTextWordsCount < TextWordsCount)
	{
		Index = 0;
		while (TextWords[Index])
		{
			sldMemFree(TextWords[Index]);
			Index++;
		}
		sldMemFree(TextWords);
		sldMemFree(logicalOperation);
		return eCommonTooLargeText;
	}

	// Создаем запрос
	for (Int32 wordIndex = 0; wordIndex < TextWordsCount; wordIndex++)
	{
		if(aExpressionBox->Count > SLD_DEFAULT_NUMBER_OF_REQUEST_WORD)
		{
			break;
		}
		if (CSldCompare::StrLen(TextWords[wordIndex]) == 0)
		{
			continue;
		}

		UInt16* TextWord = TextWords[wordIndex];
		UInt32 wordHasWildCardSymbols = CSldCompare::IsWordHasWildCardSymbols(TextWord);
			
		#ifndef NOT_USING_MORPHOLOGY_IN_AUXILIARY

		const bool isFullMorphologyWordFormsList = pListInfo->IsFullMorphologyWordFormsList();
		if (aMorpho && !isFullMorphologyWordFormsList && !wordHasWildCardSymbols)
		{
			// Массив форм слов
			UInt16 WordFormsArray[MaxMorphoWordForms][MAXIMUM_WORD_LENGTH];
			sldMemZero(WordFormsArray, sizeof(WordFormsArray));
			Int32 WordFormsCount = 0;
			
			// Копируем в массив само слово
			CSldCompare::StrCopy(WordFormsArray[WordFormsCount], TextWord);
			WordFormsCount++;
			
			WritingVersionIterator vIter;
			const MorphoInflectionRulesSetHandle* ruleset = NULL;
			UInt16 morphoVariant[MAXIMUM_WORD_LENGTH] = {0};
			UInt16 morphoWordBase[MAXIMUM_WORD_LENGTH] = {0};
			UInt16 morphoWordForm[MAXIMUM_WORD_LENGTH] = {0};
			const MCHAR* questionPtr = NULL;
			const MCHAR* namePtr = NULL;
			
			UInt32 TextWordLen = (UInt32)CSldCompare::StrLen(TextWord);
			aMorpho->WritingVersionIteratorInitW(vIter, TextWord, TextWordLen);
			while (aMorpho->GetNextWritingVersionW(vIter, morphoVariant))
			{
				BaseFormsIterator bIter;
				aMorpho->BaseFormsIteratorInitW(bIter, morphoVariant, DefaultMorphologyFlag);
				while (aMorpho->GetNextBaseFormW(bIter, morphoWordBase, &ruleset))
				{
					WordFormsIterator fIter;
					aMorpho->WordFormsIteratorInitW(fIter, morphoWordBase, ruleset, M_FORMS_ONLY_FORMS);
					while (aMorpho->GetNextWordFormW(fIter, morphoWordForm, &questionPtr, &namePtr))
					{
						if (*morphoWordForm == 0)
							continue;

						// Проверяем, может мы уже эту форму слова добавляли
						bool present = false;
						for (Int32 k=0;k<MaxMorphoWordForms;k++)
						{
							if (CSldCompare::StrCmp(morphoWordForm, WordFormsArray[k])==0)
							{
								present = true;
								break;
							}
						}
						
						// Если не добавляли - добавляем
						if (!present)
						{
							CSldCompare::StrCopy(WordFormsArray[WordFormsCount], morphoWordForm);
							WordFormsCount++;
							
							if (WordFormsCount >= MaxMorphoWordForms)
								break;
						}
					}
					if (WordFormsCount >= MaxMorphoWordForms)
						break;
				}
				if (WordFormsCount >= MaxMorphoWordForms)
					break;
			}
			
			// Добавляем формы из массива форм			
			for (Int32 j = 0 ; j < WordFormsCount; j++)
			{
				if (j)
				{
					if (!aExpressionBox->AddExpression(WordFormsArray[j], eTokenType_Operation_OR))
						return eMemoryNotEnoughMemory;
				}
				else
				{
					if (!aExpressionBox->AddExpression(WordFormsArray[j], logicalOperation[wordIndex]))
						return eMemoryNotEnoughMemory;
				}
			}			
		}
		else
		{
			if (!aExpressionBox->AddExpression(TextWord, logicalOperation[wordIndex]))
				return eMemoryNotEnoughMemory;
		}
		#else
		if (!aExpressionBox->AddExpression(TextWord, logicalOperation[wordIndex]))
			return eMemoryNotEnoughMemory;
		#endif
	}
	
	// Удаляем массив исходных слов
	Index = 0;
	while (TextWords[Index])
	{
		sldMemFree(TextWords[Index]);
		Index++;
	}
	sldMemFree(TextWords);
	sldMemFree(logicalOperation);
	
	return eOK;
}

ESldError  PrepareQueryForFullTextSearch(const UInt16 *aText, CSldDictionary *aDict, Int32 aListIndex, SldU16String& aResultQuery, SldU16WordsArray * aMorphoForms)
{
	if (!aText || !aDict)
		return eMemoryNullPointer;
	
	aResultQuery.clear();
	ESldError error;
	const CSldListInfo* pListInfo = 0;
	error = aDict->GetWordListInfo(aListIndex, &pListInfo);
	if (error != eOK)
		return error;

	if (!pListInfo->IsFullTextSearchList())
		return eCommonWrongList;

	// Длина строки исходного запроса
	Int32 textLen = CSldCompare::StrLen(aText);
	if (!textLen)
	{
		return eOK;
	}

	// Если поисковый запрос является умным - ничего не меняем
	UInt32 isSmartQuery = CSldCompare::IsSmartFullTextSearchQuery(aText);
	if (isSmartQuery)
	{
		aResultQuery += SldU16String(aText);
		return eOK;
	}

	const UInt32 langFrom = pListInfo->GetLanguageFrom();

	const CSldCompare *CMP = 0;
	error = aDict->GetCompare(&CMP);
	if (error != eOK)
		return error;

	SldU16WordsArray textWords;
	error = CMP->DivideQueryByParts(aText, textWords);
	if (error != eOK || !textWords.size())
	{
		return eOK;
	}

	// Строка с результирующим запросом
	bool isNotFirst1 = false;
	for (const auto & word : textWords)
	{
		if (isNotFirst1)
			aResultQuery.push_back('&');
		else
			isNotFirst1 = true;

		#ifndef NOT_USING_MORPHOLOGY_IN_AUXILIARY

		if (pListInfo->IsFullMorphologyWordFormsList())
		{
			aResultQuery.push_back('(');
			aResultQuery += word;
			aResultQuery.push_back(')');
		}
		else
		{
			CSldVector<TSldMorphologyWordStruct> morfologyForms;
			if (pListInfo->GetMorphologyId() == aListIndex)
			{
				error = GetMorphoForms(word.c_str(), aDict, langFrom, morfologyForms, 0, eMorphoBase);
				if (error != eOK)
					return error;
			}
			else
			{
				error = GetMorphoForms(word.c_str(), aDict, langFrom, morfologyForms);
				if (error != eOK)
					return error;
			}

			if (morfologyForms.size())
			{
				aResultQuery.push_back('(');
				bool isPresent = false;
				for (const auto & form : morfologyForms)
				{
					if (CSldCompare::StrCmp(word.c_str(), form.MorphologyForm.c_str()) == 0)
					{
						isPresent = true;
						break;
					}
				}

				if (!isPresent)
				{
					aResultQuery.push_back('(');
					aResultQuery += word;
					aResultQuery.push_back(')');
					aResultQuery.push_back('|');
				}

				bool isNotFirst2 = false;
				for (const auto & form : morfologyForms)
				{
					if (isNotFirst2)
						aResultQuery.push_back('|');
					else
						isNotFirst2 = true;
				
					aResultQuery.push_back('(');
					aResultQuery += form.MorphologyForm;
					aResultQuery.push_back(')');
					if (aMorphoForms != NULL)
						aMorphoForms->push_back(sld2::move(form.MorphologyForm));
				}
				aResultQuery.push_back(')');
			}
			else
			{
				aResultQuery.push_back('(');
				aResultQuery += word;
				aResultQuery.push_back(')');
				if (aMorphoForms != NULL)
					aMorphoForms->push_back(sld2::move(word));
			}
		}
		#else
		aResultQuery.push_back('(');
		aResultQuery += word;
		aResultQuery.push_back(')');
		#endif
	}

	return eOK;
}

#ifndef USE_ALL_POSSIBLE_FORMS

/***********************************************************************
* Преобразует строку поиска, введенную пользователем, в расширенный запрос для полнотекстового поиска
*
* @param aText			- исходная строка запроса
* @param aDict			- указатель на класс открытого словаря
* @param aListIndex		- номер списка слов в словаре, в котором предполагаем осуществлять поиск
* @param aMorpho		- указатель на класс морфологии для получения базовых форм слов
*						  может быть NULL, тогда морфология использоваться не будет
* @param aResultQuery	- строка с созданным запросом.
*
* @return код ошибки
************************************************************************/
ESldError PrepareQueryForFullTextSearch(const UInt16* aText, CSldDictionary* aDict, Int32 aListIndex, MorphoDataType* aMorpho, SldU16String& aResultQuery, SldU16WordsArray * aMorphoForms)
{
	if (!aText || !aDict)
		return eMemoryNullPointer;
	
	aResultQuery.clear();
	
	ESldError ProcessError;
	const CSldListInfo* pListInfo = NULL;
	
	ProcessError = aDict->GetWordListInfo(aListIndex, &pListInfo);
	if (ProcessError != eOK)
		return ProcessError;

	if (!pListInfo->IsFullTextSearchList())
		return eCommonWrongList;

	const bool isFullMorphologyWordFormsList = pListInfo->IsFullMorphologyWordFormsList();
	const UInt32 morphoID = pListInfo->GetMorphologyId();

	const MorphoDataHeader* morphoHeader = NULL;
	if (aMorpho)
	{
		aMorpho->GetMorphoHeader(&morphoHeader);
	}

	// Длина строки исходного запроса
	Int32 TextLen = CSldCompare::StrLen(aText);
	if (!TextLen)
	{
		return eOK;
	}

	// Если поисковый запрос является умным - ничего не меняем
	UInt32 IsSmartQuery = CSldCompare::IsSmartFullTextSearchQuery(aText);
	if (IsSmartQuery)
	{
		aResultQuery += SldU16String(aText);
		return eOK;
	}
	
	const CSldCompare *CMP;
	ProcessError = aDict->GetCompare(&CMP);
	if (ProcessError != eOK)
		return ProcessError;

	SldU16WordsArray TextWords;
	SldU16WordsArray AlternativeWords;
	ProcessError = CMP->DivideQuery(aText, TextWords, AlternativeWords);
	if (ProcessError != eOK || TextWords.empty())
	{
		return eOK;
	}
	
	// Максимально поддерживаемое количество слов в исходном запросе
	const Int32 MaxTextWordsCount = 16;

	if (MaxTextWordsCount < TextWords.size())
		return eCommonTooLargeText;
	
	const UInt16* alternative = NULL;
	// Создаем запрос
	for (UInt32 i = 0; i < TextWords.size(); i++)
	{
		if (CSldCompare::StrCmp(alternative, AlternativeWords[i].c_str()))
		{
			aResultQuery.push_back(')');
			aResultQuery.push_back(')');
			aResultQuery.push_back('&');
			alternative = NULL;
		}
		else if (i)
			aResultQuery.push_back('&');

		if (AlternativeWords[i].size() && !alternative)
		{
			alternative = AlternativeWords[i].c_str();

			aResultQuery.push_back('(');
			aResultQuery.push_back('(');
			aResultQuery += AlternativeWords[i];
			aResultQuery.push_back(')');
			aResultQuery.push_back('|');
			aResultQuery.push_back('(');
		}
		
		#ifndef NOT_USING_MORPHOLOGY_IN_AUXILIARY
		if (aMorpho && !isFullMorphologyWordFormsList)
		{
			WritingVersionIterator vIter;
			const MorphoInflectionRulesSetHandle* ruleset = NULL;
			UInt16 morphoVariant[MAXIMUM_WORD_LENGTH] = {0};
			UInt16 morphoWordBase[MAXIMUM_WORD_LENGTH] = {0};
			UInt16 morphoWordForm[MAXIMUM_WORD_LENGTH] = {0};
			const MCHAR* questionPtr = NULL;
			const MCHAR* namePtr = NULL;
			
			SldU16WordsArray mophoForms;
			UInt32 TextWordLen = TextWords[i].size();
			aMorpho->WritingVersionIteratorInitW(vIter, TextWords[i].c_str(), TextWordLen);
			while (aMorpho->GetNextWritingVersionW(vIter, morphoVariant))
			{
				BaseFormsIterator bIter;
				aMorpho->BaseFormsIteratorInitW(bIter, morphoVariant, DefaultMorphologyFlag);
				while (aMorpho->GetNextBaseFormW(bIter, morphoWordBase, &ruleset))
				{
					WordFormsIterator fIter;
					aMorpho->WordFormsIteratorInitW(fIter, morphoWordBase, ruleset, M_FORMS_ONLY_FORMS);
					while (aMorpho->GetNextWordFormW(fIter, morphoWordForm, &questionPtr, &namePtr))
					{
						if (*morphoWordForm == 0)
							continue;

						sld2::sorted_insert(mophoForms, SldU16String(morphoWordForm));
						if (morphoHeader && morphoHeader->DictID == morphoID)
							break;
					}
				}
			}

			if (mophoForms.empty())
			{
				// Копируем в массив само слово
				mophoForms.push_back(TextWords[i]);
			}
			
			// Добавляем формы из массива форм
			if (mophoForms.size() > 1)
				aResultQuery.push_back('(');
				
			for (auto & form : mophoForms)
			{
				if (form != *mophoForms.begin())
					aResultQuery.push_back('|');

				aResultQuery.push_back('(');
				aResultQuery += form;
				aResultQuery.push_back(')');
				if (aMorphoForms != NULL)
					aMorphoForms->push_back(sld2::move(form));
			}
			
			if (mophoForms.size() > 1)
				aResultQuery.push_back(')');
		}
		else
		{
			aResultQuery.push_back('(');
			aResultQuery += TextWords[i];
			aResultQuery.push_back(')');
			if (aMorphoForms != NULL)
				aMorphoForms->push_back(TextWords[i]);
		}
		#else
		aResultQuery.push_back('(');
		aResultQuery += TextWords[i];
		aResultQuery.push_back(')');
		if (aMorphoForms != NULL)
			aMorphoForms->push_back(TextWords[i]);
		#endif

		if (alternative && (i + 1) == TextWords.size())
		{
			aResultQuery.push_back(')');
			aResultQuery.push_back(')');
		}
	}

	return eOK;
}

#else

/***********************************************************************
* Преобразует строку поиска, введенную пользователем, в расширенный запрос для полнотекстового поиска
*
* @param aText			- исходная строка запроса
* @param aDict			- указатель на класс открытого словаря
* @param aListIndex		- номер списка слов в словаре, в котором предполагаем осуществлять поиск
* @param aMorpho		- указатель на класс морфологии для получения базовых форм слов
*						  может быть NULL, тогда морфология использоваться не будет
* @param aResultQuery	- строка с созданным запросом.
*
* @return код ошибки
************************************************************************/
ESldError PrepareQueryForFullTextSearch(const UInt16* aText, CSldDictionary* aDict, Int32 aListIndex, MorphoDataType* aMorpho, SldU16String&  aResultQuery, SldU16WordsArray * aMorphoForms)
{
	if (!aText || !aDict)
		return eMemoryNullPointer;
	
	aResultQuery.clear();
	
	ESldError ProcessError;
	const CSldListInfo* pListInfo = NULL;
	
	ProcessError = aDict->GetWordListInfo(aListIndex, &pListInfo);
	if (ProcessError != eOK)
		return ProcessError;
	
	UInt32 isFullTextSearchList = 0;
	ProcessError = pListInfo->IsFullTextSearchList(&isFullTextSearchList);
	if (ProcessError != eOK)
		return ProcessError;
	
	if (!isFullTextSearchList)
		return eCommonWrongList;
	
	UInt32 isFullMorphologyWordFormsList = 0;
	ProcessError = pListInfo->IsFullMorphologyWordFormsList(&isFullMorphologyWordFormsList);
	if (ProcessError != eOK)
		return ProcessError;
	
	// Длина строки исходного запроса
	Int32 TextLen = CSldCompare::StrLen(aText);
	if (!TextLen)
	{
		return eOK;
	}

	// Если поисковый запрос является умным - ничего не меняем
	UInt32 IsSmartQuery = CSldCompare::IsSmartFullTextSearchQuery(aText);
	if (IsSmartQuery)
	{
		aResultQuery += SldU16String(aText);
		return eOK;
	}
	
	// Максимальное количество слов в исходном запросе
	Int32 TextWordsMaxCount = TextLen;
	
	// Количество вариантов разбиения исходного запроса на слова
	Int32 TextWordsVariantsCount = 0;
	// Массив вариантов разбиения исходного запроса на слова
	// (разделение на варианты необходимо, т.к. внутри одного варианта слова объединяются с помощью '&', а варианты должны объединяться с помощью '|')
	UInt16*** TextWordsVariants = (UInt16***)sldMemNewZero((TextWordsMaxCount+1)*sizeof(TextWordsVariants[0]));
	if (!TextWordsVariants)
		return eMemoryNotEnoughMemory;

	// Количество слов в исходном запросе
	Int32* TextWordsCount = (Int32*)sldMemNewZero((TextWordsMaxCount+1)*sizeof(TextWordsCount[0]));
	if (!TextWordsCount)
		return eMemoryNotEnoughMemory;

	// Разбиваем исходный запрос на слова
	const UInt16* SourcePtr = aText;
	UInt16* DestPtr = NULL;
	ProcessError = eOK;
	UInt32 isDelimiterSymbol = 0;
	UInt32 isValidResult = 0;
	Int32 Index = 0;

	UInt32 numberOfWordsPerItem = 1;
	UInt32 maxNumberOfWordsPerItem = TextWordsMaxCount;
	while (numberOfWordsPerItem <= maxNumberOfWordsPerItem)
	{
		// Массив слов в исходном запросе
		UInt16** TextWords = (UInt16**)sldMemNewZero((TextWordsMaxCount+1)*sizeof(TextWords[0]));
		if (!TextWords)
			return eMemoryNotEnoughMemory;

		SourcePtr = aText;
		while (*SourcePtr)
		{
			// Пропускаем разделители
			while (*SourcePtr)
			{
				ProcessError = aDict->IsSymbolBelongToLanguage(*SourcePtr, SldLanguage::Delimiters, &isDelimiterSymbol, &isValidResult);
				if (ProcessError != eOK)
					break;
					
				if (isValidResult)
				{
					if (!isDelimiterSymbol)
						break;
				}
				else
				{
					if (*SourcePtr!=' ' && *SourcePtr!='\t')
						break;
				}
				SourcePtr++;
			}
			if (ProcessError != eOK)
				break;
			
			if (!(*SourcePtr))
				break;
			
			// Выделяем память под новое слово
			DestPtr = (UInt16*)sldMemNew((TextLen+1)*sizeof(DestPtr[0]));
			if (!DestPtr)
			{
				ProcessError = eMemoryNotEnoughMemory;
				break;
			}
			TextWords[TextWordsCount[TextWordsVariantsCount]] = DestPtr;
			
			// количество входящих в словосочетание групп разделителей
			// (несколько идущих подряд разделителей считаются одной группой)
			UInt32 delimiterCount = 0;
			// флаг для контроля идущих подряд разделителей
			UInt32 isDelimiterPrevSymbol = 0;
			// Указатель, хранящий начало следующего обрабатываемого словосочетания
			// (для того, чтобы всегда смещаться на одно слово при их построении, а не на numberOfWordsPerItem слов)
			const UInt16* FirstWordPassedSourcePtr = NULL;
			// Копируем словосочетание из numberOfWordsPerItem слов
			while (*SourcePtr)
			{
				ProcessError = aDict->IsSymbolBelongToLanguage(*SourcePtr, SldLanguage::Delimiters, &isDelimiterSymbol, &isValidResult);
				if (ProcessError != eOK)
					break;
					
				if (isValidResult)
				{
					if (isDelimiterSymbol)
					{
						if (!isDelimiterPrevSymbol)
							delimiterCount++;

						isDelimiterPrevSymbol = true;
					}
					else
						isDelimiterPrevSymbol = false;
				}
				else
				{
					if (*SourcePtr==' ' || *SourcePtr=='\t')
					{
						if (!isDelimiterPrevSymbol)
							delimiterCount++;

						isDelimiterPrevSymbol = true;
					}
					else
						isDelimiterPrevSymbol = false;
				}

				if (delimiterCount == 1 && !FirstWordPassedSourcePtr)
					FirstWordPassedSourcePtr = SourcePtr;

				if (delimiterCount == numberOfWordsPerItem)
					break;

				*DestPtr++ = *SourcePtr++;
			}
			
			if (ProcessError != eOK)
				break;

			if (!FirstWordPassedSourcePtr)
				FirstWordPassedSourcePtr = SourcePtr;
			
			*DestPtr++ = 0;
			TextWordsCount[TextWordsVariantsCount]++;

			// проверка на последнее словосочетание в строке
			if (!(*SourcePtr) && delimiterCount + 1 == numberOfWordsPerItem)
				break;

			SourcePtr = FirstWordPassedSourcePtr;
		}
		
		if (ProcessError != eOK)
		{
			Index = 0;
			while (TextWords[Index])
			{
				sldMemFree(TextWords[Index]);
				Index++;
			}
			sldMemFree(TextWords);
			break;
		}

		// на первой итерации (разбиение на отдельные слова) в TextWordsCount хранится
		// реальное количество слов в запросе - исправим условие выхода для избежания лишних итераций
		if (numberOfWordsPerItem == 1)
			maxNumberOfWordsPerItem = TextWordsCount[TextWordsVariantsCount];

		TextWordsVariants[TextWordsVariantsCount] = TextWords;

		numberOfWordsPerItem++;
		TextWordsVariantsCount++;
	}

	Int32 VariantIndex;
	if (ProcessError != eOK)
	{
		VariantIndex = 0;
		while (TextWordsVariants[VariantIndex])
		{
			Index = 0;
			UInt16** TextWords = TextWordsVariants[VariantIndex];
			while (TextWords[Index])
			{
				sldMemFree(TextWords[Index]);
				Index++;
			}
			sldMemFree(TextWords);

			VariantIndex++;
		}
		sldMemFree(TextWordsVariants);
		sldMemFree(TextWordsCount);

		return ProcessError;
	}

	if (!TextWordsCount)
	{
		VariantIndex = 0;
		while (TextWordsVariants[VariantIndex])
		{
			Index = 0;
			UInt16** TextWords = TextWordsVariants[VariantIndex];
			while (TextWords[Index])
			{
				sldMemFree(TextWords[Index]);
				Index++;
			}
			sldMemFree(TextWords);

			VariantIndex++;
		}
		sldMemFree(TextWordsVariants);
		sldMemFree(TextWordsCount);
		
		aResultQuery += SldU16String(aText);
		return eOK;
	}
	
	// Максимально поддерживаемое количество слов в исходном запросе
	const Int32 MaxTextWordsCount = 16;
	// Максимальное количество поддерживаемых уникальных словоформ для одного слова
	const Int32 MaxMorphoWordForms = 1024;
	// Максимальная длина строки с результирующим запросом (3 - две скобки и знак логической операции на каждое слово, с запасом)
	const Int32 ResultMaxStrLen = (MaxTextWordsCount*MaxMorphoWordForms)*(MAXIMUM_WORD_LENGTH+3) + MaxTextWordsCount*3;
	
	// ни в одном из вариантов разбиения запроса количество слов не должно превышать лимит
	VariantIndex = 0;
	while (TextWordsVariants[VariantIndex])
	{
		if (MaxTextWordsCount < TextWordsCount[VariantIndex])
		{
			Int32 memFreeVariantIndex = 0;
			while (TextWordsVariants[memFreeVariantIndex])
			{
				Index = 0;
				UInt16** TextWords = TextWordsVariants[memFreeVariantIndex];
				while (TextWords[Index])
				{
					sldMemFree(TextWords[Index]);
					Index++;
				}
				sldMemFree(TextWords);
				
				memFreeVariantIndex++;
			}
			sldMemFree(TextWordsVariants);
			sldMemFree(TextWordsCount);

			return eCommonTooLargeText;
		}

		VariantIndex++;
	}
	
	if (TextWordsVariantsCount > 1)
		aResultQuery.push_back('(');

	// Создаем запрос
	for (Int32 j=0;j<TextWordsVariantsCount;j++)
	{
		UInt16** TextWords = TextWordsVariants[j];

		if (j)
			aResultQuery.push_back('|');

		if (TextWordsVariantsCount > 1)
			aResultQuery.push_back('(');

		for (Int32 i=0;i<TextWordsCount[j];i++)
		{
			UInt16* TextWord = TextWords[i];
			if (i)
				aResultQuery.push_back('&');
			
			#ifndef NOT_USING_MORPHOLOGY_IN_AUXILIARY
			if (aMorpho && !isFullMorphologyWordFormsList)
			{
				// Массив форм слов
				UInt16 WordFormsArray[MaxMorphoWordForms][MAXIMUM_WORD_LENGTH];
				sldMemZero(WordFormsArray, sizeof(WordFormsArray));
				Int32 WordFormsCount = 0;
				
				// Копируем в массив само слово
				CSldCompare::StrCopy(WordFormsArray[WordFormsCount], TextWord);
				WordFormsCount++;
				
				WritingVersionIterator vIter;
				const MorphoInflectionRulesSetHandle* ruleset = NULL;
				UInt16 morphoVariant[MAXIMUM_WORD_LENGTH] = {0};
				UInt16 morphoWordBase[MAXIMUM_WORD_LENGTH] = {0};
				UInt16 morphoWordForm[MAXIMUM_WORD_LENGTH] = {0};
				const MCHAR* questionPtr = NULL;
				const MCHAR* namePtr = NULL;
				
				UInt32 TextWordLen = (UInt32)CSldCompare::StrLen(TextWord);
				aMorpho->WritingVersionIteratorInitW(vIter, TextWord, TextWordLen);
				while (aMorpho->GetNextWritingVersionW(vIter, morphoVariant))
				{
					BaseFormsIterator bIter;
					aMorpho->BaseFormsIteratorInitW(bIter, morphoVariant, DefaultMorphologyFlag);
					while (aMorpho->GetNextBaseFormW(bIter, morphoWordBase, &ruleset))
					{
						WordFormsIterator fIter;
						aMorpho->WordFormsIteratorInitW(fIter, morphoWordBase, ruleset, M_FORMS_ONLY_FORMS);
						while (aMorpho->GetNextWordFormW(fIter, morphoWordForm, &questionPtr, &namePtr))
						{
							if (*morphoWordForm == 0)
								continue;

							// Проверяем, может мы уже эту форму слова добавляли
							bool present = false;
							for (Int32 k=0;k<MaxMorphoWordForms;k++)
							{
								if (CSldCompare::StrCmp(morphoWordForm, WordFormsArray[k])==0)
								{
									present = true;
									break;
								}
							}
							
							// Если не добавляли - добавляем
							if (!present)
							{
								CSldCompare::StrCopy(WordFormsArray[WordFormsCount], morphoWordForm);
								WordFormsCount++;
								
								if (WordFormsCount >= MaxMorphoWordForms)
									break;
							}
						}
						if (WordFormsCount >= MaxMorphoWordForms)
							break;
					}
					if (WordFormsCount >= MaxMorphoWordForms)
						break;
				}
				
				// Добавляем формы из массива форм
				if (WordFormsCount > 1)
					aResultQuery.push_back('(');
					
				for (Int32 j=0;j<WordFormsCount;j++)
				{
					if (j)
						aResultQuery.push_back('|');
					
					aResultQuery.push_back('(');
					aResultQuery += SldU16String(WordFormsArray[j]);
					aResultQuery.push_back(')');
					if (aMorphoForms != NULL)
						aMorphoForms->push_back(WordFormsArray[j]);
				}
				
				if (WordFormsCount > 1)
					aResultQuery.push_back(')');
			}
			else
			{
				aResultQuery.push_back('(');
				aResultQuery += SldU16String(TextWord);
				aResultQuery.push_back(')');
				if (aMorphoForms != NULL)
					aMorphoForms->push_back(TextWord);
			}
			#else
			aResultQuery.push_back('(');
			aResultQuery += SldU16String(TextWord);
			aResultQuery.push_back(')');
			if (aMorphoForms != NULL)
				aMorphoForms->push_back(TextWord);
			#endif
		}

		if (TextWordsVariantsCount > 1)
			aResultQuery.push_back(')');
	}

	if (TextWordsVariantsCount > 1)
		aResultQuery.push_back(')');
	
	// Удаляем массив исходных слов
	VariantIndex = 0;
	while (TextWordsVariants[VariantIndex])
	{
		Index = 0;
		UInt16** TextWords = TextWordsVariants[VariantIndex];
		while (TextWords[Index])
		{
			sldMemFree(TextWords[Index]);
			Index++;
		}
		sldMemFree(TextWords);

		VariantIndex++;
	}
	sldMemFree(TextWordsVariants);
	sldMemFree(TextWordsCount);
	
	return eOK;
}

#endif

/***********************************************************************
* Преобразует строку поиска, в набор морфологических форм введенных слов
*
* @param aText					- исходная строка запроса
* @param aDict					- указатель на класс открытого словаря
* @param aMorpho				- указатель на класс морфологии для получения базовых форм слов
*								  может быть NULL, тогда морфология использоваться не будет
* @param aMorphologyForms		- указатель, по которому будут записаны строки словоформ
*
* @return код ошибки
************************************************************************/
ESldError PrepareQueryForSortingSearchResults(const UInt16* aText, CSldDictionary* aDict, MorphoDataType* aMorpho, CSldVector<TSldMorphologyWordStruct>& aMorphologyForms)
{
	if (!aText || !aDict)
		return eMemoryNullPointer;

	aMorphologyForms.clear();

	if (!aMorpho)
		return eOK;
	
	#ifndef NOT_USING_MORPHOLOGY_IN_AUXILIARY
		ESldError ProcessError;

		// Длина строки исходного запроса
		Int32 TextLen = CSldCompare::StrLen(aText);
		if (!TextLen)
			return eOK;

		const CSldCompare *CMP;
		ProcessError = aDict->GetCompare(&CMP);
		if (ProcessError != eOK)
			return ProcessError;

		SldU16WordsArray TextWords;
		ProcessError = CMP->DivideQueryByParts(aText, TextWords);
		if (ProcessError != eOK || TextWords.empty())
			return ProcessError;

		// Создаем запрос
		for (const auto & word : TextWords)
		{
			WritingVersionIterator vIter;
			const MorphoInflectionRulesSetHandle* ruleset = NULL;
			UInt16 morphoVariant[MAXIMUM_WORD_LENGTH] = {0};
			UInt16 morphoWordBase[MAXIMUM_WORD_LENGTH] = {0};
			UInt16 morphoWordForm[MAXIMUM_WORD_LENGTH] = {0};
			const MCHAR* questionPtr = NULL;
			const MCHAR* namePtr = NULL;

			UInt32 TextWordLen = word.size();
			aMorpho->WritingVersionIteratorInitW(vIter, word.c_str(), TextWordLen);
			while (aMorpho->GetNextWritingVersionW(vIter, morphoVariant))
			{
				BaseFormsIterator bIter;
				aMorpho->BaseFormsIteratorInitW(bIter, morphoVariant, DefaultMorphologyFlag);
				while (aMorpho->GetNextBaseFormW(bIter, morphoWordBase, &ruleset))
				{
					WordFormsIterator fIter;
					aMorpho->WordFormsIteratorInitW(fIter, morphoWordBase, ruleset, M_FORMS_ONLY_FORMS);
					while (aMorpho->GetNextWordFormW(fIter, morphoWordForm, &questionPtr, &namePtr))
					{
						if (*morphoWordForm == 0)
							continue;

						// Проверяем, может мы уже эту форму слова добавляли
						bool present = false;
						for (const auto & form : aMorphologyForms)
						{
							if (CSldCompare::StrCmp(morphoWordForm, form.MorphologyForm.c_str())==0)
							{
								present = true;
								break;
							}
						}

						// Слова из исходного запроса не добавляем
						for (const auto & checkedWord : TextWords)
						{
							if (CSldCompare::StrCmp(morphoWordForm, checkedWord.c_str()) == 0)
							{
								present = true;
								break;
							}
						}

						// Если не добавляли - добавляем
						if (!present)
						{
							aMorphologyForms.push_back(TSldMorphologyWordStruct());

							aMorphologyForms.back().MorphologyForm = morphoWordForm;
							if (CSldCompare::StrCmp(morphoWordForm, morphoWordBase)==0)
								aMorphologyForms.back().MorphologyFormType = eMorphologyWordTypeBase;
							else
								aMorphologyForms.back().MorphologyFormType = eMorphologyWordTypeWordform;
						}
					}
				}
			}
		}

		if (ProcessError != eOK)
			return ProcessError;
	#endif

	return eOK;
}

#ifndef NOT_USING_MORPHOLOGY_IN_AUXILIARY
ESldError GetAllMorphoForms(const UInt16* aText, MorphoDataType* aMorpho, CSldVector<TSldMorphologyWordStruct>& aMorphologyForms)
{
	if (!aText)
		return eMemoryNullPointer;

	if (!aMorpho)
		return eOK;

	aMorphologyForms.clear();
	ESldError error = eOK;

	WritingVersionIterator vIter;
	const MorphoInflectionRulesSetHandle* ruleset = NULL;
	UInt16 morphoVariant[MAXIMUM_WORD_LENGTH] = {0};
	UInt16 morphoWordBase[MAXIMUM_WORD_LENGTH] = {0};
	UInt16 morphoWordForm[MAXIMUM_WORD_LENGTH] = {0};
	const MCHAR* questionPtr = NULL;
	const MCHAR* namePtr = NULL;
	CSldVector<SldU16StringRef> sortedForms;
			
	UInt32 TextWordLen = CSldCompare::StrLen(aText);
	aMorpho->WritingVersionIteratorInitW(vIter, aText, TextWordLen);
	while (aMorpho->GetNextWritingVersionW(vIter, morphoVariant))
	{
		BaseFormsIterator bIter;
		aMorpho->BaseFormsIteratorInitW(bIter, morphoVariant, DefaultMorphologyFlag);
		while (aMorpho->GetNextBaseFormW(bIter, morphoWordBase, &ruleset))
		{
			WordFormsIterator fIter;
			aMorpho->WordFormsIteratorInitW(fIter, morphoWordBase, ruleset, M_FORMS_ONLY_FORMS);

			if (sld2::sorted_find(sortedForms, SldU16StringRef(morphoWordBase)) == sortedForms.npos)
			{
				aMorphologyForms.emplace_back(morphoWordBase, eMorphologyWordTypeBase);
				sld2::sorted_insert(sortedForms, aMorphologyForms.back().MorphologyForm);
			}

			while (aMorpho->GetNextWordFormW(fIter, morphoWordForm, &questionPtr, &namePtr))
			{
				if (*morphoWordForm == 0)
					continue;

				if (sld2::sorted_find(sortedForms, SldU16StringRef(morphoWordForm)) == sortedForms.npos)
				{
					aMorphologyForms.emplace_back(morphoWordForm, eMorphologyWordTypeWordform);
					sld2::sorted_insert(sortedForms, aMorphologyForms.back().MorphologyForm);
				}
			}
		}
	}

	return error;
}

ESldError GetAllMorphoForms(const UInt16* aText, CSldDictionary *aDict, UInt32 aLangFrom, CSldVector<TSldMorphologyWordStruct> & aMorphologyForms, UInt32 aWithoutRequest)
{
	return GetMorphoForms(aText, aDict, aLangFrom, aMorphologyForms, aWithoutRequest);
}

ESldError GetMorphoForms(const UInt16 *aText, CSldDictionary *aDict, UInt32 aLangFrom, CSldVector<TSldMorphologyWordStruct> & aMorphologyForms, UInt32 aWithoutRequest, const EMorphoFormsType aType)
{
	if (!aText || !aDict)
		return eMemoryNullPointer;

	aMorphologyForms.clear();
	ESldError error = eOK;
	Int32 inflectionsListIndex = -1;
	Int32 baseFormsListIndex = -1;
	Int32 listCount = -1; 
	error = aDict->GetNumberOfLists(&listCount);
	if (error != eOK)
		return error;
	
	for (Int32 listIndex = 0; listIndex < listCount; ++listIndex) {
		const CSldListInfo *listInfo = 0;
		error = aDict->GetWordListInfo(listIndex, &listInfo);
		if (error != eOK)
			return error;

		const UInt32 listLangFrom = listInfo->GetLanguageFrom();
		const EWordListTypeEnum usage = listInfo->GetUsage();
		if (usage == eWordListType_MorphologyBaseForm && 
        (aLangFrom == listLangFrom || aLangFrom == ESldLanguage::Unknown))
		{
			baseFormsListIndex = listIndex;
			if (inflectionsListIndex != -1)
				break;
		}
		else if (usage == eWordListType_MorphologyInflectionForm && 
             (aLangFrom == listLangFrom || aLangFrom == ESldLanguage::Unknown))
		{
			inflectionsListIndex = listIndex;
			if (baseFormsListIndex != -1)
				break;
		}
	}

	if (baseFormsListIndex == -1 || inflectionsListIndex == -1)
		return eOK;

	CSldList *inflectionsList = 0;
	error = aDict->GetWordList(inflectionsListIndex, (ISldList**) &inflectionsList);
	if (error != eOK) 
		return error;

	CSldList *baseFormsList = 0;
	error = aDict->GetWordList(baseFormsListIndex, (ISldList**) &baseFormsList);
	if (error != eOK) 
		return error;
		
	// Получаем базовую форму
	UInt32 wordIsFound = 0;
	error = inflectionsList->GetMostSimilarWordByText(aText, &wordIsFound);
	if (error != eOK)
		return error;

	if (!wordIsFound)
		return eOK;

	Int32 globalIndex = -1;
	error = inflectionsList->GetCurrentGlobalIndex(&globalIndex);
	if (error != eOK) 
		return error;
			
	Int32 referenceCount = 0;
	error = inflectionsList->GetReferenceCount(globalIndex, &referenceCount);
	if (error != eOK)
		return error;
				
	Int32 listIdx = 0;
	Int32 listEntryIdx = 0;
	Int32 transIdx = 0;
	Int32 shiftIdx = 0;

	for (Int32 j = 0; j < referenceCount; ++j)
	{
		error = inflectionsList->GetFullTextTranslationData(globalIndex, j, &listIdx, &listEntryIdx, &transIdx, &shiftIdx);
		if (error != eOK)
			break;
				
		CSldList *list = 0;
		error = aDict->GetWordList(listIdx, (ISldList**)&list);
		if (error != eOK) 
			break;

		error = list->GetWordByIndex(listEntryIdx);
		if (error != eOK)
			break;

		const CSldListInfo *listInfo = 0;
		error = list->GetWordListInfo(&listInfo);
		if (error != eOK) 
			break;

		UInt16 *word = 0;
		error = list->GetCurrentWord(listInfo->GetShowVariantIndex(), &word);
		if (error != eOK)
			break;
			
		bool present = false;
		for (const auto & form : aMorphologyForms)
		{
			if (CSldCompare::StrCmp(word, form.MorphologyForm.c_str()) == 0)
			{
				present = true;
				break;
			}
		}

		if (!present && !(aWithoutRequest && CSldCompare::StrCmp(word, aText) == 0) && aType != eMorphoDerivative)
		{
			aMorphologyForms.push_back(TSldMorphologyWordStruct());

			aMorphologyForms.back().MorphologyForm = word;
			aMorphologyForms.back().MorphologyFormType = eMorphologyWordTypeBase;
		}
					
		UInt32 baseFormIsFound = 0;
		error = baseFormsList->GetMostSimilarWordByText(word, &baseFormIsFound);
		if (error != eOK)
			break;

		if (aType == eMorphoBase)
			continue;
					
		if (baseFormIsFound)
		{
			Int32 globalIndex = -1;
			error = baseFormsList->GetCurrentGlobalIndex(&globalIndex);
			if (error != eOK)
				break;

			Int32 referenceCount = 0;
			error = baseFormsList->GetReferenceCount(globalIndex, &referenceCount);
			if (error != eOK)
				break;
			
			Int32 inflListIdx = 0;
			Int32 inflListEntryIdx = 0;
			Int32 inflTransIdx = 0;
			Int32 inflShiftIdx = 0;	
			for (Int32 translationIndex = 0; translationIndex < referenceCount; translationIndex++)
			{
				error = baseFormsList->GetFullTextTranslationData(globalIndex, translationIndex, &inflListIdx, &inflListEntryIdx, &inflTransIdx, &inflShiftIdx);
				if (error != eOK)
					break;

				CSldList *inflList = 0;
				error = aDict->GetWordList(inflListIdx, (ISldList**)&inflList);
				if (error != eOK) 
					break;

				error = inflList->GetWordByIndex(inflListEntryIdx);
				if (error != eOK)
					break;
						
				UInt16 *inflWord = 0;
				error = inflList->GetCurrentWord(0, &inflWord);
				if (error != eOK) 
					break;
						
				bool present = false;
				for (const auto & form : aMorphologyForms)
				{
					if (CSldCompare::StrCmp(inflWord, form.MorphologyForm.c_str()) == 0) {
						present = true;
						break;
					}
				}
				if (!present && !(aWithoutRequest && CSldCompare::StrCmp(inflWord, aText) == 0))
				{
					aMorphologyForms.push_back(TSldMorphologyWordStruct());

					aMorphologyForms.back().MorphologyForm = inflWord;
					aMorphologyForms.back().MorphologyFormType = eMorphologyWordTypeWordform;
				}
			}
		}
	}

	return eOK;
}

/***********************************************************************
* Преобразует строку поиска, в набор морфологических форм введенных слов
*
* @param aText					- исходная строка запроса
* @param aDict					- указатель на класс открытого словаря
* @param aLangCode				- код языка запроса
* @param aMorphologyForms		- указатель, по которому будут записаны строки словоформ
*
* @return код ошибки
************************************************************************/
ESldError PrepareQueryForSortingSearchResults(const UInt16* aText, CSldDictionary* aDict, UInt32 aLangCode, CSldVector<TSldMorphologyWordStruct>& aMorphologyForms)
{
	if (!aText || !aDict)
		return eMemoryNullPointer;

	aMorphologyForms.clear();

#ifndef NOT_USING_MORPHOLOGY_IN_AUXILIARY
	ESldError ProcessError;

	// Длина строки исходного запроса
	Int32 TextLen = CSldCompare::StrLen(aText);
	if (!TextLen)
		return eOK;

	const CSldCompare *CMP;
	ProcessError = aDict->GetCompare(&CMP);
	if (ProcessError != eOK)
		return ProcessError;

	SldU16WordsArray TextWords;
	ProcessError = CMP->DivideQueryByParts(aText, TextWords);
	if (ProcessError != eOK || TextWords.empty())
		return ProcessError;

	// Создаем запрос
	for (const auto & word : TextWords)
	{
		ProcessError = GetMorphoForms(word.c_str(), aDict, aLangCode, aMorphologyForms, 1);
		if (ProcessError != eOK)
			return ProcessError;
	}
#endif

	return eOK;
}
#endif