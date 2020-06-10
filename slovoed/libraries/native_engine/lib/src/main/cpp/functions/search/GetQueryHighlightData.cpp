//
// Created by popov on 15.07.2019.
//

#include "SldDictionaryHelper.h"
#include "context/dictionary_context.h"
#include "java/java_objects.h"
#include "GetQueryHighlightData.h"

jobject GetQueryHighlightData::native(
    JNIEnv* env,
    CDictionaryContext* context,
    jobject _listIndex,
    jobject _articleText,
    jobject _headWord,
    jobject _phrase,
    jobject _query



) const noexcept
{
  jint listIndex = JavaObjects::GetInteger( env, _listIndex );
  JavaObjects::string articleText = JavaObjects::GetString( env, _articleText );
  JavaObjects::string headWord = JavaObjects::GetString( env, _headWord );
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
  ESldError error = helper.GetQueryHighlightData(articleText.c_str(), phrase.c_str(), query.c_str(), headWord.c_str(), morphos, wordRefs, isInternalMorphoAvailable);
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