//
// Created by borbat on 16.10.2018.
//

#include <java/java_objects.h>
#include "GetWordByText.h"

/*
 * https://bugs.paragon-software.com/projects/techno_sdc/wiki/%D0%A0%D0%B0%D0%B1%D0%BE%D1%82%D0%B0_%D1%81_%D0%BF%D0%BE%D0%B8%D1%81%D0%BA%D0%BE%D0%BC
 */

static ESldError helper( CSldDictionary* Dict, const UInt16* searchQuery, UInt32* crossrefSuccessful, UInt32 listVariant )
{
  ESldError error = eOK;
  UInt32 resultFlag = 0;
  *crossrefSuccessful = 0;

  const CSldCompare* compareTablePtr = NULL;
  error = Dict->GetCompare( &compareTablePtr );
  if ( error != eOK )
  {
    return error;
  }

  Int32 listIndex = SLD_DEFAULT_LIST_INDEX;
  error = Dict->GetCurrentWordList( &listIndex );
  if ( error != eOK )
  {
    return error;
  }

// нужно найти индекс отображаемого варианта (с ним будем сравнивать результаты поиска)
  const CSldListInfo* pListInfo = NULL;
  error = Dict->GetWordListInfo( listIndex, &pListInfo );
  if ( error != eOK )
  {
    return error;
  }

  UInt32 numberOfVariants = pListInfo->GetNumberOfVariants();

  UInt32 showVariantIndex = 0;
  for ( UInt32 variantIndex = 0 ; variantIndex < numberOfVariants ; variantIndex++ )
  {
    UInt32 variantType = pListInfo->GetVariantType( variantIndex );

    if ( variantType == listVariant )
    {
      showVariantIndex = variantIndex;
    }
  }

// ищем точное совпадение переданного текста с заголовком статьи (Show вариантом написания) или альтернативным заголовком
//  error = Dict->GetWordByTextExtended( searchQuery, &resultFlag );

  error = Dict->GetWordByTextExtended( searchQuery, &resultFlag, 1 );
  if ( error != eOK )
  {
    return error;
  }

  // получаем текущий Show вариант
  UInt16* currentWord = NULL;
  error = Dict->GetCurrentWord( showVariantIndex, &currentWord );
  if ( error != eOK )
  {
    return error;
  }

// статья нашлась по основному или альтернативному заголовку
// (альтернативных заголовков нет в индексе, но при вводе такого заголовка индекс подматывается к соответствующему основному заголовку - это есть, например, в CALD)
  if ( resultFlag )
  {
    if ( !compareTablePtr->StrICmp( currentWord, searchQuery ) )
    {
      *crossrefSuccessful = 1;
      return eOK;
    }
  }

// сравниваем начало заголовка/поискового запроса
// нужно для OALD10, чтобы, например, по запросу 'cesar' подматывалось к 'César Chávez'
// Approved and written by Semen Remizov
  SldU16String query(searchQuery);
  SldU16String searchResult(currentWord);
  if (query.length() > searchResult.length())
    query = query.substr(0, searchResult.length());
  else
    searchResult = searchResult.substr(0, query.length());
  if (!compareTablePtr->StrICmp(query, searchResult))
    return eOK;

// ищем слово с точностью до символов одного веса (Abandon -> abandon)
// при этом список подматывается к наиболее похожей статье
// если слова нет в словаре, то подмотка происходит к первой статье, большей или равной тексту запроса
  error = Dict->GetMostSimilarWordByText( searchQuery, &resultFlag );
  if ( error != eOK )
  {
    return error;
  }

// нужно проверить, совпадает ли бинарно найденное слово (отображаемый вариант или ключ сортировки) с запросом
  currentWord = NULL;
  error = Dict->GetCurrentWord( showVariantIndex, &currentWord );
  if ( error != eOK )
  {
    return error;
  }

// если Show вариант совпадает с запросом, считаем, что слово нашлось
  if ( !compareTablePtr->StrICmp( currentWord, searchQuery ) )
  {
    *crossrefSuccessful = 1;
    return eOK;
  }

// если Show вариант не первый, значит список сортирован по какому то специфическому варианту написанию (например, ключу сортировки)
// проверим его
  if ( showVariantIndex != 0 )
  {
    error = Dict->GetCurrentWord( 0, &currentWord );
    if ( error != eOK )
    {
      return error;
    }

    if ( !compareTablePtr->StrICmp( currentWord, searchQuery ) )
    {
      *crossrefSuccessful = 1;
    }
  }

  return eOK;
}

jobject GetWordByText::native(
    JNIEnv* env,
    CSldDictionary* dictionary,
    jobject _listIndex,
    jobject _searchQuery,
    jobject _needExact,
    jobject _listVariant
) const noexcept
{
  jint listIndex = JavaObjects::GetInteger( env, _listIndex );
  jint listVariant = JavaObjects::GetInteger( env, _listVariant );
  JavaObjects::string searchQuery = JavaObjects::GetString( env, _searchQuery );
  jboolean needExact = JavaObjects::GetBoolean( env, _needExact );
  jint res = -1;

  if ( searchQuery.length() > 0u )
  {
    ESldError error = dictionary->SetCurrentWordlist( listIndex );
    if ( error == eOK )
    {
      UInt32 crossrefSuccessful = 0u;
      error = helper( dictionary, searchQuery.c_str(), &crossrefSuccessful, (UInt32) listVariant );
      if ( ( error == eOK ) && ( ( crossrefSuccessful ) || ( !needExact ) ) )
      {
        Int32 wordIndex;
        error = dictionary->GetCurrentIndex( &wordIndex );
        if ( error == eOK )
        {
          res = wordIndex;
        }
      }
    }
  }
  else
  {
    if ( !needExact )
    {
      res = 0;
    }
  }

  return JavaObjects::GetInteger( env, res );
}
