//
// Created by IMZhdanov on 09.08.2018.
//

#ifndef NATIVE_FUNCTION_H
#define NATIVE_FUNCTION_H

#include <jni.h>
#include <android/log.h>

template < class T >
class CNativeFunction
{
  public:
    virtual ~CNativeFunction()
    {

    }

  protected:
    virtual jobject native( JNIEnv* env, T ) const noexcept
    {
      __android_log_print( ANDROID_LOG_ERROR, "NativeEngine", "Not implemented native(0)" );
      return nullptr;
    }

    virtual jobject native( JNIEnv* env, T, const jobject arg1 ) const noexcept
    {
      __android_log_print( ANDROID_LOG_ERROR, "NativeEngine", "Not implemented native(1)" );
      return nullptr;
    }

    virtual jobject native( JNIEnv* env, T, const jobject arg1, const jobject arg2 ) const noexcept
    {
      __android_log_print( ANDROID_LOG_ERROR, "NativeEngine", "Not implemented native(2)" );
      return nullptr;
    }

    virtual jobject native(
        JNIEnv* env, T, const jobject arg1, const jobject arg2,
        const jobject arg3
    ) const noexcept
    {
      __android_log_print( ANDROID_LOG_ERROR, "NativeEngine", "Not implemented native(3)" );
      return nullptr;
    }

    virtual jobject
    native(
        JNIEnv* env, T, const jobject arg1, const jobject arg2, const jobject arg3,
        const jobject arg4
    ) const noexcept
    {
      __android_log_print( ANDROID_LOG_ERROR, "NativeEngine", "Not implemented native(4)" );
      return nullptr;
    }

    virtual jobject
    native(
        JNIEnv* env, T, const jobject arg1, const jobject arg2, const jobject arg3,
        const jobject arg4, const jobject arg5
    ) const noexcept
    {
      __android_log_print( ANDROID_LOG_ERROR, "NativeEngine", "Not implemented native(5)" );
      return nullptr;
    }

    virtual jobject
    native(
        JNIEnv* env, T, const jobject arg1, const jobject arg2, const jobject arg3,
        const jobject arg4, const jobject arg5, const jobject arg6
    ) const noexcept
    {
      __android_log_print( ANDROID_LOG_ERROR, "NativeEngine", "Not implemented native(6)" );
      return nullptr;
    }

    virtual jobject
    native(
        JNIEnv* env, T, const jobject arg1, const jobject arg2, const jobject arg3,
        const jobject arg4, const jobject arg5, const jobject arg6,
        const jobject arg7
    ) const noexcept
    {
      __android_log_print( ANDROID_LOG_ERROR, "NativeEngine", "Not implemented native(7)" );
      return nullptr;
    }

    virtual jobject
    native(
        JNIEnv* env, T, const jobject arg1, const jobject arg2, const jobject arg3,
        const jobject arg4, const jobject arg5, const jobject arg6, const jobject arg7,
        const jobject arg8
    ) const noexcept
    {
      __android_log_print( ANDROID_LOG_ERROR, "NativeEngine", "Not implemented native(8)" );
      return nullptr;
    }

    virtual jobject
    native(
        JNIEnv* env, T, const jobject arg1, const jobject arg2, const jobject arg3,
        const jobject arg4, const jobject arg5, const jobject arg6, const jobject arg7,
        const jobject arg8, const jobject arg9
    ) const noexcept
    {
      __android_log_print( ANDROID_LOG_ERROR, "NativeEngine", "Not implemented native(9)" );
      return nullptr;
    }

    virtual jobject
    native(
        JNIEnv* env, T, const jobject arg1, const jobject arg2, const jobject arg3,
        const jobject arg4, const jobject arg5, const jobject arg6, const jobject arg7,
        const jobject arg8, const jobject arg9, const jobject arg10
    ) const noexcept
    {
      __android_log_print( ANDROID_LOG_ERROR, "NativeEngine", "Not implemented native(10)" );
      return nullptr;
    }

  public:
    virtual jobject native_a( JNIEnv* env, T instance, jobjectArray args ) const noexcept
    {
      jsize arguments_count = env->GetArrayLength( args );
      switch ( arguments_count )
      {
        case 0:
          return native( env, instance );
        case 1:
          return native(
              env, instance, env->GetObjectArrayElement( args, 0 )
          );
        case 2:
          return native(
              env, instance, env->GetObjectArrayElement( args, 0 ), env->GetObjectArrayElement( args, 1 )
          );
        case 3:
          return native(
              env,
              instance,
              env->GetObjectArrayElement( args, 0 ),
              env->GetObjectArrayElement( args, 1 ),
              env->GetObjectArrayElement( args, 2 )
          );
        case 4:
          return native(
              env,
              instance,
              env->GetObjectArrayElement( args, 0 ),
              env->GetObjectArrayElement( args, 1 ),
              env->GetObjectArrayElement( args, 2 ),
              env->GetObjectArrayElement( args, 3 )
          );
        case 5:
          return native(
              env,
              instance,
              env->GetObjectArrayElement( args, 0 ),
              env->GetObjectArrayElement( args, 1 ),
              env->GetObjectArrayElement( args, 2 ),
              env->GetObjectArrayElement( args, 3 ),
              env->GetObjectArrayElement( args, 4 )
          );
        case 6:
          return native(
              env,
              instance,
              env->GetObjectArrayElement( args, 0 ),
              env->GetObjectArrayElement( args, 1 ),
              env->GetObjectArrayElement( args, 2 ),
              env->GetObjectArrayElement( args, 3 ),
              env->GetObjectArrayElement( args, 4 ),
              env->GetObjectArrayElement( args, 5 )
          );
        case 7:
          return native(
              env,
              instance,
              env->GetObjectArrayElement( args, 0 ),
              env->GetObjectArrayElement( args, 1 ),
              env->GetObjectArrayElement( args, 2 ),
              env->GetObjectArrayElement( args, 3 ),
              env->GetObjectArrayElement( args, 4 ),
              env->GetObjectArrayElement( args, 5 ),
              env->GetObjectArrayElement( args, 6 )
          );
        case 8:
          return native(
              env,
              instance,
              env->GetObjectArrayElement( args, 0 ),
              env->GetObjectArrayElement( args, 1 ),
              env->GetObjectArrayElement( args, 2 ),
              env->GetObjectArrayElement( args, 3 ),
              env->GetObjectArrayElement( args, 4 ),
              env->GetObjectArrayElement( args, 5 ),
              env->GetObjectArrayElement( args, 6 ),
              env->GetObjectArrayElement( args, 7 )
          );
        case 9:
          return native(
              env,
              instance,
              env->GetObjectArrayElement( args, 0 ),
              env->GetObjectArrayElement( args, 1 ),
              env->GetObjectArrayElement( args, 2 ),
              env->GetObjectArrayElement( args, 3 ),
              env->GetObjectArrayElement( args, 4 ),
              env->GetObjectArrayElement( args, 5 ),
              env->GetObjectArrayElement( args, 6 ),
              env->GetObjectArrayElement( args, 7 ),
              env->GetObjectArrayElement( args, 8 )
          );
        case 10:
          return native(
              env,
              instance,
              env->GetObjectArrayElement( args, 0 ),
              env->GetObjectArrayElement( args, 1 ),
              env->GetObjectArrayElement( args, 2 ),
              env->GetObjectArrayElement( args, 3 ),
              env->GetObjectArrayElement( args, 4 ),
              env->GetObjectArrayElement( args, 5 ),
              env->GetObjectArrayElement( args, 6 ),
              env->GetObjectArrayElement( args, 7 ),
              env->GetObjectArrayElement( args, 8 ),
              env->GetObjectArrayElement( args, 9 )
          );
        default:
          __android_log_print(
              ANDROID_LOG_ERROR, "NativeEngine", "Not implemented native(%d)",
              arguments_count
          );
          return nullptr;
      }
    }
};

#endif //NATIVE_FUNCTION_H
