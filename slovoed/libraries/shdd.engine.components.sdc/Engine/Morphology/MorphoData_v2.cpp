///////////////////////////////////////////////////////////////////////////////////
///
/// @file MorphoData.cpp
///
/// Copyright Paragon Software (SHDD), 2004
///
/// Файл содержит код основных методов класса MorphoData для работы
/// с морфологическими данными, которые одинаковы как для ml-парсера, 
/// так и для ядра морфологии. 
///
/// \author Исаев Андрей.
///
///////////////////////////////////////////////////////////////////////////////////

#include "MorphoData_v2.h"

// варианты написания слова относительно регистра букв
// используется функциями MorphoData::InitWritingVersionIterator, MorphoData::GetNextWritingVersion
#define VARIANT_TO_TRY__AS_IS				0x01
#define VARIANT_TO_TRY__ALL_LOWER			0x02
#define VARIANT_TO_TRY__FIRST_UPPER			0x04


///////////////////////////////////////////////////////////////////////////////////
/// 
/// Проверяет слово на соответсвие условию, заполняет буфер сответствия.
///
/// @param word   - Указатель на слово
///
///	@param posBuf - Указатель на буфер для хранения
///				    ссылок на соответсвующие символы
///					слова для каждого символа условия ("буфер соответсвия").
///
///	@param precondition - Указатель на условие
///
/// @return		1 если слово удовлетворяет правилу, иначе 0
///
///////////////////////////////////////////////////////////////////////////////////
Int32 MorphoData_v2::ApplyPrecondition(const MCHAR *word, const MCHAR **posBuf, const MCHAR* precondition)
{
	while (*precondition && *word)
	{
		// save position of this rule
		*posBuf = word;

		switch(*precondition)
		{
		case '*':

			if ( *(precondition+1) == 0 )
			{
				// this '*' is on the end of precondition rule - we will surely match it
				*(posBuf+1) = 0;
//				return 1;	// 
			}
			else
			{
			Int32 end;

				// find end of the word. 
				// Current char is obviouesly != 0
				for( end=1; word[end]; end++);

				// check hypoteses starting from end of the word
				// ('*' should represen at least 1 character)
				while( end > 0 ) 
				{
					// found matching sequence - stop
					if ( ApplyPrecondition(word+end, posBuf+1, precondition+1) )
						break;

					end--;
				}

				// if unable to find matching sequence - stop
				if ( end == 0 )
					return 0;
				// else - whole string match, don't have to continue
/*				else
					return 1;
*/			}
			return 1;
			//break;


		case '$':

			if ( *(precondition+1) == 0 )
			{
				// this '$' is on the end of precondition rule - we will surely match it
				*(posBuf+1) = 0;
//				return 1;
			}
			else
			{
			Int32 pos;

				// check hypoteses starting from next char
				// ('$' could represen any char number, even zero)
				for( pos=0; ; pos++)
				{
					// found matching sequence - stop
					if ( ApplyPrecondition(word+pos, posBuf+1, precondition+1) )
						break;

					// unable to find matching position - return
					if ( word[pos] == 0 )
						return 0;
				}

				// whole string match, array is filled, don't have to continue
//				return 1;
			}

			return 1;
//			break;

		case '(':
		case '<':
		case '[':
			{
			const MCHAR *t;
			const MCHAR *w;
			const MCHAR *end;

				// найдем конец условия
				for ( end = precondition+1; *end != ')' && *end != '>' && *end != ']' ; end++ )
				{
					// encountered line end before ')' - this is error in template
					if ( *end == 0 )
						return 0;
				}

				// следующий символ за концом...
				end++;

				// первый вариант из условия
				t = precondition+1;

				// look for appropriate variant from listed ones
				while (1)
				{
					// return to first char of remaining word portion
					w = word;

					// compare this variant
					while(1)
					{
						// encountered line end before ')' - this is error in template
						if ( *t == 0 )
							return 0;

						// reached end of this variant
						if ( *t == ',' || *t == ')' || *t == '>' || *t == ']' )
							break;

						// characters differ - stop
						if ( *t != *w )
							break;

						// compare next characters
						t++;
						w++;
					}

					// was comparison succesfull?
					if ( *t == ',' || *t == ')' || *t == '>' || *t == ']' )
					{
						// рекурсивно проверим остаток слова
						if ( ApplyPrecondition(w, posBuf + (end-precondition), end) )
						{
							// слово подошло

							// осталось дозаполнить буфер
							while( precondition < end )
							{
								precondition++;
								posBuf++;
								*posBuf = w;
							}

							return 1;
						}
						// иначе просто продолжаем просматирвать варианты...
					}

					// comparison was unsuccessful... look for next variant
					while(1)
					{
						// encountered line end before ')' - this is error in template
						if ( *t == 0 )
							return 0;

						// reached end of variants list - comparison is unsuccessful
						if ( *t == ')' || *t == '>' || *t == ']' )
							return 0;

						if ( *t == ',' )
							break;

						t++;
					}

					// skip ',' and continue
					t++;
				} // while (1)
			}

		case '?':
			// single char template - always match
			word++;
			precondition++;
			posBuf++;
			break;

		case '!':
			// special sign - we should ignore it 
			precondition++;
			posBuf++;
			break;

		default:
			// none of the special chars - just try to match it
			if ( *precondition == *word )
			{
				word++;
				precondition++;
				posBuf++;
			}
			else
				return 0; // do not match
			break;
		}
	}

	// if both strings have finished simultaneously - comarison is done
	if ( *word == 0 &&  *precondition == 0 )
	{
		// set end mark in position array
		*posBuf = 0;
		return 1;
	}
	else if (  *word == 0 &&  *precondition == '$' && *(precondition+1) == 0 )
	{
		// save position of this rule
		*posBuf = word;
		*(posBuf+1) = 0;
		return 1;
	}
	else
		return 0; // strings do not match
}


///////////////////////////////////////////////////////////////////////////////////
///
/// Применяет правило изменения к слову.
///
/// @param word         - Указатель на слово (базовая форма)
///
///	@param precondition - Указатель на предусловие
///
///	@param rule         - Указатель на правило
///
///	@param buf          - Буфер, куда будет записана форма слова
///
///	@param pos          - Временный буфер для храниения ссылок
///
/// @return		        указатель буфер с результатом
///
///////////////////////////////////////////////////////////////////////////////////
const MCHAR* MorphoData_v2::InflectWord(const MCHAR *word, const MCHAR *precondition, const MCHAR *rule, MCHAR *buf, const MCHAR **pos)
{
const MCHAR				*w,*next;
Int32						i;
MCHAR					*b;

	// fill pos array and check for matching
	if ( ApplyPrecondition(word, pos, precondition) )
	{
		// init variables
		i = 0;
		b = buf;
		
		// <germ>
		// Буфер для приставки, в предусловии стоит в ()
		MCHAR apref[MAX_WORD_LEN] = {0};
		MCHAR* ap = apref;
		MCHAR ch;
		// </germ>

		while(*rule)
		{
			switch(*rule)
			{
			case '*':
			case '$':
			case '?':
			case '!':
			case '(':
			case '<':
			case '[':

				// look for next similar directive in precondition
				
				// <germ>
				ch = 0;
				if (*rule == '<')
					ch = '(';
					
				for( ; precondition[i] ; i++)
				{
					if ( precondition[i] == *rule || precondition[i] == ch )
						break;
				}
				// </germ>

				// check for error
				if ( precondition[i] == 0 )
				{
					// did not found similar directive in precondition
					return("<< ERROR2!!! >>>");
				}

				// directive found. copy its content
				w    = pos[i];
				next = pos[i+1]; // if we have come to postion of next char from precondition - break
				
				// </germ>
				if (*rule == '<')
				{
					// Копируем приставку не сразу в конечный буфер, а в специальный, чтобы потом ее отделить от формы,
					// то есть дописать в конец формы слова
					while( *w && w != next )
						*ap++ = *w++;
				}
				else
				{
					while( *w && w != next )
						*b++ = *w++;
				}
				// </germ>
				
				// skip this character from rule
				rule++;
				i++;

				break;

			case '/':

				// here comes another varion of this word
				
				// <germ>
				*ap = 0;
				ap = apref;
				if (*ap)
				{
					// копируем приставку
					*b++ = ' ';
					while (*ap)
						*b++ = *ap++;
					
					// устанавливаемся на начало буфера, могут быть еще отделяемые приставки
					ap = apref;
				}
				// </germ>
				
				// copy slash itself
				*b++ = *rule++;
				
				// rewind rule back
				i = 0;
				
				break;

			default:
			
				// all other chars just copy
				*b++ = *rule++;
				break;

			}
		}

		// <germ>
		*ap = 0;
		ap = apref;
		if (*ap)
		{
			// копируем приставку
			*b++ = ' ';
			while (*ap)
				*b++ = *ap++;
		}
		// </germ>
		
		// write end of line
		*b = 0;

		// return pointer to resulting buffer
		return buf;
	}
	else
	{
		// ERROR! word does not match its
		return("<< ERROR1!!! >>>");
	}
}


///////////////////////////////////////////////////////////////////////////////////
///
/// Восстанавливает исходное слово по этому правилу
///
/// @param          word_form		Указатель на в этой форме
///
///	@param			precondition	Указатель на условие
///
///	@param			rule			Указатель на правило
///
///	@param			rule_len		Длина правила (она может быть
///								    меньше реальной длины правила)
///
///	@param			restored		Буфер для хранения 
///								    восстановленного слова
///
///	@param			onlyOneVariant  если \b true - в этом правиле точно 
///                                 нет разделителя '/'
///
/// @return		\b true если удалось или нет применить правило, иначе \b false
///
///////////////////////////////////////////////////////////////////////////////////
bool MorphoData_v2::RestoreWord(const MCHAR *word_form, const MCHAR *precondition, const MCHAR *rule, Int32 rule_len, MCHAR* restored, Int32 onlyOneVariant)
{
const MCHAR				*w,*p;
MCHAR					*r;
Int32						left;
MCHAR					c;

	// если в правиле несколько вариантов - обработаем их рекурсивно
	if ( !onlyOneVariant )
	{
	bool found;
	Int32 start = 0 , cur;

		for(cur=0; cur<rule_len; cur++)
		{
			if ( rule[cur] == '/' )
			{
				// call recursively for this variant only
				found = RestoreWord(word_form, precondition, rule + start, cur, restored, true);
				
				// found??? COOL!
				if (found)
				{
					return true;
				}

				// skip first variant
				start = cur+1;
			}
		}

		// остался один необработаный вариант - обрабатываем его прямо тут...

		// отрезаем проверенные варианты
		rule = rule + start;
		rule_len = rule_len - start;
	}

	// простейшая проверка для ускорения рекурсивных вызовов

	// если правило не пустое, а слово пустое
	if ( rule_len )
	{
		// тогда слово не может быть ен пустым
		if ( ! *word_form )
		{
			return false;
		}
	}
	// дополнительное ускорение на случай, если правило пустое
	else if ( *word_form )
	{
		// при пустом правиле слово не может быть не пустым...
		return false;
	}


	// простейшее условие выполнилось - пробуем более точные методы
	
	left = 0;
	w = word_form;
	p = precondition;
	r = restored;


	MCHAR ch;
	// Движемся слева направо по правилу
	while (1)
	{
		ch = 0;
		if ( left < rule_len )
		{
			c = rule[left];
			// <germ>
			if (c == '<')
				ch = '(';
			// </germ>
		}
		else
			c = 0;

		// если встретили особый символ - восстанавливаем 
		// соответсвующую позицию в предусловии
		if ( c == 0 || c == '(' || c == '<' || c == '*' || c == '$' || c == '?' || c == '[' )
		{
			// ищем аналогичную конструкцию в исходном слове
			while(1)
			{
				if ( *p == c || *p == ch )
					// нашли искомую конструкцию
					break;

				if ( *p == 0 )
					// тоже хорошо
					break;

				// нет такой конструкции в предусловии !!!
				// Видимо, ошибка в правилах...
				if ( *p == 0 )
				{
					return false;
				}

				// необратимое правило? (нарушен порядок следования спец-символов)
				if ( *p == '(' || *p == '<' || *p == '$' || *p == '*' || *p == '?' )
				{
					return false;
				}

				// обычный символ - восстанавливаем его в исходном слове
				*r++ = *p++;
			}
		}


		// достигли конца слова или варианта
		if ( c == 0 )
		{
			// если правило уже закончилось, а слово еще нет 
			// - окончание у слова оказалось длиннеее
			if ( *w )
			{
				return false;
			}

			// ставим нолик в конце
			*r++ = 0;

			// слово успешно восстановлено
			return true;
		}

		// единичный символ из исходного слова
		if ( c == '?' )
		{
			// восстанавливаем его в исходном слове
			*r++ = *w++;

			// пропускаем '?' в предусловии
			p++;

			// следующий...
			left++;
			continue;
		}

		// спецсимол - игнорируем
		if ( c == '!' )
		{
			// пропускаем '!' в предусловии
			p++;

			// следующий...
			left++;
			continue;
		}

		// обрабатываем альтернативную конструкцию
		if ( c == '(' || c == '<' || c == '[' )
		{
		const MCHAR *variant;

			p++; // skip '(' in precondition

			// look for appropriate variant from listed ones
			while (1)
			{
				// return to first char of remaining word portion
				variant = w;

				// compare this variant
				while(1)
				{
					// encountered line end before ')' - this is error in precondition
					if ( *p == 0 )
					{
						return false;
					}

					// reached end of this variant
					if ( *p == ',' || *p == ')' || *p == '>' || *p == ']' )
						break;

					// characters differ - stop
					if ( *p != *variant )
						break;

					// compare next characters
					p++;
					variant++;
				}

				// was comparison succesfull?
				if ( *p == ',' || *p == ')' || *p == '>' || *p == ']' )
				{
					// restore proper variant
					while ( w < variant )
						*r++ = *w++;

					// skip rest variants
					while(true)
					{
						// encountered line end before ')' - this is error in precondition
						if ( *p == 0 )
						{
							return false;
						}

						// reached end of this variant
						if ( *p == ')' || *p == '>' || *p == ']')
						{
							p++;
							break;
						}

						// compare next characters
						p++;
					}

					// просматриваем правило дальше
					left++;
					break;
				}

				// comparison was unsuccessful... look for next variant
				while(1)
				{
					// encountered line end before ')' - this is error in template
					if ( *p == 0 )
					{
						return false;
					}

					// reached end of variants list - comparison is unsuccessful
					if ( *p == ')' || *p == '>' || *p == ']' )
					{
						return false;
					}

					if ( *p == ',' )
						break;

					p++;
				}

				// skip ',' and continue
				p++;
			} // while (1)

			// continue moving thru rule
			continue;
		}

		// максимально замещающий символ
		if ( c == '*' )
		{
			Int32 match;

			// для начала возмем сопадающую длину равной всей длине слова
			for( match=0; w[match]; match++);

			// проверим различные гипотезы для '*'
			// содержащие как минимум один символ
			while ( match > 0 )
			{

				if ( RestoreWord( w + match, p+1, rule+left+1, rule_len-left-1, r + match, true) )
					break;

				match --;
			}

			if ( match > 0 )
			{
				// удалось найти подходящую гипотезу
				// копируем часть, означающую '*'
				// (хвост уже заполнен)
				while(match--)
					*r++ = *w++;

				return true;
			}

			// такой гипотезы не существует
			return false;
		}

		// миниматьно замещающий символ
		if ( c == '$' )
		{
			Int32 match;

			// проверим различные гипотезы для '$'
			// вклюяающие пустое множество
			for ( match = 0;  ; match++ )
			{
				if ( RestoreWord( w + match, p+1, rule+left+1, rule_len-left-1, r + match, true) )
					break;

				// не подошла последняя гипотеза - облом...
				if ( w[match] == 0 )
				{
					return false;
				}
			}

			// удалось найти подходящую гипотезу
			// копируем часть, означающую '$'
			// (хвост уже заполнен)
			while(match--)
				*r++ = *w++;
			return true;
		}

		// если ни один из выше перечисленных спец-символов - обычный символ,
		// - он должен присутсвовать в результирующем слове
		if ( c != *w )
		{
			return false;
		}

		// следующий символ - этот просто пропускаем, 
		// так как он был сгенерирован правилом, а не из 
		// исходного слова.
		left++;
		w++;
	}
}


///////////////////////////////////////////////////////////////////////////////////
///
/// Проверяет по внешним признакам, может ли слово 
/// изменяться по данному набору правил.
///
/// @param   word        Указатель на слово
///
///	@param	 ruleset     Указатель на набор правил
///
///	@param	 posBuf      Указатель на буфер для хранения
///						 ссылок на соответсвующие символы
///						 слова для каждого символа условия
///
///	@param	 flags      Флаги, указывающие на то, какую 
///						проверку надо делать.
///
///						Флаг #M_RULE_TOPMOST_PRECONDITION - укзывает,
///						что нужно проверить условие корневого набора правил.
///
///						Флаг #M_RULE_LOWEST_PRECONDITIONS - укзывает,
///						что нужно проверить условия конечных наборов правил.
///
///						Флаг #M_RULE_CLOSED_RULE_IS_OK - укзывает,
///						что не смотря на то, что правило закрытое,
///						нужно проверить условие. По умалчанию, никакое
///						слово не удовлетворяет закрытому правилу (начинающемуся
///						с восклицательного знака).
///
/// @return		\b true если слово удовлетворяет правилу, иначе \b false
///
///////////////////////////////////////////////////////////////////////////////////
bool MorphoData_v2::IsRuleApplyable(const MCHAR *word, const MorphoInflectionRulesSetHandle* ruleset_ptr, const MCHAR **posBuf,  Int32 flags) const
{
	// проверяли ли мы уже на предусловие этого правила
	bool checkedForThisPrecondition = false;

	const MorphoInflectionRulesSet_v2* ruleset = (const MorphoInflectionRulesSet_v2*)ruleset_ptr;
	
	// check for closed rule
	MorphoTestPtr(ruleset);
	MorphoTestPtr(&ruleset->precondition);
	
	if ( !(flags & M_RULE_CLOSED_RULE_IS_OK) && McharRef2Ptr(ruleset->precondition)[0] == '!' )
		return false;

	// check for precondition of this word
	if ( flags & M_RULE_TOPMOST_PRECONDITION )
	{
		MorphoTestPtr(ruleset);
		MorphoTestPtr(&ruleset->precondition);
		if(  !ApplyPrecondition(word, posBuf, McharRef2Ptr(ruleset->precondition) )  )
			return false;
		else
			checkedForThisPrecondition = true;
	}

	// 
	if ( flags & M_RULE_LOWEST_PRECONDITIONS )
	{
		// iterate rules
		MorphoInflectionRulesSet_v2::RulesIterator rule;
		rule = ruleset->RulesBegin();

		// check preconditions of subrules
		while(rule != ruleset->RulesEnd())
		{
			// если это поднабор правил, то слово должно ему удовлетворять
			if ( rule->IsLinkToRuleset() )
			{
				// there is a subrules list
				if ( !IsRuleApplyable(word, Rule2RuleSetPtr(*rule), posBuf, M_RULE_LOWEST_PRECONDITIONS | (flags & M_RULE_CLOSED_RULE_IS_OK) ) )
					return false;
			}
			// если же это и есть правило, то надо удовлетворять предусловию ЭТОГО правила
			else if ( Rule2MCharPtr(*rule)[0] ) // если правило вообще есть
			{
				if ( !checkedForThisPrecondition )
				{
					MorphoTestPtr(ruleset);
					MorphoTestPtr(&ruleset->precondition);
					if(  !ApplyPrecondition(word, posBuf, McharRef2Ptr(ruleset->precondition) ) )
						return false;
					else
						checkedForThisPrecondition = true;
				}
			}
			rule++;
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////
///
/// Готовит итератор для последовательного получения всех производных форм 
/// слова из базовой с помощью фукнции GetNextWordForm().
///
/// @param         iter            Ссылка на итератор, который необходимо 
///                                проинициализировать.
///
///	@param		   wordPtr         Указатель на буфер с исходным словом, формы которого мы будем 
///                                получать. Указатель должен оставаться валидным
///                                на протяжении всего процесса получения форм слова.
///
///	@param		   posBuf          Временный буфер размером не менее максимально возможной 
///                                длины формы слова. Указатель должен оставаться валидным
///                                на протяжении всего процесса получения форм слова.
///
/// @param         ruleset		   Указатель на набор правил, по которому мы будем это слово
///								   изменять. Этот указатель можно получить функцией
///								   GetNextBaseForm().
///
/// @param         flags           Флаги.
///
/// Возможные значения флагов:
///  - #M_FORMS_ONLY_FORMS \n
///              \copydoc M_FORMS_ONLY_FORMS
///  - #M_FORMS_DECORATE_WORD  \n
///              \copydoc M_FORMS_DECORATE_WORD
///  - #M_FORMS_SEPARATE_SUBFORM_NAMES \n
///              \copydoc M_FORMS_SEPARATE_SUBFORM_NAMES
///  - #M_FORMS_INCLUDE_HIDDEN_FORMS  \n  
///              \copydoc M_FORMS_INCLUDE_HIDDEN_FORMS
///
/// @see  WordFormsIterator, GetNextWordForm()
///
///////////////////////////////////////////////////////////////////////////////////
void MorphoData_v2::WordFormsIteratorInit(WordFormsIterator &iter, const MCHAR *wordPtr, const MorphoInflectionRulesSetHandle* ruleset, Int32 flags ) const
{
	// сохраняем все данные на будущее
	iter.wordPtr		= wordPtr;
	iter.ruleset		= ruleset;
	iter.cur_ruleset	= ruleset;
	iter.flags		    = flags;
	
	if (iter.posBuf)
		sldMemFree(iter.posBuf);
	iter.posBuf         = (const MCHAR**)sldMemNew(sizeof(MCHAR*)*MAX_POS_BUF_SIZE);
	if (!iter.posBuf)
		return;
	sldMemZero(iter.posBuf, sizeof(MCHAR*)*MAX_POS_BUF_SIZE);

	iter.table		    = GetTableByRulesetPtr(ruleset);
	iter.cur_table	    = iter.table;

	// нулевая глубина погружения в дерево
	iter.depth		 = 0;
	iter.alternative = 0;
	iter.vector[0]	 = 0;
}

///////////////////////////////////////////////////////////////////////////////////
///
/// Готовит итератор для последовательного получения всех производных форм 
/// слова из базовой с помощью фукнции GetNextWordForm().
///
/// @param         iter            Ссылка на итератор, который необходимо 
///                                проинициализировать.
///
///	@param		   wordPtr         Указатель на буфер с исходным словом, формы которого мы будем 
///                                получать. Указатель должен оставаться валидным
///                                на протяжении всего процесса получения форм слова.
///
///	@param		   posBuf          Временный буфер размером не менее максимально возможной 
///                                длины формы слова. Указатель должен оставаться валидным
///                                на протяжении всего процесса получения форм слова.
///
/// @param         ruleset		   Указатель на набор правил, по которому мы будем это слово
///								   изменять. Этот указатель можно получить функцией
///								   GetNextBaseForm().
///
/// @param         flags           Флаги.
///
/// Возможные значения флагов:
///  - #M_FORMS_ONLY_FORMS \n
///              \copydoc M_FORMS_ONLY_FORMS
///  - #M_FORMS_DECORATE_WORD  \n
///              \copydoc M_FORMS_DECORATE_WORD
///  - #M_FORMS_SEPARATE_SUBFORM_NAMES \n
///              \copydoc M_FORMS_SEPARATE_SUBFORM_NAMES
///  - #M_FORMS_INCLUDE_HIDDEN_FORMS  \n  
///              \copydoc M_FORMS_INCLUDE_HIDDEN_FORMS
///
/// @see  WordFormsIterator, GetNextWordForm()
///
///////////////////////////////////////////////////////////////////////////////////
void MorphoData_v2::WordFormsIteratorInitW(WordFormsIterator &iter, const UInt16 *wordPtr, const MorphoInflectionRulesSetHandle* ruleset, Int32 flags ) const
{
#ifdef _ML_PARSER_
	return;
#else
	CSldCompare::Unicode2ASCIIByLanguage(wordPtr, (UInt8 *)iter.word_form_buffer, SldLanguage::fromCode(m_Header->language));
	WordFormsIteratorInit(iter, iter.word_form_buffer, ruleset, flags);
#endif
}

///////////////////////////////////////////////////////////////////////////////////
///
/// Получить следующую форму слова	 
///
/// @param		   iter            Ссылка на итератор
///
///	@param		   formBuf         Указатель на буфер для 
///								   получения формы слова
///
///	@param		   questionPtr     Указатель на указатель
///								   для получения вопроса к этой
///								   форме слова или \b NULL, если она
///								   не интересует.
///
///	@param		   formNamePtr     Указатель на указатель
///								   для получения названия этой 
///								   формы слова или \b NULL, если оно
///								   не интересует.
///
///	@param		   isSubTableName  В случае, если был установлен
///								   флаг #M_FORMS_SEPARATE_SUBFORM_NAMES при 
///								   инициализации итератора,
///								   в эту переменную записывается
///								   информация о том, выдаем ли 
///								   мы сейчас форму слова или
///								   название подтаблицы.
///
///
/// @return	   \b true,   если форма слова получена, 
///			   \b false,  если форм больше нет
///
///
/// @see WordFormsIterator, WordFormsIteratorInit()
///
///////////////////////////////////////////////////////////////////////////////////
bool MorphoData_v2::GetNextWordForm(WordFormsIterator &iter, MCHAR *formBuf, const MCHAR**questionPtr, const MCHAR ** formNamePtr, bool *isSubTableName) const
{
const MorphoInflectionRule_v2   *rule;
const MorphoStateDescription_v2 *state;
const MCHAR* chr_rule;
UInt32 pos;
	
	// передвигаемся к следующей форме слова
	while(true)
	{
		// текцщая позиция в текущем правиле
		pos = iter.vector[iter.depth];

		// не кончилось ли уже правило?
		while ( pos >= ((const MorphoInflectionRulesSet_v2*)(iter.cur_ruleset))->rules.size() && iter.depth )
		{
			// если кончилось, надо подниматься выше к корню и сдвигаемся вправо
			pos = ++iter.vector[--iter.depth];

			// перевычисляем текущий набор правил и таблицу
			iter.cur_ruleset = iter.ruleset;
			iter.cur_table   = iter.table;

			for (Int32 i=0; i < iter.depth; i++ )
			{
				iter.cur_ruleset = Rule2RuleSetPtr(((const MorphoInflectionRulesSet_v2*)(iter.cur_ruleset))->rules[iter.vector[i]]);
				iter.cur_table   = ObjRef2Ptr(((const MorphoStateDescriptionTable_v2*)(iter.cur_table))->states[iter.vector[i]].table);
			}
		}

		// если уже поднялись до предела, а двигаться все еще некуда - обход уже закончен
		if ( pos >= ((const MorphoInflectionRulesSet_v2*)(iter.cur_ruleset))->rules.size() )
		{
			formBuf[0] = 0;
			return false;
		}

		// получаем указатель на текущее правило
		rule = &((const MorphoInflectionRulesSet_v2*)(iter.cur_ruleset))->rules[pos];

		// указаетель на описание текущей формы
		state = &(((const MorphoStateDescriptionTable_v2*)(iter.cur_table))->StateByIndex(pos));

		// если мы не игнорируем скрытые формы - обрабатываем текущую форму в любом случае.
		if ( iter.flags & M_FORMS_INCLUDE_HIDDEN_FORMS )
			break;

		// если данная форма не скрытая - обработаем ее
		
		MorphoTestPtr(state);
		MorphoTestPtr(&state->description);
		if ( McharRef2Ptr(state->description)[0] != '-' )
			break;

		// иначе - идем к следующей форме...
		iter.vector[iter.depth]++;
		iter.alternative = 0;
	}// while (true)

	// обрабатываем это правило
	if ( rule->IsLinkToRuleset() )
	{
		// случай ссылки на поднабор правил

		// опускаемся на подуровень
		iter.cur_ruleset = Rule2RuleSetPtr(((const MorphoInflectionRulesSet_v2*)(iter.cur_ruleset))->rules[pos]);
		iter.cur_table   = ObjRef2Ptr(((const MorphoStateDescriptionTable_v2*)(iter.cur_table))->states[pos].table);
		iter.vector[++iter.depth] = 0;
		iter.alternative = 0;

		// надо ли отдельно выводить название подтаблицы?
		if ( iter.flags & M_FORMS_SEPARATE_SUBFORM_NAMES )
		{
			// запишем указатель на название формы
			if ( formNamePtr )
			{
				MorphoTestPtr(state);
				MorphoTestPtr(&state->description);
				*formNamePtr = McharRef2Ptr(state->description);
			}

			// запишем указатель на название формы
			if ( questionPtr )
			{
				MorphoTestPtr(state);
				MorphoTestPtr(&state->question);
				*questionPtr = McharRef2Ptr(state->question);
			}

			// даем знать, что это название подтаблицы
			if ( isSubTableName )
				*isSubTableName = true;

			// очищаем буфер формы слова как признак того, что 
			// сейчас идет именно название подтаблицы
			formBuf[0] = 0;

			//
			return true;
		}
		else
		{
			// если нас интересуют только формы слова - вернем слово
			return GetNextWordForm(iter, formBuf, questionPtr, formNamePtr, isSubTableName);
		}
	}
	else
	{
		// не ссылка.

		// текст, описывающий правило изменения этого слова
		chr_rule = Rule2MCharPtr(*rule);

		// пустое ли правило?
		if ( chr_rule[0] )
		{
			// случай обычного конечного правила

			// запишем указатель на название формы
			if ( formNamePtr )
			{
				MorphoTestPtr(state);
				MorphoTestPtr(&state->description);
				*formNamePtr = McharRef2Ptr(state->description);
			}

			// запишем указатель на название формы
			if ( questionPtr )
			{
				MorphoTestPtr(state);
				MorphoTestPtr(&state->question);
				*questionPtr = McharRef2Ptr(state->question);
			}

			// даем знать, что это не название подтаблицы
			if ( isSubTableName )
				*isSubTableName = false;

			// получим форму слова
			MorphoTestPtr((const MorphoInflectionRulesSet_v2*)(iter.cur_ruleset));
			MorphoTestPtr(&((const MorphoInflectionRulesSet_v2*)(iter.cur_ruleset))->precondition);
			
			const MCHAR* result = InflectWord(iter.wordPtr, 
								  McharRef2Ptr(((const MorphoInflectionRulesSet_v2*)(iter.cur_ruleset))->precondition),
								  chr_rule,
								  formBuf,
								  iter.posBuf);

			// при получении слова произошла какая то ошибка и возвращенный результат некорректен
			// (на этом этапе уже сложно восстановиться после ошибки, т.к. она скорее всего связана с итератором, который мог поломаться раньше,
			// поэтому считаем, что формы больше получать нельзя)
			if (result != formBuf)
				return false;

			//
			Int32 count = 0;		// количество разделителей
			Int32 firstpos = -1;  // первая позиция символа требуемой формы
			Int32 length = 0;	    // длина нашего слова


			// сканируем строку на предмет разделителей
			for (Int32 i=0; true ; i++)
			{
				// конец всех форм слова?
				if( formBuf[i] == 0 )
					break;

				// конец этой формы слова?
				if( formBuf[i] == '/' )
				{
					count++;
					continue;
				}

				// если мы сканируем именно наше слово...
				if ( count == iter.alternative )
				{
					// запуоминаем позицию первого символа
					if ( firstpos < 0 )
						firstpos = i;

					// кописруем наше слово в начало буфера
					if ( firstpos )
						formBuf[i-firstpos] = formBuf[i];

					length++;
				}
			}

			// ставим признак конца слова
			formBuf[length] = 0;

			// надо декорировать форму слова?
			if ( iter.flags & M_FORMS_DECORATE_WORD )
			{
			const MCHAR* templ;

				// нужно в шаблоне заменить все '*' на само слово ...
				MorphoTestPtr(state);
				MorphoTestPtr(&state->templ);
				templ = McharRef2Ptr(state->templ);

				// заполним строку прямо в буфере сразу после формы слова
				MCHAR* p = formBuf+length;

				for(Int32 i=0; true; i++)
				{
					if ( templ[i] == '*' )
					{
						// вставляем слово
						for(Int32 j=0; j<length; j++)
						{
							*p = formBuf[j];
							p++;
						}
						// следующий символ из шаблона
						continue;
					}

					// копируем этот символ
					*p = templ[i];
					p++;

					// это уже конец? выходим...
					if ( templ[i] == 0 )
						break;
				}

				// а теперь сдвинем в начало массива
				sldMemMove(formBuf, formBuf+length,(Int32)(p-formBuf-length));
			}

			// если у текущего слова есть еще альтернативные формы...
			if ( iter.alternative < count )
			{
				iter.alternative++;

				// не делаем переход на следующую позицию
				return true;
			}

		}
		else if ( !state->HasSubtable() )
		{
			// правило пустое...

			// запишем указатель на название формы
			if ( formNamePtr )
			{
				MorphoTestPtr(state);
				MorphoTestPtr(&state->description);
				*formNamePtr = McharRef2Ptr(state->description);
			}

			// запишем указатель на название формы
			if ( questionPtr )
			{
				MorphoTestPtr(state);
				MorphoTestPtr(&state->question);
				*questionPtr = McharRef2Ptr(state->question);
			}

			// даем знать, что это не название подтаблицы
			if ( isSubTableName )
				*isSubTableName = false;

			// у слова просто нет такой формы.
			// ошибки нет, но форма пустая.
			formBuf[0] = 0;
		}
		else
		{
			// это слово, у которого просто отсутсвует один из поднаборов правил.
			// Но мы обязаны вернуть какое-то слово...

			// получаем следующее слово рекурсивно
			iter.vector[iter.depth]++;
			iter.alternative = 0;
			return GetNextWordForm(iter, formBuf, questionPtr, formNamePtr, isSubTableName);
		}
	}

	// переходим на следующее слово
	iter.vector[iter.depth]++;
	iter.alternative = 0;

	return true;
}

///////////////////////////////////////////////////////////////////////////////////
///
/// Получить следующую форму слова	 
///
/// @param		   iter            Ссылка на итератор
///
///	@param		   formBuf         Указатель на буфер для 
///								   получения формы слова
///
///	@param		   questionPtr     Указатель на указатель
///								   для получения вопроса к этой
///								   форме слова или \b NULL, если она
///								   не интересует.
///
///	@param		   formNamePtr     Указатель на указатель
///								   для получения названия этой 
///								   формы слова или \b NULL, если оно
///								   не интересует.
///
///	@param		   isSubTableName  В случае, если был установлен
///								   флаг #M_FORMS_SEPARATE_SUBFORM_NAMES при 
///								   инициализации итератора,
///								   в эту переменную записывается
///								   информация о том, выдаем ли 
///								   мы сейчас форму слова или
///								   название подтаблицы.
///
///
/// @return	   \b true,   если форма слова получена, 
///			   \b false,  если форм больше нет
///
///
/// @see WordFormsIterator, WordFormsIteratorInit()
///
///////////////////////////////////////////////////////////////////////////////////
bool MorphoData_v2::GetNextWordFormW(WordFormsIterator &iter, UInt16 *formBuf, const MCHAR**questionPtr, const MCHAR ** formNamePtr, bool *isSubTableName) const
{
#ifdef _ML_PARSER_
	return false;
#else
	MCHAR buff[MAX_WORD_LEN];
	bool ret = GetNextWordForm (iter, buff, questionPtr, formNamePtr, isSubTableName);
	if (ret)
		CSldCompare::ASCII2UnicodeByLanguage((UInt8 *)buff, formBuf, SldLanguage::fromCode(m_Header->language));
	return ret;
#endif
}

///////////////////////////////////////////////////////////////////////////////////
/// Получить заголовок базы морфологии
///////////////////////////////////////////////////////////////////////////////////
bool MorphoData_v2::GetMorphoHeader(const MorphoDataHeader** aHeader) const
{
	if (!m_Header)
		return false;

	*aHeader = m_Header;
	return true;
}

///////////////////////////////////////////////////////////////////////////////////
///
/// \page word_variants Работа с вариантами написания слов
///
/// Фунции MorphoData::BaseFormsIteratorInit() и MorphoData::GetNextBaseForm()
/// работают таким образом, что базовые формы проверяются на наличие в словаре
/// морфологической базы данных только с учетом регистра и различая буквы \c "е" и \c "ё", 
/// то есть точно так, как написано слово. 
///
/// Однако, это не всегда удобно. Например, если слово \c "человек" стоит в начале 
/// предложения, то оно будет написано с большой буквы. Однако, в словаре это 
/// слово пишется с маленькой буквы (с большой буквы в словаре пишутся только 
/// те слова, которые пишутся так всегда, например, имена собственные). 
/// Поэтому, при попытке найти базовую форму слова \c "Человеку", стоящего в начале 
/// предложения, способом, описанным \ref MorphoData::WordFormsIteratorInit() "ранее", мы потерпим неудачу.
///
/// Аналогичное разочарование нас ждет в случае, если мы попытаемся найти базовую
/// форму слова \c "ее", так как правильно эта форма местоимения \c "он" пишется
/// \c "её".
///
/// Для того, чтобы более гибко подойти к написанию формы слова, в ядре морфологии
/// предусмотрены функции MorphoData::WritingVersionIteratorInit() и
/// MorphoData::GetNextWritingVersion(), а так же итератор 
/// MorphoData::WritingVersionIterator. Эти функции используют некоторые специальные
/// знания о текущем языке для того, чтобы предложить все возможные формы написания
/// данной формы слова (с ё и без, с большой или маленькой буквы и т.д.), чтобы затем 
/// для каждого варианта уже попытаться восстановить базовую форму.
///
/// Пример использования:
///
/// \code
/// void ShowBaseForms(MorphoData &morpho, const char* word_form)
/// {
/// MorphoData::BaseFormsIterator bIter;
/// MorphoData::WritingVersionIterator vIter;
/// const MCHAR * posBuf[1024];          // буфер позиций
/// MCHAR buf[1024];                     // буфер для получения базовых форм
///	UInt32 wlen;
/// MCHAR variant[1024];                 // буфер для получения вариантов написания слова
///
///    // получаем длину входного слова
///    wlen = strlen();
///
///    // инициализируем итератор вариантов написания
///    morpho.WritingVersionIteratorInit(vIter,word_form,wlen,false);
///
///    // получаем вариант написания слова в буфер
///    while( wlen && morpho.GetNextWritingVersion(vIter,variant) )
///    {
///       // инициализируем итератор
///       morpho.BaseFormsIteratorInit(bIter, variant, posBuf, 
///                             M_BASEFORMS_SHOULD_MATCH_PRECONDITION |
///                             M_BASEFORMS_USE_DICTIONARY | 
///                             M_BASEFORMS_ONLY_DICTIONARY_WORDS );
/// 
///       // печатаем все базовые формы
///       while ( morpho.GetNextBaseForm(bIter, buf, &ruleset) )
///       {
///          printf("Base form: %s\n", buf);
///       }
///    }
/// }
/// \endcode
///
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
///
/// Инициализировать итератор вариантов написания слова перед вызовом 
/// GetNextWritingVersion().
/// 
/// @param		   iter            Ссылка на итератор, который необходимо 
///								   инициализировать.
///
///	@param		   wordForm        Буфер с исходным словом (не должен
///                                меняться пока вызывается GetNextWritingVersion())
///
///	@param		   length		   На входе: количество символов во входном 
///								   буфере. В буфере может находится несколько
///                                слов. Например, целая строка. Функция, используя
///                                знания о языке, выделит последовательность
///                                символов, которая может трактоваться
///                                как слово этого языка.\n
///								   На выходе: количестов символов из
///								   входного буфера, которые рассмотрено
///								   как текущее слово. Если это 0 - в начале
///                                буфера идет последовательность символов,
///								   которая не может быть словом.
///
///	@param		   strict          Строго или нет относиться к исходному 
///                                написанию слова. Имеет смысл использовать
///                                при проверке правописания слова 
///                                (в спелчекере). Напимер, если слово может
///                                писаться только с большой буквы и \b strict \b = \b true,
///                                то это слово, написанное с маленькой буквы,
///							       не будет предложено в качестве варианта,
///								   иначе будет.
///
/// @return	      Ничего не возвращает.
///
/// \copydoc word_variants
///
/// @see WritingVersionIterator, GetNextWritingVersion()
///
///
///////////////////////////////////////////////////////////////////////////////////
void MorphoData_v2::WritingVersionIteratorInit (WritingVersionIterator &iter, const MCHAR *wordForm, UInt32 &length , bool strict) const
{
UInt32 i;

	// обновляем итератор
	iter.wordForm = wordForm;
	iter.wordLen  = 0;
	iter.todo     = 0;
	iter.internalIndex1 = 0;
	iter.internalIndex2 = 0;

	// есть ли что-то в буфере?
	if ( length == 0 )
		return;

	/////////////////////////////////////////////////////////////////////////////////
	//
	// сначала выделим целое слово среди предложенного потока символов
	//

	// проверим первый символ слова. Может ли он служить началом слова?
	if ( 0 == (LetterAttributes(wordForm[0]) & CHARACTER_ATTRIBUTE_CHARACTER_IS_ALLOWED_IN_BEGINNING_OF_WORD ) )
	{
		length = 0;
		return;
	}

	// добавляем длину по одному символу
	UInt32 attr, curlen = 1;
	bool separator_included = false;

	while ( wordForm[curlen] && (curlen < length) )
	{
		attr = LetterAttributes(wordForm[curlen]);

		// символ не принадлежит алфавиту - значит, такое слово не может быть ни базовой формой, ни одной из производных
		if (!attr)
		{
			iter.wordLen = 0;
			length = 0;
			return;
		}

		// Если этот символ разделитель, тогда не смотря на другие символы считаем что мы достигли конца строчки.
		if (attr & CHARACTER_ATTRIBUTE_CHARACTER_IS_ALLOWED_SEPARATOR)
		{
			// После разделителя может быть отделяемая приставка немецкого глагола
			if ( GetLanguageCode() == MORPHO_LANGUAGE_GERMAN  && !separator_included )
			{
				MCHAR pref[MAX_WORD_LEN] = {0};
				MCHAR* pp = pref;
				UInt32 aattr;
				i = curlen;
				
				// Копируем часть после первого разделителя
				while (wordForm[i] && (i < length))
				{
					aattr = LetterAttributes(wordForm[i]);
					if (aattr & CHARACTER_ATTRIBUTE_CHARACTER_IS_ALLOWED_SEPARATOR)
					{
						// Слово после разделителя уже выделено
						if (*pref)
							break;
							
						// С другими разделителями отделенная часть не может являться отделяемой приставкой
						if ( !(wordForm[i]==' ' || wordForm[i]==0x9) )
							break;
							
						i++;
						continue;
					}
					*pp++ = wordForm[i++];
				}
				
				if ( !langSpecData.IsWordGermDetachablePrefix(pref) )
					break;
				
				separator_included = true;
			}
			else
				break;
		}

		// может ли следующий символ быть в середине или в конце слова?
		if ( (attr & CHARACTER_ATTRIBUTE_CHARACTER_IS_ALLOWED_IN_MIDDLE_OF_WORD) || (attr & CHARACTER_ATTRIBUTE_CHARACTER_IS_ALLOWED_AT_END_OF_WORD) )
		{
			// а может ли предыдущий символ быть в середине слова, или только в конце?
			// самый первый не проверяем, достаточно того, что он может быть в начале слова...
			if ( (curlen == 1) || ( LetterAttributes(wordForm[curlen-1]) & CHARACTER_ATTRIBUTE_CHARACTER_IS_ALLOWED_IN_MIDDLE_OF_WORD ) )
			{
				// добавляем символ к слову
				curlen++;
			}
			else
				// предыдущий символ мог стоять в конце слова, а не в середине
				break;
		}
		else
			// иначе, мы, вероятно, достигли конца допустимого слова
			break;
	}

	// случай символов, которые не могут стоять в конце слова, но могут в в середине - отрезаем их от конца слова
	// например,  дефис в русском языке.
	while ( (curlen > 1) && (( LetterAttributes(wordForm[curlen-1]) & CHARACTER_ATTRIBUTE_CHARACTER_IS_ALLOWED_AT_END_OF_WORD ) == 0 ) )
	{
		curlen--;
	}

	/////////////////////////////////////////////////////////////////////////////////
	//
	// теперь проверим регистр слова
	//
	bool haveLetters  = false;
	bool firstLetterIsUpper  = false;
	bool haveUpperAfterFirst = false;
	bool haveLowerAfterFirst = false;

	for (i=0; i<curlen; i++ )
	{
		attr = LetterAttributes(wordForm[i]);

		// первая буква в слове? (может быть и не буква: "'bout = about")
		if ( (attr & CHARACTER_ATTRIBUTE_IS_LETTER) && haveLetters == false )
		{
			haveLetters  = true;
			firstLetterIsUpper = ( attr & CHARACTER_ATTRIBUTE_IS_UPPER ) ? true : false;
		}
		else if ( attr & CHARACTER_ATTRIBUTE_IS_LETTER )
		{
			// не первая буква в слове
			if ( attr & CHARACTER_ATTRIBUTE_IS_UPPER )
				haveUpperAfterFirst = true;
			else
				haveLowerAfterFirst = true;
		}
	}

	if ( !haveLetters )
	{
		// какая-то херь: в слове нет букв!
		length = 0;
		return;
	}

	/////////////////////////////////////////////////////////////////////////////////
	//
	// инициализируем список вариантов написания слова относительно
	// строчности/заглавности букв
	//

	if ( firstLetterIsUpper == true && haveLowerAfterFirst == false )
	{
		// слово полностью написано большими буквами
		//
		// это может быть одна заглавная буква, 
		// либо аббриваетура, 
		// либо просто слово, выделенное для усиления смысла.
		//

		if ( haveUpperAfterFirst )
			// больше одной буквы в слове
            iter.todo = VARIANT_TO_TRY__AS_IS | VARIANT_TO_TRY__ALL_LOWER | VARIANT_TO_TRY__FIRST_UPPER;
		else
			// только одна буква в слове, поэтому VARIANT_TO_TRY__AS_IS == VARIANT_TO_TRY__FIRST_UPPER
            iter.todo = VARIANT_TO_TRY__AS_IS | VARIANT_TO_TRY__ALL_LOWER;

	}
	else if ( firstLetterIsUpper == true && haveUpperAfterFirst == false )
	{
		// только первая буква заглавная
		//
		// это может быть одна заглавная буква, 
		// либо имя собственное, 
		// либо просто слово, стоящее в начале предлжения.
		//

		if ( haveLowerAfterFirst )
			// больше одной буквы в слове ( VARIANT_TO_TRY__AS_IS == VARIANT_TO_TRY__FIRST_UPPER )
            iter.todo = VARIANT_TO_TRY__AS_IS | VARIANT_TO_TRY__ALL_LOWER;
		else
			// только одна буква в слове, поэтому  VARIANT_TO_TRY__AS_IS == VARIANT_TO_TRY__FIRST_UPPER
            iter.todo = VARIANT_TO_TRY__AS_IS | VARIANT_TO_TRY__ALL_LOWER;
	}
	else if ( firstLetterIsUpper == false && haveUpperAfterFirst == false )
	{
		// все буквы прописные
		//
		// либо обычное слово, либо имя собственное, написанное с ошибкой
		// все равно попробуем вариант с первой заглавной буквой just in case
		//
		if ( strict ) // если слово пишется только с заглавной буквы, а написано с маленькой - это ошибка
	        iter.todo = VARIANT_TO_TRY__AS_IS;
		else
	        iter.todo = VARIANT_TO_TRY__AS_IS | VARIANT_TO_TRY__FIRST_UPPER;
	}
	else
	{
		// последний вариант - в слове намешаны и строчные и прописные буквы
		//
		// Либо это особый случай, типа "СловоЕд", либо херь какая-то,
		// Пробуем только как есть, либо только все строчные.

		if ( strict ) // написали по-уродски - значит ошибка...
	        iter.todo = VARIANT_TO_TRY__AS_IS;
		else
	        iter.todo = VARIANT_TO_TRY__AS_IS | VARIANT_TO_TRY__ALL_LOWER;
	}

	// TO DO... аббривеатуры, написанные маленькими буквами?

	// специализированная обработка для каждого языка
	switch(GetLanguageCode())
	{
	
	case MORPHO_LANGUAGE_GERMAN:
	{
		// считаем количество эсцетов и ss в слове
		UInt32 sharp = 0, ss = 0;

		for (i=0; i<curlen; i++)
		{
			MCHAR ch1 = wordForm[i];
			MCHAR ch2 = wordForm[i+1];
			if ( ch1 == (MCHAR)0xDF /*'Я'*/)
				sharp++;
			else if ( ch1 == 's' && ch2 == 's' )
			{
				ss++;
				i++;	// некрасиво конечно, но допустимо
			}
		}

		// анализируем результаты
		if ( sharp == 1 && ss == 0 )
		{
			// Один эсцет, пытаемся заменить его на "ss"
			iter.internalIndex1 = 0;
			iter.internalIndex2 = sharp + 1;
		}
		else if ( ss == 1 && sharp == 0)
		{
			iter.internalIndex1 = 0;
			iter.internalIndex2 = ss + 1;
		}
		break;
	}
	
	case MORPHO_LANGUAGE_RUSSIAN:
		{
			// считаем количество Ё и Е в слове
			UInt32 yo = 0, ye = 0;

			for (i=0; i<curlen; i++ )
			{
				MCHAR ch = wordForm[i];

				if ( ch == (MCHAR)0xB8 /*'ё'*/ || ch == (MCHAR)0xA8 /*'Ё'*/ )
					yo++;
				else if ( ch == (MCHAR)0xE5 /*'е'*/ || ch == (MCHAR)0xC5 /*'Е'*/ )
					ye++;
			}

			// анализируем результаты
			if ( yo > 1 )
			{
				// Слова, в которых есть несколько букв "Ё" есть ("трёхколёсный", "трёхрублёвка") 
				// но их очень мало. Поэтому, просто рассматриваем слово как есть.
				break;
			}
			else if ( yo == 1 )
			{
				// в слове уже есть Ё - не надо пытаться восстанавливать её из Е
				// ничего не делаем
				break;
			}
			else if ( ye > 0 )
			{
				// в слове нет Ё, зато есть Е.
				// надо пытаться восстанавливать исходную позицию Ё
				iter.internalIndex1 = 0;
				iter.internalIndex2 = ye + 1; // включая вариант совсем без Ё
			}
			break;
		}

	case MORPHO_LANGUAGE_SPANISH:
		{
			// вообще говоря в испанском опускать ударения при письме нельзя
			if ( !strict )
			{
				// считаем количество ударных и безударных гласных в слове в слове
				UInt32 yo = 0, ye = 0;

				for (i=0; i<curlen; i++ )
				{
					MCHAR ch = wordForm[i];
					UInt8 srcChars[] = {0xC1, 0xE1, 0xC9, 0xE9, 0xCD, 0xED, 0xD3, 0xF3, 0xDA, 0xFA, 0};
					//if ( CSldCompare::StrChrA((UInt8 *)"БбЙйНнУуЪъ", ch) )
					if ( CSldCompare::StrChrA(srcChars, ch) )
						yo++;
					else if ( CSldCompare::StrChrA((UInt8 *)"AaEeIiOoUu", ch) )
						ye++;
				}

				// анализируем результаты
				if ( yo > 1 )
				{
					// Возможно, существуют слова, в которых несколько ударений. Оставим всё как есть.
					break;
				}
				else if ( yo == 1 )
				{
					// в слове уже есть Ё - не надо пытаться восстанавливать её из Е
					// ничего не делаем
					break;
				}
				else if ( ye > 0 )
				{
					// в слове нет Ё, зато есть Е.
					// надо пытаться восстанавливать исходную позицию Ё
					iter.internalIndex1 = 0;
					iter.internalIndex2 = ye + 1; // включая вариант совсем без Ё
				}
			}
			break;
		}
	
		case MORPHO_LANGUAGE_FRENCH:
		{
			if ( !strict )
			{
				for (i=0; i<curlen-1; i++ )
				{
					MCHAR ch1 = wordForm[i], ch2 = wordForm[i+1];
					if ((ch1 == 'o' || ch2 == 'O') && (ch2 == 'e' || ch2 == 'E')){
						iter.internalIndex2 = 2; // 2 варианта: без замены и с заменой
						iter.internalIndex1 = 0;
					}
				}

			}
			break;
		}

	}

	// сохраним длину слова
	length = curlen;
	iter.wordLen = curlen;
}

///////////////////////////////////////////////////////////////////////////////////
///
/// Инициализировать итератор вариантов написания слова перед вызовом 
/// GetNextWritingVersion().
/// 
/// @param		   iter            Ссылка на итератор, который необходимо 
///								   инициализировать.
///
///	@param		   wordForm        Буфер с исходным словом (не должен
///                                меняться пока вызывается GetNextWritingVersion())
///
///	@param		   length		   На входе: количество символов во входном 
///								   буфере. В буфере может находится несколько
///                                слов. Например, целая строка. Функция, используя
///                                знания о языке, выделит последовательность
///                                символов, которая может трактоваться
///                                как слово этого языка.\n
///								   На выходе: количестов символов из
///								   входного буфера, которые рассмотрено
///								   как текущее слово. Если это 0 - в начале
///                                буфера идет последовательность символов,
///								   которая не может быть словом.
///
///	@param		   strict          Строго или нет относиться к исходному 
///                                написанию слова. Имеет смысл использовать
///                                при проверке правописания слова 
///                                (в спелчекере). Напимер, если слово может
///                                писаться только с большой буквы и \b strict \b = \b true,
///                                то это слово, написанное с маленькой буквы,
///							       не будет предложено в качестве варианта,
///								   иначе будет.
///
/// @return	      Ничего не возвращает.
///
/// \copydoc word_variants
///
/// @see WritingVersionIterator, GetNextWritingVersion()
///
///
///////////////////////////////////////////////////////////////////////////////////
void MorphoData_v2::WritingVersionIteratorInitW (WritingVersionIterator &iter, const UInt16 *wordForm, UInt32 &length , bool strict) const
{
#ifdef _ML_PARSER_
	return;
#else
	CSldCompare::Unicode2ASCIIByLanguage(wordForm, (UInt8 *)iter.word_form_buffer, SldLanguage::fromCode(m_Header->language));
	WritingVersionIteratorInit(iter, iter.word_form_buffer, length, strict);
#endif
}

///////////////////////////////////////////////////////////////////////////////////
///
/// Предназначена для того, чтобы по очереди выдавать различные варианты 
/// формы слова (с ё, без ё, с большой или с маленькой буквы и т.д.)
///
/// @param     iter              Ссылка на итератор версий, инициализированный
///								 функцией WritingVersionIteratorInit().
///
///	@param     outBuf            Буфер для хранения следующего 
///                              варианта написания слова.
///
/// @return	   \b true,   если базовая форма слова получена, 
///			   \b false,  если гипотез больше нет
///
/// \copydoc word_variants
///
/// @see WritingVersionIterator, WritingVersionIteratorInit()
///
///////////////////////////////////////////////////////////////////////////////////
bool MorphoData_v2::GetNextWritingVersion (WritingVersionIterator &iter, MCHAR* outBuf) const
{
UInt32 current_variant = 0;

	// защита от дурака
	if ( iter.wordLen == 0 )
		return false;

	// кончились варианты, что нужно попробовать?
	if ( iter.todo == 0 )
		return false;

	// выберем оставшийся вариант
	if ( iter.todo & VARIANT_TO_TRY__AS_IS )
		current_variant = VARIANT_TO_TRY__AS_IS;
	else if ( iter.todo & VARIANT_TO_TRY__ALL_LOWER )
		current_variant = VARIANT_TO_TRY__ALL_LOWER;
	else if ( iter.todo & VARIANT_TO_TRY__FIRST_UPPER )
		current_variant = VARIANT_TO_TRY__FIRST_UPPER;

	switch(current_variant)
	{
	case VARIANT_TO_TRY__AS_IS:
		{
		UInt32 i;
			// просто копируем в выходной буфер
			for(i=0; i<iter.wordLen; i++)
				outBuf[i] = iter.wordForm[i];
			// конец слова
			outBuf[i] = 0;
			break;
		}
	case VARIANT_TO_TRY__ALL_LOWER:
		{
		UInt32 i;
			// копируем, приводя к нижнему регистру
			for(i=0; i<iter.wordLen; i++)
				outBuf[i] = ToLower(iter.wordForm[i]);
			// конец слова
			outBuf[i] = 0;
			break;
		}
	case VARIANT_TO_TRY__FIRST_UPPER:
		{
		UInt32 i;
			// первый - к верхнему регистру
			outBuf[0] = ToUpper(iter.wordForm[0]);

			// копируем остаток, приводя к нижнему регистру
			for(i=1; i<iter.wordLen; i++)
				outBuf[i] = ToLower(iter.wordForm[i]);

			// конец слова
			outBuf[i] = 0;
			break;
		}
	}

	// специализированная обработка для каждого языка
	switch(GetLanguageCode())
	{
	
	case MORPHO_LANGUAGE_GERMAN:
	{
		// нужна ли специальная обработка 0xDF /*'Я'*/ или 'ss'?
		if ( iter.internalIndex2 > 0 )
		{
			// самый первый вариант - без замены 0xDF /*'Я'*/ на 'ss' или наоборот
			if ( iter.internalIndex1 )
			{
				// делаем очередную замену
				UInt32 i, count=0;

				for (i=0; i<iter.wordLen; i++)
				{
					// cчитаем символы, которые можно заменить
					if ( outBuf[i] == (MCHAR)0xDF /*'Я'*/ )
					{
						count++;
						// заменяем нужную 0xDF /*'Я'*/ на 'ss'
						if ( count == iter.internalIndex1 )
						{
							MCHAR* dst = &outBuf[i+1];
							MCHAR* src = &outBuf[i];
							sldMemMove(dst, src, CSldCompare::StrLenA((UInt8 *)src)+1);
							outBuf[i] = 's';
							outBuf[i+1] = 's';
							
							break;
						}
					}
					else if ( outBuf[i] == 's' && outBuf[i+1] == 's' )
					{
						count++;
						// заменяем 'ss' на 0xDF /*'Я'*/
						if ( count == iter.internalIndex1 )
						{
							MCHAR* src = &outBuf[i+1];
							MCHAR* dst = &outBuf[i];
							sldMemMove(dst, src, CSldCompare::StrLenA((UInt8 *)src)+1);
							outBuf[i] = 0xDF /*'Я'*/;

							break;
						}
					}
				}
			}
			iter.internalIndex1++;
		}
		break;
	}
	
	case MORPHO_LANGUAGE_RUSSIAN:
		
		// нужна ли специальная обработка Ё?
		if ( iter.internalIndex2 > 0 )
		{
			// самый первый вариант - совсем без Ё
			if ( iter.internalIndex1 )
			{
				// меняем очередную Е на Ё
				UInt32 i,count=0;

				for (i=0; i<iter.wordLen; i++)
				{
					// cчитаем е
					if ( outBuf[i] == (MCHAR)0xE5 /*'е'*/ || outBuf[i] == (MCHAR)0xC5 /*'Е'*/ )
					{
						count++;

						// заменяем нужную Е на Ё
						if ( count == iter.internalIndex1 )
						{
							if ( outBuf[i] == (MCHAR)0xE5 /*'е'*/ )
								outBuf[i] = 0xB8 /*'ё'*/;
							else
								outBuf[i] = 0xA8 /*'Ё'*/;
							break;
						}
					}
				}
			}
			iter.internalIndex1++;
		}
		break;

	case MORPHO_LANGUAGE_SPANISH:
		
		// нужна ли специальная обработка Ё?
		if ( iter.internalIndex2 > 0 )
		{
			// самый первый вариант - совсем без Ё
			if ( iter.internalIndex1 )
			{
				// меняем очередную Е на Ё
				UInt32 i,count=0;

				for (i=0; i<iter.wordLen; i++)
				{
					const MCHAR* srcChars = "AaEeIiOoUu";
					const MCHAR* p = (const MCHAR*)CSldCompare::StrChrA((UInt8 *)srcChars, outBuf[i] );
					const MCHAR srcCharsExtended[] = {(MCHAR)0xC1, (MCHAR)0xE1, (MCHAR)0xC9, (MCHAR)0xE9, (MCHAR)0xCD, (MCHAR)0xED, (MCHAR)0xD3, (MCHAR)0xF3, (MCHAR)0xDA, (MCHAR)0xFA, 0};
					
					// cчитаем е
					if ( p )
					{
						count++;

						// заменяем нужную Е на Ё
						if ( count == iter.internalIndex1 )
						{
							//outBuf[i] = "БбЙйНнУуЪъ"[ p - srcChars ];
							outBuf[i] = srcCharsExtended[ p - srcChars ];
							break;
						}
					}
				}
			}
			iter.internalIndex1++;
		}
		break;

		case MORPHO_LANGUAGE_FRENCH:
		{
			if ( iter.internalIndex2 > 0 )
			{
				// Первый вариант - вообще без замены
				if ( iter.internalIndex1 ) 
				{
					UInt32 j=0, i=0;
					while (i<iter.wordLen)
					{
						MCHAR ch1 = outBuf[i], ch2 = outBuf[i+1], OE=(MCHAR)0x8C /*'Њ'*/, oe=(MCHAR)0x9C /*'њ'*/, newChar;
						if ((ch1 == 'o' || ch2 == 'O') && (ch2 == 'e' || ch2 == 'E')){
							if (ch1 == 'o' && ch2 == 'e'){
								newChar = oe;
							}
							else{
								newChar = OE;
							}
							outBuf[j++] = newChar;
							i+=2;
						}else{
							outBuf[j++] = outBuf[i++];
						}
					}
					outBuf[j] = '\0';

				}
				iter.internalIndex1++;
			}

			break;
		}

	}

	// текущий вариант опробован полностью?
	if ( iter.internalIndex2 == 0 || iter.internalIndex1 == iter.internalIndex2 )
	{
		// помечаем данный регистр как обработанный
		iter.todo &= ~current_variant;

		// сбрасываем внутренний индекс
		iter.internalIndex1 = 0;
	}

	// в выходном буфере есть результат
	return true;
}

///////////////////////////////////////////////////////////////////////////////////
///
/// Предназначена для того, чтобы по очереди выдавать различные варианты 
/// формы слова (с ё, без ё, с большой или с маленькой буквы и т.д.)
///
/// @param     iter              Ссылка на итератор версий, инициализированный
///								 функцией WritingVersionIteratorInit().
///
///	@param     outBuf            Буфер для хранения следующего 
///                              варианта написания слова.
///
/// @return	   \b true,   если базовая форма слова получена, 
///			   \b false,  если гипотез больше нет
///
/// \copydoc word_variants
///
/// @see WritingVersionIterator, WritingVersionIteratorInit()
///
///////////////////////////////////////////////////////////////////////////////////
bool MorphoData_v2::GetNextWritingVersionW (WritingVersionIterator &iter, UInt16* outBuf) const
{
#ifdef _ML_PARSER_
	return false;
#else
	MCHAR buff[MAX_WORD_LEN];
	bool ret = GetNextWritingVersion(iter, buff);
	if (ret)
		CSldCompare::ASCII2UnicodeByLanguage((UInt8 *)buff, outBuf, SldLanguage::fromCode(m_Header->language));
	return ret;
#endif
}

///////////////////////////////////////////////////////////////////////////////////
///
/// \page get_base_form Получение базовой формы слова с учетом регистра
///
/// Для восставновления \ref baseform "базовой формы слова" используются функции 
/// MorphoData::BaseFormsIteratorInit(), MorphoData::GetNextBaseForm(), а так
/// же итератор MorphoData::BaseFormsIterator.
///
/// Пример использования:
///
/// \code
/// void ShowBaseForms(MorphoData &morpho, const char* word_form)
/// {
/// MorphoData::BaseFormsIterator bIter; // итератор
/// const MCHAR * posBuf[1024];          // буфер позиций
/// MCHAR buf[1024];                     // буфер для получения базовых форм
///
///    // инициализируем итератор
///    morpho.BaseFormsIteratorInit(bIter, word_form, posBuf, 
/// 							M_BASEFORMS_SHOULD_MATCH_PRECONDITION |
/// 							M_BASEFORMS_USE_DICTIONARY | 
/// 							M_BASEFORMS_ONLY_DICTIONARY_WORDS );
/// 
///    // печатаем все базовые формы
///    while ( morpho.GetNextBaseForm(bIter, buf, &ruleset) )
///    {
///       printf("Base form: %s\n", buf);
///    }
/// }
/// \endcode
///
/// \attention Необходимо помнить, что базовые формы проверяются на наличие в словаре
/// с учетом регистра и различая буквы "е и ё", то есть точно так, как написано
/// слово. Для того, чтобы более гибко подойти к написанию формы слова,
/// нужно дополнительно воспользоваться функциями MorphoData::WritingVersionIteratorInit()
/// и MorphoData::GetNextWritingVersion().
///
///////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////
///
/// Функция иницализирует итератор для последующего использования
/// функцией GetNextBaseForm() для получения \ref baseform "базовых форм слова".
///
/// @param     iter              Ссылка на итератор, который нужно
///								 инициализировать
///
///	@param	   wordFormPtr       Указатель на буфер с произвольной 
///								 формой слова, для которого будем
///								 искать базовые. Слово должно 
///								 заканчиваться нулевым байтом.
///                              Указатель должен оставаться валидным на 
///                              протяжении всего процесса получения
///                              базовых форм для данного слова.
///
///	@param     posBuf            Указатель на временный буфер 
///								 размером не менее максимально возможной 
///                              длины базовой формы слова. 
///                              Указатель должен оставаться валидным на 
///                              протяжении всего процесса получения
///                              базовых форм для данного слова.
///
/// @param     flags			 Флаги. 
///
/// Возможны различные комбинации флагов для аргумента flags:
///
///       - #M_BASEFORMS_ALL_GUESSES \n
///             \copydoc M_BASEFORMS_ALL_GUESSES
///
///       - #M_BASEFORMS_SHOULD_MATCH_PRECONDITION \n
///             \copydoc M_BASEFORMS_SHOULD_MATCH_PRECONDITION
///
///       - #M_BASEFORMS_USE_DICTIONARY \n
///             \copydoc M_BASEFORMS_USE_DICTIONARY
///
///       - #M_BASEFORMS_ONLY_DICTIONARY_WORDS \n
///             \copydoc M_BASEFORMS_ONLY_DICTIONARY_WORDS
///
/// Рекоммендуется всегда использовать следующую комбинацию флагов:
/// (#M_BASEFORMS_SHOULD_MATCH_PRECONDITION | #M_BASEFORMS_USE_DICTIONARY | #M_BASEFORMS_ONLY_DICTIONARY_WORDS).
/// При такой комбинации, будут восстанавливаться только те базовые формы,
/// которые есть в морфологической базе данных, и только используя
/// те правила, которые относятся к данному слову.
///
/// @return		Ничего не возвращает.
///
/// \copydoc get_base_form 
///
/// @see GetNextBaseForm(), BaseFormsIterator
///
///////////////////////////////////////////////////////////////////////////////////
void MorphoData_v2::BaseFormsIteratorInit (BaseFormsIterator &iter, const MCHAR *wordFormPtr, Int32 flags) const
{
	// save word pointer
	iter.word_form		= wordFormPtr;

	// calculate length of word
	Int32 length			= (Int32)CSldCompare::StrLenA((UInt8 *)wordFormPtr);
	if ( length == 0 )
		return;
	
	// create positions bufer
	if (iter.posBuf)
		sldMemFree(iter.posBuf);
	iter.posBuf         = (const MCHAR**)sldMemNew(sizeof(MCHAR*)*MAX_POS_BUF_SIZE);
	if (!iter.posBuf)
		return;
	sldMemZero(iter.posBuf, sizeof(MCHAR*)*MAX_POS_BUF_SIZE);

	iter.pass			= 0;

	// calculate last and pre-last characters
	iter.last_char		= wordFormPtr[length-1];
	iter.pre_last_char  = (length>1) ? wordFormPtr[length-2] : 0;

	// get first ruleset for this word
	iter.call_param     = lastCharMap.GetFirstRuleset(iter.iter, iter.last_char);

	// save flags
	iter.flags          = flags;


	// В итераторе находится информация о базовых формах, найденная
	// в рекурсивном вызове
	iter.afterRecursion = false;
	// Количество одновременно найденных базовых форм
	iter.simultaneousFormsNumber = 0;
	// Счетчик пройденных форм при выдаче их после рекурсии
	iter.curFormAfterRecursion = 0;
	// Флаг - чтобы делать языкоспецифичные проверки только один раз
	iter.langSpecificTrialsDone = false;

	// Полное количество уже найденных форм
	iter.foundFormsNumber = 0;
	// Найдены ли в процессе специальных языковых проверок совпадения с формами, 
	// уже имеющимися
	iter.areCoincidences = false;

}
/**
*********************************************************************
*
* <b>FUNCTION:   MorphoData::BaseFormsIteratorInitW</b>
*
*	Подготавливае итератор для поиска слов. Юникодный аналог функции
*	#MorphoData::BaseFormsIteratorInit.
*
/// @param     iter              Ссылка на итератор, который нужно
///								 инициализировать
///
///	@param	   wordFormPtr       Указатель на буфер с произвольной 
///								 формой слова, для которого будем
///								 искать базовые. Слово должно 
///								 заканчиваться нулевым байтом.
///                              Указатель должен оставаться валидным на 
///                              протяжении всего процесса получения
///                              базовых форм для данного слова.
///
///	@param     posBuf            Указатель на временный буфер 
///								 размером не менее максимально возможной 
///                              длины базовой формы слова. 
///                              Указатель должен оставаться валидным на 
///                              протяжении всего процесса получения
///                              базовых форм для данного слова.
///
/// @param     flags			 Флаги. 
***********************************************************************/
void MorphoData_v2::BaseFormsIteratorInitW (BaseFormsIterator &iter, const UInt16 *wordFormPtr, Int32 flags) const
{
#ifdef _ML_PARSER_
	return;
#else
	CSldCompare::Unicode2ASCIIByLanguage(wordFormPtr, (UInt8 *)iter.word_form_buffer, SldLanguage::fromCode(m_Header->language));
	iter.word_form = iter.word_form_buffer;
	BaseFormsIteratorInit(iter, iter.word_form_buffer, flags);
#endif
}

// вполне неплохое ускорение, а всего 20 байт. Дальше увеличивать смысла нет - ускоренее менее процента
#define RULES_CACHE_SIZE 5 

///////////////////////////////////////////////////////////////////////////////////
///
/// Функция поочередно возвращает \ref baseform "базовые формы слова".
/// Количество и достоверность выдаваемых гипотез можно котроллировать 
/// флагами при инициализации итератора.
///
/// @param		iter              Ссылка на итератор, проинициализированный 
///								  функцией BaseFormsIteratorInit()
///
///	@param		baseFormBuf       Указатель на буфер куда
///								  будет записана базовая формы слова,
///								  заканчивающаяся нулевым байтом.
///
///	@param		rulesetPtr        Указатель на указатель для получение ссылки на 
///								  набор правил, по которому получена форма слова.
///								  Успользуя данный указатель можно получить
///								  название части речи этой базовой формы,
///                               передав в функцию ClassNameByRulesetPtr().
///								  Так же, этот указатель понадобится при
///								  получении списка всех форм слова функциями
///								  WordFormsIteratorInit() и GetNextWordForm().
///
///
/// @return	   \b true,   если базовая форма слова получена, 
///			   \b false,  если гипотез больше нет
///
/// \copydoc get_base_form 
///
/// @see BaseFormsIteratorInit(), BaseFormsIterator
///
//////////////////////////////////////////////////////////////////////////////////////
bool MorphoData_v2::GetNextBaseForm (BaseFormsIterator &iter, MCHAR *baseFormBuf, const MorphoInflectionRulesSetHandle** rulesetPtr) const
{
	////////////////////////////////////////////////////
	//
	// fool protection
	//

	if ( lastCharMap.CallParamsTableSize() == 0 )
		return false;

	if ( iter.word_form[0] == 0 )
		return false;

	// Если ранее в процессе языкоспецифичных проверок были найдены куски слова, 
	// для которых обнаружены базовые формы (относится к полисинтетическим
	// языкам), то надо их по очереди выдавать, пока не кончатся.
	if (iter.afterRecursion && iter.curFormAfterRecursion < BaseFormsIterator::MAX_SIMULTANEOUS_FORMS){
		// Проверим, нет ли среди этих форм совпадающих с теми, что уже были.
		// В будущем, когда (если) будет сделана возможность выдавать формы с указанием
		// их принадлежности к частям слова, эту проверку нужно будет отключить
		bool coincedences = false;
		bool shouldReturn = false;
		do{
			for (Int32 i=0; i<iter.foundFormsNumber; i++){
				coincedences = coincedences || !CSldCompare::StrCmpA((UInt8*)iter.foundForms[i],(UInt8*) iter.simultaneousForms[iter.curFormAfterRecursion]); 
				coincedences = coincedences || (iter.foundFormsRulesets[i] == iter.simultaneousFormsRulesets[iter.curFormAfterRecursion]);
			}
			// Установим флаг совпадения - для проверок во всяческих утилитах
			// (хотя в корректной базе совпадения тоже могут быть)
			iter.areCoincidences = iter.areCoincidences || coincedences;

			// Если совпадений не было, выдаем эту форму
			if (!coincedences){
				CSldCompare::StrNCopyA((UInt8 *)baseFormBuf, (UInt8 *)iter.simultaneousForms[iter.curFormAfterRecursion], MAX_WORD_LEN);
				*rulesetPtr = iter.simultaneousFormsRulesets[iter.curFormAfterRecursion];
				iter.saveFoundForm(baseFormBuf, *rulesetPtr);
				shouldReturn = true;
			}	
			// В любом случае - увеличиваем счетчик
			iter.curFormAfterRecursion++;
			
			// Если список языкоспецифичных форм закончился, то пора выходить
			if (iter.curFormAfterRecursion >= iter.simultaneousFormsNumber){
				iter.afterRecursion = false;
				iter.curFormAfterRecursion = 0;
				iter.simultaneousFormsNumber = 0;
				break;
			}
		}while(coincedences);

		// Если найдена новая форма (уникальная), выдаем ее
		if (shouldReturn) return true;
	}


	////////////////////////////////////////////////////
	//
	// на случай, если мы только что восстановили 
	// и вернули форму слова, нет смысла повторять
	// полностью идентичные формы
	//
	// ... (до тех пор, пока мы не интересуемся
	// конкрентым правилом)...
	//

	/*
	while ( iter.call_param && iter.call_param->matchPrevious )
	{
		// пропускаем полностью совпадающую форму
		iter.call_param =  lastCharMap.GetNextRuleset(iter.iter);
		iter.pass       = 0;
	}
	*/ 
	
	// - а вдруг одинаковые правила из разных классов???? 


	////////////////////////////////////////////////////
	//
	// detect suitable rules for this word
	//

	bool wordWasRestored = false;

	// Счетчик числа восстановленных вариантов (за счет квадратных скобок, 
	// уже после работы RestoreWord
	Int32 passesCount = 0; 

	// Нижеследующий отвратительный прием с двумя указателями на один и тот же буфер
	// приводит к тому, что буфер портится, если
	// у нас есть несколько форм слова (заданных квадратными скобками); и 
	// поэтому буфер надо каждый раз восстанавливать. За счет этого образовался 
	// труднонаходимый баг когда невосстановленный буфер пытались использовать
	// в процессе оптимизации (если правила повторялись).
	MCHAR* buf2 = baseFormBuf;

	//
	// Ускорение для того, чтобы избежать повторного 
	// поиска слова в словаре.
	//
	// savedRulesetNum = -1 - слово надо искать заново
	// savedRulesetNum = 0  - слово уже искали, его нет в словаре
	// savedRulesetNum > 0  - в savedRuleset лежат указатели 
	//                        на правила для этого слова
	//
	Int32 savedRulesetNum = -1;
	const MorphoInflectionRulesSet_v2* savedRuleset[RULES_CACHE_SIZE];

	//
	// TO DO...
	//
	// Пролема: теперь будет медленее из-за того, что мы каждый 
	// раз будем переделывать базоваую форму, если мы ее возвращаем 
	// пользователю, так как мы не знаем, изменил ли он буфер или 
	// нет...
	//
	// Та же фигня и в случае нескольких равноправных базовых форм 
	// в одном правиле...
	//

	while( iter.call_param )
	{
		MorphoTestPtr((const RulesByLastChar_v2::OneCallParameters*)(iter.call_param));
		MorphoTestPtr(&((const RulesByLastChar_v2::OneCallParameters*)(iter.call_param))->precondition);
		MorphoTestPtr(&((const RulesByLastChar_v2::OneCallParameters*)(iter.call_param))->rule);
		
		// предпоследний символ этого правила не подходит?
		if ( ((const RulesByLastChar_v2::OneCallParameters*)(iter.call_param))->pre_last && ((const RulesByLastChar_v2::OneCallParameters*)(iter.call_param))->pre_last != iter.pre_last_char )
		{
			// не удалось восстановить слово
			wordWasRestored = false;
		}
		// мы уже имеем слово восстановленное на предыдущем шаге?
		else if ( wordWasRestored && ((const RulesByLastChar_v2::OneCallParameters*)(iter.call_param))->matchPrevious && passesCount == 1 )
		{
			// ничего не делаем, слово уже есть в буфере...
		}
		
		else if (RestoreWord(iter.word_form, 
#ifdef _ML_PARSER_
			                  ((const RulesByLastChar_v2::OneCallParameters*)(iter.call_param))->precondition, 
							  ((const RulesByLastChar_v2::OneCallParameters*)(iter.call_param))->rule, 
#else
							  McharRef2Ptr(((const RulesByLastChar_v2::OneCallParameters*)(iter.call_param))->precondition), 
							  McharRef2Ptr(((const RulesByLastChar_v2::OneCallParameters*)(iter.call_param))->rule), 
#endif
							  ((const RulesByLastChar_v2::OneCallParameters*)(iter.call_param))->rule_len,
							  baseFormBuf, 
							  ((const RulesByLastChar_v2::OneCallParameters*)(iter.call_param))->onlyOneVariant) )
		{
			// слово удалось восстановить
			wordWasRestored = true;
			
			// получено новое слово - надо заново получать для него правила
			savedRulesetNum = -1;
		}
		else
			// не удалось восстановить слово
			wordWasRestored = false;

		// уже есть в буфере восстановленное слово?
		// А если оно уже проверено и точно нет в словаре, то пропускаем (дает ускорение до 80%)
		if ( wordWasRestored  &&  ( (savedRulesetNum != 0) || !(iter.flags & M_BASEFORMS_ONLY_DICTIONARY_WORDS) ) )
		{
			bool morePasses = true;
			
			// случай нескольких равноправных исходных форм
			for(passesCount = 0 ; morePasses ;  passesCount++)
			{
				// генерируем один из предложеных вариантов
				Int32 i=0, j=0, toSkip;
				morePasses = false;

				// доплнительный флаг для проверок 
				bool everythin_is_ok = true;

				// копируем нужную форму слова
				for( ; baseFormBuf[i] ; )
				{
					if ( baseFormBuf[i] == '[' )
					{
						// пропускаем скобку
						i++;

						// пропускаем ранее обработанные правила
						toSkip = iter.pass;
						while(toSkip)
						{
							if (baseFormBuf[i]==',')
								toSkip--;
							i++;
						}

						// копируем значащую часть
						while ( baseFormBuf[i] != 0 &&  baseFormBuf[i] != ',' &&  baseFormBuf[i] != ']' )
						{
							buf2[j++] = baseFormBuf[i++];
						}

						// есть ли еще варианты
						if ( baseFormBuf[i] == ',' )
							morePasses = true;

						// пропускаем пока следующие варианты
						while ( baseFormBuf[i] != 0 &&  baseFormBuf[i] != ']' )
						{
							i++;
						}

						// пропускаем скобку
						if ( baseFormBuf[i] == ']' )
							i++;
					}
					else
						buf2[j++] = baseFormBuf[i++];
				}

				// завершаем строку
				buf2[j] = 0;

				// удалось восстановить иходное словов по этому правилу. 
				// А подходит ли теперь это слово под исходное предусловие?
				if ( iter.flags & M_BASEFORMS_SHOULD_MATCH_PRECONDITION )
				{
#ifdef _ML_PARSER_
					if( !IsRuleApplyable(baseFormBuf, ((const RulesByLastChar_v2::OneCallParameters*)(iter.call_param))->ruleset, iter.posBuf, M_RULE_TOPMOST_PRECONDITION | M_RULE_CLOSED_RULE_IS_OK ) )
						everythin_is_ok = false;
#else
					const MorphoInflectionRulesSet_v2* mp = ObjRef2Ptr(((const RulesByLastChar_v2::OneCallParameters*)(iter.call_param))->ruleset);
					if( !IsRuleApplyable(baseFormBuf, mp, iter.posBuf, M_RULE_TOPMOST_PRECONDITION | M_RULE_CLOSED_RULE_IS_OK ) )
						everythin_is_ok = false;
#endif
				}

				// теперь попытаемся отфильтровать слова по словарю
				if ( everythin_is_ok && ( iter.flags & M_BASEFORMS_USE_DICTIONARY )  )
				{
				WordSet_v2::WordRuleSetsIterator	rulesetsIter;
				const MorphoInflectionRulesSet_v2* realRuleset;

					// мы уже нашли это слово ранее в базе?
					if ( savedRulesetNum > 0 )
					{
					Int32 k=0;

						// ищем это правило среди ранне считанных, вместо того, чтобы считать заново
						// (дополнительное ускорение около 20%)
						while(true)
						{
							// прошли все правила, а ничего так и не нашли?
							if ( k == savedRulesetNum )
							{
								everythin_is_ok = false;
								break;
							}

#ifdef _ML_PARSER_
							if ( savedRuleset[k] == ((const RulesByLastChar_v2::OneCallParameters*)(iter.call_param))->ruleset )

								// есть такое правило!
								break;
#else

							if ( savedRuleset[k] == ObjRef2Ptr(((const RulesByLastChar_v2::OneCallParameters*)(iter.call_param))->ruleset) )
								
								// есть такое правило!
								break;
#endif
							k++;
						}
					}



					// ищем базовую форму в базе слов
					else if ( words.SearchWord( baseFormBuf, rulesetsIter) )
					{
					// обнуляем счетчик правил для слова
						savedRulesetNum = 0;

						bool rulesetMatch = false;

						// слово есть в базе, сопоставим 
						// правило, по которому получена базоавя форма слова
						// с теми правилами, которые вообще применимы к этому слову
						while ( (realRuleset = words.GetNextRuleSet(rulesetsIter)) )
						{
							// сохраняем правило на будущее
							if ( savedRulesetNum < RULES_CACHE_SIZE )
								savedRuleset[savedRulesetNum++] = realRuleset;
#ifdef _ML_PARSER_
							if ( realRuleset == ((const RulesByLastChar_v2::OneCallParameters*)(iter.call_param))->ruleset )
								// есть такое правило!
								rulesetMatch = true;
#else
							if ( realRuleset == ObjRef2Ptr(((const RulesByLastChar_v2::OneCallParameters*)(iter.call_param))->ruleset) )
								// есть такое правило!
								rulesetMatch = true;
#endif
						}

						// возникло переполнение буфера - мы не можем доверять
						// хранимой в нем информации, в следующий раз слово надо 
						// восстанавливать заново...
						if ( savedRulesetNum >= RULES_CACHE_SIZE )
							savedRulesetNum = -1;

						// прошли все правила, а ничего так и не нашли?
						if ( !rulesetMatch )
							everythin_is_ok = false;
					}
					else 
					{
						// обнуляем счетчик правил для слова
						savedRulesetNum = 0;

						// если стоит флаг, что все слова должны обязательно присутсвовать 
						// в базе, а слово не нашли, то ошибка...
						if ( iter.flags & M_BASEFORMS_ONLY_DICTIONARY_WORDS )
							everythin_is_ok = false;
					}
				}

				// предыдущая проверка прошла успешно?
				if ( everythin_is_ok )
				{
					// восстанавливаем название класса
#ifdef _ML_PARSER_
					if ( rulesetPtr )
						*rulesetPtr = ((const RulesByLastChar_v2::OneCallParameters*)(iter.call_param))->ruleset;
#else
					if ( rulesetPtr )
						*rulesetPtr = ObjRef2Ptr(((const RulesByLastChar_v2::OneCallParameters*)(iter.call_param))->ruleset);
#endif

					// переходим на следующую базовую форму
					if ( morePasses )
					{
						iter.pass++;
					}
					else
					{
						// get next ruleset ...
						iter.call_param =  lastCharMap.GetNextRuleset(iter.iter);
						iter.pass       = 0;
					}

					// Сохраняем возвращаемую форму, чтобы потом не возвращать ее
					// по второму разу в процессе языкоспецифичных проверок
					iter.saveFoundForm(baseFormBuf, *rulesetPtr);

					// наконец, возвращаем результат
					return true;
				}
				else if ( morePasses )
				{
					// у нас слово из нескольких частей, а мы проверили только первую,
					// а буфер уже разрушили... Надо восстановить
					
					MorphoTestPtr((const RulesByLastChar_v2::OneCallParameters*)(iter.call_param));
					MorphoTestPtr(&((const RulesByLastChar_v2::OneCallParameters*)(iter.call_param))->precondition);
					MorphoTestPtr(&((const RulesByLastChar_v2::OneCallParameters*)(iter.call_param))->rule);
					
					RestoreWord(iter.word_form, 
#ifdef _ML_PARSER_
								((const RulesByLastChar_v2::OneCallParameters*)(iter.call_param))->precondition, 
								((const RulesByLastChar_v2::OneCallParameters*)(iter.call_param))->rule, 
#else
								McharRef2Ptr(((const RulesByLastChar_v2::OneCallParameters*)(iter.call_param))->precondition), 
								McharRef2Ptr(((const RulesByLastChar_v2::OneCallParameters*)(iter.call_param))->rule),
#endif
								((const RulesByLastChar_v2::OneCallParameters*)(iter.call_param))->rule_len,
								baseFormBuf, 
								((const RulesByLastChar_v2::OneCallParameters*)(iter.call_param))->onlyOneVariant);

					// слово получено заново, надо заново получать для него правила
					// (случай, когда несколько альтернативных базовых форм для одного правила)
					savedRulesetNum = -1;
				}

				// какое-то условие не выполнилось - следующй проход...
				iter.pass++;
			}
		}

		// get next ruleset
		iter.call_param =  lastCharMap.GetNextRuleset(iter.iter);
		iter.pass       = 0;
	}

	// Стандатрные проверки не прошли, но может быть это специфичная 
	// для какого-то из языков форма?
	// Первый из найденных вариантов возвращаем рекурсивным вызовом, 
	// поскольку список вариантов сохранен в итераторе и рассчитывается, 
	// что каждый следующий вариант выдается именно при вызове
	// GetNextbaseForm

	// Сперва проверим, что мы не запускали уже языкоспецифичные проверки.
	// Кроме того, может быть запрещено запускать их, если уже найдены 
	// базовые формы
	if (!iter.langSpecificTrialsDone && !(iter.foundFormsNumber && (iter.flags & M_DONT_DO_LANGSPEC_IF_BASEFORMS_FOUND)))
	{
		iter.langSpecificTrialsDone = true;
#ifndef _ML_PARSER_
		if (LanguageSpecificTrials(iter, baseFormBuf, rulesetPtr, m_Header->language))
			return GetNextBaseForm(iter, baseFormBuf, rulesetPtr);
#endif
	}

	return false;

}
/**
*********************************************************************
*
* <b>FUNCTION:   MorphoData::GetNextBaseFormW</b>
*
*	Полный аналог функции #MorphoData::GetNextBaseForm
*
* Функция поочередно возвращает \ref baseform "базовые формы слова".
* Количество и достоверность выдаваемых гипотез можно котроллировать 
* флагами при инициализации итератора.
*
*	@param		iter              Ссылка на итератор, проинициализированный 
*								  функцией BaseFormsIteratorInit()
*
*	@param		baseFormBuf       Указатель на буфер куда
*								  будет записана базовая формы слова,
*								  заканчивающаяся нулевым байтом.
*
*	@param		rulesetPtr        Указатель на указатель для получение ссылки на 
*								  набор правил, по которому получена форма слова.
*								  Успользуя данный указатель можно получить
*								  название части речи этой базовой формы,
*                               передав в функцию ClassNameByRulesetPtr().
*								  Так же, этот указатель понадобится при
*								  получении списка всех форм слова функциями
*								  WordFormsIteratorInit() и GetNextWordForm().
*
*
*	@return	   \b true,   если базовая форма слова получена, 
*			   \b false,  если гипотез больше нет
*
*	\copydoc get_base_form 
*
* @see BaseFormsIteratorInit(), BaseFormsIterator
***********************************************************************/
bool MorphoData_v2::GetNextBaseFormW (BaseFormsIterator &iter, UInt16 *baseFormBuf, const MorphoInflectionRulesSetHandle** rulesetPtr) const
{
#ifdef _ML_PARSER_
	return false;
#else
	MCHAR buff[MAX_WORD_LEN];
	bool ret = GetNextBaseForm (iter, buff, rulesetPtr);
	if (ret)
		CSldCompare::ASCII2UnicodeByLanguage((UInt8 *)buff, baseFormBuf, SldLanguage::fromCode(m_Header->language));
	return ret;
#endif
}


///////////////////////////////////////////////////////////////////////////////////
///
/// Восстановливает название класса по указателю на таблицу изменения слова.
///
/// @param    ruleset   Cсылка на набор правил изменения.
///
/// @return	            \b NULL, если не найден ни в одном классе.
///						Иначе, указатель на текстовую стрку с 
///						названием класса слов языка.
///
///////////////////////////////////////////////////////////////////////////////////
const MCHAR* MorphoData_v2::ClassNameByRulesetPtr(const MorphoInflectionRulesSetHandle* ruleset_ptr) const
{
const MorphoClass_v2* cls;
ClassesIterator    citer;
MorphoClass_v2::InflTablesIterator infliter;
STAT_VECTOR_SIZE_V2 size_of_first_layer;

	const MorphoInflectionRulesSet_v2* ruleset = (const MorphoInflectionRulesSet_v2*)ruleset_ptr;

	// проходим по всем классам
	ClassesIteratorInit(citer);

	// количество элементов в первом слое таблицы правил
	size_of_first_layer = (STAT_VECTOR_SIZE_V2)(ruleset->rules.size());

	while( (cls = GetNextClass(citer)) )
	{
		// для ускорения проверяем только те классы, размерность первого слоя которых
		// совпадает с искомой размерностью
		if ( size_of_first_layer == ObjRef2Ptr(cls->table)->states.size() )
		{
			infliter = cls->InflTablesBegin();

			while( infliter != cls->InflTablesEnd() )
			{
				if ( ObjRef2Ptr(*infliter) == ruleset )
				{
					MorphoTestPtr(cls);
					MorphoTestPtr(&cls->name);
					return McharRef2Ptr(cls->name);
				}

				infliter++;
			}
		}
	}

	return 0;
}

/**
**********************************************************************************
*
*	Восстановливает краткое название класса по указателю на таблицу изменения слова.
*
*	@param		ruleset		Cсылка на набор правил изменения.
*
*	@param		buf			Указатель на буффер для записи краткого названия класса. 
*							Если возвращена пустая строка, значит класс не найден или
*							не найдена краткая часть названия класса.
*
***********************************************************************************/
void MorphoData_v2::GetBriefClassNameByRulesetPtr(const MorphoInflectionRulesSetHandle* ruleset, MCHAR* buf) const
{
const MCHAR	*ClassName, *BriefName;

	// Получаем название класса(включая как полное так и сокращенное название).
	ClassName = ClassNameByRulesetPtr(ruleset);
	if (!ClassName)
	{
		buf[0] = 0;
		return;
	}
	
	// Ищем начало краткой части.
	BriefName = ClassName;
	while (*BriefName && *BriefName != MORPHO_CLASS_SEPARATOR_CHAR)
		BriefName++;
	
	// Если мы дошли до разделитея, тогда пропускаем его.
	if (*BriefName == MORPHO_CLASS_SEPARATOR_CHAR)
		BriefName++;
		
	CSldCompare::StrCopyA((UInt8 *)buf, (UInt8 *)BriefName);
}

/**
**********************************************************************************
*
*	Восстановливает краткое название класса по указателю на таблицу изменения слова.
*
*	@param		ruleset		Cсылка на набор правил изменения.
*
*	@param		buf			Указатель на буффер для записи краткого названия класса. 
*							Если возвращена пустая строка, значит класс не найден или
*							не найдена краткая часть названия класса.
*
***********************************************************************************/
void MorphoData_v2::GetBriefClassNameByRulesetPtrW(const MorphoInflectionRulesSetHandle* ruleset, UInt16* buf) const
{
#ifdef _ML_PARSER_
	return;
#else
	MCHAR buff[MAX_WORD_LEN];
	GetBriefClassNameByRulesetPtr (ruleset, buff);
	CSldCompare::ASCII2UnicodeByLanguage((UInt8 *)buff, buf, SldLanguage::fromCode(m_Header->language));
#endif
}

/**
**********************************************************************************
*
*	Восстановливает полное название класса по указателю на таблицу изменения слова.
*
*	@param		ruleset		Cсылка на набор правил изменения.
*
*	@param		buf			Указатель на буффер для записи краткого названия класса. 
*							Если возвращена пустая строка, значит класс не найден.
*
***********************************************************************************/
void MorphoData_v2::GetFullClassNameByRulesetPtr(const MorphoInflectionRulesSetHandle* ruleset, MCHAR* buf) const
{
const MCHAR	*ClassName;
MCHAR	*BriefName;

	// Получаем название класса(включая как полное так и сокращенное название).
	ClassName = ClassNameByRulesetPtr(ruleset);
	if (!ClassName)
	{
		buf[0] = 0;
		return;
	}
	
	// Ищем начало краткой части.
	CSldCompare::StrCopyA((UInt8 *)buf, (UInt8 *)ClassName);
	BriefName = buf;
	while (*BriefName && *BriefName!= MORPHO_CLASS_SEPARATOR_CHAR)
		BriefName++;
	
	// Если мы дошли до разделитея, тогда отрезаем то, что идет дальше.
	if (*BriefName == MORPHO_CLASS_SEPARATOR_CHAR)
		*BriefName = 0;
}

/**
**********************************************************************************
*
*	Восстановливает полное название класса по указателю на таблицу изменения слова.
*
*	@param		ruleset		Cсылка на набор правил изменения.
*
*	@param		buf			Указатель на буффер для записи краткого названия класса. 
*							Если возвращена пустая строка, значит класс не найден.
*
***********************************************************************************/
void MorphoData_v2::GetFullClassNameByRulesetPtrW(const MorphoInflectionRulesSetHandle* ruleset, UInt16* buf) const
{
#ifdef _ML_PARSER_
	return;
#else
	MCHAR buff[MAX_WORD_LEN];
	GetFullClassNameByRulesetPtr (ruleset, buff);
	CSldCompare::ASCII2UnicodeByLanguage((UInt8 *)buff, buf, SldLanguage::fromCode(m_Header->language));
#endif
}

///////////////////////////////////////////////////////////////////////////////////
///
/// FUNCITON:	   MorphoData::GetTableByRulesetPtr 
///
/// DESCRIPTION: восстановливает указатель на таблицу форм класса 
///              по указателю на  таблицу изменения слова
///
/// PARAMETERS:  ruleset       /// ссылка на набор правил изменения
///
///
/// @return	   \b NULL, если не найден ни в одном классе
///
///////////////////////////////////////////////////////////////////////////////////
const MorphoStateDescriptionTableHandle* MorphoData_v2::GetTableByRulesetPtr(const MorphoInflectionRulesSetHandle* ruleset_ptr) const
{
const MorphoClass_v2* cls;
ClassesIterator    citer;
MorphoClass_v2::InflTablesIterator infliter;
STAT_VECTOR_SIZE_V2  size_of_first_layer;

MorphoInflectionRulesSet_v2* ruleset = (MorphoInflectionRulesSet_v2*)ruleset_ptr;

	// проходим по всем классам
	ClassesIteratorInit(citer);

	// количество элементов в первом слое таблицы правил
	size_of_first_layer = (STAT_VECTOR_SIZE_V2)(ruleset->rules.size());

	while( (cls = GetNextClass(citer)) )
	{
		// для ускорения проверяем только те классы, размерность первого слоя которых
		// совпадает с искомой размерностью
		if ( size_of_first_layer == ObjRef2Ptr(cls->table)->states.size() )
		{
			infliter = cls->InflTablesBegin();

			while( infliter != cls->InflTablesEnd() )
			{
				if ( ObjRef2Ptr(*infliter) == ruleset )
					return ObjRef2Ptr(cls->table);

				infliter++;
			}
		}
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////
///
/// Функция отвечает за инициализацию ядра морфологии данными из 
/// морфологической базы данных.
///
/// @param data	Ссылка на область памяти, куда целиком загружена
///             морфологическая база данных. Данные не копируются, 
///             поэтому указатель должен оставаться валидным на 
///             все последующее время работы с классом.
///					
/// @param size	Размер морфологической базы данных в байтах.
///
/// @return \b true, если инициализация прошла успешно, иначе, \b false.
///
/// Иницаилизация может не произойти в следующих случаях:
///   - агрумент data равен нулю
///   - морфологическая база данных имеет другую версию формата.
///     Текущая версия задается макросом #DATABASE_FORMAT_VERSION
///
/// Невозможно инициализировать ядро несколькими базами данных. Если
/// есть необходимость использовать несколько языков одновременно,
/// нужно заводить несколько экземпляров класса.
///
/// Как промежуточный этап работы над проектом, база данных морфологии
/// хранится в виде сплошого бинарного файла, который позже будет
/// переведен в хранение в формате .prc
/// 
/// Для инициализации ядра морфологии, необходимо загрузить в память
/// всю базу данных, и передать в функцию MorphoData::Init() указатель
/// на начало памяти с загруженной базой и ее размер.
///
/// Память должна оставаться выделенной до окончания всей работы с ядром
/// морфологии.
/// 
/// \code
/// // открываем базу данных морфологии
/// FILE *in = fopen("имя_базы_данных.bin","rb");
/// 
/// // получаем размер базы данных
/// size_t size;
/// fseek(in,0,SEEK_END);
/// size = ftell(in); 
/// fseek(in,0,SEEK_SET);
/// 
/// // загружаем всю базу данных в память
/// void *mem;
/// mem = malloc(size);
/// fread(mem,size,1,in);
/// fclose(in);
/// 
/// // инициализируем ядро морфологии
/// MorphoData morpho;
/// if ( morpho.Init(mem, (Int32)size) )
/// {
///    // делаем, все что нужно
///    DoAllThings(&morpho);
/// }
/// 
/// // освобождаем память из под базы данных
/// free(mem);
/// 
/// // класс явно не уничтожаем, будет уничтожен 
/// // автоматически при выходе за границы видимости
/// \endcode
///
////////////////////////////////////////////////////////////////////////////
#ifndef _ML_PARSER_

namespace {

struct ResourceLoader
{
	CSDCReadMy& reader;
	CSldVector<CSDCReadMy::Resource> resources;

	ResourceLoader(CSDCReadMy& reader) : reader(reader) {}

	bool load(UInt32 aType, sld2::DynArray<UInt8> &aResource)
	{
		resources.clear();
		while (true)
		{
			auto res = reader.GetResource(aType, resources.size());
			if (res != eOK)
				break;
			resources.push_back(res.resource());
		}
		if (resources.empty())
			return false;

		UInt32 len = 0;
		for (auto&& res : resources)
			len += res.size();

		if (!aResource.resize(sld2::default_init, len))
			return false;

		UInt8 *ptr = aResource.data();
		for (auto&& res : resources)
		{
			sldMemCopy(ptr, res.ptr(), res.size());
			ptr += res.size();
		}
		resources.clear();
		return true;
	}
};

} // anon namespace

bool MorphoData_v2::Init(CSDCReadMy &aMorphology)
{

	// clear data fields
//	MorphoData::data        = 0;
//	MorphoData::data_text   = 0;
//	MorphoData::data_struct = 0;
//	MorphoData::data_size   = 0;

	MorphoData_v2::character_attributes_table  = 0;
	MorphoData_v2::to_upper_table              = 0;
	MorphoData_v2::to_lower_table              = 0;

	sldMemZero(&m_HeaderData, sizeof(m_HeaderData));

	// Зaгрузчик ресурсов для загрузки всех ресурсов определенного типа и создания из них одного
	ResourceLoader loader(aMorphology);

	// читаем заголовок
	auto res = aMorphology.GetResource(MORPHODATA_RES_HEADER, 0);
	if (res != eOK)
		return false;

	sldMemMove(&m_HeaderData, res.ptr(), res.size());
	m_Header = &m_HeaderData;

	// cверим версию формата
	UInt32 core_format = MORPHO_SWAP_32(m_Header->format);
	if (!IsCoreSupportFormat(core_format))
		return false;

	// Савельев: считываем несколько ресурсов с несжатыми строками и создаем из них один
	if (!loader.load(MORPHODATA_RES_TEXT, m_resText))
		return false;

	// Савельев: считываем несколько ресурсов с правилами и создаем из них один
	if (!loader.load(MORPHODATA_RES_RULESSET, m_resRulesSet))
		return false;

	// Савельев: считываем несколько ресурсов с классами и создаем из них один
	if (!loader.load(MORPHODATA_RES_CLASS, m_resClass))
		return false;

	classes_begin = 0;
	classes_end = m_resClass.size();

	// Считываем структуры
//	if (sldGetResource(&m_resStruct, &m_database, MORPHODATA_RES_STRUCT, 0)!=eOK)
//		return false;

	// init data fields
/*
	MorphoData::data      = data;
	MorphoData::data_size = size;
*/
//	MorphoData::data_text   = m_resText.Pointer;
//	MorphoData::data_struct = m_resRulesSet.Pointer;

	MorphoData_v2::character_attributes_table  = m_Header->character_attributes_array;
	MorphoData_v2::to_upper_table              = m_Header->to_upper_array;
	MorphoData_v2::to_lower_table              = m_Header->to_lower_array;

	m_resCharMapIdx = 0;
	lastCharMap.callParams_used = 0;
	lastCharMap.callParams_allocated = 0;
	lastCharMap.callParamsIdx = 0;
	UInt16 maxCharMapResCount = sizeof(m_resCharMap)/sizeof(m_resCharMap[0]);
	while (true)
	{
		if (m_resCharMapIdx >= maxCharMapResCount)
			return false;

		UInt32 count = 0;
		// Считываем все данные для обратного преобразования.
		if ((res = aMorphology.GetResource(MORPHODATA_RES_CHARMAP, m_resCharMapIdx)) != eOK)
			break;
		m_resCharMap[m_resCharMapIdx] = res.resource();

		count = m_resCharMap[m_resCharMapIdx].size() / sizeof(RulesByLastChar_v2::OneCallParameters);
		lastCharMap.callParams_used+=count;
		lastCharMap.AddBlock((RulesByLastChar_v2::OneCallParameters *)m_resCharMap[m_resCharMapIdx].ptr(), count);
		m_resCharMapIdx++;
	}

	// выделяем память для таблицы обратного преобразования один раз 
	// (чтобы не перевыделять ее каждый раз при динамическом приращении массива
/*
	lastCharMap.callParams_allocated = m_Header->num_items_in_LCM;
	UInt32 size = lastCharMap.callParams_allocated * sizeof(RulesByLastChar::OneCallParameters);
	lastCharMap.callParams = (RulesByLastChar::OneCallParameters*)M_MEMALLOC(lastCharMap.callParams_allocated * sizeof(RulesByLastChar::OneCallParameters));

	// инициализирум таблицу быстрого обратного преобразования
	ClassesIterator classIter;
	const MorphoClass* classPtr;

	ClassesIteratorInit(classIter);
	while ( classPtr = GetNextClass(classIter) )
	{
		// пройдемся по всем наборам правил
		MorphoClass::InflTablesIterator tablesIter;

		tablesIter = classPtr->InflTablesBegin();
		while(tablesIter != classPtr->InflTablesEnd())
		{
			// добавим правило в таблицу
			lastCharMap.AddRuleset( *this, ObjRef2Ptr(*tablesIter) );
			tablesIter++;
		}
	}
*/
	
	// количество элементов должно в точности совпадать
	if (lastCharMap.callParams_used != m_Header->num_items_in_LCM)
		return false;

	// Считываем таблицу с последними символами слов (во временный ресурс).
	res = aMorphology.GetResource(MORPHODATA_RES_LASTCHAR, 0);
	if (res != eOK)
		return false;

	// Размер поля с индексом равен 2 байта (UInt16)
	if (core_format < 107)
	{
		UInt32 elemCount = sizeof(lastCharMap.lastCharMap)/sizeof(lastCharMap.lastCharMap[0]);
		UInt32 elemSourceSize = res.size() / elemCount;
		if (elemSourceSize != 2)
			return false;

		for (UInt32 i=0;i<elemCount;i++)
		{
			lastCharMap.lastCharMap[i].index = (UInt32)(*((UInt16*)(res.ptr() + elemSourceSize*i)));
		}
	}
	// Размер поля с индексом равен 4 байта (UInt32)
	else
	{
		if (res.size() != sizeof(lastCharMap.lastCharMap))
			return false;

		sldMemMove(lastCharMap.lastCharMap, res.ptr(), res.size());
	}

	// инициализируем WordSet
//	pBlockType	*ArchiveBlockArray;
//	ArchiveBlockArray = (pBlockType *)sldMemNew(sizeof(pBlockType) * MAX_WORDS_BLOCKS_NUMBER);
/*
	// после того, как мета-блоки архива будут завернуты в собственные prc-обертки, 
	// нижеследующие 12 строк можно удалить
	const char *wordSetPtr = ((const char *)data + ((const MorphoDataHeader*)data)->words_set);
	UInt32 memShift = 0;

	// пропускаем таблицы
	memShift += sizeof(WordSet::Data);
	memShift += ((const WordSet::Data *)wordSetPtr)->IndexTableSize; 
	memShift += ((const WordSet::Data *)wordSetPtr)->RuleSetArraySize;

	// получаем указатель на структуру с размерами блоков
	const Int32 *BlockSizeArray;
	BlockSizeArray = (Int32 *)(wordSetPtr + memShift);

	// количество блоков
	UInt32 ArchiveBlockCounter; 
	ArchiveBlockCounter = ((const WordSet::Data *)wordSetPtr)->ArchiveBlockCounter;

	memShift += ArchiveBlockCounter*sizeof(Int32);
	memShift += ((const WordSet::Data *)wordSetPtr)->HaffmanTableSize;
*/

	// восстановление указателей на блоки архива
	// после введения prc-индексации эти указатели должны будут указывать 
	// на начала соотв. ресурсов.
	m_resWordSetIdx = 0;
	UInt16 maxWordSetResCount = sizeof(m_resWordSet)/sizeof(m_resWordSet[0]);
	while (true)
	{
		if (m_resWordSetIdx >= maxWordSetResCount)
			return false;

		// Считываем сжатые данные
		if ((res = aMorphology.GetResource(MORPHODATA_RES_WORDSET, m_resWordSetIdx)) != eOK)
			break;
		m_resWordSet[m_resWordSetIdx++] = res.resource();
//		ArchiveBlockArray[m_resWordSetIdx] = (pBlockType)m_resWordSet[m_resWordSetIdx].pBin;
//		MemMove(ArchiveBlockArray+size, m_resWordSet[m_resWordSetIdx].pBin, m_resWordSet[m_resWordSetIdx].Len);
	}

	// Считываем заголовок для сжатых данных
	if ((res = aMorphology.GetResource(MORPHODATA_RES_ARCH_HEADER, 0)) != eOK)
		return false;
	m_resArchHeader = res.resource();

	// Савельев: считываем несколько ресурсов со сжатыми правилами и создаем из них один
	if (!loader.load(MORPHODATA_RES_RUL2, m_resRules2))
		return false;

	// Савельев: считываем несколько ресурсов с индексами быстрого доступа и создаем из них один
	if (!loader.load(MORPHODATA_RES_QAINDEX, m_resQAIndex))
		return false;

	// Считываем ресурс с деревом Хаффмана.
	if ((res = aMorphology.GetResource(MORPHODATA_RES_TREE, 0)) != eOK)
		return false;
	m_resTree = res.resource();

	// Савельев: считываем несколько ресурсов с таблицами и создаем из них один
	if (!loader.load(MORPHODATA_RES_TABLE, m_resTable))
		return false;

	MAX_POS_BUF_SIZE = MORPHO_SWAP_16(m_Header->MaximumPreconditionLength);
	if (MAX_POS_BUF_SIZE == 0)
		MAX_POS_BUF_SIZE = 2*1024;
	
	// Обнуляем старший байт в значении номера бита для каждой записи таблицы быстрого доступа,
	// он нигде не используется, но в старых версиях там хранится какое-то значение
	// Это обеспечит совместимость версий 106 и 107
	// Начиная с версии 107 все 4 байта в номере бита используются для хранения собственно номера бита
	if (core_format < 107)
	{
		WordSet_v2::IndexArrayElement* pIndexArray = (WordSet_v2::IndexArrayElement*)m_resQAIndex.data();
		Int32 count = m_resQAIndex.size() / sizeof(pIndexArray[0]);
		for (Int32 i=0;i<count;i++)
		{
			(pIndexArray+i)->bit &= 0x00FFFFFF;
		}
	}

	words.Init( m_resArchHeader.ptr(),
				m_resWordSet,
				m_resQAIndex.data(),
				(STAT_VECTOR_SIZE_V2*)m_resRules2.data(),
				m_resTree.ptr(),
				this);

	return true;
}
#endif

UInt32 MorphoData_v2::GetNumberOfBaseForms(void) const
{
	if (m_Header)
		return MORPHO_SWAP_32(m_Header->NumberOfBaseForms);
	return 0;
}

UInt32 MorphoData_v2::GetNumberOfWordForms(void) const
{
	if (m_Header)
		return MORPHO_SWAP_32(m_Header->NumberOfWordForms);
	return 0;
}

bool MorphoData_v2::IsMorphologySimple(void) const
{
	if (m_Header)
		return (m_Header->Flags & (UInt16)MORPHODATA_FLAGS_MASK_IS_SIMPLE) ? true : false;
	return false;
}


///////////////////////////////////////////////////////////////////////////////////
///
/// FUNCITON:		MorphoData::Close 
///
/// DESCRIPTION:	Закрывает все открытые ресурсы, освобождает выделенную память, 
///					обнуляет переменные.
///
/// PARAMETERS:		none
///
///////////////////////////////////////////////////////////////////////////////////
#ifndef _ML_PARSER_
void MorphoData_v2::Close()
{
	sldMemZero(&m_HeaderData, sizeof(m_HeaderData));
	m_Header = NULL;

	words.Close();
}
#endif


///////////////////////////////////////////////////////////////////////////////////////////////////
// Автор: Павел Морозов. Октябрь 2007
// Получение всех базовых форм всех слов морфологического модуля с правилами по которым они изменяются
// EnumWords - вызывается извне, EnumWordsCallBack и WalkRuleSet используется внутри
///////////////////////////////////////////////////////////////////////////////////////////////////
struct EnumParamStruct_v2
{
	const MorphoData_v2*	This;
	EnumCallBackT*			callback;
	void*					callbackParam;
};

bool WalkRuleSet_v2(const MCHAR* word, const MorphoInflectionRulesSet_v2* ruleset, MCHAR**	rulesCache, Int32& nCacheSize, void* param)
{
	const MorphoData_v2* This = ((EnumParamStruct_v2*)param)->This;

	MCHAR form[MAX_WORD_LEN];
	const MCHAR* posBuf[2048];

	sldMemZero(form, sizeof(form));

	MorphoInflectionRulesSet_v2::RulesIterator rule;
	rule = ruleset->RulesBegin();

	while(rule != ruleset->RulesEnd())
	{
		if ( rule->IsLinkToRuleset() )
		{
			const MorphoInflectionRulesSet_v2* subset;
			subset = This->Rule2RuleSetPtr(*rule);
			WalkRuleSet_v2(word, subset, rulesCache, nCacheSize, param);
		}
		else if ( This->Rule2MCharPtr(*rule)[0] )
		{
			const MCHAR* sRule = This->Rule2MCharPtr(*rule);

			Int32 i = 0;
			for(; i<nCacheSize; i++)
				if(!CSldCompare::StrCmpA((UInt8 *)sRule, (UInt8 *)rulesCache[i]))
					break;
			if(i == nCacheSize)
			{
				MorphoTestPtr(ruleset);
				MorphoTestPtr(&ruleset->precondition);
				
				This->InflectWord(word, This->McharRef2Ptr(ruleset->precondition), sRule, form, posBuf);
				rulesCache[nCacheSize] = (MCHAR*)sRule;
				nCacheSize++;

				((EnumParamStruct_v2*)param)->callback(form, ((EnumParamStruct_v2*)param)->callbackParam);
			}
		}
		rule++;
	}

	return true;
}
bool EnumWordsCallBack_v2(const MCHAR* word, WordSet_v2::WordRuleSetsIterator &iter, void* param)
{
	MCHAR*	rulesCache[1000];
	Int32	nCacheSize = 0;

	const MorphoData_v2* This = ((EnumParamStruct_v2*)param)->This;

	const MorphoInflectionRulesSet_v2* ruleset;
	while ( (ruleset = This->words.GetNextRuleSet(iter)) )
		WalkRuleSet_v2(word, ruleset, rulesCache, nCacheSize, param);

	return true;
}

bool MorphoData_v2::Enum(bool bAllForms, EnumCallBackT* callback, void* param) const
{
	EnumParamStruct_v2 eps = {this, callback, param};

	return words.EnumWords(EnumWordsCallBack_v2, &eps);
}

//**********************************************************************************************

struct EnumPartialParamStruct_v2
{
	const MorphoData_v2*	This;
	EnumPartialCallBackT*	callback;
	void*					callbackParam;
	UInt32					FirstIndex;
	UInt32					LastIndex;
	UInt32					CurrentIndex;
	const MCHAR*			BaseForm;
};

bool PartialWalkRuleSet_v2(const MCHAR* word, const MorphoInflectionRulesSet_v2* ruleset, MCHAR** rulesCache, Int32& nCacheSize, void* param)
{
	const MorphoData_v2* This = ((EnumPartialParamStruct_v2*)param)->This;

	MCHAR form[MAX_WORD_LEN] = {0};
	const MCHAR* posBuf[2048] = {0};

	MorphoInflectionRulesSet_v2::RulesIterator rule;
	rule = ruleset->RulesBegin();

	while (rule != ruleset->RulesEnd())
	{
		if ( rule->IsLinkToRuleset() )
		{
			const MorphoInflectionRulesSet_v2* subset;
			subset = This->Rule2RuleSetPtr(*rule);
			PartialWalkRuleSet_v2(word, subset, rulesCache, nCacheSize, param);
		}
		else if ( This->Rule2MCharPtr(*rule)[0] )
		{
			const MCHAR* sRule = This->Rule2MCharPtr(*rule);

			Int32 i = 0;
			for(; i<nCacheSize; i++)
				if(!CSldCompare::StrCmpA((const UInt8*)sRule, (const UInt8*)rulesCache[i]))
					break;
			if(i == nCacheSize)
			{
				MorphoTestPtr(ruleset);
				MorphoTestPtr(&ruleset->precondition);
				
				This->InflectWord(word, This->McharRef2Ptr(ruleset->precondition), sRule, form, posBuf);
				rulesCache[nCacheSize] = (MCHAR*)sRule;
				nCacheSize++;

				((EnumPartialParamStruct_v2*)param)->callback(form, ((EnumPartialParamStruct_v2*)param)->BaseForm, ((EnumPartialParamStruct_v2*)param)->callbackParam);
			}
		}
		rule++;
	}

	return true;
}

bool EnumPartialWordsCallBack_v2(const MCHAR* word, WordSet_v2::WordRuleSetsIterator &iter, void* param)
{
	MCHAR* rulesCache[1000] = {0};
	Int32 nCacheSize = 0;
	MCHAR baseForm[MAX_WORD_LEN] = {0};
	EnumPartialParamStruct_v2* pParam = (EnumPartialParamStruct_v2*)param;

	if (pParam->CurrentIndex < pParam->FirstIndex || pParam->CurrentIndex > pParam->LastIndex)
	{
		pParam->CurrentIndex++;
		return true;
	}

	// Базовая форма
	CSldCompare::StrNCopyA((UInt8*)baseForm, (const UInt8*)word, MAX_WORD_LEN-1);
	pParam->BaseForm = baseForm;

	const MorphoInflectionRulesSet_v2* ruleset = 0;
	while ((ruleset = pParam->This->words.GetNextRuleSet(iter)))
	{
		PartialWalkRuleSet_v2(word, ruleset, rulesCache, nCacheSize, param);
	}

	pParam->CurrentIndex++;
	pParam->BaseForm = 0;

	return true;
}

bool MorphoData_v2::EnumPartial(EnumPartialCallBackT* callback, UInt32 aBaseFormIndexFirst, UInt32 aBaseFormIndexLast, void* aParam) const
{
	EnumPartialParamStruct_v2 eps;
	sldMemZero(&eps, sizeof(eps));
	eps.This = this;
	eps.callback = callback;
	eps.callbackParam = aParam;
	eps.FirstIndex = aBaseFormIndexFirst;
	eps.LastIndex = aBaseFormIndexLast;
	eps.CurrentIndex = 0;

	return words.EnumWords(EnumPartialWordsCallBack_v2, &eps);
}
