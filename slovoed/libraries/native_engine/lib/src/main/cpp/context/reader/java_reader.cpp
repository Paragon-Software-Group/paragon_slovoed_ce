#include "java_reader.h"
#include "java/java_callback.h"

JavaReader::JavaReader(
    JNIEnv* env,
    jobject isOpenedCallback,
    jobject readCallback,
    jobject getSizeCallback
) :
    mIsOpenedCallback( env->NewGlobalRef( isOpenedCallback ) ),
    mReadCallback( env->NewGlobalRef( readCallback ) ),
    mGetSizeCallback( env->NewGlobalRef( getSizeCallback ) )
{

}

JavaReader::~JavaReader()
{
  GetEnv()->DeleteGlobalRef( mIsOpenedCallback );
  GetEnv()->DeleteGlobalRef( mReadCallback );
  GetEnv()->DeleteGlobalRef( mGetSizeCallback );
}

Int8 JavaReader::IsOpened() const
{
  JNIEnv* env = GetEnv();
  return JavaObjects::ReleaseBoolean( env, JavaCallback::Call( env, mIsOpenedCallback ) );
}

UInt32 JavaReader::Read( void* aDestPtr, UInt32 aSize, UInt32 aOffset )
{
  JNIEnv* env = GetEnv();
  jobject offset = JavaObjects::GetLong( env, (jlong) (UInt64) aOffset );
  jbyteArray buffer = env->NewByteArray( aSize );
  UInt32 res =
      (UInt32) JavaObjects::ReleaseInteger( env, JavaCallback::Call( env, mReadCallback, {offset, buffer} ) );
  if ( res > 0 )
  {
    env->GetByteArrayRegion( buffer, 0, ( res > aSize ) ? aSize : res, (jbyte*) aDestPtr );
  }
  env->DeleteLocalRef( offset );
  env->DeleteLocalRef( buffer );
  return res;
}

UInt32 JavaReader::GetSize() const
{
  JNIEnv* env = GetEnv();
  return (UInt32) (unsigned long) JavaObjects::ReleaseLong( env, JavaCallback::Call( env, mGetSizeCallback ) );
}
