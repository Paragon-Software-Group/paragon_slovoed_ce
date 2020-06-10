//
// Created by borbat on 10.09.2018.
//

#include "GetDictionaryVersion.h"

jobject GetDictionaryVersion::native( JNIEnv* env, CSldDictionary* dictionary ) const noexcept
{
  UInt32 version[2];
  ESldError error1 = dictionary->GetDictionaryMajorVersion( version );
  ESldError error2 = dictionary->GetDictionaryMinorVersion( version + 1 );
  if ( ( error1 != eOK ) || ( error2 != eOK ) )
  {
    version[0] = version[1] = 0u;
  }
  jintArray res = env->NewIntArray( 2u );
  env->SetIntArrayRegion( res, 0u, 2u, (jint*) version );
  return res;
}
