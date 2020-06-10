//
// Created by borbat on 12.09.2018.
//

#include "GetWordByLocalIndex.h"
#include "java/java_objects.h"

jobject GetWordByLocalIndex::native(
    JNIEnv* env,
    CSldDictionary* dictionary,
    jobject listIndex,
    jobject wordIndex,
    jobject variantIndexArray,
    jobject wordArray
) const noexcept
{
  Int32 globalIdx;
  jint listIdx = JavaObjects::GetInteger( env, listIndex );
  jint wordIdx = JavaObjects::GetInteger( env, wordIndex );
  ESldError error1 = dictionary->GetWordByIndex( listIdx, wordIdx );
  ESldError error2 = dictionary->GetCurrentGlobalIndex( listIdx, &globalIdx );
  jsize i, n = env->GetArrayLength( (jintArray) variantIndexArray );
  jint* variants = env->GetIntArrayElements( (jintArray) variantIndexArray, nullptr );
  if ( ( error1 == eOK ) && ( error2 == eOK ) )
  {
    for ( i = 0u ; i < n ; i++ )
    {
      ESldError error = eOK;
      UInt16* word = nullptr;
      jint variant = variants[i];
      if ( variant >= 0 )
      {
        error = dictionary->GetCurrentWord( listIdx, variant, &word );
      }
      if ( ( error == eOK ) && ( word != nullptr ) )
      {
        jstring str = JavaObjects::GetString( env, word );
        env->SetObjectArrayElement( (jobjectArray) wordArray, i, str );
        env->DeleteLocalRef( str );
      }
      else
      {
        env->SetObjectArrayElement( (jobjectArray) wordArray, i, nullptr );
      }
    }
  }
  else
  {
    for ( i = 0u ; i < n ; i++ )
    {
      env->SetObjectArrayElement( (jobjectArray) wordArray, i, nullptr );
    }
  }
  env->ReleaseIntArrayElements( (jintArray) variantIndexArray, variants, JNI_ABORT );
  return JavaObjects::GetInteger( env, globalIdx );
}
