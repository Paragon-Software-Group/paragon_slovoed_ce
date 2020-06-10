//
// Created by borbat on 14.09.2018.
//

#include <SldAuxiliary.h>
#include "FullTextSearch.h"
#include "java/java_objects.h"

jobject FullTextSearch::native(
    JNIEnv* env,
    CDictionaryContext* context,
    jobject _listIndex,
    jobject _word,
    jobject _maximumWords,
    jobject _sortType
) const noexcept
{
  Int32 listIndex = JavaObjects::GetInteger( env, _listIndex );
  JavaObjects::string word = JavaObjects::GetString( env, _word );
  Int32 maximumWords = JavaObjects::GetInteger( env, _maximumWords );
  int sortType = JavaObjects::GetInteger( env, _sortType );
  jobject errorValue = JavaObjects::GetInteger( env, -1 );

  CSldDictionary* dictionary = context->GetDictionary();
  ESldLanguage languageFrom = dictionary->GetListLanguageFrom( listIndex );
  bool isInternalMorphoAvailable = context->IsInternalMorphoAvailable( languageFrom, CDictionaryContext::eFts );
  MorphoData* morphoData = nullptr;
  if ( !isInternalMorphoAvailable )
  {
    morphoData = context->GetExternalMorpho( languageFrom, CDictionaryContext::eFts );
  }

  SldU16String preparedQuery;
  ESldError error;
  if ( isInternalMorphoAvailable )
  {
    error = PrepareQueryForFullTextSearch( word.c_str(), dictionary, listIndex, preparedQuery );
  }
  else
  {
    error = PrepareQueryForFullTextSearch( word.c_str(), dictionary, listIndex, morphoData, preparedQuery );
  }
  if ( error != eOK )
  {
    return errorValue;
  }

  error = dictionary->DoFullTextSearch( listIndex, preparedQuery.c_str(), maximumWords );
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

  if ( sortType >= 0 )
  {
    CSldVector< TSldMorphologyWordStruct > morphoForms;
    if ( isInternalMorphoAvailable )
    {
      error = PrepareQueryForSortingSearchResults( word.c_str(), dictionary, languageFrom, morphoForms );
    }
    else
    {
      error = PrepareQueryForSortingSearchResults( word.c_str(), dictionary, morphoData, morphoForms );
    }
    if ( error != eOK )
    {
      return errorValue;
    }

    error =
        dictionary->SortSearchListRelevant( currentList, word.c_str(), morphoForms, (ESldFTSSortingTypeEnum) sortType );
    if ( error != eOK )
    {
      return errorValue;
    }
  }

  return JavaObjects::GetInteger( env, currentList );
}
