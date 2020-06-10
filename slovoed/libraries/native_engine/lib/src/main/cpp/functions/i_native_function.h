//
// Created by IMZhdanov on 09.08.2018.
//

#ifndef NATIVE_FUNCTION_VA_H
#define NATIVE_FUNCTION_VA_H

#include <jni.h>
#include <android/log.h>

template < class T >
class INativeFunction
{
  public:
    virtual jobject call( JNIEnv* env, T instance, jobjectArray args ) const noexcept = 0;

    virtual ~INativeFunction()
    {

    };
};

#endif //NATIVE_FUNCTION_VA_H
