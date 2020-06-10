//
// Created by borbat on 11.09.2018.
//

#include "GetEngineVersion.h"

#include "SldVersionInfo.h"

jobject GetEngineVersion::native( JNIEnv* env, jbyteArray array ) const noexcept
{
  jint versionAndBuild[] = {SLOVOED2_ENGINE_VERSION, SLOVOED2_ENGINE_BUILD};
  jintArray res = env->NewIntArray( 2u );
  env->SetIntArrayRegion( res, 0u, 2u, versionAndBuild );
  return res;
}
