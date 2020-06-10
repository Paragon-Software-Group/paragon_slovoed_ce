#ifndef JAVA_CALLBACK_H
#define JAVA_CALLBACK_H

#include <jni.h>
#include <initializer_list>

namespace JavaCallback
{

void Init( JNIEnv* env, const char* className, const char* methodName, const char* methodSignature );

void Uninit( JNIEnv* env );

jobject Call( JNIEnv* env, jobject object );

jobject Call( JNIEnv* env, jobject object, std::initializer_list< jobject > args );

jobject Call( JNIEnv* env, jobject object, jobjectArray args );

}

#endif //JAVA_CALLBACK_H
