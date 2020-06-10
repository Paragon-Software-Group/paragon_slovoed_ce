/**
****************************************************************
*   @file LastCharMap.cpp
*
*   (c) Paragon Software (SHDD) 2004
*
*   Имплементация класса RulesByLastChar
*
*   \author Исаев Андрей.
*
************************************************************** */

#include "MorphoData_v1.h"

#ifdef _ML_PARSER_

//////////////////////////////////////////////////////////////////////
//
// конструктор/деструктор
//
//////////////////////////////////////////////////////////////////////
RulesByLastChar_v1::RulesByLastChar_v1()
{
	callParams_allocated = 0;
	callParams_used		 = 0;
	callParamsIdx		 = 0;
	sldMemZero(callParams,sizeof(callParams),0);

	sldMemZero(lastCharMap,sizeof(lastCharMap),0);
};


RulesByLastChar_v1::~RulesByLastChar_v1()
{
}


//////////////////////////////////////////////////////////////////////
///
/// вставляем одну структуру в вектор и корректируем таблицу
///
//////////////////////////////////////////////////////////////////////
void RulesByLastChar_v1::InsertOne(OneCallParameters *str, MCHAR lastchar, Int32 pos)
{
	/// запихиваем в массив

	// не пора ли переаллокировать?
	if ( callParams_used + 1 > callParams_allocated )
	{
	OneCallParameters *newMem;

		// allocate new chunk of memory
		newMem = (OneCallParameters*)sldMemNew( (callParams_allocated + MEM_ALLOCATION_INCREMENT_STEP) * sizeof(OneCallParameters));

		if ( callParams->callParams )
		{
			// move old data
			MemMove( newMem, callParams->callParams, callParams_used * sizeof(OneCallParameters) );

			// free old memory
			sldMemFree(callParams->callParams);
		}

		// adjust data
		callParams->callParams = newMem;
		callParams_allocated += MEM_ALLOCATION_INCREMENT_STEP;
	}

	// сдвигаем память
	MemMove( &callParams->callParams[pos+1], &callParams->callParams[pos], (callParams_used - pos) * sizeof(OneCallParameters) );

	// вставляем
	callParams->callParams[pos] = *str;
	callParams_used++;
	callParams->num++;


	// корректируем ссылки
	for(UInt32 i = (UInt8)lastchar + 1 ; i<257; i++)
		lastCharMap[i].index++;
}


//////////////////////////////////////////////////////////////////////
///
/// Ускорение процедуры восстановления формы слова.
///
/// Добавление в класс одного конкретного правила
///
//////////////////////////////////////////////////////////////////////
void RulesByLastChar_v1::AddSingleRule(const MCHAR* precondition, const MCHAR* rule, const MorphoInflectionRulesSet_v1 *ruleset)
{
OneCallParameters		rul;
const MCHAR*			rule_part = rule;
UInt8			last_char;

	// vector iterator
	Int32 l_rulesIter;

	// check for closed class precondition
	if ( precondition[0] == '!' )
		precondition++;

	// то, что общее для всех половинок правила
	rul.precondition	= precondition;
	rul.ruleset			= ruleset;
	rul.onlyOneVariant  = 1;

	// добавляем для всех половинок правила
	while( *rule_part )
	{
		// init structure
		rul.rule			= rule_part;
		rul.pre_last        = 0;
		rul.matchPrevious   = 0;

		// меряем длину данной половинки правила
		for( rul.rule_len = 0 ; rule_part[rul.rule_len] ; rul.rule_len++ )
		{
			if ( rule_part[rul.rule_len] == '/' )
				break;
		}

		if ( rul.rule_len )
		{
			// если возможно, получим предпоследний символ для ускорения
			if ( rul.rule_len > 1 )
			{
				// предпоследний символ
				last_char = rule_part[rul.rule_len-2];

				// спецсимвол?
				if ( last_char == '*' || last_char == '$' || last_char == '?' || last_char == '(' || last_char == '<' || last_char == '[' )
					// такое правило надо применять ко всем словам...
					last_char = 0;

				rul.pre_last = last_char;
			}

			// определим, что же за последний символ должен быть у слова, если его сделали этим правилом
			last_char = rule_part[rul.rule_len-1];

			// спецсимвол?
			if ( last_char == '*' || last_char == '$' || last_char == '(' || last_char == '<' || last_char == '[' )
			{
				// такое правило надо применять ко всем словам...
				last_char = 0;
				rul.pre_last = 0;
			}
			else if ( last_char == '?' )
				// такое правило надо применять ко всем словам...
				last_char = 0;

			// find (or insert new) this char rules list
			l_rulesIter = lastCharMap[last_char].index;

			// флаг - еще не добавили
			bool hasBeenAdded = false;

			// сравним с ранее добавленными правилами, сортируя на ходу
			while( l_rulesIter != lastCharMap[last_char+1].index )
			{
				// меньшие буквы пердпосление буквы правила сортируем вперед
				if ( (UInt8)rul.pre_last < (UInt8)callParams->callParams[l_rulesIter].pre_last )
					break;
				else if ( (UInt8)rul.pre_last == (UInt8)callParams->callParams[l_rulesIter].pre_last )
				{
					// короткие правила вперед
					if ( rul.rule_len < callParams->callParams[l_rulesIter].rule_len  )
						break;
					else if ( rul.rule_len == callParams->callParams[l_rulesIter].rule_len  )
					{
						// сравниваем правило
						bool eqRule = true;

						// проводим побайтное сравнение, так как мы обрабатываем
						// и частичные правила
						if ( callParams->callParams[l_rulesIter].rule != rule_part )
						{
							for(Int32 i=0; i < rul.rule_len ; i++)
							{
								MCHAR ch1 = callParams->callParams[l_rulesIter].rule[i];
								MCHAR ch2 = rule_part[i];
								if ( ch1 != ch2 )
								{
									// <germ>
									// Считаем эти спецсимволы равноценными в правиле при восстановлении исходной формы слова
									if ((ch1=='('&&ch2=='<') || (ch1=='<'&&ch2=='('))
										continue;
									// </germ>
									
									eqRule = false;
									break;
								}
							}
						}

						// сравниваем предусловие
						bool eqPreconditions = true;

						// так как у некоторых предусловий может стоять в начале !, 
						// но мы его пропускаем, необходимо провести еще и побайтное 
						// сравнение
						//
						// а если правила уже отличаются, то и сравнимать нечего...
						if ( eqRule && callParams->callParams[l_rulesIter].precondition != precondition )
						{
							for(Int32 i=0; true; i++)
							{
								if ( callParams->callParams[l_rulesIter].precondition[i] != precondition[i] )
								{
									eqPreconditions = false;
									break;
								}

								if ( !precondition[i] )
									break;
							}
						}

						// полностью совпадает?
						if( eqRule && eqPreconditions )
						{
							// правило и предусловие полностью совпадают...
							// проверим, а не одно и тот же ли это подкласс?
							if ( callParams->callParams[l_rulesIter].ruleset == ruleset )
							{
								// просто игнорируем его, не добавляя
								hasBeenAdded = true;
								break;
							}

							// это правило полностью совпадает с предыдущим
							rul.matchPrevious = 1;

							// пока не добавляем, смотрим дальше...
						}
						else if ( rul.matchPrevious == 1 )
						{
							// если мы только что просмотрели группу полностью совпадающих
							// правил - добавляем в конец этой группы
							InsertOne(&rul,last_char,l_rulesIter);
							hasBeenAdded = true;

							// воткнули, прекращаем работу
							break;
						}
					} // rul.rule_len
				} // rul.pre_last

				l_rulesIter++;
			}

			// еще не добавили? добавляем в конец
			if ( !hasBeenAdded )
				InsertOne(&rul,last_char,l_rulesIter);

		} // if (rul.rule_len)

		// переходим к следующей половинке
		rule_part += rul.rule_len;
		if ( *rule_part == '/' )
			rule_part++;

	} // while (*rule_part)
}


//////////////////////////////////////////////////////////////////////
///
/// Добавление во внутренний индексированный массив еще одного 
/// MorphoInflectionRulesSet (набора правил изменения слова), 
/// включая все его поднаборы.
///
/// @param morpho  - cсылка на класс морфологии для разрешения
///                 ссылок на текстовые строки в базе данных
///
/// @param ruleset - указатель на добавляемый набор правил
///
/// @param base_ruleset - указатель на исходный набор правил для
///            этого поднабора. Используется только для рекурсивных
///            вызовов, во всех остальных слуаях должно быть 0.
///
/// @return  Ничего не возвращает.
///
//////////////////////////////////////////////////////////////////////
void RulesByLastChar_v1::AddRuleset(const MorphoData_v1 &morpho, const MorphoInflectionRulesSet_v1 *ruleset, const MorphoInflectionRulesSet_v1 *base_ruleset)
{
	if ( base_ruleset == NULL )
		base_ruleset = ruleset;

	// iterate rules
	MorphoInflectionRulesSet_v1::RulesIterator rule;
	rule = ruleset->RulesBegin();

	// check preconditions of subrules
	while(rule != ruleset->RulesEnd())
	{
		if ( rule->IsLinkToRuleset() )
		{
			AddRuleset(morpho, morpho.Rule2RuleSetPtr(*rule), base_ruleset);
		}
		else if ( morpho.Rule2MCharPtr(*rule)[0] )
		{
			AddSingleRule(morpho.McharRef2Ptr(ruleset->precondition),morpho.Rule2MCharPtr(*rule), base_ruleset);
		}
		rule++;
	}
}

#endif

//////////////////////////////////////////////////////////////////////
///
/// Получает первый вариант для восстановления исходной формы слова
/// по его произвольной форме. Находит первый подходящий по последней
/// букве слова вариант, и возвращает указатель на него. Так же инициализирует
/// итератор для быстрого дальнейшего получания следующих вариантов.
/// Класс должен быть предварительно инициализирован с помощью AddRuleset()
///
/// @param iter  - Указатель на итератор. Он будет проинициализирован 
///          для последующих вызовов GetNextRuleset()
///
/// @param last_char - последняя буква слова.
///
/// @return  Указатель на структуру OneCallParameters, содержащую
///          все необходимые данные, для того, чтобы попытаться
///          восстановить исходную форму слова, либо NULL,
///			 если нет правил порождающие слова с такой последней буквой.
///
/// @see GetNextRuleset()
///
//////////////////////////////////////////////////////////////////////
const RulesByLastChar_v1::OneCallParameters* RulesByLastChar_v1::GetFirstRuleset(CallParamIterator &iter, MCHAR last_char) const
{
	iter.curIndex  = MORPHO_SWAP_16(lastCharMap[(UInt8)last_char  ].index);
	iter.lastIndex = MORPHO_SWAP_16(lastCharMap[(UInt8)last_char+1].index);

	// if records for this las char
	if ( iter.curIndex == iter.lastIndex )
	{
		// try to use default records list
		iter.curIndex  = MORPHO_SWAP_16(lastCharMap[0].index);
		iter.lastIndex = MORPHO_SWAP_16(lastCharMap[1].index);
	}

	// empty list - return
	if ( iter.curIndex == iter.lastIndex )
		return NULL;

	// return first result
	return GetNextRuleset(iter);
}

//////////////////////////////////////////////////////////////////////
///
/// Получает следующий вариант для восстановления исходной формы слова
/// по его произвольной форме.
///
/// @param iter  - Указатель на итератор, ранее проинициализированный
///          функцией GetFirstRuleset()
///
/// @return  Указатель на структуру OneCallParameters, содержащую
///          все необходимые данные, для того, чтобы попытаться
///          восстановить исходную форму слова, либо NULL,
///			 если больше нет правил порождающие слова с такой последней буквой.
///
/// @see GetFirstRuleset()
///
//////////////////////////////////////////////////////////////////////
const RulesByLastChar_v1::OneCallParameters* RulesByLastChar_v1::GetNextRuleset(CallParamIterator &iter) const
{
	// reached end ?
	if ( iter.curIndex == iter.lastIndex )
	{
		// is this default sequence?
		if ( iter.lastIndex == MORPHO_SWAP_16(lastCharMap[1].index) )
			return NULL;
		else
		{
			// try to use default records list
			iter.curIndex  = MORPHO_SWAP_16(lastCharMap[0].index);
			iter.lastIndex = MORPHO_SWAP_16(lastCharMap[1].index);

			// empty list - return
			if ( iter.curIndex == iter.lastIndex )
				return NULL;

			// return first result
			return GetParamByIndex(iter.curIndex++);
		}
	}
	else
		// return result and increment iterator
		return GetParamByIndex(iter.curIndex++);
}
