//
// Created by IMZhdanov on 09.08.2018.
//

#ifndef NATIVE_ENGINE_JAVA_OBJECTS_H
#define NATIVE_ENGINE_JAVA_OBJECTS_H

#include <jni.h>
#include <string>
#include <SldPlatform.h>

namespace JavaObjects
{
typedef std::basic_string< UInt16 > string;

void Init( JNIEnv* env );

void Uninit( JNIEnv* env );

bool IsInstanceOfBoolean( JNIEnv* env, jobject object );

bool IsInstanceOfInteger( JNIEnv* env, jobject object );

bool IsInstanceOfLong( JNIEnv* env, jobject object );

bool IsInstanceOfString( JNIEnv* env, jobject object );

jobject GetBoolean( JNIEnv* env, jboolean value );

jboolean GetBoolean( JNIEnv* env, jobject object );

jboolean ReleaseBoolean( JNIEnv* env, jobject object );

jobject GetInteger( JNIEnv* env, jint value );

jint GetInteger( JNIEnv* env, jobject object );

jint ReleaseInteger( JNIEnv* env, jobject object );

jobject GetLong( JNIEnv* env, jlong value );

jlong GetLong( JNIEnv* env, jobject object );

jlong ReleaseLong( JNIEnv* env, jobject object );

jobject GetFloat( JNIEnv* env, jfloat value );

jfloat GetFloat( JNIEnv* env, jobject object );

jfloat ReleaseFloat( JNIEnv* env, jobject object );

jstring GetString( JNIEnv* env, const UInt16* value );

string GetString( JNIEnv* env, jobject object );

string ReleaseString( JNIEnv* env, jobject object );

};


namespace JavaSoundData
{
void Init( JNIEnv* env );

void Uninit( JNIEnv* env );

jobject GetSoundData( JNIEnv* env, jbyteArray data, UInt32 frequency, UInt8 format );
};

namespace JavaLinkInfo
{
  void Init( JNIEnv* env );

  void Uninit( JNIEnv* env );

  jobject GetLinkInfo( JNIEnv* env, Int32 listId, Int32 wordId, jstring dictId, jstring key );
};

namespace JavaHtmlBuilderParams
{
void Init( JNIEnv* env );

void Uninit( JNIEnv* env );

jfloat GetScale( JNIEnv* env, jobject jHtmlParams );

jfloat GetHorizontalPadding( JNIEnv* env, jobject jHtmlParams );

bool IsHidePhonetics( JNIEnv* env, jobject jHtmlParams );

bool IsHideExamples( JNIEnv* env, jobject jHtmlParams );

bool IsHideImages( JNIEnv* env, jobject jHtmlParams );

bool IsHideIdioms( JNIEnv* env, jobject jHtmlParams );

bool IsHidePhrase( JNIEnv* env, jobject jHtmlParams );

jobjectArray HiddenSoundIcons ( JNIEnv* env, jobject jHtmlParams);

bool IsRemoveBodyMargin( JNIEnv* env, jobject jHtmlParams );
};


namespace JavaFontUtils
{
void Init( JNIEnv* env );

void Uninit( JNIEnv* env );

jobjectArray GetAvailableFonts( JNIEnv* env );

const char* GetFontFilePath( JNIEnv* env, jobject jFont );

const char* GetFontFamily( JNIEnv* env, jobject jFont );

bool IsBold( JNIEnv* env, jobject jFont );

bool IsSemiBold( JNIEnv* env, jobject jFont );

bool IsItalic( JNIEnv* env, jobject jFont );
};


namespace JavaPreloadedWordsCallback
{
void Init( JNIEnv* env );

void Uninit( JNIEnv* env );

void AddNewChildDirectory( JNIEnv* env, jobject jCallback, jstring jName );

void NavigateToUpDirectory( JNIEnv* env, jobject jCallback );

void AddArticleItem( JNIEnv* env, jobject jCallback, Int32 listIndex, Int32 globalIndex );

}

#endif //NATIVE_ENGINE_JAVA_OBJECTS_H
