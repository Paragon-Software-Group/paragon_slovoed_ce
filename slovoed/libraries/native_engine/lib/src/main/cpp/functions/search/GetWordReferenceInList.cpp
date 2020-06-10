//
// Created by ulyanov on 13.03.2019.
//

#include <SldDictionaryHelper.h>
#include "GetWordReferenceInList.h"
#include "java/java_objects.h"

/**
 * @return array with data structure:<br>
 * <br>
 * [ [A_0] [B_0] [C_0] [D_0]   [A_1] [B_1] [C_1] [D_1]  ... [A_N] [B_N] [C_N] [D_N] ]<br>
 * <br>
 * A - SubstringStart<br>
 * B - SubstringEnd<br>
 * C - Type<br>
 * D - WordGlobalIndex<br>
 */
jobject GetWordReferenceInList::native(
    JNIEnv* env,
    CDictionaryContext* context,
    jobject _listIndex,
    jobject _query
) const noexcept
{
  const UInt32 subArrayLength = 4;
  ESldError error;
  jint listIndex = JavaObjects::GetInteger( env, _listIndex );
  JavaObjects::string query = JavaObjects::GetString( env, _query );

  CSldDictionary* dictionary = context->GetDictionary();
  CSldDictionaryHelper helper = CSldDictionaryHelper( *dictionary );
  ESldLanguage languageFrom = dictionary->GetListLanguageFrom( listIndex );
  bool isInternalMorphoAvailable = context->IsInternalMorphoAvailable( languageFrom, CDictionaryContext::eFts );
  CSldVector< TWordRefInfo > wordRefs;
  MorphoData* morphoData = nullptr;
  CSldVector< MorphoDataType* > morphos;

  if ( !isInternalMorphoAvailable )
  {
    morphoData = context->GetExternalMorpho( languageFrom, CDictionaryContext::eFts );
  }

  if ( morphoData )
  {
    morphos.push_back( morphoData );
  }

  error = helper.GetWordReferenceInList( SldU16StringRef( query.c_str() ), morphos, wordRefs, listIndex, isInternalMorphoAvailable );

  UInt32 wordReferencesCounter = ( error == eOK ) ? ( wordRefs.size() ) : 0;
  UInt32 resultReferencesCounter = 0;
  for ( int i = 0 ; i < wordReferencesCounter ; ++i )
  {
    resultReferencesCounter += wordRefs[i].Refs.size();
  }

  UInt32 jArrayLength = subArrayLength * resultReferencesCounter;
  jintArray jData = env->NewIntArray( jArrayLength );

  if ( jData && jArrayLength > 0 )
  {
    jint* data = (jint*) env->GetPrimitiveArrayCritical( jData, nullptr );
    UInt32 position = 0;
    for ( int i = 0 ; i < wordReferencesCounter ; ++i )
    {
      for ( int j = 0 ; j < wordRefs[i].Refs.size() ; ++j )
      {
        TWordRefInfo::type type = wordRefs[i].Refs[j].second;
        data[position] = wordRefs[i].StartPos;
        data[position + 1] = wordRefs[i].EndPos;
        data[position + 2] = static_cast<Int32>(type);
        data[position + 3] = wordRefs[i].Refs[j].first.WordIndex;
        position += subArrayLength;
      }
    }
    env->ReleasePrimitiveArrayCritical( jData, data, 0 );
  }
  return jData;
}
