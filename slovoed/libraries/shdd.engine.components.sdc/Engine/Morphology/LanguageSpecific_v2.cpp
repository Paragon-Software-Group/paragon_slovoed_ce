///////////////////////////////////////////////////////////////////////////////////
///
/// @file LanguageSpecific.h
///
/// Copyright Paragon Software (SHDD), 2006
///
/// Файл содержит определения классов и функций для поддержки 
/// морфологических особенностей, специфических для какого-нибудь 
/// конкретного языка
///
/// \author Капустин Михаил
///
///////////////////////////////////////////////////////////////////////////////////


#include "MorphoData_v2.h"


// Инициализируем данные для обработки морфологических особенностей, 
// специфических для конкретных языков
//LanguageSpecificData MorphoData::langSpecData;

// Местоимения и их базовые формы
// Массив местоимений, которые стоят на первом месте
// Порядок важен: надо, чтобы nos стояло раньше os, 
// ибо os является его частью
//ArrOf2MCHARS LanguageSpecificData::firstPronounsArray[] = 
//{{"me", "yo"}, {"te", "tu"}, {"le", "el/ella/usted"}, {"les", "ellos/ellas/ustedes"}, {"nos", "nosotros"}, {"os", "vosotros"}, {"se", "se"}};
// Массив местоимений, которые стоят на втором месте
//ArrOf2MCHARS LanguageSpecificData::secondPronounsArray[] = 
//{{"lo", "ello"}, {"la", "ella"}, {"los", "ellos"}, {"las", "ellas"}};
// Массив окончаний для герундия (с ударениями)


// Таблицы, в которых перечислены формы, к коим разрешено
// прилепляться местоимениям
//MCHAR * LanguageSpecificData::subtablesNames[] = 
//{"Formas principales", "Imperativo afirmativo"};


LanguageSpecificData_v2::LanguageSpecificData_v2()
{
	MCHAR buf[MAX_PRONOUN_LENGTH];

	FillPronounArrays();

	// Инициализация массива обращенных (задом наперед) местоимений
	for(Int32 i=0; i < GetPronounsVectorSize(0); i++)
	{
		CSldCompare::StrNCopyA((UInt8 *)buf, (UInt8 *)GetPronounForm(0, i, false), MAX_PRONOUN_LENGTH);
		//StrReverse(buf);
		{
			Int8* szTempString;
			Int32 nLen = CSldCompare::StrLenA((UInt8 *)buf);
			szTempString=(Int8*)sldMemNew(sizeof(Int8)*(nLen+1));
			sldMemZero(szTempString, sizeof(Int8)*(nLen+1));
			for(Int32 i=0; i<nLen; i++)
				szTempString[nLen-i-1]=buf[i];
			CSldCompare::StrCopyA((UInt8 *)buf, (UInt8 *)szTempString);
			sldMemFree(szTempString);
		}
		CSldCompare::StrNCopyA((UInt8 *)pronounsRevArrays[0][i], (UInt8 *)buf, MAX_PRONOUN_LENGTH);
	}
	for(Int32 i=0; i < GetPronounsVectorSize(1); i++)
	{
		CSldCompare::StrNCopyA((UInt8 *)buf, (UInt8 *)GetPronounForm(1, i, false), MAX_PRONOUN_LENGTH);
		//StrReverse(buf);
		{
			Int8* szTempString;
			Int32 nLen = CSldCompare::StrLenA((UInt8 *)buf);
			szTempString=(Int8*)sldMemNew(sizeof(Int8)*(nLen+1));
			sldMemZero(szTempString, sizeof(Int8)*(nLen+1));
			for(Int32 i=0; i<nLen; i++)
				szTempString[nLen-i-1]=buf[i];
			CSldCompare::StrCopyA((UInt8 *)buf, (UInt8 *)szTempString);
			sldMemFree(szTempString);
		}
		CSldCompare::StrNCopyA((UInt8 *)pronounsRevArrays[1][i], (UInt8 *)buf, MAX_PRONOUN_LENGTH);
	}
}

LanguageSpecificData_v2::~LanguageSpecificData_v2()
{
	for (Int32 i=0;i<FIRST_ARRAY_SIZE;i++)
	{
		sldMemFree(firstPronounsArray[i][0]);
		sldMemFree(firstPronounsArray[i][1]);
	}

	for (Int32 i=0;i<SECOND_ARRAY_SIZE;i++)
	{
		sldMemFree(secondPronounsArray[i][0]);
		sldMemFree(secondPronounsArray[i][1]);
	}

	sldMemFree(subtablesNames[0]);
	sldMemFree(subtablesNames[1]);
}
UInt16 LanguageSpecificData_v2::FillPronounArrays()
{

	for (Int32 i=0;i<FIRST_ARRAY_SIZE;i++)
	{
		firstPronounsArray[i][0] = (MCHAR *)sldMemNew(MAX_ARRAY_ITEM_SIZE);
		if (!firstPronounsArray[i][0])
			return eMemoryNotEnoughMemory;
		firstPronounsArray[i][1] = (MCHAR *)sldMemNew(MAX_ARRAY_ITEM_SIZE);
		if (!firstPronounsArray[i][1])
			return eMemoryNotEnoughMemory;
	}

	for (Int32 i=0;i<SECOND_ARRAY_SIZE;i++)
	{
		secondPronounsArray[i][0] = (MCHAR *)sldMemNew(MAX_ARRAY_ITEM_SIZE);
		if (!secondPronounsArray[i][0])
			return eMemoryNotEnoughMemory;
		secondPronounsArray[i][1] = (MCHAR *)sldMemNew(MAX_ARRAY_ITEM_SIZE);
		if (!secondPronounsArray[i][1])
			return eMemoryNotEnoughMemory;
	}

	CSldCompare::StrCopyA((UInt8 *)firstPronounsArray[0][0], (UInt8 *)"me");
	CSldCompare::StrCopyA((UInt8 *)firstPronounsArray[0][1], (UInt8 *)"yo");

	CSldCompare::StrCopyA((UInt8 *)firstPronounsArray[1][0], (UInt8 *)"te");
	CSldCompare::StrCopyA((UInt8 *)firstPronounsArray[1][1], (UInt8 *)"tu");

	CSldCompare::StrCopyA((UInt8 *)firstPronounsArray[2][0], (UInt8 *)"le");
	CSldCompare::StrCopyA((UInt8 *)firstPronounsArray[2][1], (UInt8 *)"el/ella/usted");

	CSldCompare::StrCopyA((UInt8 *)firstPronounsArray[3][0], (UInt8 *)"les");
	CSldCompare::StrCopyA((UInt8 *)firstPronounsArray[3][1], (UInt8 *)"ellos/ellas/ustedes");

	CSldCompare::StrCopyA((UInt8 *)firstPronounsArray[4][0], (UInt8 *)"nos");
	CSldCompare::StrCopyA((UInt8 *)firstPronounsArray[4][1], (UInt8 *)"nosotros");

	CSldCompare::StrCopyA((UInt8 *)firstPronounsArray[5][0], (UInt8 *)"os");
	CSldCompare::StrCopyA((UInt8 *)firstPronounsArray[5][1], (UInt8 *)"vosotros");

	CSldCompare::StrCopyA((UInt8 *)firstPronounsArray[6][0], (UInt8 *)"se");
	CSldCompare::StrCopyA((UInt8 *)firstPronounsArray[6][1], (UInt8 *)"se");
	//	{{"me", "yo"}, {"te", "tu"}, {"le", "el/ella/usted"}, {"les", "ellos/ellas/ustedes"}, 
	//	{"nos", "nosotros"}, {"os", "vosotros"}, {"se", "se"}};
	// Массив местоимений, которые стоят на втором месте


	CSldCompare::StrCopyA((UInt8 *)secondPronounsArray[0][0], (UInt8 *)"lo");
	CSldCompare::StrCopyA((UInt8 *)secondPronounsArray[0][1], (UInt8 *)"ello");

	CSldCompare::StrCopyA((UInt8 *)secondPronounsArray[1][0], (UInt8 *)"la");
	CSldCompare::StrCopyA((UInt8 *)secondPronounsArray[1][1], (UInt8 *)"ella");

	CSldCompare::StrCopyA((UInt8 *)secondPronounsArray[2][0], (UInt8 *)"los");
	CSldCompare::StrCopyA((UInt8 *)secondPronounsArray[2][1], (UInt8 *)"ellos");

	CSldCompare::StrCopyA((UInt8 *)secondPronounsArray[3][0], (UInt8 *)"las");
	CSldCompare::StrCopyA((UInt8 *)secondPronounsArray[3][1], (UInt8 *)"ellas");
	//ArrOf2MCHARS LanguageSpecificData::secondPronounsArray[] = 
	//{{"lo", "ello"}, {"la", "ella"}, {"los", "ellos"}, {"las", "ellas"}};
	// Массив окончаний для герундия (с ударениями)

	subtablesNames[0] = (MCHAR *)sldMemNew(MAX_ARRAY_ITEM_SIZE);
	if (!subtablesNames[0])
		return eMemoryNotEnoughMemory;
	subtablesNames[1] = (MCHAR *)sldMemNew(MAX_ARRAY_ITEM_SIZE);
	if (!subtablesNames[1])
		return eMemoryNotEnoughMemory;

	CSldCompare::StrCopyA((UInt8 *)subtablesNames[0], (UInt8 *)"Formas principales");
	CSldCompare::StrCopyA((UInt8 *)subtablesNames[1], (UInt8 *)"Imperativo afirmativo");
	// Таблицы, в которых перечислены формы, к коим разрешено
	// прилепляться местоимениям
	//MCHAR * LanguageSpecificData::subtablesNames[] = 
	//{"Formas principales", "Imperativo afirmativo"};

	return eOK;
}


// Размер массива местоимений (первого или второго)
Int32 LanguageSpecificData_v2::GetPronounsVectorSize(bool numb)const{
	if (numb == 0) return sizeof(firstPronounsArray)/sizeof(ArrOf2MCHARS);
	return sizeof(secondPronounsArray)/sizeof(ArrOf2MCHARS);
}

// Выдаем обращенное местоимение из одного из массивов
const MCHAR * LanguageSpecificData_v2::GetRevesedPronoun(bool vectNumb, Int32 pronNumb)const{
	return GetPronounForm(vectNumb, pronNumb);
}

// Выдаем базовую форму местоимения (из одного из массивов)
const MCHAR * LanguageSpecificData_v2::GetPronounBaseForm(bool vectNumb, Int32 pronNumb) const{
	return GetPronounForm(vectNumb, pronNumb, false, true);
}

// Приватная функция, через которую работают две предыдущие
const MCHAR * LanguageSpecificData_v2::GetPronounForm(bool vectNumb, Int32 pronNumb, bool reverse, bool baseForm )const{
	if (pronNumb < 0 || pronNumb >= GetPronounsVectorSize(vectNumb)) return NULL;

	if (reverse) return pronounsRevArrays[vectNumb][pronNumb];
	return vectNumb ? secondPronounsArray[pronNumb][baseForm] : firstPronounsArray[pronNumb][baseForm];
}

// Выдает размер массива названий таблиц (таблицы, к которым принадлежат
// нужные формы глаголов).
Int32 LanguageSpecificData_v2::GetSubtablesNumber()const{
	return sizeof(subtablesNames)/sizeof(char*);
}

// Выдаем одной из таблиц, к которой принадлежат нужные формы глаголов.
const MCHAR * LanguageSpecificData_v2::GetSubtableName(Int32 numb)const{
	if (numb < 0 && numb >= GetSubtablesNumber()) return NULL;

	return subtablesNames[numb];	
}

#ifdef SLD_LANG_SPECIFIC_SPANISH

void DelAccentsInSpanishWord_v2(MCHAR * word){
	for (Int32 i=0; i<StrLen(word); i++){
		//const MCHAR *srcChars = "БбЙйНнУуЪъ";
		const MCHAR srcChars[] = {(MCHAR)0xC1, (MCHAR)0xE1, (MCHAR)0xC9, (MCHAR)0xE9, (MCHAR)0xCD, (MCHAR)0xED, (MCHAR)0xD3, (MCHAR)0xF3, (MCHAR)0xDA, (MCHAR)0xFA, 0};
		const MCHAR *p = StrChr(srcChars, word[i] );

		// cчитаем е
		if ( p )
		{
			word[i] = "AaEeIiOoUu"[ p - srcChars ];
			break;
		}
	}
}

#endif // SLD_LANG_SPECIFIC_SPANISH


bool LanguageSpecificData_v2::IsWordGermDetachablePrefix(const MCHAR* Word) const
{
	if ( !Word || !(*Word) )
		return false;
	
	// Приставки, которые могут использоваться как отделяемые с глаголами
	/*
	if
	(
		StrCmp((UInt8*)Word, (UInt8*)"ab")==0	||
		StrCmp((UInt8*)Word, (UInt8*)"an")==0	||
		StrCmp((UInt8*)Word, (UInt8*)"auf")==0	||
		StrCmp((UInt8*)Word, (UInt8*)"aus")==0	||
		StrCmp((UInt8*)Word, (UInt8*)"ein")==0	||
		StrCmp((UInt8*)Word, (UInt8*)"zu")==0	||
		StrCmp((UInt8*)Word, (UInt8*)"mit")==0	||
		StrCmp((UInt8*)Word, (UInt8*)"hin")==0	||
		StrCmp((UInt8*)Word, (UInt8*)"vor")==0	||
		StrCmp((UInt8*)Word, (UInt8*)"entgegen")==0	||
		StrCmp((UInt8*)Word, (UInt8*)"bei")==0	||
		StrCmp((UInt8*)Word, (UInt8*)"fest")==0	||
		StrCmp((UInt8*)Word, (UInt8*)"her")==0	||
		StrCmp((UInt8*)Word, (UInt8*)"los")==0	||
		StrCmp((UInt8*)Word, (UInt8*)"nach")==0	||
		StrCmp((UInt8*)Word, (UInt8*)"weg")==0
	)
	return true;
	
	return false;
	*/
	
	// Это неотделяемые приставки
	if
	(
		CSldCompare::StrCmpA((UInt8*)Word, (UInt8*)"be")==0	||
		CSldCompare::StrCmpA((UInt8*)Word, (UInt8*)"ge")==0	||
		CSldCompare::StrCmpA((UInt8*)Word, (UInt8*)"er")==0	||
		CSldCompare::StrCmpA((UInt8*)Word, (UInt8*)"ver")==0	||
		CSldCompare::StrCmpA((UInt8*)Word, (UInt8*)"zer")==0	||
		CSldCompare::StrCmpA((UInt8*)Word, (UInt8*)"ent")==0	||
		CSldCompare::StrCmpA((UInt8*)Word, (UInt8*)"emp")==0	||
		CSldCompare::StrCmpA((UInt8*)Word, (UInt8*)"miЯ")==0
	)
	return false;
	
	// Любое другое слово (при условии что это существующее немецкое слово) в принципе может
	// являться отделяемой приставкой (очень много различных случаев), поэтому можно попробовать
	return true;
}

// Языкоспецифичные проверки
bool MorphoData_v2::LanguageSpecificTrials ( BaseFormsIterator &iter, MCHAR *baseFormBuf,  const MorphoInflectionRulesSetHandle** rulesetPtr, UInt32 language ) const
{
	language = EnsureNativeByteOrder_32(language);
	
	if (language == 'span')
	{
		// Для испанского проверяем, нет ли прилепленных к глаголу
		// местоимений
		
		MCHAR *wordForm;//[MAX_WORD_LEN]; // здесь храним исходную форму слова
		MCHAR *copyOfWordForm;//[MAX_WORD_LEN]; // копия исходной формы для служебных целей
		MCHAR *reversedWord;//[MAX_WORD_LEN]; // обращенная исходная форма слова

		// Заводим итераторы и буфера для вызовов GetNextBaseForm и GetNextWordForm
		BaseFormsIterator *bIter = sldNew<BaseFormsIterator>();
		BaseFormsIterator *bIterForPron = sldNew<BaseFormsIterator>();
		WordFormsIterator formsIter;
		MCHAR *baseForm;//[BUFLEN_FOR_ITERATOR];
		MCHAR *baseFormForPron;//[BUFLEN_FOR_ITERATOR];
		MCHAR *generatedForm;//[BUFLEN_FOR_ITERATOR];
		const MCHAR * question;
		const MCHAR * nameOfForm;
		const MorphoInflectionRulesSetHandle *ruleset;
		const MorphoInflectionRulesSetHandle *rulesetForPron;
		MCHAR *bufForPronounsBaseForms;//[MAX_WORD_LEN];

		const MCHAR * oneBaseForm; // Вспомогательный указатель

		// Заводим массивы для хранения откушенных местоимений (а заодно - переменные
		// для хранения количества найденный местоимений
		Int32 pronounsFound = 0;
		MCHAR (*pronouns)[MAX_WORD_LEN];

		// Здесь храним базовые формы - сразу все вместе для одного местоимения
		// Они перечислены в каждой строке через слэш /
		MCHAR (*pronounsCompoundBaseForms)[MAX_WORD_LEN];
		// Здесь храним базовые формы, отделенные друг от друга
		MCHAR (*pronounsSeparatedBaseForms)[MAX_WORD_LEN];// [BaseFormsIterator::MAX_SIMULTANEOUS_FORMS][MAX_WORD_LEN];
		Int32 prSepBaseFormsNumber = 0; // Сколько для всех местоимений нашлось разделенных базовых форм

		wordForm = (MCHAR *)sldMemNew(MAX_WORD_LEN*sizeof(MCHAR));
		copyOfWordForm = (MCHAR *)sldMemNew(MAX_WORD_LEN*sizeof(MCHAR));
		reversedWord = (MCHAR *)sldMemNew(MAX_WORD_LEN*sizeof(MCHAR));
		bufForPronounsBaseForms = (MCHAR *)sldMemNew(MAX_WORD_LEN*sizeof(MCHAR));
		baseForm = (MCHAR *)sldMemNew(BUFLEN_FOR_ITERATOR*sizeof(MCHAR));
		baseFormForPron = (MCHAR *)sldMemNew(BUFLEN_FOR_ITERATOR*sizeof(MCHAR));
		generatedForm = (MCHAR *)sldMemNew(BUFLEN_FOR_ITERATOR*sizeof(MCHAR));
		pronouns = (MCHAR(*)[MAX_WORD_LEN])sldMemNew(2*MAX_WORD_LEN*sizeof(MCHAR));
		pronounsCompoundBaseForms = (MCHAR(*)[MAX_WORD_LEN])sldMemNew(2*MAX_WORD_LEN*sizeof(MCHAR));
		pronounsSeparatedBaseForms = (MCHAR(*)[MAX_WORD_LEN])sldMemNew(MAX_WORD_LEN*BaseFormsIterator::MAX_SIMULTANEOUS_FORMS*sizeof(MCHAR));

	//	if (StrCmp(baseFormBuf, ))
	//	{
	//	}

		// Здесь храним базовые формы - сразу все вместе для одного местоимения
		// Они перечислены в каждой строке через слэш /
	//	MCHAR pronounsCompoundBaseForms[2][MAX_WORD_LEN];
		// Здесь храним базовые формы, отделенные друг от друга
	//	MCHAR pronounsSeparatedBaseForms[BaseFormsIterator::MAX_SIMULTANEOUS_FORMS][MAX_WORD_LEN];

		if (!wordForm || !copyOfWordForm || !reversedWord || !bufForPronounsBaseForms || !baseForm ||
			!baseFormForPron || !generatedForm || !pronouns ||
			!pronounsCompoundBaseForms || !pronounsSeparatedBaseForms || !bIterForPron  || !bIter )
		{
			sldMemFree(wordForm);
			sldMemFree(copyOfWordForm);
			sldMemFree(reversedWord);
			sldMemFree(bufForPronounsBaseForms);
			sldMemFree(baseForm);
			sldMemFree(baseFormForPron);
			sldMemFree(generatedForm);
			sldMemFree(pronouns);
			sldMemFree(pronounsCompoundBaseForms);
			sldMemFree(pronounsSeparatedBaseForms);
			sldDelete(bIterForPron);
			sldDelete(bIter);

			return false;
		}

		// Флаги для использования в нижеследующих циклах
		bool isSubtable = false, isAppropriateSubTable = false, separatedBaseFormFound;

		// Инициализируем переменные, относящиеся к форме слова
		CSldCompare::StrNCopyA((UInt8 *)wordForm, (UInt8 *)iter.word_form, MAX_WORD_LEN);
	//	DelAccentsInSpanishWord(wordForm);

		CSldCompare::StrNCopyA((UInt8 *)copyOfWordForm, (UInt8 *)wordForm, MAX_WORD_LEN);
		CSldCompare::StrNCopyA((UInt8 *)reversedWord, (UInt8 *)wordForm, MAX_WORD_LEN);
		StrReverse((Int8*)reversedWord);

		// Такой цикл находит либо местоимение 1 на самом конце, 
		// либо 2 на конце и 1 перед ним, либо же только 2. 
		// То есть автоматически проверяется порядок.
		for(Int32 i=2; i--; ){
			// Во внутреннем цикле порядок тоже важен: сначало проверяются
			// местоимения, частью которых могут являться другие
			// (они стоят в массиве раньше)
			for (Int32 j=0; j<langSpecData.GetPronounsVectorSize(i); j++){
				Int8 * rp = (Int8 *)langSpecData.GetRevesedPronoun(i, j);
				if (!sld2::StrNCmp(reversedWord, (MCHAR*)rp, CSldCompare::StrLenA((UInt8*)rp))){
					// Найденное местоимение выворачиваем обратно
					// и копируем в массив местоимений
					CSldCompare::StrNCopyA((UInt8*)pronouns[pronounsFound], (UInt8*)rp, MAX_WORD_LEN);
					StrReverse((Int8*)pronouns[pronounsFound]);
					
					CSldCompare::StrNCopyA((UInt8*)pronounsCompoundBaseForms[pronounsFound], 
							 (UInt8*)langSpecData.GetPronounBaseForm(i, j), MAX_WORD_LEN);
					

					// Отрезаем его от слова
					CSldCompare::StrNCopyA((UInt8*)wordForm, (UInt8*)copyOfWordForm, 
						CSldCompare::StrLenA((UInt8*)copyOfWordForm) - CSldCompare::StrLenA((UInt8*)rp));
					wordForm[CSldCompare::StrLenA((UInt8*)copyOfWordForm) - CSldCompare::StrLenA((UInt8*)rp)] = '\0';
					CSldCompare::StrNCopyA((UInt8*)copyOfWordForm, (UInt8*)wordForm, MAX_WORD_LEN);

					// Создаем новую вывернутую копию слова
					CSldCompare::StrNCopyA((UInt8 *)reversedWord, (UInt8 *)wordForm, MAX_WORD_LEN);
					StrReverse((Int8*)reversedWord);
					
					// Увеличиваем счетчик найденных местоимений	
					pronounsFound++;

					// Выходим из внутреннего цикла
					break;
				}
			}
		}

		// Местоимений в хвосте слова не обнаружилось - больше ничего делать не надо
		if (!pronounsFound)
		{
			sldMemFree(wordForm);
			sldMemFree(copyOfWordForm);
			sldMemFree(reversedWord);
			sldMemFree(bufForPronounsBaseForms);
			sldMemFree(baseForm);
			sldMemFree(baseFormForPron);
			sldMemFree(generatedForm);
			sldMemFree(pronouns);
			sldMemFree(pronounsCompoundBaseForms);
			sldMemFree(pronounsSeparatedBaseForms);
			sldDelete(bIterForPron);
			sldDelete(bIter);
			return false;
		}

		// Проверим, является ли получившееся слово глаголом
		// в одной из разрешенных для приклеивания форм
		
		// Может ли это быть формой герундия? Ее надо обработать 
		// особо из-за ударений
		// Герундий обязан кончаться на "ndo"
		MCHAR revNdo[MAX_WORD_LEN];
		CSldCompare::StrNCopyA((UInt8*)revNdo, (UInt8*)"ndo", MAX_WORD_LEN);
		StrReverse((Int8*)revNdo);
		
		Int32 ndoLen = CSldCompare::StrLenA((UInt8*)revNdo);
		Int32 wordFormLen = CSldCompare::StrLenA((UInt8*)wordForm);
		Int32 posForAccent = wordFormLen - ndoLen - 1;
		if (!sld2::StrNCmp(reversedWord, revNdo, ndoLen) && posForAccent >= 0){
			// Безударной эта форма герундия быть не может
			// Обнаружили безударную - надо выйти, чтобы не было 
			// повторных находок одного и того же
			if (reversedWord[ndoLen] == 'a' || reversedWord[ndoLen] == 'e')
			{
				sldMemFree(wordForm);
				sldMemFree(copyOfWordForm);
				sldMemFree(reversedWord);
				sldMemFree(bufForPronounsBaseForms);
				sldMemFree(baseForm);
				sldMemFree(baseFormForPron);
				sldMemFree(generatedForm);
				sldMemFree(pronouns);
				sldMemFree(pronounsCompoundBaseForms);
				sldMemFree(pronounsSeparatedBaseForms);
				sldDelete(bIterForPron);
				sldDelete(bIter);
				return false;
			}
			// А вот если она под ударением - надо заменить на безударную
			if (reversedWord[ndoLen] == (MCHAR)0xE1 /*'б'*/) wordForm[posForAccent] = 'a';
			if (reversedWord[ndoLen] == (MCHAR)0xE9 /*'й'*/) wordForm[posForAccent] = 'e';
		}

		// Будем искать базовые формы для того, что осталось после откусывания местоимений
		BaseFormsIteratorInit(*bIter, wordForm, M_BASEFORMS_SHOULD_MATCH_PRECONDITION | M_BASEFORMS_USE_DICTIONARY | M_BASEFORMS_ONLY_DICTIONARY_WORDS );

		while(GetNextBaseForm(*bIter, baseForm, &ruleset)){
			// Базовую форму нашли, но разрешается ли к ней прикреплять местоимение?
			// Сейчас переберем формы слова и поищем один из разрешенных подклассов
			WordFormsIteratorInit(formsIter, baseForm, ruleset, M_FORMS_DECORATE_WORD | M_FORMS_SEPARATE_SUBFORM_NAMES);
			
			while( GetNextWordForm(formsIter, generatedForm, &question, &nameOfForm, &isSubtable) )
			{
				if ( isSubtable )
				{
					// Это подкласс, он разрешен?
					isAppropriateSubTable = false;
					for(Int32 i=0; i<langSpecData.GetSubtablesNumber(); i++){
						if (!CSldCompare::StrCmpA((UInt8*)langSpecData.GetSubtableName(i), (UInt8*)nameOfForm)) isAppropriateSubTable = true;
					}
				}
				else{
					// Проверим, в нужном ли мы подклассе и совпадает ли получанная
					// форма глагола с исходной (после откусывания местоимений)
					if (isAppropriateSubTable && !CSldCompare::StrCmpA((UInt8*)generatedForm, (UInt8*)wordForm)){
						
						// Мы нашли то, что нужную форму глагола!
						// Заполняем поля итератора
						// Записываем в него найденную базовую форму глагола и ее рулесет
						CSldCompare::StrNCopyA((UInt8*)iter.simultaneousForms[iter.simultaneousFormsNumber], (UInt8*)baseForm, MAX_WORD_LEN);
						iter.simultaneousFormsRulesets[iter.simultaneousFormsNumber++] = ruleset;
						
						// Если не стоит флаг "не выдавать формы местоимений", 
						// то ищем эти формы
						if (!(iter.flags & M_IGNORE_BASEFORMS_OF_PARTS)){
								// Заполняем вспомогательный массив отделенных друг от друга базовых
								// форм местоимений
							for (Int32 i=pronounsFound; i--; ){
								CSldCompare::StrNCopyA((UInt8*)bufForPronounsBaseForms, (UInt8*)pronounsCompoundBaseForms[i], MAX_WORD_LEN);
								MCHAR *tmpStr = bufForPronounsBaseForms;
								oneBaseForm = (MCHAR*)StrTok((Int8**)&tmpStr, (const Int8*)"/");
								do{
									CSldCompare::StrNCopyA((UInt8*)pronounsSeparatedBaseForms[prSepBaseFormsNumber++], (UInt8*)oneBaseForm, MAX_WORD_LEN);
								}while((oneBaseForm = (MCHAR*)StrTok((Int8**)&tmpStr, (const Int8*)"/")));

							}

							// Выясняем, какая возможных базовых форм соответствует
							// имеющейся форме местоимения
							// Это делается, чтобы найти нужный рулесет
							for (Int32 i=0; i<prSepBaseFormsNumber; i++){
								
								BaseFormsIteratorInit(*bIterForPron, pronounsSeparatedBaseForms[i], M_BASEFORMS_SHOULD_MATCH_PRECONDITION | M_BASEFORMS_USE_DICTIONARY | M_BASEFORMS_ONLY_DICTIONARY_WORDS );
								
								separatedBaseFormFound = false;

								// Перебираем базовые формы имеющегося местоимения
								// пока не найдем совпадающую с записанной в массиве
								// местоимений и их форм.
								while(GetNextBaseForm(*bIterForPron, baseFormForPron, &rulesetForPron)){
									if (!CSldCompare::StrCmpA((UInt8*)baseFormForPron, (UInt8*)pronounsSeparatedBaseForms[i])){
										separatedBaseFormFound = true;
										break;
									}
								}

								// Какая-то ошибка, такого вообще-то не должно быть.
								// Видимо, записанная в массиве базовая форма местоимения
								// записана неверно...
								if (!separatedBaseFormFound)
								{
									sldMemFree(wordForm);
									sldMemFree(copyOfWordForm);
									sldMemFree(reversedWord);
									sldMemFree(bufForPronounsBaseForms);
									sldMemFree(baseForm);
									sldMemFree(baseFormForPron);
									sldMemFree(generatedForm);
									sldMemFree(pronouns);
									sldMemFree(pronounsCompoundBaseForms);
									sldMemFree(pronounsSeparatedBaseForms);
									sldDelete(bIterForPron);
									sldDelete(bIter);
									return false;
								}
								
								// Все нашли, записываем базовую форму местоимения
								CSldCompare::StrNCopyA(
									(UInt8*)iter.simultaneousForms[iter.simultaneousFormsNumber], 
									(UInt8*)pronounsSeparatedBaseForms[i], 
									MAX_WORD_LEN
								);	
								// Записываем рулесет
								iter.simultaneousFormsRulesets[iter.simultaneousFormsNumber++] = rulesetForPron;
							}					
						}


						// Устанавливаем флаг для того, чтобы потом по очереди 
						// выдать найденные варианты.
						iter.afterRecursion = true;
						sldMemFree(wordForm);
						sldMemFree(copyOfWordForm);
						sldMemFree(reversedWord);
						sldMemFree(bufForPronounsBaseForms);
						sldMemFree(baseForm);
						sldMemFree(baseFormForPron);
						sldMemFree(generatedForm);
						sldMemFree(pronouns);
						sldMemFree(pronounsCompoundBaseForms);
						sldMemFree(pronounsSeparatedBaseForms);
						sldDelete(bIterForPron);
						sldDelete(bIter);
						return true;
					}
				}

			}

		}

		sldMemFree(wordForm);
		sldMemFree(copyOfWordForm);
		sldMemFree(reversedWord);
		sldMemFree(bufForPronounsBaseForms);
		sldMemFree(baseForm);
		sldMemFree(baseFormForPron);
		sldMemFree(generatedForm);
		sldMemFree(pronouns);
		sldMemFree(pronounsCompoundBaseForms);
		sldMemFree(pronounsSeparatedBaseForms);
		sldDelete(bIterForPron);
		sldDelete(bIter);
	}
	else if (language == 'germ')
	{
		// Проверим, может быть нам передали форму глагола с отделяемой приставкой, типа "sehen an"

		// Переданные параметры		
		// BaseFormsIterator &iter
		// MCHAR *baseFormBuf
		// const MorphoInflectionRulesSetHandle** rulesetPtr
		// UInt32 language
		
		if ( !baseFormBuf || !iter.word_form)
			return false;
		
		UInt32 length = CSldCompare::StrLenA((UInt8*)iter.word_form);
		UInt32 attr;
		UInt32 curlen = 0;
		
		MCHAR pref[MAX_WORD_LEN] = {0};
		MCHAR base[MAX_WORD_LEN] = {0};
		MCHAR* pp = pref;
		MCHAR* pb = base;

		// Выделим слова до и после разделителя
		while ( iter.word_form[curlen] && (curlen < length) )
		{
			attr = LetterAttributes(iter.word_form[curlen]);
			if (attr & CHARACTER_ATTRIBUTE_CHARACTER_IS_ALLOWED_SEPARATOR)
			{
				UInt32 i = curlen;
				while (iter.word_form[i] && (i < length))
				{
					attr = LetterAttributes(iter.word_form[i]);
					if (attr & CHARACTER_ATTRIBUTE_CHARACTER_IS_ALLOWED_SEPARATOR)
					{
						if (*pref)
							break;
							
						// С другими разделителями отделенная часть не может являться отделяемой приставкой
						if ( !(iter.word_form[i]==' ' || iter.word_form[i]==0x9) )
							break;
							
						i++;
						continue;
					}
					*pp++ =iter.word_form[i++];
				}
				break;
			}
			else
				*pb++ = iter.word_form[curlen++];
		}
		
		// Необходимые переменные для последовательного получения форм слова
		MCHAR baseForm[MAX_WORD_LEN] = {0};
		MCHAR Form[MAX_WORD_LEN] = {0};
		const MorphoInflectionRulesSetHandle* ruleset;
		const MCHAR* question;
		const MCHAR* nameOfForm;
		bool isSubtable;
		
		// Найдем базовые формы для слова с перенесенной вперед приставкой (если это приставка и она отделяемая)
		if ( langSpecData.IsWordGermDetachablePrefix(pref) )
		{
			pb = base;
			attr = LetterAttributes(*pb);
			if (attr & CHARACTER_ATTRIBUTE_IS_UPPER)
				*pp++ = ToLower(*pb++);
			
			while (*pb)
				*pp++ = *pb++;
			
			pp = pref;
			if (attr & CHARACTER_ATTRIBUTE_IS_UPPER)
				*pp = ToUpper(*pp);
			
			// Ищем базовые формы
			BaseFormsIterator bIter;
			BaseFormsIteratorInit(bIter, pref, M_BASEFORMS_SHOULD_MATCH_PRECONDITION | M_BASEFORMS_USE_DICTIONARY | M_BASEFORMS_ONLY_DICTIONARY_WORDS );
			while(GetNextBaseForm(bIter, baseForm, &ruleset))
			{
				// Среди словоформ найденных базовых форм ищем ту форму, которую у нас запросили (baseFormBuf)
				WordFormsIterator formsIter;
				WordFormsIteratorInit(formsIter, baseForm, ruleset, M_FORMS_DECORATE_WORD | M_FORMS_SEPARATE_SUBFORM_NAMES);
				while( GetNextWordForm(formsIter, Form, &question, &nameOfForm, &isSubtable) )
				{
					if (!isSubtable)
					{
						if (CSldCompare::StrCmpA((UInt8*)Form, (UInt8*)iter.word_form) == 0)
						{
							// Словоформа базовой формы совпала с той, которую мы ищем; нужно запомнить базовую форму
							// Может быть должна быть более жесткая проверка?
							CSldCompare::StrNCopyA((UInt8*)iter.simultaneousForms[iter.simultaneousFormsNumber], (UInt8*)baseForm, MAX_WORD_LEN);
							iter.simultaneousFormsRulesets[iter.simultaneousFormsNumber++] = ruleset;
							
							// Далее проверять словоформы нет смысла, базовая форма подходит
							break;
						}
					}
				}
			}
		}
		
		if (iter.simultaneousFormsNumber > 0)
		{
			// Базовые формы нашлись
			iter.afterRecursion = true;
			return true;
		}
		
		// Если ни одной базовой формы для словоформы с перенесенной вперед приставкой не найдено,
		// попробуем найти базовые формы для слова вообще без приставки (если мы этого уже не делали).
		
		// Отделенной приставки не было - не надо еще раз искать базовые формы,
		// они уже найдены в предыдущих вызовах GetNextBaseForm()
		if ( (iter.foundFormsNumber > 0) || !(*pref) )
			return false;

		// Мы здесь в том случае, если не нашли базовых форм для словоформы с перенесенной вперед приставкой
		BaseFormsIterator bIter;
		BaseFormsIteratorInit(bIter, base, M_BASEFORMS_SHOULD_MATCH_PRECONDITION | M_BASEFORMS_USE_DICTIONARY | M_BASEFORMS_ONLY_DICTIONARY_WORDS ); 
		while( GetNextBaseForm(bIter, baseForm, &ruleset) )
		{
			CSldCompare::StrNCopyA((UInt8*)iter.simultaneousForms[iter.simultaneousFormsNumber], (UInt8*)baseForm, MAX_WORD_LEN);
			iter.simultaneousFormsRulesets[iter.simultaneousFormsNumber++] = ruleset;
		}
		
		if (iter.simultaneousFormsNumber > 0)
		{
			// Базовые формы нашлись
			iter.afterRecursion = true;
			return true;
		}
	}

	return false;
}
