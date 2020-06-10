//
// Created by mamedov on 16.05.2019.
//

#include <SldDictionaryHelper.h>
#include "GetQueryReferenceInPhrase.h"
#include "java/java_objects.h"

jobject GetQueryReferenceInPhrase::native(
    JNIEnv* env,
    CDictionaryContext* context,
    jobject _listIndex,
    jobject _phrase,
    jobject _query
) const noexcept
{
  jint listIndex = JavaObjects::GetInteger( env, _listIndex );
  JavaObjects::string phrase = JavaObjects::GetString( env, _phrase );
  JavaObjects::string query = JavaObjects::GetString( env, _query );

  CSldDictionary* dictionary = context->GetDictionary();
  ESldLanguage languageFrom = dictionary->GetListLanguageFrom( listIndex );
  bool isInternalMorphoAvailable = context->IsInternalMorphoAvailable( languageFrom, CDictionaryContext::eFts );
  MorphoData* morphoData = nullptr;
  if ( !isInternalMorphoAvailable )
  {
    morphoData = context->GetExternalMorpho( languageFrom, CDictionaryContext::eFts );
  }

  CSldVector< MorphoDataType* > morphos;
  if ( morphoData )
  {
    morphos.push_back( morphoData );
  }

  CSldVector< UInt32 > wordRefs;
  CSldDictionaryHelper helper = CSldDictionaryHelper( *dictionary );
  ESldError error = helper.GetWordReferenceInQuery( SldU16StringRef( phrase.c_str() ), morphos, wordRefs, SldU16StringRef( query.c_str() ), isInternalMorphoAvailable );

  UInt32 wordReferencesCounter = ( error == eOK ) ? ( wordRefs.size() ) : 0;
  jintArray jData = env->NewIntArray( wordReferencesCounter );

  if ( jData && wordReferencesCounter > 0 )
  {
    jint* data = (jint*) env->GetPrimitiveArrayCritical( jData, nullptr );
    for ( int i = 0 ; i < wordReferencesCounter ; ++i )
    {
      data[i] = wordRefs[i];
    }
    env->ReleasePrimitiveArrayCritical( jData, data, 0 );
  }
  return jData;
}