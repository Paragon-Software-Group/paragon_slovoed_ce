//
// Created by Ivan M. Zhdanov on 16.06.18.
//

#ifndef SEARCH_ALL_DICTIONARY_FUNCTIONS_H
#define SEARCH_ALL_DICTIONARY_FUNCTIONS_H

#include <jni.h>
#include <android/log.h>

namespace
{
class IFunctions
{
  public:
    constexpr IFunctions() noexcept = default;

    virtual jobject native1( JNIEnv* env, const jobject arg1 ) const noexcept
    {
      __android_log_print( ANDROID_LOG_ERROR, "NativeEngine", "Not implemented native1" );
      return nullptr;
    }

    virtual jobject native2( JNIEnv* env, const jlong arg1, const jobject arg2, const jobject arg3 ) const noexcept
    {
      __android_log_print( ANDROID_LOG_ERROR, "NativeEngine", "Not implemented native2" );
      return nullptr;
    }
};
}
#endif //SEARCH_ALL_DICTIONARY_FUNCTIONS_H
