//
// Created by borbat on 13.09.2018.
//

#include "GetListType.h"
#include "java/java_objects.h"

jobject GetListType::native( JNIEnv* env, CSldDictionary* dictionary, jobject listNumber ) const noexcept
{
  UInt32 listType[2];
  ESldError error = dictionary->GetListUsage( JavaObjects::GetInteger( env, listNumber ), 0, listType );
  if ( error != eOK )
  {
    listType[0] = eWordListType_Unknown;
  }
  listType[1] = 0u; //TODO: get offset after engine update
  jintArray res = env->NewIntArray( 2u );
  env->SetIntArrayRegion( res, 0u, 2u, (jint*) listType );
  return res;
}
