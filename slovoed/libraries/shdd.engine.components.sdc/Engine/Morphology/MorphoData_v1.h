#ifndef __MORPHO_DATA_INCLUDE_V1__
#define __MORPHO_DATA_INCLUDE_V1__

///////////////////////////////////////////////////////////////////////////////////
///
/// @file MorphoData.h
///
/// Copyright Paragon Software (SHDD), 2004
///
/// Файл описывает весь интефейс ядра морфологии, в частности класс MorphoData,
/// а так же все вспомогательные структуры и макросы.
///
/// Прииспользовании ядра морфологии в своей программе, достаточно подключить
/// этот файл, остальные необходимые .h-файлы будут подключены автоматически 
/// из него.
///
/// \author Исаев Андрей.
///
///////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////
///
/// \if MORHO_DOCUMENTATION
/// \mainpage Интерфейс ядра морологии для семейства продуктов "СловоЕд".
///
/// \author Исаев Андрей
///
///////////////////////////////////////////////////////////////////////////////////
/// <hr> \section sect1 Назначение модуля морфологии.
///
/// Модуль морфологии разрабатывался как компонента для включения в в состав 
/// продуктов семейства "СловоЕд" компании Paragon Software.
/// 
/// Модуль морфологии состоит из двух компонент:
///  - базы данных, содержащие информацию о словах одного языка и их формах
///    (\b морфологические \b базы \b данных),
///  - программное \b ядро, обеспечивающее возможность работы с базами данных.
///
/// Модуль морфологии решает две основные задачи в рамках одного языка: 
///  - \ref baseform "получение базовой формы слова" (как это слово пишут 
///    в словарях) из его произвольной формы (или \b лемматизация), 
///  - \ref paradigm "получение всех форм слова" из его базовой формы 
///    (или \b парадигмы слова).
///
/// В качестве дополнительной возможности, модуль морфологии можно использовать 
/// для \ref spellcheck "проверки правописания слов" языка (в качестве спеллчекера), 
/// так как он хранит информацию о большинстве слов языка, включая все их формы 
/// написания.
///
/// В будущем, возможно будет добавлена возможность получения нескольких вариантов
/// верного написания слова, если само слово написано с ошибкой, наподобии того,
/// как Microsoft Word предлагает варианты для слова с ошибкой.
///
///////////////////////////////////////////////////////////////////////////////////
/// <hr> \section usage Использование
///
/// Для того, чтобы подключить к программе ядро морфологии, необходимо включить
/// в состав программы следующие файлы:
///  - LastCharMap.cpp
///  - MorphoData.cpp
///  - MorphoData.h
///  - WordSet.cpp
///  - WordSet.h
///
/// Интерфейсом ядра морфологии является класс MorphoData. Он предоставляет
/// все необходимые методы для работы с морфологической базой данных. 
/// Для использования этого класса необходимо подключить хедер MorphoData.h
/// с описанием класса командой \b "#include".
///
///////////////////////////////////////////////////////////////////////////////////
/// \subsection initi Инициализация
/// @see MorphoData::Init()
///
///////////////////////////////////////////////////////////////////////////////////
/// \subsection baseform Получение базовой формы слова по произвольной форме
///
/// \par Базовая форма слова - определение.
/// Базовой формой слова называется та форма слова, в которой это слово обычно пишут 
/// в словарях. Например, в русском языке для существительных, это именительный падеж
/// единственного числа. Для прилагательных, это мужской род, именительный падеж,
/// единственное число. В лингвистике базовую форму иногда называют \b леммой, а процеcc
/// её получения из произвольной формы, соответсвенно, \b лемматизацией.
/// 
/// Важно понимать, что у одного слова может быть несколько базовых форм. Точнее 
/// говоря, не у слова, а у последовательности символов, так как с точки зрения
/// лингвистики, у слова может быть лишь одна базовая форма, но разные слова могут 
/// писаться одинаково (например, омонимы). Пример: \c "какая" - \c "какой" (прилагательное),
/// \c "какой" (местоимение), \c "какать" (глагол).
/// 
/// Ядро морфологии восстановит все известные ему варианты базовых форм для
/// данной последовательности символов.
///
/// @see MorphoData::BaseFormsIteratorInit() \n
///		 MorphoData::GetNextBaseForm() \n
///      MorphoData::WritingVersionIteratorInit() \n
///      MorphoData::GetNextWritingVersion() \n
///      MorphoData::Init() \n
///
///////////////////////////////////////////////////////////////////////////////////
/// \subsection paradigm Получение всех форм слова из его базовой формы
///
/// @see MorphoData::WordFormsIteratorInit() \n
///      MorphoData::GetNextWordForm() \n
///      MorphoData::Init() \n
///
///////////////////////////////////////////////////////////////////////////////////
/// \subsection spellcheck Проверка правописания
///
/// В ядро морфологии не входит какой-либо специальной функциональности,
/// связаной с проверкой правописания. Однако, эта возможность является
/// прямым следствием наличия функциональности восстановления базовой формы
/// слова из ее произвольной. Таким образом, если мы не сможем восстановить
/// исходную форму слова для данного слова, значит, это слово не является
/// формой ни одного из хранимых в базе данных слов. Все спелчекеры работают
/// абсолютно аналогичным образом: ищут форму слова в своей базе данных.
///
/// Важно только отметить, что интерфейс функции MorphoData::WritingVersionIteratorInit()
/// спроектирован таким образом, что он очень удобен для разбора текстовых строк
/// и последовательного вычленения из них слов. Получив из строки слово и 
/// распознав его, необходимо сдвинуть указатель на длину распознанного слова
/// и пропустить разделители (пробелы, запятые и т.д.) за словом, используя
/// функцию MorphoData::LetterAttributes().
///
/// @see MorphoData::BaseFormsIteratorInit() \n
///		 MorphoData::GetNextBaseForm() \n
///      MorphoData::WritingVersionIteratorInit() \n
///      MorphoData::GetNextWritingVersion() \n
///      MorphoData::LetterAttributes() \n
///      MorphoData::Init() \n
///
///////////////////////////////////////////////////////////////////////////////////
/// \subsection classname Получение названия части речи для слова
/// Для того, чтобы получить название части речи, необходимо получить все базовые 
/// формы слова (даже, если оно и было в базовой форме). При этом, фунция
/// MorphoData::GetNextBaseForm() будет не только возвращать не только 
/// форму слова, но и указатель на набор правил изменения этого слова.
/// Передав, этот указатель в функцию MorphoData::ClassNameByRulesetPtr(),
/// можно получить указатель на название части речи, ассоциированное с этим
/// набором правил изменния слова.
///
/// @see MorphoData::ClassNameByRulesetPtr() \n
///      \ref baseform "Получение базовой формы слова".
///      
/// \subsection letters Работа с символами языка
///
/// @see MorphoData::LetterAttributes() \n
///      MorphoData::ToLower() \n
///      MorphoData::ToUpper() \n
///
///////////////////////////////////////////////////////////////////////////////////
/// <hr> \section  theory Структура морфологических данных
///
/// TO DO...
///
/// \endif
///
///////////////////////////////////////////////////////////////////////////////////

//#define  _ML_PARSER_ - should be defined in project settings of mlParser

//////////////////////////////////////////////////////////////////////
//
//	определяем стандартные функции операционной системы
//
//////////////////////////////////////////////////////////////////////

//#include <stdlib.h>
//#include <string.h>
//#include <crtdbg.h>

// подключаем прослойку от ядра СловоЕда.
#include "../SldCompare.h"
#include "LanguageSpecific_v1.h"
#include "IMorphoData.h"
#include "../ISldLayerAccess.h"

/// Тип данных, используемый для адресации
/// данных внутри морфологической базы данных.
/// Мы используем два байта, поэтому, все данные
/// собраны в сегменты по 64 килобайта.
typedef UInt16 STAT_VECTOR_SIZE_V1;

#define M_ASSERT(x)					

#ifdef _ML_PARSER_
	/// используем обычный vector
	#include <map> /// TO DO.. remove it..
	#include <vector>
	///	Наш внутренний базовый внутренний тип - линейный контейнер элементов
	#define MVECTOR_V1	std::vector
#else

	//////////////////////////////////////////////////////////////////////
	///
	/// @brief Реализация шаблона, аналогичного vector
	///
	///	Шаблон нашего vector, который работает как интерфейс
	/// к статическим данным, сохраненным в файле.
	/// Является аналогом vector из STL.
	///
	//////////////////////////////////////////////////////////////////////
	template <typename _DATA> class stat_vector_v1
	{
	private:

		/// private data (const only)

		const STAT_VECTOR_SIZE_V1	bsize;		/** размер массива в байтах */
		const _DATA					array[1];	/** массив элементов начинается тут */

	public:

		stat_vector_v1()
		{
			bsize = 0;		/** размер массива в байтах */
//			sldMemZero(array, 0, sizeof(array));
		}

		///////////////////////////////////////////
		///
		/// @brief Константный итератор
		///
		/// Итератор для доступа к элементам вектора.
		///
		///////////////////////////////////////////
		class const_iterator
		{
		private:
			/// poninter to const data
			const _DATA   *ptr;

		public:

			/// Default constructor. Does nothing.
			inline const_iterator() {};

			/// Copy constructor
			inline const_iterator(const _DATA *p) { ptr = p; };

			/// Conversion to a pointer
			inline operator const _DATA *() const { return ptr; };

			/// Return reference of class object
			inline const _DATA * operator->() const { return ptr; };

			/// Return reference of class object
			inline const _DATA & operator*() const { return *ptr; };

			/// Increment pointer (move to next array item)
			inline const_iterator& operator++(int/*to make this increment PSTFIX */) { ptr++; return *this; };
		};

		/// standard vector functions

		/// Возвращает количество элементов в массиве
		inline STAT_VECTOR_SIZE_V1 size(void) const { return (MORPHO_SWAP_16(bsize)/sizeof(_DATA)); };

		/// Возвращает константный итератор, указывающий на первый элемент.
		inline const_iterator begin(void) const { return const_iterator(array); };

		/// Возвращает константный итератор, указывающий на конец массива (за последним элементом).
		inline const_iterator end(void)   const { return (const_iterator((_DATA*)((char*)array + MORPHO_SWAP_16(bsize)))); };

		/// Возвращает элемент массива по инексу. @attention Не проверяет на предмет выхода за границы.
		inline const _DATA & operator[](STAT_VECTOR_SIZE_V1 index) const { return array[index]; };

		/// additional function

		/// Возвращает размер массива в байтах.
		inline STAT_VECTOR_SIZE_V1 size_in_bytes(void) const { return (MORPHO_SWAP_16(bsize) + sizeof(STAT_VECTOR_SIZE_V1)); };

	};

	///	Наш внутренний базовый внутренний тип - линейный контейнер элементов
	#define MVECTOR_V1 stat_vector_v1
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//========================================================================================================================================//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/// forward declaration
struct MorphoData_v1;

//////////////////////////////////////////////////////////////////////
///
/// @brief Ссылка на текстовую строку.
///
///	Базовый внутренний тип, ссылка на текстовую строку внутри
/// морфологической базы данных.
///
//////////////////////////////////////////////////////////////////////
class MCHARREF_V1
{
	friend struct MorphoData_v1;

#ifdef _ML_PARSER_
	
	///////////////////////////////////////////////////////
	///
	/// mlParser version
	///
private:

	const MCHAR* ptr;

public:
	/// Вefault constuctor
	inline MCHARREF_V1() { ptr = NULL; };
	
	/// Copy constructor
	inline void init(const MCHAR* strPtr) { ptr = strPtr; };

	/// Save to database
	void Save(FILE * out, const char *filename) const;
	
	const MCHAR* GetDataPtr() const {return ptr;}

	/// сравнение (по памяти) - для того чтобы можно было исплользовать контейнер set для рулесетов
	inline bool operator<(const MCHARREF_V1& other) const { return (ptr < other.ptr) ? true : false ; };
#else
	///////////////////////////////////////////////////////
	//
	/// core version
	//
private: 

	/// Двухбайтная ссылка внутри файла
	const STAT_VECTOR_SIZE_V1 shift;

	/// Конструктор по умолчанию запрещен.
	MCHARREF_V1();

#endif
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//========================================================================================================================================//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
///
/// @brief Ссылка на объект/структуру
///
///	Базовый внутренний тип , ссылка на объект внутри
/// морфологической базы данных. Может указывать на объект
/// в базе данных, либо на \b NULL.
///
//////////////////////////////////////////////////////////////////////
template <typename _TYPE> class OBJREF_V1
{
	/// Чтобы разрешить доступ к внутренним данным для фукции MorphoData::ObjRef2Ptr
	friend struct MorphoData_v1;
	friend struct MorphoClass_v1;

private:

	/// тайпдеф для автоматизации преобразования к базовому классу через шаблон MorphoData::ObjRef2Ptr
	typedef _TYPE _basetype;

#ifdef _ML_PARSER_
	///////////////////////////////////////////////////////
	//
	/// mlParser version
	//
private: 
	const _TYPE *ptr;

public:
	/// default constuctor
	inline OBJREF_V1() { ptr = NULL; };

	/// other constuctor
	inline OBJREF_V1(const _TYPE* strPtr) { ptr = strPtr; };

	/// initialisation
	inline void init(const _TYPE* strPtr) { ptr = strPtr; };

	/// сохранение этого типа данных
	inline void Save(FILE * out, const char *filename) const { WriteItemReference2b(ptr, ftell(out), out, filename); }

	/// проверка на ноль не требует преобразования к указателю
	/// на базовый тип
	inline bool isNULL() const { return ptr ? false : true; };

	/// сравнение другой ссылкой этого же типа не требует преобразования к указателю
	/// на базовый тип
	inline bool operator==(const OBJREF_V1& other) const { return (ptr == other.ptr) ? true : false ; };
	inline bool operator!=(const OBJREF_V1& other) const { return (ptr != other.ptr) ? true : false ; };

	/// сравнение (по памяти) - для того чтобы можно было исплользовать контейнер set для рулесетов
	inline bool operator<(const OBJREF_V1& other) const { return (ptr < other.ptr) ? true : false ; };

#else
	///////////////////////////////////////////////////////
	//
	/// core version
	//

private: 
	/// Двухбайтная ссылка внутри файла
	const STAT_VECTOR_SIZE_V1 shift;

	/// default constuctor is disabled
	OBJREF_V1();

public:

	/// Проверка на \b NULL без преобразования к указателю на базовый тип
	inline bool isNULL() const { return shift ? false : true; };
	/// Сравнение другой ссылкой этого же типа без преобразования к указателю на базовый тип
	inline bool operator==(OBJREF_V1& other) { return (shift == other.shift) ? true : false ; };
	/// Сравнение другой ссылкой этого же типа без преобразования к указателю на базовый тип
	inline bool operator!=(OBJREF_V1& other) { return (shift != other.shift) ? true : false ; };

#endif
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//========================================================================================================================================//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
///	Morphological types
//////////////////////////////////////////////////////////////////////

/// forward declarations
struct MorphoInflectionRule_v1;
struct MorphoInflectionRulesSet_v1;
struct MorphoStateDescription_v1;
struct MorphoStateDescriptionTable_v1;
class  RulesByLastChar_v1;


#include "WordSet_v1.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//========================================================================================================================================//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////
///
/// Структура, хранимая в морфологической базе данных,
/// описывающая одно состояние слова, наборы которых
/// задаются директивой 'tabe' в ml-языке.
///
//////////////////////////////////////////////////////
struct MorphoStateDescription_v1
{
	//////////////////////////////////////////
	//
	/// собственно, данные
	//
	//////////////////////////////////////////

	/// Description for word state
	MCHARREF_V1 description;

	/// Additional info for this rule (set only if table=\b NULL)
	MCHARREF_V1 templ;

	/// Additional info for this rule (set only if table=\b NULL)
	MCHARREF_V1 question;

	/// Link to other table
	OBJREF_V1 <const MorphoStateDescriptionTable_v1> table;

	//////////////////////////////////////////
	//
	/// Полезные функции
	//
	//////////////////////////////////////////

	/// Это конечное состояние или это ссылка на подтаблицу состояний?
	inline bool HasSubtable (void) const { return ( !table.isNULL() ); };

#ifdef _ML_PARSER_
	void Save(FILE * out, const char *filename) const;
#endif

};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//========================================================================================================================================//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////
///
/// Структура, хранимая в морфологической базе данных,
/// описывающая массив состояний слова, который
/// задаются директивой 'tabe' в ml-языке.
///
//////////////////////////////////////////////////////
struct MorphoStateDescriptionTable_v1
{
	//////////////////////////////////////////
	//
	/// собственно, данные
	//
	//////////////////////////////////////////

	/// Vector of inflection states for this table
	typedef MVECTOR_V1 <MorphoStateDescription_v1> StatesList;
	StatesList states;

	//////////////////////////////////////////
	//
	/// Полезные функции
	//
	//////////////////////////////////////////

	/// Итератор
	typedef StatesList::const_iterator StatesIterator;

	/// Вернуть начало списка состояний
	inline StatesIterator StatesBegin(void) const { return states.begin(); };

	/// Вернуть конец списка состояний
	inline StatesIterator StatesEnd(void) const { return states.end(); };

	/// Вернуть количество состояний
	inline Int32 StatesNumber(void) const { return (Int32)states.size(); };

	/// Доступ к состоянию по индексу
	inline const MorphoStateDescription_v1& StateByIndex (Int32 index) const { return states[index]; };

#ifdef _ML_PARSER_
	Int32 depth;
	void Save(FILE * out, const char *filename) const;
#endif
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//========================================================================================================================================//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////
///
/// Структура, хранимая в морфологической базе данных,
/// описывающая правило изменения для одной формы слова, 
/// наборы которых 'rule' в ml-языке.
///
/// Данные организованы таким образом,
/// что структура либо показывает на подтаблицу (тогда
/// указатель на правило больше не имеет смысла)
/// Либо подтаблицы нет, то она обязательно указывает
/// на текстовую строку, описывающую правило.
///
/// Но она не молжет не указывать никуда. В случае
/// пустого правила, она указывает на пустую строку
/// (содержащую только 0)
///
//////////////////////////////////////////////////////
struct MorphoInflectionRule_v1
{
	//////////////////////////////////////////
	//
	/// собственно, данные
	//
	//////////////////////////////////////////
#ifdef _ML_PARSER_
	/// указатель в памяти
	const void *ptr;
	/// на что же он указывает
	bool isLinkToRuleset;
	/** является ли это правило ссылкой на поднабор правил */
	inline bool IsLinkToRuleset(void) const { return isLinkToRuleset; };
#else

	/// Двухбайтная ссылка внутри файла.
	const STAT_VECTOR_SIZE_V1 shift;
	
	/// На что указывает shift мы определяем по последнему биту.
	/// Все струкутры выравнены по четным адресам, а 
	/// строки - по нечетным.
	//
	/// Является ли это правило ссылкой на поднабор правил?
	inline bool IsLinkToRuleset(void) const { return (MORPHO_SWAP_16(shift)&1) ? false: true; };
#endif

	//////////////////////////////////////////
	//
	/// разные функции
	//
	//////////////////////////////////////////

#ifdef _ML_PARSER_
	void Save(FILE * out, const char *filename) const;

	/// оператор сравнения для того, чтобы отсекать повторяющиеся элементы
	/// при хранении
	bool operator < (const struct MorphoInflectionRule_v1 &other) const
	{
		if ( !isLinkToRuleset && other.isLinkToRuleset )
			return true;

		if ( isLinkToRuleset && !other.isLinkToRuleset  )
			return false;

		return( ptr < other.ptr ) ? true : false;
	}
#endif
/*
public:
	/// Конструктор.
	MorphoInflectionRule_v1::MorphoInflectionRule_v1()
	{
		MorphoInflectionRule_v1::shift = 0;
	}
*/
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//========================================================================================================================================//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////
///
/// Структура, хранимая в морфологической базе данных,
/// описывающая массив правил изменения для слова, 
/// который задаётся директивой 'rule' в ml-языке.
///
//////////////////////////////////////////////////////
struct MorphoInflectionRulesSet_v1
{

	//////////////////////////////////////////
	//
	/// собственно, данные
	//
	//////////////////////////////////////////

	/// Предусловие для всех правил набора
	MCHARREF_V1	precondition;

	/// Набор правил
	typedef MVECTOR_V1 <MorphoInflectionRule_v1> RulesList;
	RulesList rules;

	//////////////////////////////////////////
	//
	/// Полезные функции
	//
	//////////////////////////////////////////

	/// Итератор
	typedef RulesList::const_iterator RulesIterator;

	/// Вернуть начало списка правил
	inline RulesIterator RulesBegin(void) const { return rules.begin(); };

	/// Вернуть конец списка правил
	inline RulesIterator RulesEnd(void) const { return rules.end(); };

	//////////////////////////////////////////
	//
	/// Та часть, которая используется 
	/// только в парсере, но не используется
	/// в ядре.
	//
	//////////////////////////////////////////
	#ifdef _ML_PARSER_

		/// различная внутренняя статистика для mlParser
		Int32	mistake1;
		Int32 mistake2;
		Int32 correct;

		bool referenced;

		/// сохранение
		void Save(FILE * out, const char *filename) const;

		/// ссылка на базовую таблицу (для контроля типов в mlParser)
		const MorphoStateDescriptionTable_v1* table;

		/// точно такой же рулесет, но, находящийся в set (для того, чтобы убедиться в уникальности)
		MorphoInflectionRulesSet_v1 *shadow;

		/// оператор сравнения для того, чтобы отсекать повторяющиеся элементы
		/// при хранении
		bool operator < (const struct MorphoInflectionRulesSet_v1 &other) const
		{
			if ( precondition < other.precondition )
				return true;

			if ( other.precondition < precondition )
				return false;

			if ( rules.size() < other.rules.size() )
				return true;

			if ( rules.size() > other.rules.size() )
				return false;

			for (UInt32 i=0; i < rules.size(); i++ )
			{
				if ( rules[i] < other.rules[i] )
					return true;

				if ( other.rules[i] < rules[i] )
					return false;
			}

			return false;
		}
	#endif
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//========================================================================================================================================//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////
///
/// Структура, описывающая класс слов языка (название
/// класса и набор ссылок на массив правил изменения).
///
/// Задаётся директивой 'class' в ml-языке.
///
//////////////////////////////////////////////////////
struct MorphoClass_v1
{
	//////////////////////////////////////////
	//
	/// собственно, данные
	//
	//////////////////////////////////////////

	/// Название этого класса ("Предлог", "Наречие"...)
	MCHARREF_V1 name;

	/// Указатель на таблицу форм этого класса
	OBJREF_V1 <const MorphoStateDescriptionTable_v1> table;

	typedef MVECTOR_V1 < const OBJREF_V1<MorphoInflectionRulesSet_v1> > InflTablesList;

	/// Массив ссылок на MorphoInflectionRulesSet
	InflTablesList iflectionTables;

	//////////////////////////////////////////
	//
	/// Полезные функции
	//
	//////////////////////////////////////////

	/// Итератор
	typedef InflTablesList::const_iterator InflTablesIterator;

	/// Вернуть начало массива ссылок на правила
	inline InflTablesIterator InflTablesBegin(void) const { return iflectionTables.begin(); };

	/// Вернуть конец массива ссылок на правила
	inline InflTablesIterator InflTablesEnd(void) const { return iflectionTables.end(); };

#ifdef _ML_PARSER_
	void Save(FILE * out, const char *filename) const;
#endif
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//========================================================================================================================================//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
///
/// Класс для хранения и быстрого доступа к правилам изменения слов.
/// Используется для ускорения процедуры восстановления исходной
/// формы слова.
///
/// Ускорение достигается за счет того, что мы пытаемся применить
/// для восстановления исходной формы слова только те правила,
/// окончания которых либо совпадают с данным словом, либо пустые.
///
//////////////////////////////////////////////////////////////////////
class RulesByLastChar_v1
{
	friend struct MorphoData_v1;
public:

	//////////////////////////////////////////////////////////////////////
	///
	/// Структура, хрянящая все необходимое, чтобы восстановить исходную 
	/// форму слова. 
	///
	//////////////////////////////////////////////////////////////////////
#ifdef _ML_PARSER_
	struct OneCallParameters
	{
		const MCHAR*  precondition;				/// ссылка на предусловие
		const MCHAR*  rule;						/// ссылка на правило

		MCHAR	      pre_last;					/// предпоследний символ
		UInt8 rule_len;				    /// количество символов в правиле (чтобы не считать каждый раз заново)
		UInt8 onlyOneVariant;			/// есть ли в правиле варианты (чтобы каждый раз не считать)
		UInt8 matchPrevious;			/// совпадает ли это правило с предыдущим правилом?

		const MorphoInflectionRulesSet_v1* ruleset; /// ссылка на сам набор правил, откуда это все взято
	};
#else
	struct OneCallParameters
	{
		MCHARREF_V1	precondition;				/// ссылка на предусловие
		MCHARREF_V1	rule;						/// ссылка на правило

		MCHAR			pre_last;					/// предпоследний символ
		UInt8	rule_len;				    /// количество символов в правиле (чтобы не считать каждый раз заново)
		UInt8	onlyOneVariant;			/// есть ли в правиле варианты (чтобы каждый раз не считать)
		UInt8	matchPrevious;			/// совпадает ли это правило с предыдущим правилом?

		OBJREF_V1<MorphoInflectionRulesSet_v1>	ruleset; /// ссылка на сам набор правил, откуда это все взято
	};
#endif


private:

	//////////////////////////////////////////
	//
	/// собственно, данные
	//
	//////////////////////////////////////////

	/// Карта, которая ставит соответствие последнему 
	/// символу слова набор структур OneCallParameters
	struct _charMapItem
	{
		/// индекс первой структуры для 
		/// этого символа в векторе
		UInt16 index;
	}
	lastCharMap [257];

	/// хранилище всех структур
	UInt32		callParams_allocated;
	UInt32		callParams_used;

	struct _callParamBlock
	{
		OneCallParameters	*callParams;
		UInt16		num;
	}callParams[MAX_WORDS_BLOCKS_NUMBER];
	UInt16 callParamsIdx;

	//////////////////////////////////////////////////////////////////////
	//
	/// внутренные методы - для заполнения
	//
	//////////////////////////////////////////////////////////////////////
#ifdef _ML_PARSER_

	/// add single sule
	void AddSingleRule(const MCHAR* precondition, const MCHAR* rule, const MorphoInflectionRulesSet_v1 *ruleset);

	/// вставляем одну структуру в вектор и корректируем таблицу
	void InsertOne(OneCallParameters *str, MCHAR lastchat, Int32 pos);
#endif
	//////////////////////////////////////////
	//
	/// То, что доступно снаружи
	//
	//////////////////////////////////////////

public:
#ifdef _ML_PARSER_
	/// Конструктор
	RulesByLastChar_v1();

	/// Деструктор
	~RulesByLastChar_v1();

	/// Добавить еще один набор правил
	void AddRuleset(const MorphoData_v1& morpho, const MorphoInflectionRulesSet_v1 *ruleset, const MorphoInflectionRulesSet_v1 *base_ruleset = 0);

	/// Сохраняет в файл индексы обратного преобразования.
	UInt32 Save(char *filename);

	/// Сохраняет в файл ссылки на правила в соответствии с последними буквами слов.
	UInt32 SaveLastChar(char *filename);
#endif

	/// Получить ссылку на первый подходящий вариант для восстановления формы слова
	const OneCallParameters* GetFirstRuleset(CallParamIterator &iter, MCHAR last_char) const;

	/// Получить ссылку на следующий подходящий вариант для восстановления формы слова
	const OneCallParameters* GetNextRuleset(CallParamIterator &iter) const;

	/// Добавляет очередной блок в список блоков обратного преобразования.
	void AddBlock(OneCallParameters *block, UInt16 count)
	{
		callParams[callParamsIdx].callParams = block;
		callParams[callParamsIdx].num = count;
		callParamsIdx++;
	}
	
	/// Возвращает интересующуй нас блок с данными для быстрого обратного преобразования.
	const OneCallParameters *GetParamByIndex(UInt16 index) const
	{
		UInt16 resIdx;
		UInt16 localIdx;

#ifdef _ML_PARSER_
		// При работе MLParser у нас нет разбиения таблицы быстрого обратного 
		// преобразования на части, поэтому все находится в одном ресурсе.
		resIdx = 0;
		localIdx = index;
#else
		
		// В одном ресурсе может содержаться максимум 4096 записей(размер записи - 10 байт)
		// Здесь мы вычисляем номер ресурса для указанного номера записи.
		// Эта запись аналогична resIdx = index / 4096;
		
		resIdx = index>>(10+2);
		
		// Здесь вычисляется номер записи внутри ранее вычисленного ресурса.
		localIdx = index & ( (1<<(10+2)) - 1);
		
		
		// Савельев: чтобы сделать все назад как было раньше, нужно раскомментировать предыдущий блок
		// комментариев /**/ и закомментировать этот {}.
		//{
			// Размер записи не обязательно 10 байт - вычислим его
			// не очень красивый способ, но пока так
			/*
			UInt16 rec_size = 0;
			RulesByLastChar::OneCallParameters* tmp_struct;
			rec_size+=sizeof(tmp_struct->pre_last);
			rec_size+=sizeof(tmp_struct->rule_len);
			rec_size+=sizeof(tmp_struct->onlyOneVariant);
			rec_size+=sizeof(tmp_struct->matchPrevious);
			// Еще 3 ссылки на 3 элемента структуры
			rec_size+=3*sizeof(STAT_VECTOR_SIZE);
			
			resIdx = index>>(rec_size+2);
			
			// Здесь вычисляется номер записи внутри ранее вычисленного ресурса.
			localIdx = index & ( (1<<(rec_size+2)) - 1);
			*/
			
			// 2048 - количество записей в одном ресурсе - на столько разбивается в парсере
			//resIdx = index/2048;
			
			// Здесь вычисляется номер записи внутри ранее вычисленного ресурса.
			//localIdx = index - (2048*resIdx);
		//}

		// Возвращаем указатель на данные.
#endif
		return callParams[resIdx].callParams+localIdx;
	}

	/// Возвращает количество элементов во внутреннем массиве.
	inline UInt32 CallParamsTableSize(void) const { return callParams_used; };

};

// Предварительное объявление класса, ответственного за работу
// с языкоспецифичными морфологическими фичами
//class LanguageSpecificData;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//========================================================================================================================================//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////
///
/// @brief Главный интерфейс для работы с морфологией.
///
/// Класс отвечает за всю работу с морфологической базой данных.
///
///////////////////////////////////////////////////////////////////////
struct MorphoData_v1 : public IMorphoData
{
#ifndef _ML_PARSER_

	// хендлеры загруженных ресусров
	CSDCReadMy::Resource	m_resHeader;
	CSDCReadMy::Resource	m_resText;
	CSDCReadMy::Resource	m_resClass;
	CSDCReadMy::Resource	m_resArchHeader;
	CSDCReadMy::Resource	m_resRules2;
	sld2::DynArray<UInt8>	m_resRulesSet;
	CSDCReadMy::Resource	m_resQAIndex;
	CSDCReadMy::Resource	m_resTree;
	CSDCReadMy::Resource	m_resTable;
	CSDCReadMy::Resource	m_resLastChar;
	CSDCReadMy::Resource	m_resWordSet[MAX_WORDS_BLOCKS_NUMBER];
	UInt16	m_resWordSetIdx;
	CSDCReadMy::Resource	m_resCharMap[MAX_WORDS_BLOCKS_NUMBER];
	UInt16	m_resCharMapIdx;
	
	/// Указатель на заголовок базы морфологии.
	MorphoDataHeader	*m_Header;

	/// Начало классов в файле морфологии.
	STAT_VECTOR_SIZE_V1 classes_begin;

	/// Конец классов в файле морфологии.
	STAT_VECTOR_SIZE_V1 classes_end;
	
	//
	// данные, загруженные из файла
	//
/*	/// Начало файла (сегмент 1)
	const void *    data;       
*/
	/// Начало текстовых строк (сегмент 2)
	//const void *    data_text;   

	/// Начало структур (сегмент 3)
	//const void *    data_struct; 

	/// Размер всего файла с данными 
	//Int32	            data_size;   

	/// Преробразование ссылки на таблицу в указатель
	inline const MorphoStateDescriptionTable_v1* ObjRef2Ptr(OBJREF_V1<const MorphoStateDescriptionTable_v1> ref) const { return (const MorphoStateDescriptionTable_v1*)( (char*)m_resTable.ptr() + MORPHO_SWAP_16(ref.shift) ); };

	/// Преробразование ссылки на набор правил в указатель
	inline const MorphoInflectionRulesSet_v1* ObjRef2Ptr(OBJREF_V1<const MorphoInflectionRulesSet_v1> ref) const { return (const MorphoInflectionRulesSet_v1*)( (char*)m_resRulesSet.data() + MORPHO_SWAP_16(ref.shift) ); };
	inline const MorphoInflectionRulesSet_v1* ObjRef2Ptr(OBJREF_V1<MorphoInflectionRulesSet_v1> ref) const { return (const MorphoInflectionRulesSet_v1*)( (char*)m_resRulesSet.data() + MORPHO_SWAP_16(ref.shift) ); };

	/// Преробразование ссылки на строку в указатель
	inline const MCHAR* McharRef2Ptr(MCHARREF_V1 ref) const { return (const MCHAR*)( (char*)m_resText.ptr() + MORPHO_SWAP_16(ref.shift) ); };

	/// Преобразование элемента таблицы правил к указателю на текстовое правило или \b NULL
	inline const MCHAR* Rule2MCharPtr(struct MorphoInflectionRule_v1 ruleref) const
	{ 
		M_ASSERT( ! ruleref.IsLinkToRuleset() );
		return (const MCHAR*)( (char*)m_resText.ptr() + MORPHO_SWAP_16(ruleref.shift) );
	};

	/// Преробразование элемента таблицы правил к указателю на поднабор правил или \b NULL
	inline const MorphoInflectionRulesSet_v1* Rule2RuleSetPtr(struct MorphoInflectionRule_v1 ruleref) const
	{
		M_ASSERT( ruleref.IsLinkToRuleset() );
		return (const MorphoInflectionRulesSet_v1*)( (char*)m_resRulesSet.data() + MORPHO_SWAP_16(ruleref.shift) );
	};

	/// Возвращает четырехбайтный код языка морфологической базы в формате СловоЕд и PiLoc ("russ", "engl", ...)
	ESldLanguage GetLanguageCode(void) const { return SldLanguage::fromCode(m_Header->language); };

#else

	MorphoData_v1()
	{
		this->languageCode = 0;
		this->DictID = 0;
		sldMemZero(FullLanguageName, sizeof(FullLanguageName), 0);
		sldMemZero(Author, sizeof(Author), 0);
		sldMemZero(WWWAuthor, sizeof(WWWAuthor), 0);

		character_attributes_table = NULL;
		to_upper_table = NULL;
		to_lower_table = NULL;
		
		MAX_POS_BUF_SIZE = 2048;
	};
	
	~MorphoData_v1()
	{
	};

	/// Хранилище классов
	MVECTOR_V1<MorphoClass_v1> classes;
	
	/// Код языка
	UInt32 languageCode;
	/// Код базы данных куда нужно сохранить языковую базу.
	UInt32 DictID;
	/// Полное имя языка
	char FullLanguageName[32];
	/// Имя автора
	char Author[32];
	/// сайт автора
	char WWWAuthor[32];

	/// преробразование ссылки на объект в указатель
	template<class REF> inline const typename REF::_basetype* ObjRef2Ptr(REF ref) const { return ref.ptr; };

	/// преобразование ссылки на строку в указатель
	inline const MCHAR* McharRef2Ptr(MCHARREF_V1 ref) const { return ref.ptr; };

	/// преобразование элемента таблицы правил к указателю на текстовое правило или \b NULL
	inline const MCHAR* Rule2MCharPtr(struct MorphoInflectionRule_v1 ruleref) const
	{ 
		M_ASSERT( ! ruleref.IsLinkToRuleset() );
		return (const MCHAR*)ruleref.ptr; 
	};

	/// преобразование элемента таблицы правил к указателю на поднабор правил или \b NULL
	inline const MorphoInflectionRulesSet_v1* Rule2RuleSetPtr(struct MorphoInflectionRule_v1 ruleref) const
	{
		M_ASSERT( ruleref.IsLinkToRuleset() );
		return (const MorphoInflectionRulesSet_v1*)ruleref.ptr;
	};

	/// код языка
	ESldLanguage GetLanguageCode(void) const { return SldLanguage::fromCode(languageCode); };

#endif
	
	/// В парсере означает максимальную длину предусловия.
	/// При использовании морфологии означает максимальную длину предусловия + 1 (берется из заголовка базы),
	/// что и является максимальным размером для внутреннего буфера позиций.
	UInt16	MAX_POS_BUF_SIZE;

	/////////////////////////////////////////
	//
	// данные
	//
	/////////////////////////////////////////

	/// Класс, отвечающий за ускоренное востановление
	/// базовой формы слова из произвольной.
	/// @see RulesByLastChar
	RulesByLastChar_v1 lastCharMap;

	/// Список всех слов морфологической базы данных с указанием их правил склонения.
	WordSet_v1			words;

public:
	
	LanguageSpecificData_v1 langSpecData;

	/// Инициализация ядра морфологии
	#ifndef _ML_PARSER_
		bool Init(CSDCReadMy &aMorphology);
		void	Close();
	#endif

	/////////////////////////////////////////////////////////////////////////////
	//
	// Частные функции работы с морфологическими данными
	// (которые не требуют ссылки на все морфологические данные)
	//
	/////////////////////////////////////////////////////////////////////////////


	///////////////////////////////////////////////////////////////////////////////////////////////////
	// Автор: Павел Морозов. Октябрь 2007
	// Получение всех базовых форм всех слов морфологического модуля 
	//						с правилами по которым они изменяются
	///////////////////////////////////////////////////////////////////////////////////////////////////
	bool Enum(bool bAllForms, EnumCallBackT* callback, void* param) const;
	
	//////////////////////////////////////////////////////////////////////////////////////////////////
	// Получение базовых форм морфологического модуля в опредленном диапазоне и их словоформ, а также
	// правил, по которым эти базовые формы изменяются
	///////////////////////////////////////////////////////////////////////////////////////////////////
	bool EnumPartial(EnumPartialCallBackT* callback, UInt32 aBaseFormIndexFirts, UInt32 aBaseFormIndexLast, void* aParam) const;

	
	/// Применить правило к слову
	/// В IMorphoData не реализована
	static const MCHAR*  InflectWord      (const MCHAR *word,      const MCHAR *precondition, const MCHAR *rule,  MCHAR *buf,   const MCHAR **pos);
	
	/// Подходит ли слово под это предусловие?
	/// В IMorphoData не реализована
	static Int32           ApplyPrecondition(const MCHAR *word,      const MCHAR **posBuf,      const MCHAR* precondition);

	/// Произвести обратное преобразование по правилу
	/// В IMorphoData не реализована
	static bool          RestoreWord      (const MCHAR *word_form, const MCHAR *precondition, const MCHAR *rule,  Int32 rule_len, MCHAR* restored, Int32 onlyOneVariant = false);

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Функции, которые используют полный набор морфологических данных
	//
	/////////////////////////////////////////////////////////////////////////////

	/// Применим ли этот набор правил к этому слову?
	bool     IsRuleApplyable  (const MCHAR *word, const MorphoInflectionRulesSetHandle* ruleset_ptr, const MCHAR **posBuf, Int32 flags) const;

	/// Флаг функции MorphoData::IsRuleApplyable().
	/// Проверить предусловие верхнего уровня.
	#define  M_RULE_TOPMOST_PRECONDITION			1
	/// Флаг функции MorphoData::IsRuleApplyable().
	/// Проверить предусловия нижних уровней.
	#define  M_RULE_LOWEST_PRECONDITIONS			2
	/// Флаг функции MorphoData::IsRuleApplyable().
	/// Если это правило защищено символом '!' - не надо пугаться, все равно проверять
	#define  M_RULE_CLOSED_RULE_IS_OK				4

	/////////////////////////////////////////////////////////////////////////////
	//
	// Последовательное получение всех форм данного слова по правилу
	//
	/////////////////////////////////////////////////////////////////////////////

	/// @brief Итератор форм слова 
	///
	/// Итератор, который необходим для последовательного 
	/// получения всех производных форм слова из базовой.
	/// @see WordFormsIteratorInit(), GetNextWordForm()
	
	
	/// @brief Флаг функции MorphoData::WordFormsIteratorInit().
	///
	/// Получать только формы слов, но не получать названия подгрупп форм
	#define  M_FORMS_ONLY_FORMS 					0
	/// @brief Флаг функции MorphoData::WordFormsIteratorInit().
	///
	/// Декорировать слова спомощью правил, указанных в третьей колонки 
	/// таблицы, заданной директирвой 'table' ml-языка
	#define  M_FORMS_DECORATE_WORD					1
	/// @brief Флаг функции MorphoData::WordFormsIteratorInit().
	///
	/// ???
	#define  M_FORMS_SEPARATE_SUBFORM_NAMES			2
	/// @brief Флаг функции MorphoData::WordFormsIteratorInit().
	///
	/// Получать даже скрытые формы слова (правило которых начинается на '!').
	#define  M_FORMS_INCLUDE_HIDDEN_FORMS			4

	/// Подготовить итератор к получению форм слова.
	void WordFormsIteratorInit ( WordFormsIterator &iter, const MCHAR *wordPtr, const MorphoInflectionRulesSetHandle* ruleset, Int32 flags) const;

	/// Подготовить итератор к получению форм слова.
	void WordFormsIteratorInitW ( WordFormsIterator &iter, const UInt16 *wordPtr, const MorphoInflectionRulesSetHandle* ruleset, Int32 flags) const;


	/// Получить следующую форму слова.
	bool GetNextWordForm ( WordFormsIterator &iter, MCHAR *formBuf, const MCHAR** questionPtr, const MCHAR** formNamePtr, bool *isSubTableName = 0) const;

	/// Получить следующую форму слова.
	bool GetNextWordFormW ( WordFormsIterator &iter, UInt16 *formBuf, const MCHAR** questionPtr, const MCHAR** formNamePtr, bool *isSubTableName = 0) const;

	/// Возвращает версию ядра.
	inline UInt32 GetMorphoEngineVersion(){return MORPHO_ENGINE_BUILD;};

	/// Возвращает структуру с заголовком морфологической базы.
	bool GetMorphoHeader(const MorphoDataHeader** aHeader) const;

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Работа со списком классов
	//
	/////////////////////////////////////////////////////////////////////////////

	/// @brief Итератор для получения классов 
	///
	/// Итератор, необходимый для последовательного получения всех 
	/// всех классов, хранящихся в морфологической базе данных.
	///
	/// Класс - это подмножество слов одной части речи, объединенных
	/// общей таблицей форм и названием класса. Нарпимер,
	/// "Имя существительное, мужской род, одушевленное".
	///
	/// @see ClassesIteratorInit(), GetNextClass()
	#ifdef _ML_PARSER_	
		typedef MVECTOR_V1<MorphoClass_v1>::const_iterator ClassesIterator;
	#else
		typedef STAT_VECTOR_SIZE_V1 ClassesIterator;
	#endif


	/// @brief Инициализировать итератор для получения всех классов в базе данных функцией GetNextClass().
	///
	/// Класс - это подмножество слов одной части речи, объединенных
	/// общей таблицей форм и названием класса. Нарпимер,
	/// "Имя существительное, мужской род, одушевленное".
	///
	/// @see ClassesIterator, GetNextClass(), ClassNameByRulesetPtr(), GetTableByRulesetPtr()
	inline void ClassesIteratorInit (ClassesIterator &iter) const
		#ifdef _ML_PARSER_
			{
				iter = classes.begin();
			};
		#else
			{
				// первый класс в списке
				iter = classes_begin;
			};
		#endif

	/// @brief Получить следующий класс
	///
	/// Класс - это подмножество слов одной части речи, объединенных
	/// общей таблицей форм и названием класса. Нарпимер,
	/// "Имя существительное, мужской род, одушевленное".
	///
	/// @see ClassesIterator, ClassesIteratorInit(), ClassNameByRulesetPtr(), GetTableByRulesetPtr()
	const MorphoClass_v1* GetNextClass (ClassesIterator &iter) const
	#ifdef _ML_PARSER_
		{
			return ( iter == classes.end() ) ? NULL : &*iter++;
		};
	#else
		{
			const MorphoClass_v1* classPtr;

			// достигли конца?
			if ( iter >= classes_end )
				return (MorphoClass_v1*)0;

			classPtr = (const MorphoClass_v1*)( (char*)m_resClass.ptr() + iter );

			// следующий класс в списке
			iter += ((const MorphoClass_v1*)( (char*)m_resClass.ptr() + iter ))->iflectionTables.size_in_bytes() // размер вектора
					+ (STAT_VECTOR_SIZE_V1)(UInt4Ptr)(&((MorphoClass_v1*)0)->iflectionTables);  // смещение вектора от начала структуры

			// возвращаем указатель
			return  classPtr;
		};
	#endif

	/// @brief Восстановить название класса по указателю на таблицу изменения слова
	///
	/// Класс - это подмножество слов одной части речи, объединенных
	/// общей таблицей форм и названием класса. Например,
	/// "Имя существительное, мужской род, одушевленное".
	///
	/// @see ClassesIteratorInit(), GetNextClass(), GetTableByRulesetPtr()
	const MCHAR* ClassNameByRulesetPtr(const MorphoInflectionRulesSetHandle* ruleset_ptr) const;

	/// @brief Восстановливает краткое название класса по указателю на таблицу изменения слова.
	///
	/// Класс - это подмножество слов одной части речи, объединенных
	/// общей таблицей форм и названием класса. Нарпимер,
	/// "Сущ.", "Глаг.", "Союз", "Нареч." - для краткого названия.
	///
	/// @see ClassesIteratorInit(), GetNextClass(), GetTableByRulesetPtr(), ClassNameByRulesetPtr()
	void GetBriefClassNameByRulesetPtr(const MorphoInflectionRulesSetHandle* ruleset, MCHAR* buf) const;
	/// @see GetBriefClassNameByRulesetPtr(), ClassesIteratorInit(), GetNextClass(), GetTableByRulesetPtr(), ClassNameByRulesetPtr()
	void GetBriefClassNameByRulesetPtrW(const MorphoInflectionRulesSetHandle* ruleset, UInt16* buf) const;

	/// @brief Восстановливает полное название класса по указателю на таблицу изменения слова.
	///
	/// Класс - это подмножество слов одной части речи, объединенных
	/// общей таблицей форм и названием класса. Нарпимер,
	/// "Имя существительное, мужской род, одушевленное" - для полного названия.
	///
	/// @see ClassesIteratorInit(), GetNextClass(), GetTableByRulesetPtr(), ClassNameByRulesetPtr()
	void GetFullClassNameByRulesetPtr(const MorphoInflectionRulesSetHandle* ruleset, MCHAR* buf) const;
	/// @see GetFullClassNameByRulesetPtr(), ClassesIteratorInit(), GetNextClass(), GetTableByRulesetPtr(), ClassNameByRulesetPtr()
	void GetFullClassNameByRulesetPtrW(const MorphoInflectionRulesSetHandle* ruleset, UInt16* buf) const;

	/// @brief Восстановить таблицу форм по указателю на таблицу изменения слова
	///
	/// Класс - это подмножество слов одной части речи, объединенных
	/// общей таблицей форм и названием класса. Нарпимер,
	/// "Имя существительное, мужской род, одушевленное".
	///
	/// @see ClassesIteratorInit(), GetNextClass(), ClassNameByRulesetPtr()
	const MorphoStateDescriptionTableHandle* GetTableByRulesetPtr(const MorphoInflectionRulesSetHandle* ruleset_ptr) const;


	/////////////////////////////////////////////////////////////////////////////
	//
	/// Получение информации о символе и манипуляции с ним
	//
	/////////////////////////////////////////////////////////////////////////////

	/// таблица аттрибутов символов языка
	const UInt8 *character_attributes_table;
	/// таблица преобразования к верхнему регистру
	const MCHAR *to_upper_table;
	/// таблица преобразования к нижнему регистру
	const MCHAR *to_lower_table;

	/// @brief Флаг, возвращаемый функцией MorphoData::LetterAttributes().
	///
	/// Установлен, если этот символ принадлежит алфавиту языка.
	#define CHARACTER_ATTRIBUTE_IS_LETTER						            0x01 
	/// @brief Флаг, возвращаемый функцией MorphoData::LetterAttributes().
	///
	/// Установлен, если эта буква является заглавной.
	#define CHARACTER_ATTRIBUTE_IS_UPPER						            0x02 
	/// @brief Флаг, возвращаемый функцией MorphoData::LetterAttributes().
	///
	/// Установлен, если этот символ является допустимым разделителем.
	#define CHARACTER_ATTRIBUTE_CHARACTER_IS_ALLOWED_SEPARATOR              0x04 
	/// @brief Флаг, возвращаемый функцией MorphoData::LetterAttributes().
	///
	/// Установлен, если этот символ может встречаться в начале слова.
	#define CHARACTER_ATTRIBUTE_CHARACTER_IS_ALLOWED_IN_BEGINNING_OF_WORD	0x10 
	/// @brief Флаг, возвращаемый функцией MorphoData::LetterAttributes().
	///
	/// Установлен, если этот символ может встречаться в середине слова.
	#define CHARACTER_ATTRIBUTE_CHARACTER_IS_ALLOWED_IN_MIDDLE_OF_WORD      0x20 
	/// @brief Флаг, возвращаемый функцией MorphoData::LetterAttributes().
	///
	/// Установлен, если этот символ может встречаться в конце слова.
	#define CHARACTER_ATTRIBUTE_CHARACTER_IS_ALLOWED_AT_END_OF_WORD         0x40 

	/// Получение аттрибутов символа данного языка
	/// \param ch символ
	/// \return Возвращаемое значение является комбинацие флагов:
	///
	/// - #CHARACTER_ATTRIBUTE_IS_LETTER \n
	///      \copydoc CHARACTER_ATTRIBUTE_IS_LETTER
	///
	/// - #CHARACTER_ATTRIBUTE_IS_UPPER \n
	///      \copydoc CHARACTER_ATTRIBUTE_IS_UPPER
	///
	/// - #CHARACTER_ATTRIBUTE_CHARACTER_IS_ALLOWED_SEPARATOR \n
	///      \copydoc CHARACTER_ATTRIBUTE_CHARACTER_IS_ALLOWED_SEPARATOR
	///
	/// - #CHARACTER_ATTRIBUTE_CHARACTER_IS_ALLOWED_IN_BEGINNING_OF_WORD \n
	///      \copydoc CHARACTER_ATTRIBUTE_CHARACTER_IS_ALLOWED_IN_BEGINNING_OF_WORD
	///
	/// - #CHARACTER_ATTRIBUTE_CHARACTER_IS_ALLOWED_IN_MIDDLE_OF_WORD \n
	///      \copydoc CHARACTER_ATTRIBUTE_CHARACTER_IS_ALLOWED_IN_MIDDLE_OF_WORD
	///
	/// - #CHARACTER_ATTRIBUTE_CHARACTER_IS_ALLOWED_AT_END_OF_WORD \n
	///      \copydoc CHARACTER_ATTRIBUTE_CHARACTER_IS_ALLOWED_AT_END_OF_WORD
	///
	inline UInt32 LetterAttributes(MCHAR ch) const { return (UInt32)character_attributes_table[(UInt8)ch]; };

	/// Приведени символа к верхнему регистру
	inline MCHAR ToLower(MCHAR ch) const { return to_lower_table[(UInt8)ch]; };

	/// Приведени символа к нижнему регистру
	inline MCHAR ToUpper(MCHAR ch) const { return to_upper_table[(UInt8)ch]; };

	/////////////////////////////////////////////////////////////////////////////
	//
	// Последоваельное получение различных вариантов написания слова
	//
	/////////////////////////////////////////////////////////////////////////////

	/// @brief Итератор вариантов написания слова
	///
	/// Итератор, который необходим для последовательного 
	/// получения всех возможных варинатов написания слова
	/// (с большой буквы, с маленькой и т.д.) для произвольного
	/// слова из текста, чтобы найти перебором ту формы, которая 
	/// храниться в базе данных.
	///
	/// @see WritingVersionIteratorInit(), GetNextWritingVersion()

	
	/// Инициализировать итератор
	void WritingVersionIteratorInit (WritingVersionIterator &iter, const MCHAR *wordForm, UInt32 &length , bool strict = false) const;
	/// Инициализировать итератор в юникодном вызове.
	void WritingVersionIteratorInitW (WritingVersionIterator &iter, const UInt16 *wordForm, UInt32 &length , bool strict = false) const;

	/// Получить очередной варант написания слова для восстановления его базовой формы.
	bool GetNextWritingVersion (WritingVersionIterator &iter, MCHAR* outBuf) const;
	/// Получить очередной варант написания слова для восстановления его базовой формы.
	bool GetNextWritingVersionW (WritingVersionIterator &iter, UInt16* outBuf) const;

	/////////////////////////////////////////////////////////////////////////////
	//
	// Последоваельное получение гипотез о базовой форме слова
	//
	/////////////////////////////////////////////////////////////////////////////


	/// @brief Флаг функции MorphoData::BaseFormsIteratorInit(). \n
	///
	/// Если используется в комбинации с другими флагами, то игнорируется.
	/// Данный флаг означает, что при восстановлении 
	/// \ref baseform "базовой формы слова" необходимо выдавать
	/// все возможные гипотезы. Используется только в отладочных целях.
	#define  M_BASEFORMS_ALL_GUESSES						0		
	/// @brief Флаг функции MorphoData::BaseFormsIteratorInit(). \n
	///
	/// При восстановлении \ref baseform "базовой формы слова" необходимо 
	/// проверять все гипотезы на соответствие на исходному предусловию 
	/// набора правил, по которому форма была восстановлена.
	///	Позволяет ускорить работу в случае использования 
	///	флага #M_BASEFORMS_USE_DICTIONARY.
	/// Рекоммендуется всегда использовать.
	#define  M_BASEFORMS_SHOULD_MATCH_PRECONDITION			1		
	/// @brief Флаг функции MorphoData::BaseFormsIteratorInit().
	///
	/// При восстановлении \ref baseform "базовой формы слова" необходимо 
	/// проверять все гипотезы вида "слово - правила изменения" на наличие
	/// в морфологической базе данных. В случае, если такое слово есть в базе,
	/// но у него нет такого правила изменения, необходимо эту гипотезу отбросить.
	/// Рекоммендуется всегда использовать.
	#define  M_BASEFORMS_USE_DICTIONARY					    2		
	/// @brief Флаг функции MorphoData::BaseFormsIteratorInit().
	///
	/// При восстановлении \ref baseform "базовой формы слова" и при
	/// установленном флаге #M_BASEFORMS_USE_DICTIONARY отбрасывать
	/// все гипотезы, базовые формы которых не найдены в словаре
	/// морфологической базы данных.
	/// Рекоммендуется всегда использовать.
	#define  M_BASEFORMS_ONLY_DICTIONARY_WORDS				4
	/// @brief Флаг функции MorphoData::BaseFormsIteratorInit().
	///
	/// В полисинтетических языках (например, в испаснком) слова могут
	/// "склеиваться" вместе. В испанском, например, к глаголам приклеиваются
	/// местоимения. По умолчанию выдаются формы всех "склеенных" слов. 
	/// Если же установить этот флаг, будет выдана только та форма,
	/// которая считается "основной".
	/// Например:
	/// fumarse -> первое слово fumar + второе слово se
	/// Если данный флаг указан, то выдастся только fumar.	
	#define  M_IGNORE_BASEFORMS_OF_PARTS					8


	/// @brief Флаг функции MorphoData::BaseFormsIteratorInit().
	///
	/// Если этот флаг не установлен, то языкоспецифичные проверки
	/// все равно делаются, даже если базовая форма найдена обычным методом. 
	/// Этот флаг хорошо бы использовать, если пользовательский интерфейс
	/// не обеспечивает сортировку и группировку найденных базовых форм.
	/// Иначе плохо знающего язык человека мы запутаем редкими формами, 
	/// в то время как есть формы вполне очевидные.
	#define  M_DONT_DO_LANGSPEC_IF_BASEFORMS_FOUND				16


	/// Погдотовить итератор к получению базовых форм слова
	void BaseFormsIteratorInit ( BaseFormsIterator &iter, const MCHAR *wordFormPtr, Int32 flags) const;
	/// Погдотовить итератор к получению базовых форм слова
	void BaseFormsIteratorInitW (BaseFormsIterator &iter, const UInt16 *wordFormPtr, Int32 flags) const;

	/// Получить следующую гипотезу базовой формы слова
	bool GetNextBaseForm       ( BaseFormsIterator &iter, MCHAR *baseFormBuf,  const MorphoInflectionRulesSetHandle** rulesetPtr ) const;
	/// Получить следующую гипотезу базовой формы слова
	bool GetNextBaseFormW      ( BaseFormsIterator &iter, UInt16 *baseFormBuf,  const MorphoInflectionRulesSetHandle** rulesetPtr ) const;
	
	//// Следующие далее функции и константы относятся к работе с языкоспецифичными 
	// морфологическими особенностями
	enum { BUFLEN_FOR_ITERATOR = 1024 };
	bool LanguageSpecificTrials ( BaseFormsIterator &iter, MCHAR *baseFormBuf,  const MorphoInflectionRulesSetHandle** rulesetPtr, UInt32 language  ) const;

private:
	
	/// Диапазон поддерживаемых форматов данной версией ядра
	enum
	{
		M_DATABASE_FORMAT_MIN_VERSION = 105,
		M_DATABASE_FORMAT_MAX_VERSION = 105
	};
	
public:

	/// Проверяет, поддерживается ли данной версией ядра данный формат
	static bool IsCoreSupportFormat(UInt32 Format)
	{
		return ((Format >= M_DATABASE_FORMAT_MIN_VERSION) && (Format <= M_DATABASE_FORMAT_MAX_VERSION)) ? true : false;
	};
	
	UInt32 GetNumberOfBaseForms(void) const;
	UInt32 GetNumberOfWordForms(void) const;
	bool IsMorphologySimple(void) const;

}; /// struct MorphoData_v1

#endif
