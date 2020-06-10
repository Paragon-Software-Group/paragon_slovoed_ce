//
// Created by borbat on 11.09.2018.
//

#include "GetListLanguages.h"
#include "java/java_objects.h"

jobject GetListLanguages::native( JNIEnv* env, CSldDictionary* dictionary, jobject listIndex ) const noexcept
{
  int index = JavaObjects::GetInteger( env, listIndex );
  jint languages[] = {(jint) dictionary->GetListLanguageFrom( index ), (jint) dictionary->GetListLanguageTo( index )};
  jintArray res = env->NewIntArray( 2u );
  env->SetIntArrayRegion( res, 0u, 2u, languages );
  return res;
}
