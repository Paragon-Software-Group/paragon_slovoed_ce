#include "env_aware.h"

void CEnvAware::SetEnv( JNIEnv* env )
{
  mEnv = env;
}

JNIEnv* CEnvAware::GetEnv( void ) const
{
  return mEnv;
}
