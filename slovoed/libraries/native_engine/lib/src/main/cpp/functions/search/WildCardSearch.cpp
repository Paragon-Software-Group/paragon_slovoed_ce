//
// Created by popov on 12.02.2019.
//

#include <SldAuxiliary.h>
#include "WildCardSearch.h"
#include "java/java_objects.h"

jobject WildCardSearch::native(
    JNIEnv* env,
    CDictionaryContext* context,
    jobject _listIndex,
    jobject _word,
    jobject _maximumWords
) const noexcept
{
  Int32 listIndex = JavaObjects::GetInteger( env, _listIndex );
  JavaObjects::string word = JavaObjects::GetString( env, _word );
  Int32 maximumWords = JavaObjects::GetInteger( env, _maximumWords );
  jobject errorValue = JavaObjects::GetInteger( env, -1 );

  CSldDictionary* dictionary = context->GetDictionary();
  ESldLanguage languageFrom = dictionary->GetListLanguageFrom( listIndex );
  bool isInternalMorphoAvailable = context->IsInternalMorphoAvailable( languageFrom, CDictionaryContext::eWildCard );
  MorphoData* morphoData = nullptr;
  if ( !isInternalMorphoAvailable )
  {
    morphoData = context->GetExternalMorpho( languageFrom, CDictionaryContext::eWildCard );
  }

  TExpressionBox expressionBox;
  ESldError error;
  if ( isInternalMorphoAvailable )
  {
    error = PrepareQueryForWildCardSearch( word.c_str(), dictionary, listIndex, &expressionBox);

  }
  else
  {
    error = PrepareQueryForWildCardSearch( word.c_str(), dictionary, listIndex, morphoData, &expressionBox );
  }
  if ( error != eOK )
  {
    return errorValue;
  }

//    expressionBox.AddExpression(word.c_str(), eTokenType_Operation_AND);

  error = dictionary->DoWildCardSearch(&expressionBox, maximumWords);
  if ( error != eOK )
  {
    return errorValue;
  }

  Int32 currentList;
  error = dictionary->GetCurrentWordList( &currentList );
  if ( error != eOK )
  {
    return errorValue;
  }

//  if ( sortType >= 0 )
//  {
//    CSldVector< TSldMorphologyWordStruct > morphoForms;
//    if ( isInternalMorphoAvailable )
//    {
//      error = PrepareQueryForSortingSearchResults( word.c_str(), dictionary, languageFrom, morphoForms );
//    }
//    else
//    {
//      error = PrepareQueryForSortingSearchResults( word.c_str(), dictionary, morphoData, morphoForms );
//    }
//    if ( error != eOK )
//    {
//      return errorValue;
//    }
//
//    error =
//        dictionary->SortSearchListRelevant( currentList, word.c_str(), morphoForms, (ESldFTSSortingTypeEnum) sortType );
//    if ( error != eOK )
//    {
//      return errorValue;
//    }
//  }

  return JavaObjects::GetInteger( env, currentList );
}
