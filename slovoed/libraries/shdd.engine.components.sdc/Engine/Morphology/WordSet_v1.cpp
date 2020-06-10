/** 
**************************************************************
*
*   @file WordSet.cpp
*
*   (c) Paragon Software (SHDD), 2004
*
*	Файл реализует методы класса WordSet, который предназначен
*   для создания, хранения и работы с сохраненным списком слов
*   морфологической базы данных, вместе с ссылками на правила 
*   для каждого слова.
*
*   \author Карпизин Вячеслав
*
*	Файл создан: 8.09.2004
*
************************************************************** */

#include "../SldPlatform.h"
#include "MorphoData_v1.h"

#ifdef _ML_PARSER_
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///
///			private-функция SortList.
///				Сортирует список слов в алфавитном порядке. 
///				Для сортировки используется метод Шелла, порядок сложности алгоритма - O(n^1.25)
///
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void WordSet_v1::SortList(s_word *WordList, Int32 index)
{
	s_word tempdata;
	Int32 inc, i, j, seq[] = {1,5,19,41,109,317,901};  
	Int32 s=6;  // вычисление последовательности приращений  

	while (s >= 0) {	// сортировка вставками с инкрементами inc[] 
		inc = seq[s--];    
		for (i = inc; i < index; i++) {      
			sldMemZero(tempdata.str, sizeof(s_str), 0);
			tempdata.counter = WordList[i].counter;
			for(Int32 ii = 0; ii<sizeof(s_str); tempdata.str[ii] = WordList[i].str[ii++]);
			
			for (j = i-inc; 
(j >= 0) && (StrCmp((UInt8*)WordList[j].str, (UInt8 *)tempdata.str)>0); 
									j -= inc)
			{    
				sldMemZero(WordList[j+inc].str, sizeof(s_str), 0);
				WordList[j+inc].counter = WordList[j].counter;
				for(Int32 ii = 0; ii<sizeof(s_str); WordList[j+inc].str[ii] = WordList[j].str[ii++]);
			}
			sldMemZero(WordList[j+inc].str, sizeof(s_str), 0);
			WordList[j+inc].counter = tempdata.counter;
			for(Int32 ii = 0; ii<sizeof(s_str); WordList[j+inc].str[ii] = tempdata.str[ii++]);
		}  
	}
}



//////////////////////////////////////////////////////////////////////////////////////////////
///
/// Функция CreatePrefixList создает префиксный список. Используется исключительно внутри класса.
/// 
/// Вызывается при сохранении класса в файл для уменьшения объема памяти, занимаемой списком слов
///
///
//////////////////////////////////////////////////////////////////////////////////////////////

void WordSet_v1::CreatePrefixList(s_word *WordList, Int32 index, Int32 maxrulenum)
{
	s_str pattern;
	s_str temp;
	sldMemZero(pattern, sizeof(s_str), 0);
	Int32 i;
	UInt8 j;

	Int32 rule = maxrulenum;
	short rd = 0;
	for( ; rule ; rule /= 10, rd++);

	for( i = 0; i < index; i++)
	{
		j=0; 
		while((j<sizeof(s_str))&&pattern[j]&&(pattern[j]==WordList[i].str[j])&&(j < 9)) j++;
		sldMemZero(pattern, sizeof(s_str), 0);
		sldMemZero(temp, sizeof(s_str), 0);
		StrCopy((char *)pattern, (char *)WordList[i].str);
		StrCopy((char *)temp, (char *)WordList[i].str+j); 
		sldMemZero(WordList[i].str, sizeof(s_str), 0);
		StrCopy((char *)(WordList[i].str+ rd + 1), (char *)temp);
		WordList[i].str[0] = j;

		Int32 div = 1;
		for(Int32 k = rd; k ; k--)
		{
			WordList[i].str[k] = (WordList[i].counter/div)%10;
			div *= 10;
		}


	}
}

#endif


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// public-функция Init. Инициализирует класс при работе с файлом. В качестве параметров получает указатель 
///		на начало данных класса и их суммарный размер
///				
///		ptr - указатель на заголовок данных для декодирования.
///		ArchivePtrArray - указатель на массив блоков сжатых данных
///		morphoPtr - указатель на класс морфологии.
///
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _ML_PARSER_

void WordSet_v1::Init(const UInt32 *ArchHeader,
				   const CSDCReadMy::Resource *ArchivePtrArray,
				   const void	*IndexArrayPtr,
				   const STAT_VECTOR_SIZE_V1 *RulesSetPtr,
				   const UInt8	*HuffmanTreePtr,
				   const MorphoData_v1 *morphoPtr)
{
	IndexTableSize      = MORPHO_SWAP_32(((const Data *)ArchHeader)->IndexTableSize);  
	RuleSetArraySize    = MORPHO_SWAP_32(((const Data *)ArchHeader)->RuleSetArraySize);
	ArchiveBlockCounter = MORPHO_SWAP_32(((const Data *)ArchHeader)->ArchiveBlockCounter);
	HaffmanTableSize    = MORPHO_SWAP_32(((const Data *)ArchHeader)->HaffmanTableSize);

	IndexArray = (IndexArrayElement *)IndexArrayPtr;

	RuleSetArray = RulesSetPtr;

//	BlockSizeArray = (const Int32 *)NULL;

	HaffmanTable = (const UInt8 *)HuffmanTreePtr;

	//------------------------
	ArchiveBlockArray = ArchivePtrArray;

	MorphoPtr = morphoPtr;
}
#endif

#ifndef _ML_PARSER_
void WordSet_v1::Close()
{
//	sldMemFree(ArchiveBlockArray);
}
#endif


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	komissarov
//	функция сравнения двух строк, полный аналог strncmp
//	на входе: две сравниваемые строки и кол-во сравниваемых символов
//
////////////////////////////////////////////////////////////////////////////////////////////////////
Int32 WordSet_v1::StrNCMP(UInt8* szString1, UInt8* szString2, Int32 nSize) const
{
	UInt8* szTemp1,
		* szTemp2;

	szTemp1 = (UInt8*)sldMemNew(sizeof(UInt8)*(nSize+1));
	szTemp2 = (UInt8*)sldMemNew(sizeof(UInt8)*(nSize+1));
	sldMemZero(szTemp1, sizeof(UInt8)*(nSize+1));
	sldMemZero(szTemp2, sizeof(UInt8)*(nSize+1)); 

	CSldCompare::StrNCopyA((UInt8*)szTemp1, (UInt8*)szString1, nSize); 
	CSldCompare::StrNCopyA((UInt8*)szTemp2, (UInt8*)szString2, nSize);

	Int32 nRes = CSldCompare::StrCmpA(szTemp1, szTemp2);
	sldMemFree(szTemp1);
	sldMemFree(szTemp2);
	return nRes;
}
///////////////////////////////////////////////////////////////////////////////////////////
///
/// public-функция SearchWord. Ищет слово во внутренних структурах класса; 
///		в случае успеха инициализирует итератор правил для слова  
///				
///
///
///////////////////////////////////////////////////////////////////////////////////////////

bool WordSet_v1::SearchWord(const MCHAR *word, WordRuleSetsIterator &iter, 
						 MCHAR * partialFormBuffer, Int32 bufSize, 
						 const MorphoInflectionRulesSet_v1 * rsArray, Int32 rsArrSize) const
{
#ifdef _ML_PARSER_
	std::string s(word);

	iter.itr = w.find(s);
	
	if( iter.itr != w.end() ) 
	{
		iter.inflruleset_iter = iter.itr->second.begin();
	}
	else
	{
		//iter.itr = NULL;
	}

	return ( iter.itr != w.end() )?true:false;

#else

//	Int32				WordIndex = 0; 
	Int32				WordOffset = -1; 
	UInt8		t[6];
	Int32				bit = 0; 
	Int32				leftborder = 0;
	Int32				rightborder = IndexTableSize/8-1; 
//	Int32 				prev_bit = 0;
//	Int32				prev_symb_bit = 0;
//	Int32				prev_word_bit = 0;
	Int32				prev_word_len = MAX_WORD_LEN;
	Int32				letterindex = 0;
	UInt8		pattern[MAX_WORD_LEN];
	Int32				i;
	UInt8		c = 0;
	Int32				current_bit;
	Int32				digitcount = 0; 
	Int32				RuleSetIndex = 0; 
	UInt8		digits_for_rule = 0;

	UInt8		CurrentBlock;
	UInt8		tempChar;

	// вычисление длины правила (десятичный логарифм)
	for(Int32 mult =  RuleSetArraySize/sizeof(STAT_VECTOR_SIZE_V1) ; mult; digits_for_rule++, mult /= 10);
	
	sldMemZero(pattern, MAX_WORD_LEN);

	// ищем нужную точку входа в основной массив по индексной таблице. 
	while(1)
	{
		if(rightborder-leftborder<=1) break;
		for(i = 0; i < 4; i++) t[i] = IndexArray[(leftborder+rightborder)>>1].w[i];
		t[4] = 0xFF;
		t[5] = 0;
		if(CSldCompare::StrCmpA((UInt8 *)t, (UInt8 *)word)>0) rightborder = (leftborder+rightborder+1)>>1;
		else leftborder = (leftborder+rightborder)>>1;
	}	// 
	//

	// вычисляем номер бита, с которого начинается блок, в котором может находится искомое слово
	bit = MORPHO_SWAP_32(IndexArray[leftborder].bit)&0x00ffffff;


	// вычисляем количество слов во всех блоках вплоть до предшествующего тому, 
	// в котором будем искать текущее слово. 
/*	Это действие не нужно - далее WordIndex нигде не используется!!!
	for(i = 0, WordIndex = 0; i<leftborder+1; i++)
	{
		WordIndex+=MORPHO_SWAP_32(IndexArray[i].bit)>>24;
	}
*/	
	// вычисляем номер мета-блока, т.е. единицы, на которые разбивается архив для хранения 
	// в prc-ресурсах
	CurrentBlock = (UInt8)(bit/(8*DEFAULT_BLOCK_SIZE));

	// если номер мета-блока превысил их количество - где-то косяк
	if(CurrentBlock>=ArchiveBlockCounter)
	{
		return false;
	}


	// вычисляем номер бита в текущем мета-блоке
	bit %= (8*DEFAULT_BLOCK_SIZE);

	// это вспомогательный указатель для доступа к данным типа BlockType, 
	// без него типы нихера не приводятся. 
	const UInt8* blockPtr = ArchiveBlockArray[CurrentBlock].ptr();
	UInt32 blockSize = ArchiveBlockArray[CurrentBlock].size();

	while(1)// 
	{
			leftborder = 0; 
			rightborder = HaffmanTableSize-1;

			// побитовое раскодирование одного символа - 
			while(leftborder<rightborder)
			{
				// получаем байт, в котором содержится бит с нужным номером
				tempChar = blockPtr[bit>>3];

				// сдвигаем байт, пока искомый бит не станет младшим, после чего обнуляем
				// остальные биты по маске. 
				current_bit = (tempChar>>(bit&0x07))&0x01;

				// если соотв. бит установлен - идем по дереву Хаффмана влево
				if(current_bit==1) 
					leftborder = HaffmanTable[leftborder];
				else // иначе идем вправо
				{
					rightborder = HaffmanTable[leftborder]-1;
					leftborder++;
				}
				bit++;

				// если номер бита стал больше, чем количество бит в одном мета-блоке - перейдем к следующему мета-блоку.  
				if(bit >= 8*blockSize)
				{
					bit -= 8*blockSize;
					CurrentBlock++;
					if(CurrentBlock>=ArchiveBlockCounter)
					{
						return false;
					}
					blockPtr = ArchiveBlockArray[CurrentBlock].ptr();
					blockSize = ArchiveBlockArray[CurrentBlock].size();
				}
			}
			// получаем раскодирвоанный символ по его индексу в таблице Хаффмана. 
			c = HaffmanTable[leftborder];
			
			// если это буква 
			if(c>9) 
			{
				pattern[letterindex++] = c;
				digitcount = 0; 
			}
			else // иначе это цифра. Первая цифра - это длина префикса, 
				//вторая и далее до первой буквы - номер в таблице правил, 
				// с которого начинаются правила для данного слова. 
			{
				if(digitcount == 0)// цифра первая, значит длина префикса
				{
					// если в конце слова есть мусор - уберем его. 
					for(i = letterindex; i < prev_word_len; i++)pattern[i] = '\0';
					
					if (!partialFormBuffer || !rsArray){
						// Если слово не нашли, т.е текущее больше искомого, то нужно выходить  
						if(CSldCompare::StrCmpA((UInt8 *)word, (UInt8 *)pattern) <  0)
						{
							return false;
						}

						// успешно нашли слово. 
						if(CSldCompare::StrCmpA((UInt8 *)word, (UInt8 *)pattern) == 0) break; 
					}
					else{ // Если нас интересует частично совпадающее слово
							
							// Ничего не нашлось (проскочили нужные буквы), выходим
						if (StrNCMP((UInt8*)word, (UInt8*)pattern, CSldCompare::StrLenA((const UInt8 *)pattern)) < 0) return false;
						
							// Начала слов совпадают, проверим остальные условия
						if (StrNCMP((UInt8*)word, (UInt8*)pattern, CSldCompare::StrLenA((const UInt8 *)pattern)) == 0){
								// Проверяем по очереди рулесеты: имеется ли нужный?
							Int32 success = 0;
							for (success = 0, i = 0; i < rsArrSize && !success; i++){
							
									// Если имеется, то проверяем, подходит ли данное слово под правило
								success = 1;
							}
							if (success){
								CSldCompare::StrNCopyA((UInt8*)partialFormBuffer, (UInt8*)pattern, bufSize); 
								break;
							}
						}
					}


					WordOffset++;
					letterindex = c; 
					RuleSetIndex = 0;
				}
				else // если цифра, но не первая, значит будем вычислять/накапливать индекс начала правил
				{
					Int32 multiplier = c;
					for(Int32 j = digits_for_rule; j > digitcount; j--) multiplier *= 10;
					RuleSetIndex += multiplier;
				}
				// количество цифр в текущем наборе. 
				digitcount++;
				
				// если количество цифр зашкалило, то это либо признак конца списка слов (5 нулей подряд)
				// либо где-то закралась ошибка. 
				if(digitcount>digits_for_rule+1)
				{
					return false;
				}
			}

	}

	// получаем ссылку на первое правило из группы (если мы успешно нашли слово)
	iter.ptr = &(RuleSetArray[RuleSetIndex]);
	iter.first = true;

	return WordOffset>=0?true:false; 	

#endif
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Автор: Павел Морозов. Октябрь 2007
// Получение всех базовых форм всех слов морфологического модуля с правилами по которым они изменяются
// EnumWords - вызывается извне, EnumWordsCallBack и WalkRuleSet используется внутри
///////////////////////////////////////////////////////////////////////////////////////////////////
bool WordSet_v1::EnumWords(EnumWordsCallBackT* callback, void* param) const
{
#ifdef _ML_PARSER_
	return true;
/*	std::string s(word);

	iter.itr = w.find(s);
	
	if( iter.itr != w.end() ) 
	{
		iter.inflruleset_iter = iter.itr->second.begin();
	}
	else iter.itr = NULL;

	return ( iter.itr != w.end() )?true:false;
*/
#else

//	Int32				WordIndex = 0; 
	Int32				WordOffset = -1; 
//	UInt8		t[6];
	Int32				bit = 0; 
	Int32				leftborder = 0;
	Int32				rightborder = IndexTableSize/8-1; 
//	Int32 				prev_bit = 0;
//	Int32				prev_symb_bit = 0;
//	Int32				prev_word_bit = 0;
	Int32				prev_word_len = MAX_WORD_LEN;
	Int32				letterindex = 0;
	UInt8		pattern[MAX_WORD_LEN];
	Int32				i;
	UInt8		c = 0;
	Int32				current_bit;
	Int32				digitcount = 0; 
	Int32				RuleSetIndex = 0; 
	UInt8		digits_for_rule = 0;

	UInt8		CurrentBlock;
	UInt8		tempChar;

	// вычисление длины правила (десятичный логарифм)
	for(Int32 mult =  RuleSetArraySize/sizeof(STAT_VECTOR_SIZE_V1) ; mult; digits_for_rule++, mult /= 10);
	
	sldMemZero(pattern, MAX_WORD_LEN);
	
	// вычисляем номер бита, с которого начинается блок, в котором может находится искомое слово
	bit = 0;


	// вычисляем количество слов во всех блоках вплоть до предшествующего тому, 
	// в котором будем искать текущее слово. 
/*	Это действие не нужно - далее WordIndex нигде не используется!!!
	for(i = 0, WordIndex = 0; i<leftborder+1; i++)
	{
		WordIndex+=MORPHO_SWAP_32(IndexArray[i].bit)>>24;
	}
*/	
	// вычисляем номер мета-блока, т.е. единицы, на которые разбивается архив для хранения 
	// в prc-ресурсах
	CurrentBlock = (UInt8)(bit/(8*DEFAULT_BLOCK_SIZE));

	// если номер мета-блока превысил их количество - где-то косяк
	if(CurrentBlock>=ArchiveBlockCounter)
	{
		return false;
	}


	// это вспомогательный указатель для доступа к данным типа BlockType, 
	// без него типы нихера не приводятся. 
	const UInt8* blockPtr = ArchiveBlockArray[CurrentBlock].ptr();
	UInt32 blockSize = ArchiveBlockArray[CurrentBlock].size();


	while(1)// 
	{
			leftborder = 0; 
			rightborder = HaffmanTableSize-1;

			// побитовое раскодирование одного символа - 
			while(leftborder<rightborder)
			{
				// получаем байт, в котором содержится бит с нужным номером
				tempChar = blockPtr[bit>>3];

				// сдвигаем байт, пока искомый бит не станет младшим, после чего обнуляем
				// остальные биты по маске. 
				current_bit = (tempChar>>(bit&0x07))&0x01;

				// если соотв. бит установлен - идем по дереву Хаффмана влево
				if(current_bit==1) 
					leftborder = HaffmanTable[leftborder];
				else // иначе идем вправо
				{
					rightborder = HaffmanTable[leftborder]-1;
					leftborder++;
				}
				bit++;

				// если номер бита стал больше, чем количество бит в одном мета-блоке - перейдем к следующему мета-блоку.  
				if(bit >= 8*blockSize)
				{
					bit -= 8*blockSize;
					CurrentBlock++;
					if(CurrentBlock>=ArchiveBlockCounter)
					{
						return false;
					}
					blockPtr = ArchiveBlockArray[CurrentBlock].ptr();
					blockSize = ArchiveBlockArray[CurrentBlock].size();

				}
			}
			// получаем раскодирвоанный символ по его индексу в таблице Хаффмана. 
			c = HaffmanTable[leftborder];
			
			// если это буква 
			if(c>9) 
			{
				pattern[letterindex++] = c;
				digitcount = 0; 
			}
			else // иначе это цифра. Первая цифра - это длина префикса, 
				//вторая и далее до первой буквы - номер в таблице правил, 
				// с которого начинаются правила для данного слова. 
			{
				if(digitcount == 0)// цифра первая, значит длина префикса
				{
					// если в конце слова есть мусор - уберем его. 
					for(i = letterindex; i < prev_word_len; i++)pattern[i] = '\0';
					
					//Int32 stop = 0;
					WordRuleSetsIterator iter;
					iter.ptr = &(RuleSetArray[RuleSetIndex]);
					iter.first = true;
					if(!callback((MCHAR*)pattern, iter, param))
						return false;


					WordOffset++;
					letterindex = c; 
					RuleSetIndex = 0;
				}
				else // если цифра, но не первая, значит будем вычислять/накапливать идекс начала правил
				{
					Int32 multiplier = c;
					for(Int32 j = digits_for_rule; j > digitcount; j--) multiplier *= 10;
					RuleSetIndex += multiplier;
				}
				// количество цифр в текущем наборе. 
				digitcount++;
				
				// если количество цифр зашкалило, то это либо признак конца списка слов (5 нулей подряд)
				// либо где-то закралась ошибка. 
				if(digitcount>digits_for_rule+1)
				{
					return false;
				}
			}

	}

	// получаем ссылку на первое правило из группы (если мы успешно нашли слово)
//	iter.ptr = &(RuleSetArray[RuleSetIndex]);
//	iter.first = true;

	return WordOffset>=0?true:false; 	

#endif

}

///////////////////////////////////////////////////////////////////////////////////
///
/// public-функция Insert. Добавляет к содержимому класса слово вместе с соотв. группой правил
///
///
///////////////////////////////////////////////////////////////////////////////////


#ifdef _ML_PARSER_
bool WordSet_v1::Insert(const MCHAR *word,  MorphoInflectionRulesSet_v1* Mo)
{
	std::string s(word);

	std::map<std::string,std::vector <MorphoInflectionRulesSet_v1 *> > ::iterator itr;

	itr = w.find( s ); 

	if(itr != w.end() )
		itr->second.push_back(Mo);
	else 
	{
		std::vector <MorphoInflectionRulesSet_v1 *> a;
		a.push_back(Mo);
		w.insert( w.end(), make_pair(s, a));
	}

	return ( itr != w.end() )?true:false;

}
#endif


///////////////////////////////////////////////////////////////////////////////////
///
/// public-функция GetNextRuleSet. Возвращает указатель на группу правил для слова. В качестве параметра передается  
///				итератор, который был возвращен функцией SearchWord.
///
///
///////////////////////////////////////////////////////////////////////////////////
#ifdef _ML_PARSER_
	const MorphoInflectionRulesSet_v1* WordSet_v1::GetNextRuleSet(WordRuleSetsIterator &iter)const
	{
		if( iter.inflruleset_iter != iter.itr->second.end() )
		{
			// если вызывают первый раз, то нужно вернуть значение текущее значение
			// и оставить указатель на месте. При последующих вызовах указатель можно
			// перемещать на следующие элементы.
			if(!iter.first)
			{
				iter.inflruleset_iter++;
			}
/*			else
			{
				iter.first = false;
			}
*/
			return *(iter.inflruleset_iter);
		}
		else 
			return NULL;
	}
#else
const MorphoInflectionRulesSet_v1* WordSet_v1::GetNextRuleSet(WordRuleSetsIterator &iter)const
	{
		// работа с файлом
		if(iter.ptr == NULL) return 0;

		const STAT_VECTOR_SIZE_V1 *temp = iter.ptr;
		// если вызывают первый раз, то нужно вернуть значение перед указателем
		// с последним битом, установленным в 0, и оставить указатель на месте. 
		if(iter.first)
			iter.first = false;
		// иначе
		else 
		{
			if( (*temp) & MORPHO_SWAP_16(0x0001))
			{
				iter.ptr = NULL;
				return 0;
			}
			iter.ptr++;
		}
		temp = iter.ptr;

		// сбрасываем последний бит - он у нас означает, что данная ссылка указывает на первое правило в группе. 
		
		// Савельев: пока все затачивается под 4-х байтные адреса - MORPHO_SWAP_32 вместо MORPHO_SWAP_16
		// offset типа STAT_VECTOR_SIZE вместо UInt16
		//UInt16 offset = ( (*temp) & MORPHO_SWAP_16(0xFFFE) );
		STAT_VECTOR_SIZE_V1 offset = ( (*temp) & MORPHO_SWAP_16(0xFFFE) );

		return MorphoPtr->ObjRef2Ptr( *(OBJREF_V1<const MorphoInflectionRulesSet_v1> *)&offset );

	}
#endif
