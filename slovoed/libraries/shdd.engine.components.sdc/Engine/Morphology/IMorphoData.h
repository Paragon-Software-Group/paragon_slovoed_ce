#ifndef _I_MORPHO_DATA_INCLUDE___
#define _I_MORPHO_DATA_INCLUDE___


#include "../SldPlatform.h"
#include "../SldSDCReadMy.h"
#include "../SldCompare.h"


typedef void MorphoInflectionRulesSetHandle;
typedef void MorphoStateDescriptionTableHandle;
typedef void OneCallParametersHandle;


/// Тип символов, с которыми мы работаем. 
/** В данном случае мы используем однобайтные символы. */
typedef char MCHAR;

/// Тип для функции bool Enum(bool bAllForms, EnumCallBackT* callback, void* param) const;
typedef bool EnumCallBackT(const MCHAR* word, void* param);

/// Тип для функции bool EnumPartial(EnumPartialCallBackT* callback, UInt32 aBaseFormIndexFirst, UInt32 aBaseFormIndexLast, void* aParam) const;
typedef bool EnumPartialCallBackT(const MCHAR* aWordForm, const MCHAR* aBaseForm, void* aParam);

/// @brief Внутренний параметр ядра.
///
/// Текущая версия формата базы данных с морфологическими данными.
/// Используется для защиты от работы с данными старой версии формата.
/// При изменении формата данных, значение необходимо увеличивать.
#define DATABASE_FORMAT_VERSION					107

/// Версия ядра морфологии. Эта переменная автоматически увеличивается внешней утилитой.
#define MORPHO_ENGINE_BUILD					18

/// @brief Внутренний параметр ядра.
///
/// Максимальная глубина дерева форм слова.
#define MAX_DEPTH_OF_FORMS_TREE				14

/// @brief Внутренний параметр ядра.
///
/// Прирост таблицы быстрого обратного преобразования при добавлении новых элементов (в элементах).
#define	MEM_ALLOCATION_INCREMENT_STEP		64

/// Идентификатор русского языка
#define MORPHO_LANGUAGE_RUSSIAN				MORPHO_SWAP_32('ssur')
/// Идентификатор испанского языка
#define MORPHO_LANGUAGE_SPANISH				MORPHO_SWAP_32('naps')
/// Идентификатор английского языка
#define MORPHO_LANGUAGE_ENGLISH				MORPHO_SWAP_32('lgne')
/// Идентификатор немецкого языка
#define MORPHO_LANGUAGE_GERMAN				MORPHO_SWAP_32('mreg')
/// Идентификатор французского языка
#define MORPHO_LANGUAGE_FRENCH				MORPHO_SWAP_32('nerf')

/// Символ разделяющий полную информацию о части речи и прочих данных слова от сокращенных данных.
#define MORPHO_CLASS_SEPARATOR_CHAR			('|')

#define MAX_WORD_LEN	200


#define MORPHODATA_RES_HEADER		EnsureNativeByteOrder_32('HEAD')
#define MORPHODATA_RES_TEXT			EnsureNativeByteOrder_32('STRI')
#define MORPHODATA_RES_CHARMAP		EnsureNativeByteOrder_32('CMAP')
#define MORPHODATA_RES_WORDSET		EnsureNativeByteOrder_32('WORD')
#define MORPHODATA_RES_ARCH_HEADER	EnsureNativeByteOrder_32('AHDR')
#define MORPHODATA_RES_RUL2			EnsureNativeByteOrder_32('RUL2')
#define MORPHODATA_RES_QAINDEX		EnsureNativeByteOrder_32('INDX')
#define MORPHODATA_RES_TREE			EnsureNativeByteOrder_32('TREE')
#define MORPHODATA_RES_TABLE		EnsureNativeByteOrder_32('TABL')
#define MORPHODATA_RES_CLASS		EnsureNativeByteOrder_32('CLAS')
#define MORPHODATA_RES_RULESSET		EnsureNativeByteOrder_32('RULS')
#define MORPHODATA_RES_LASTCHAR		EnsureNativeByteOrder_32('LCHR')


/// Маска для флага MorphoDataHeader::Flags, является ли база морфологии "простой", 
/// то есть такой, которая содержит только сами базовые формы слов и их словоформы, но не содержит 
/// в таблицах грамматической информации о формах (часть речи, время, падеж и т.д.)
#define MORPHODATA_FLAGS_MASK_IS_SIMPLE		1


//////////////////////////////////////////////////////////////////////
///
/// @brief Заголовок файла с морфологическими даннымы
///
//////////////////////////////////////////////////////////////////////
struct MorphoDataHeader
{
	/// Размер структуры #MorphoDataHeader
	UInt32 HeaderSize;

	/// @brief Код языка
	/// Код языка в формате PiLoc и SlovoEd - четыре первые 
	/// буквы английского названия языка ('russ', 'engl'...)
	UInt32	language;

	/// Идентификатор базы данных.
	UInt32 DictID;

	/// Версия формата файла @see DATABASE_FORMAT_VERSION
	UInt32	format;

	/// Версия и билд файла (1.000) (пока не используется)
	UInt32	version;

	/// Полное имя языка
	Int8 FullLanguageName[32];

	/// Имя автора
	Int8 Author[32];

	/// сайт автора
	Int8 WWWAuthor[32];

	/// Количество элементов, которое необходимо
	/// для таблицы быстрого обратного преобразования
	/// (для того, чтобы выделять пямять только один раз).
	UInt32	num_items_in_LCM;

	/*	/// Начало классов в файле морфологии.
	STAT_VECTOR_SIZE classes_begin;

	/// Конец классов в файле морфологии.
	STAT_VECTOR_SIZE classes_end;

	/// Начало всех строк в файле морфологии.
	UInt32   all_strings;

	/// Начало всех структур в файле морфологии.
	UInt32   all_structs;

	/// Начало списка слов в файле морфологии.
	UInt32   words_set;
	*/
	
	/// Аттрибуты всех символов языка
	UInt8   character_attributes_array[256];
	/// Массив преобразования к верхнему регистру
	MCHAR           to_upper_array[256];
	/// Массив преобразования к нижнему регистру
	MCHAR           to_lower_array[256];

	/// Количество форм слов которые распознаются данной базой морфологии.
	/** Количество форм слов которые распознаются данной базой морфологии.
		Количество получается как сумма форм для каждого слова.
	*/
	UInt32	NumberOfWordForms;

	/// Количество базовых форм слов которые распознаются данной базой морфологии.
	/** Количество базовых форм слов которые распознаются данной базой морфологии.
		Количество получается как сумма всех написаний слов.
	*/
	UInt32	NumberOfBaseForms;
	
	/// Максимальная длина предусловия правил в базе
	UInt16	MaximumPreconditionLength;
	
	/// Битовые флаги свойств
	UInt16	Flags;
};



struct WordFormsIterator
{
	friend struct MorphoData_v1;
	friend struct MorphoData_v2;
	friend struct MorphoData_v3;

	WordFormsIterator()
	{
		posBuf = NULL;
	};
	
	~WordFormsIterator()
	{
		if (posBuf)
		{
			sldMemFree(posBuf);
			posBuf = NULL;
		}
	}
	
private:
	/// указатель на исходное слово
	const MCHAR *wordPtr;

	/// буффер для юникодных вызовов.
	MCHAR word_form_buffer[MAX_WORD_LEN];

	/// указатель на буфер позиций
    const MCHAR** posBuf;

	/// указатель на корень набора правил
	const MorphoInflectionRulesSetHandle* ruleset;

	/// указатель на текущий набор правил
	const MorphoInflectionRulesSetHandle* cur_ruleset;

	/// указатель на корень набора правил
	const MorphoStateDescriptionTableHandle* table;

	/// указатель на текущий набор правил
	const MorphoStateDescriptionTableHandle* cur_table;
	/// флаги форматирования
	Int32	 flags;

	/// текущая глубина вектора состояния
	UInt8 depth;
	UInt8 alternative;
	/// текущая позиция в дереве состояний
	UInt8 vector[MAX_DEPTH_OF_FORMS_TREE];

	SldU16WordsArray wordForms;
};


struct WritingVersionIterator
{
	friend struct MorphoData_v1;
	friend struct MorphoData_v2;
	friend struct MorphoData_v3;
	
private:

	/// указатель на исходное слово.
	const MCHAR *wordForm;
	/// Буффер для юникодных вызовов.
	MCHAR		word_form_buffer[MAX_WORD_LEN];

	/// длина слова во входном буфере
	UInt32 wordLen;

	/// флаги с указанием тех форм, которые надо попробовать
	UInt32 todo;

	/// индексы, значение которых варьируется в зависимости от языка
	UInt32 internalIndex1;
	UInt32 internalIndex2;

	SldU16String string;
};


struct CallParamIterator
{
	friend class RulesByLastChar_v1;
	friend class RulesByLastChar_v2;
	
private:
	UInt32 curIndex;
	UInt32 lastIndex;
};


/// @brief Итератор базовых форм слова
///
/// Итератор, который необходим для последовательного 
/// получения всех \ref baseform "базовых форм слова" из слова
/// в произвольной форме.
///
/// @see BaseFormsIteratorInit(), GetNextBaseForm()
struct BaseFormsIterator
{
	enum { MAX_SIMULTANEOUS_FORMS = 10};
	enum { MAX_SAVED_FORMS = 20};

	friend struct MorphoData_v1;
	friend struct MorphoData_v2;
	friend struct MorphoData_v3;

	BaseFormsIterator()
	{
		posBuf = NULL;
	};
	
	~BaseFormsIterator()
	{
		if (posBuf)
		{
			sldMemFree(posBuf);
			posBuf = NULL;
		}
	}
	
private:

	/// форма слова
	const MCHAR*  word_form;

	/// Буффер для формы слова в случае если создание итератора происходит в юникодном варианте функции.
	MCHAR	word_form_buffer[MAX_WORD_LEN];

	/// pointer to one call parameters
	//const RulesByLastChar_v2::OneCallParameters *call_param;
	const OneCallParametersHandle* call_param;

	/// pointer to bufer
	const MCHAR** posBuf;
	
	/// iterator
	//RulesByLastChar_v2::CallParamIterator iter;
	CallParamIterator iter;

	//
	Int32		  pass;

	Int32		  flags;

	/// последний и предпоследний символы
	MCHAR         last_char;
	MCHAR         pre_last_char;

	// Сигнал, что в итераторе хранятся данные, которые вернул рекурсивный вызов
	bool    afterRecursion;
	// Базовые формы разделенных слов из полисинтетического языка
	MCHAR	simultaneousForms[MAX_SIMULTANEOUS_FORMS][MAX_WORD_LEN];
	// Количество одновременно найденных базовых форм
	Int32     simultaneousFormsNumber;
	// Массив для сохранения рулесетов форм, найденных в процессе языкоспецифичных
	// проверок
	const MorphoInflectionRulesSetHandle* simultaneousFormsRulesets[MAX_SAVED_FORMS];
	// Для итерирования по формам, которые были найдены в рекурсивном вызове
	Int32 curFormAfterRecursion;
	// Флаг - чтобы делать языкоспецифичные проверки только один раз
	bool langSpecificTrialsDone;

	// Массив для сохранения уже найденных базовых форм
	MCHAR	foundForms[MAX_SAVED_FORMS][MAX_WORD_LEN];
	// Массив для сохранения рулесетов уже найденных форм
	const MorphoInflectionRulesSetHandle* foundFormsRulesets[MAX_SAVED_FORMS];
	// Счетчик всех найденных форм
	Int32 foundFormsNumber;
	// Найдены ли в процессе специальных языковых проверок совпадения с формами, 
	// уже имеющимися
	bool areCoincidences;

	Int32 saveFoundForm(const MCHAR* form, const MorphoInflectionRulesSetHandle* rs)
	{
		if (foundFormsNumber >= MAX_SAVED_FORMS) return -1;
		CSldCompare::StrNCopyA((UInt8*)foundForms[foundFormsNumber], (UInt8*)form, MAX_WORD_LEN);
		foundFormsRulesets[foundFormsNumber++] = rs;
		return 0;
	}

	SldU16WordsArray baseForms;
};


struct IMorphoData
{
public:

	virtual ~IMorphoData() {};

	virtual bool Init(CSDCReadMy &aMorphology) = 0;
	virtual void Close() = 0;
	
	
	virtual ESldLanguage GetLanguageCode(void) const = 0;
	virtual UInt32 GetMorphoEngineVersion() = 0;
	virtual bool GetMorphoHeader(const MorphoDataHeader** aHeader) const = 0;
	
	
	virtual const MCHAR* ClassNameByRulesetPtr(const MorphoInflectionRulesSetHandle* ruleset_ptr) const = 0;
	virtual void GetBriefClassNameByRulesetPtr(const MorphoInflectionRulesSetHandle* ruleset, MCHAR* buf) const = 0;
	virtual void GetBriefClassNameByRulesetPtrW(const MorphoInflectionRulesSetHandle* ruleset, UInt16* buf) const = 0;
	virtual void GetFullClassNameByRulesetPtr(const MorphoInflectionRulesSetHandle* ruleset, MCHAR* buf) const = 0;
	virtual void GetFullClassNameByRulesetPtrW(const MorphoInflectionRulesSetHandle* ruleset, UInt16* buf) const = 0;
	virtual const MorphoStateDescriptionTableHandle* GetTableByRulesetPtr(const MorphoInflectionRulesSetHandle* ruleset_ptr) const = 0;
	
	
	Int8* StrTok(Int8** szToken, const Int8* szDelimit) const;
	Int8* StrReverse(Int8* szString) const;
	
	
	virtual UInt32 LetterAttributes(MCHAR ch) const = 0;
	virtual MCHAR ToLower(MCHAR ch) const = 0;
	virtual MCHAR ToUpper(MCHAR ch) const = 0;
	
	
	virtual bool IsRuleApplyable(const MCHAR *word, const MorphoInflectionRulesSetHandle* ruleset_ptr, const MCHAR **posBuf, Int32 flags) const = 0;
	
	
	virtual void WordFormsIteratorInit(WordFormsIterator &iter, const MCHAR *wordPtr, const MorphoInflectionRulesSetHandle* ruleset, Int32 flags) const = 0;
	virtual void WordFormsIteratorInitW(WordFormsIterator &iter, const UInt16 *wordPtr, const MorphoInflectionRulesSetHandle* ruleset, Int32 flags) const = 0;
	virtual bool GetNextWordForm(WordFormsIterator &iter, MCHAR *formBuf, const MCHAR** questionPtr, const MCHAR** formNamePtr, bool *isSubTableName = 0) const = 0;
	virtual bool GetNextWordFormW(WordFormsIterator &iter, UInt16 *formBuf, const MCHAR** questionPtr, const MCHAR** formNamePtr, bool *isSubTableName = 0) const = 0;
	
	
	virtual void WritingVersionIteratorInit(WritingVersionIterator &iter, const MCHAR *wordForm, UInt32 &length , bool strict = false) const = 0;
	virtual void WritingVersionIteratorInitW(WritingVersionIterator &iter, const UInt16 *wordForm, UInt32 &length , bool strict = false) const = 0;
	virtual bool GetNextWritingVersion(WritingVersionIterator &iter, MCHAR* outBuf) const = 0;
	virtual bool GetNextWritingVersionW(WritingVersionIterator &iter, UInt16* outBuf) const = 0;
	
	
	virtual void BaseFormsIteratorInit(BaseFormsIterator &iter, const MCHAR *wordFormPtr, Int32 flags) const = 0;
	virtual void BaseFormsIteratorInitW(BaseFormsIterator &iter, const UInt16 *wordFormPtr, Int32 flags) const = 0;
	virtual bool GetNextBaseForm(BaseFormsIterator &iter, MCHAR *baseFormBuf, const MorphoInflectionRulesSetHandle** rulesetPtr) const = 0;
	virtual bool GetNextBaseFormW(BaseFormsIterator &iter, UInt16 *baseFormBuf, const MorphoInflectionRulesSetHandle** rulesetPtr) const = 0;
	
	virtual bool LanguageSpecificTrials(BaseFormsIterator &iter, MCHAR *baseFormBuf, const MorphoInflectionRulesSetHandle** rulesetPtr, UInt32 language) const = 0;
	
	virtual UInt32 GetNumberOfBaseForms(void) const = 0;
	virtual UInt32 GetNumberOfWordForms(void) const = 0;
	
	virtual bool IsMorphologySimple(void) const = 0;
	
	virtual bool Enum(bool bAllForms, EnumCallBackT* callback, void* param) const = 0;
	
	virtual bool EnumPartial(EnumPartialCallBackT* callback, UInt32 aBaseFormIndexFirst, UInt32 aBaseFormIndexLast, void* aParam) const = 0;
	
};

#endif
