#ifndef _C_SLD_AUXILIARY_
#define _C_SLD_AUXILIARY_

#include "SldDictionary.h"
#include "SldMacros.h"

// Если морфология не нужна в поиске, нужно раскомментировать данный define
// или объявить аналогичный в другом месте
//#define NOT_USING_MORPHOLOGY_IN_AUXILIARY

// В данный момент предназначен для работы только с немецкими морфологическими базами;
// в случае использования баз других языков этот define на формирование запроса не влияет
// #define USE_ALL_POSSIBLE_FORMS

#ifndef NOT_USING_MORPHOLOGY_IN_AUXILIARY
#include "Morphology/MorphoData.h"
typedef MorphoData MorphoDataType;
#define MAXIMUM_WORD_LENGTH MAX_WORD_LEN
#else
typedef void MorphoDataType;
#define MAXIMUM_WORD_LENGTH 100
#endif

/// Преобразует строку поиска, введенную пользователем, в расширенный запрос, использую морфологию встроенную в базу
ESldError PrepareQueryForFullTextSearch(const UInt16 *aText, CSldDictionary *aDict, Int32 aListIndex, SldU16String& aResultQuery, SldU16WordsArray * aMorphoForms = NULL);

/// Преобразует строку поиска, введенную пользователем, в расширенный запрос для полнотекстового поиска
ESldError PrepareQueryForFullTextSearch(const UInt16* aText, CSldDictionary* aDict, Int32 aListIndex, MorphoDataType* aMorpho, SldU16String& aResultQuery, SldU16WordsArray * aMorphoForms = NULL);

/// Преобразует строку поиска, введенную пользователем, в расширенный запрос для поиска по шаблону, использую морфологию встроенную в базу.
ESldError PrepareQueryForWildCardSearch(const UInt16* aText, CSldDictionary* aDict, Int32 aListIndex, TExpressionBox* aExpressionBox);

/// Преобразует строку поиска, введенную пользователем, в расширенный запрос для поиска по шаблону
ESldError PrepareQueryForWildCardSearch(const UInt16* aText, CSldDictionary* aDict, Int32 aListIndex, MorphoDataType* aMorpho, TExpressionBox* aExpressionBox);

/// Преобразует строку поиска, введенную пользователем, в расширенный запрос для полнотекстового поиска
ESldError PrepareQueryForSortingSearchResults(const UInt16* aText, CSldDictionary* aDict, MorphoDataType* aMorpho, CSldVector<TSldMorphologyWordStruct>& aMorphologyForms);

/// Преобразует строку поиска, введенную пользователем, в расширенный запрос для полнотекстового поиска
ESldError PrepareQueryForSortingSearchResults(const UInt16* aText, CSldDictionary* aDict, UInt32 aLangCode, CSldVector<TSldMorphologyWordStruct>& aMorphologyForms);

/// Возвращает все Морфоформы для данного слова из внешней базы
ESldError GetAllMorphoForms(const UInt16* aText, MorphoDataType* aMorpho, CSldVector<TSldMorphologyWordStruct> & aMorphologyForms);

/// Возвращает все Морфоформы для данного слова, используя встроенную морфологию
ESldError GetAllMorphoForms(const UInt16* aText, CSldDictionary *aDict, UInt32 aLangFrom, CSldVector<TSldMorphologyWordStruct> & aMorphologyForms, UInt32 aWithoutRequest = 0);

/// Возвращает все Морфоформы выбранного типа для данного слова, используя встроенную морфологию
ESldError GetMorphoForms(const UInt16* aText, CSldDictionary *aDict, UInt32 aLangFrom, CSldVector<TSldMorphologyWordStruct> & aMorphologyForms, UInt32 aWithoutRequest = 0, const EMorphoFormsType aType = eMorphoAll);

#endif //_C_SLD_AUXILIARY_
