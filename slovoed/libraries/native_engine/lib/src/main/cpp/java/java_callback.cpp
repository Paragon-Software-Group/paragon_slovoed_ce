#include "java_callback.h"

namespace JavaCallback
{

static jclass OBJECT_CLASS = nullptr;
static jclass CLASS = nullptr;
static jmethodID METHOD_ID = nullptr;

void Init( JNIEnv* env, const char* className, const char* methodName, const char* methodSignature )
{
  OBJECT_CLASS = (jclass) env->NewGlobalRef( env->FindClass( "java/lang/Object" ) );
  CLASS = (jclass) env->NewGlobalRef( env->FindClass( className ) );
  METHOD_ID = env->GetStaticMethodID( CLASS, methodName, methodSignature );
}

void Uninit( JNIEnv* env )
{
  env->DeleteGlobalRef( OBJECT_CLASS );
  env->DeleteGlobalRef( CLASS );
}

jobject Call( JNIEnv* env, jobject object )
{
  return Call( env, object, {} );
}

jobject Call( JNIEnv* env, jobject object, std::initializer_list< jobject > args )
{
  jobjectArray array = env->NewObjectArray( (jsize) args.size(), OBJECT_CLASS, nullptr );
  jint i = 0;
  for ( jobject arg : args )
  {
    env->SetObjectArrayElement( array, i++, arg );
  }
  jobject res = Call( env, object, array );
  env->DeleteLocalRef( array );
  return res;
}

jobject Call( JNIEnv* env, jobject object, jobjectArray args )
{
  return env->CallStaticObjectMethod( CLASS, METHOD_ID, object, args );
}

}
