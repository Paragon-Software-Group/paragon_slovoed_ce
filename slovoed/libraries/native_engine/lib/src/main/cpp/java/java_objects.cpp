#include <SldCompare.h>
#include "java_objects.h"

namespace JavaObjects
{

static jclass m_jBooleanClass = nullptr;
static jmethodID m_jBooleanClassCtor = nullptr;
static jmethodID m_jBooleanClassGet = nullptr;

static jclass m_jIntegerClass = nullptr;
static jmethodID m_jIntegerClassCtor = nullptr;
static jmethodID m_jIntegerClassGet = nullptr;

static jclass m_jLongClass = nullptr;
static jmethodID m_jLongClassCtor = nullptr;
static jmethodID m_jLongClassGet = nullptr;

static jclass m_jFloatClass = nullptr;
static jmethodID m_jFloatClassCtor = nullptr;
static jmethodID m_jFloatClassGet = nullptr;

static jclass m_jStringClass = nullptr;

void Init( JNIEnv* env )
{
  {
    jclass jClass = env->FindClass( "java/lang/Boolean" );
    m_jBooleanClass = (jclass) env->NewGlobalRef( jClass );
    env->DeleteLocalRef( jClass );
    m_jBooleanClassCtor = env->GetStaticMethodID( m_jBooleanClass, "valueOf", "(Z)Ljava/lang/Boolean;" );
    m_jBooleanClassGet = env->GetMethodID( m_jBooleanClass, "booleanValue", "()Z" );
  }

  {
    jclass jClass = env->FindClass( "java/lang/Integer" );
    m_jIntegerClass = (jclass) env->NewGlobalRef( jClass );
    env->DeleteLocalRef( jClass );
    m_jIntegerClassCtor = env->GetStaticMethodID( m_jIntegerClass, "valueOf", "(I)Ljava/lang/Integer;" );
    m_jIntegerClassGet = env->GetMethodID( m_jIntegerClass, "intValue", "()I" );
  }

  {
    jclass jClass = env->FindClass( "java/lang/Long" );
    m_jLongClass = (jclass) env->NewGlobalRef( jClass );
    env->DeleteLocalRef( jClass );
    m_jLongClassCtor = env->GetStaticMethodID( m_jLongClass, "valueOf", "(J)Ljava/lang/Long;" );
    m_jLongClassGet = env->GetMethodID( m_jLongClass, "longValue", "()J" );
  }

  {
    jclass jClass = env->FindClass( "java/lang/Float" );
    m_jFloatClass = (jclass) env->NewGlobalRef( jClass );
    env->DeleteLocalRef( jClass );
    m_jFloatClassCtor = env->GetStaticMethodID( m_jFloatClass, "valueOf", "(F)Ljava/lang/Float;" );
    m_jFloatClassGet = env->GetMethodID( m_jFloatClass, "floatValue", "()F" );
  }

  {
    jclass jClass = env->FindClass( "java/lang/String" );
    m_jStringClass = (jclass) env->NewGlobalRef( jClass );
    env->DeleteLocalRef( jClass );
  }
  JavaSoundData::Init(env);
  JavaLinkInfo::Init(env);
  JavaHtmlBuilderParams::Init(env);
  JavaFontUtils::Init(env);
  JavaPreloadedWordsCallback::Init(env);
}

void Uninit( JNIEnv* env )
{
  env->DeleteGlobalRef( m_jLongClass );
  env->DeleteGlobalRef( m_jIntegerClass );
  env->DeleteGlobalRef( m_jBooleanClass );
  env->DeleteGlobalRef( m_jStringClass );
  JavaSoundData::Uninit(env);
  JavaLinkInfo::Uninit(env);
  JavaHtmlBuilderParams::Uninit(env);
  JavaFontUtils::Uninit(env);
  JavaPreloadedWordsCallback::Uninit(env);
}

bool IsInstanceOfBoolean( JNIEnv* env, jobject object )
{
  return JNI_TRUE == env->IsInstanceOf( object, m_jBooleanClass );
}

bool IsInstanceOfInteger( JNIEnv* env, jobject object )
{
  return JNI_TRUE == env->IsInstanceOf( object, m_jIntegerClass );
}

bool IsInstanceOfLong( JNIEnv* env, jobject object )
{
  return JNI_TRUE == env->IsInstanceOf( object, m_jLongClass );
}

bool IsInstanceOfFloat( JNIEnv* env, jobject object )
{
  return JNI_TRUE == env->IsInstanceOf( object, m_jFloatClass );
}

bool IsInstanceOfString( JNIEnv* env, jobject object )
{
  return JNI_TRUE == env->IsInstanceOf( object, m_jStringClass );
}

//JavaObject&& JavaObjectFactory::getBoolean( JNIEnv* env, jboolean value ) const noexcept
//{
//  return std::move( JavaObject( env, env->CallStaticObjectMethod( m_jBooleanClass, m_jBooleanClassCtor, value ) ) );
//}
jobject GetBoolean( JNIEnv* env, jboolean value )
{
  return env->CallStaticObjectMethod( m_jBooleanClass, m_jBooleanClassCtor, value );
}

jboolean GetBoolean( JNIEnv* env, jobject object )
{
  if ( IsInstanceOfBoolean( env, object ) )
  {
    return env->CallBooleanMethod( object, m_jBooleanClassGet );
  }
  return JNI_FALSE;
}

jboolean ReleaseBoolean( JNIEnv* env, jobject object )
{
  jboolean res = GetBoolean( env, object );
  env->DeleteLocalRef( object );
  return res;
}

//JavaObject&& JavaObjectFactory::getInteger( JNIEnv* env, jint value ) const noexcept
//{
//  return std::move( JavaObject( env, env->CallStaticObjectMethod( m_jIntegerClass, m_jIntegerClassCtor, value ) ) );
//}
jobject GetInteger( JNIEnv* env, jint value )
{
  return env->CallStaticObjectMethod( m_jIntegerClass, m_jIntegerClassCtor, value );
}

jint GetInteger( JNIEnv* env, jobject object )
{
  if ( IsInstanceOfInteger( env, object ) )
  {
    return env->CallIntMethod( object, m_jIntegerClassGet );
  }
  return -1;
}

jint ReleaseInteger( JNIEnv* env, jobject object )
{
  jint res = GetInteger( env, object );
  env->DeleteLocalRef( object );
  return res;
}

//JavaObject&& JavaObjectFactory::getLong( JNIEnv* env, jlong value ) const noexcept
//{
//  return std::move( JavaObject( env, env->CallStaticObjectMethod( m_jLongClass, m_jLongClassCtor, value ) ) );
//}
jobject GetLong( JNIEnv* env, jlong value )
{
  return env->CallStaticObjectMethod( m_jLongClass, m_jLongClassCtor, value );
}

jlong GetLong( JNIEnv* env, jobject object )
{
  if ( IsInstanceOfLong( env, object ) )
  {
    return env->CallLongMethod( object, m_jLongClassGet );
  }
  return -1;
}

jlong ReleaseLong( JNIEnv* env, jobject object )
{
  jlong res = GetLong( env, object );
  env->DeleteLocalRef( object );
  return res;
}

jobject GetFloat( JNIEnv* env, jfloat value )
{
  return env->CallStaticObjectMethod( m_jFloatClass, m_jFloatClassCtor, value );
}

jfloat GetFloat( JNIEnv* env, jobject object )
{
  if ( IsInstanceOfFloat( env, object ) )
  {
    return env->CallFloatMethod( object, m_jFloatClassGet );
  }
  return -1;
}

jfloat ReleaseFloat( JNIEnv* env, jobject object )
{
  jfloat res = GetFloat( env, object );
  env->DeleteLocalRef( object );
  return res;
}

jstring GetString( JNIEnv* env, const UInt16* value )
{
//  UInt16 len = CSldCompare::StrUTF16_2_UTF8( nullptr, value );
//  mBuffer.resize( len );
//  CSldCompare::StrUTF16_2_UTF8( mBuffer.data(), value );
//  return env->NewStringUTF( (char*) mBuffer.data() );
  return env->NewString( value, CSldCompare::StrLen( value ) );
}

string GetString( JNIEnv* env, jobject object )
{
  if ( IsInstanceOfString( env, object ) )
  {
    jstring str = (jstring) object;
    jsize len = env->GetStringLength( str );
    const jchar* buffer = env->GetStringCritical( str, nullptr );
    string res( (UInt16*) buffer, (size_t) len );
    env->ReleaseStringCritical( str, buffer );
    return res;
  }
  else
  {
    return string( (size_t) 0, (UInt16) ' ' );
  }
}

string ReleaseString( JNIEnv* env, jobject object )
{
  string res = GetString( env, object );
  env->DeleteLocalRef( object );
  return res;
}

}

namespace JavaSoundData
{
static jclass m_jSoundDataClass = nullptr;
static jmethodID m_jSoundDataClassCtor = nullptr;

void Init( JNIEnv* env )
{
  {
    jclass jClass = env->FindClass( "com/paragon_software/native_engine/data/SoundData" );
    m_jSoundDataClass = (jclass) env->NewGlobalRef( jClass );
    env->DeleteLocalRef( jClass );
    m_jSoundDataClassCtor = env->GetMethodID( m_jSoundDataClass, "<init>", "([BII)V" );
  }
}

void Uninit( JNIEnv* env )
{
  env->DeleteGlobalRef( m_jSoundDataClass );
}

jobject GetSoundData( JNIEnv* env, jbyteArray data, UInt32 frequency, UInt8 format )
{
  return env->NewObject( m_jSoundDataClass, m_jSoundDataClassCtor, data, frequency, format );
}

}

namespace JavaLinkInfo {
  static jclass m_jLinkInfoClass = nullptr;
  static jmethodID m_jLinkInfoaClassCtor = nullptr;

  void Init(JNIEnv *env) {
    {
      jclass jClass = env->FindClass("com/paragon_software/article_manager/LinkInfo");
      m_jLinkInfoClass = (jclass) env->NewGlobalRef(jClass);
      env->DeleteLocalRef(jClass);
      m_jLinkInfoaClassCtor = env->GetMethodID(m_jLinkInfoClass, "<init>",
                                               "(IILjava/lang/String;Ljava/lang/String;)V");
    }
  }

  void Uninit(JNIEnv *env) {
    env->DeleteGlobalRef(m_jLinkInfoClass);
  }

  jobject
  GetLinkInfo(JNIEnv *env, Int32 listId, Int32 wordIndex, jstring dictId, jstring key) {
    return env->NewObject(m_jLinkInfoClass, m_jLinkInfoaClassCtor,
                          listId,
                          wordIndex,
                          dictId,
                          key);
  }

}


namespace JavaHtmlBuilderParams
{
static jclass m_jHtmlBuilderParamsClass = nullptr;
static jmethodID m_jGetScale = nullptr;
static jmethodID m_jGetHorizontalPadding = nullptr;
static jmethodID m_jIsHidePhonetics = nullptr;
static jmethodID m_jIsHideExamples = nullptr;
static jmethodID m_jIsHideImages = nullptr;
static jmethodID m_jIsHideIdioms = nullptr;
static jmethodID m_jIsHidePhrase = nullptr;
static jmethodID m_jHiddenSoundIcons = nullptr;
static jmethodID m_hRemoveBodyMargin = nullptr;

void Init( JNIEnv* env )
{
  {
    jclass jClass = env->FindClass( "com/paragon_software/native_engine/HtmlBuilderParams" );
    m_jHtmlBuilderParamsClass = (jclass) env->NewGlobalRef( jClass );
    env->DeleteLocalRef( jClass );
    m_jGetScale = env->GetMethodID( m_jHtmlBuilderParamsClass, "getScale", "()F" );
    m_jGetHorizontalPadding = env->GetMethodID( m_jHtmlBuilderParamsClass, "getHorizontalPadding", "()F" );
    m_jIsHidePhonetics = env->GetMethodID( m_jHtmlBuilderParamsClass, "isHidePhonetics", "()Z" );
    m_jIsHideExamples = env->GetMethodID( m_jHtmlBuilderParamsClass, "isHideExamples", "()Z" );
    m_jIsHideImages = env->GetMethodID( m_jHtmlBuilderParamsClass, "isHideImages", "()Z" );
    m_jIsHideIdioms = env->GetMethodID( m_jHtmlBuilderParamsClass, "isHideIdioms", "()Z" );
    m_jIsHidePhrase = env->GetMethodID( m_jHtmlBuilderParamsClass, "isHidePhrase", "()Z" );
    m_jHiddenSoundIcons = env->GetMethodID( m_jHtmlBuilderParamsClass, "getHiddenSoundIcons", "()[Ljava/lang/String;" );
    m_hRemoveBodyMargin = env->GetMethodID(m_jHtmlBuilderParamsClass, "isRemoveBodyMargin",  "()Z");
  }
}

void Uninit( JNIEnv* env )
{
  env->DeleteGlobalRef( m_jHtmlBuilderParamsClass );
}

jfloat GetScale( JNIEnv* env, jobject jHtmlParams )
{
  return env->CallFloatMethod( jHtmlParams, m_jGetScale );
}

jfloat GetHorizontalPadding( JNIEnv* env, jobject jHtmlParams )
{
  return env->CallFloatMethod( jHtmlParams, m_jGetHorizontalPadding );
}

bool IsHidePhonetics( JNIEnv* env, jobject jHtmlParams )
{
  return env->CallBooleanMethod( jHtmlParams, m_jIsHidePhonetics );
}

bool IsHideExamples( JNIEnv* env, jobject jHtmlParams )
{
  return env->CallBooleanMethod( jHtmlParams, m_jIsHideExamples );
}

bool IsHideImages( JNIEnv* env, jobject jHtmlParams )
{
  return env->CallBooleanMethod( jHtmlParams, m_jIsHideImages );
}

bool IsHideIdioms( JNIEnv* env, jobject jHtmlParams )
{
  return env->CallBooleanMethod( jHtmlParams, m_jIsHideIdioms );
}

bool IsHidePhrase( JNIEnv* env, jobject jHtmlParams )
{
  return env->CallBooleanMethod( jHtmlParams, m_jIsHidePhrase );
}

jobjectArray HiddenSoundIcons( JNIEnv* env, jobject jHtmlParams)
{
   return (jobjectArray)(env->CallObjectMethod( jHtmlParams, m_jHiddenSoundIcons));
}

bool IsRemoveBodyMargin( JNIEnv* env, jobject jHtmlParams )
{
  return env->CallBooleanMethod( jHtmlParams, m_hRemoveBodyMargin );
}

}


namespace JavaFontUtils
{
static jclass m_jFontManagerClass = nullptr;
static jmethodID m_jGetAvailableFonts = nullptr;

static jclass m_jFontClass = nullptr;
static jmethodID m_jGetFontFilePath = nullptr;
static jmethodID m_jGetFontFamily = nullptr;
static jmethodID m_jIsSemiBold = nullptr;
static jmethodID m_jIsBold = nullptr;
static jmethodID m_jIsItalic= nullptr;

void Init( JNIEnv* env )
{
  {
    jclass jClass = env->FindClass( "com/paragon_software/utils_slovoed/font/FontsUtils" );
    m_jFontManagerClass = (jclass) env->NewGlobalRef( jClass );
    env->DeleteLocalRef( jClass );
    m_jGetAvailableFonts = env->GetStaticMethodID( m_jFontManagerClass, "getAvailableFonts", "()[Lcom/paragon_software/utils_slovoed/font/Font;" );
  }
  {
    jclass jClass = env->FindClass( "com/paragon_software/utils_slovoed/font/Font" );
    m_jFontClass = (jclass) env->NewGlobalRef( jClass );
    env->DeleteLocalRef( jClass );
    m_jGetFontFilePath = env->GetMethodID( m_jFontClass, "getFilePath", "()Ljava/lang/String;" );
    m_jGetFontFamily = env->GetMethodID( m_jFontClass, "getFamily", "()Ljava/lang/String;" );
    m_jIsBold = env->GetMethodID( m_jFontClass, "isBold", "()Z" );
    m_jIsSemiBold = env->GetMethodID( m_jFontClass, "isSemiBold", "()Z" );
    m_jIsItalic = env->GetMethodID( m_jFontClass, "isItalic", "()Z" );
  }
}

void Uninit( JNIEnv* env )
{
  env->DeleteGlobalRef( m_jFontManagerClass );
  env->DeleteGlobalRef( m_jFontClass );
}

jobjectArray GetAvailableFonts( JNIEnv* env )
{
  auto res = (jobjectArray)(env->CallStaticObjectMethod( m_jFontManagerClass, m_jGetAvailableFonts, 0));
  return res;
}

const char* GetFontFilePath( JNIEnv* env, jobject jFont )
{
  auto res = (jstring)(env->CallObjectMethod( jFont, m_jGetFontFilePath ));
  return env->GetStringUTFChars( res, nullptr );
}

const char* GetFontFamily( JNIEnv* env, jobject jFont )
{
  auto res = (jstring)(env->CallObjectMethod( jFont, m_jGetFontFamily ));
  return env->GetStringUTFChars( res, nullptr );
}

bool IsBold( JNIEnv* env, jobject jFont )
{
  return env->CallBooleanMethod( jFont, m_jIsBold );
}

bool IsSemiBold( JNIEnv* env, jobject jFont )
{
  return env->CallBooleanMethod( jFont, m_jIsSemiBold );
}

bool IsItalic( JNIEnv* env, jobject jFont )
{
  return env->CallBooleanMethod( jFont, m_jIsItalic );
}

}


namespace JavaPreloadedWordsCallback
{
static jclass m_jPreloadedWordsCallbackClass = nullptr;
static jmethodID m_jAddNewChildDirectory = nullptr;
static jmethodID m_jNavigateToUpDirectory = nullptr;
static jmethodID m_jAddArticleItem = nullptr;

void Init( JNIEnv* env )
{
  {
    jclass jClass = env->FindClass( "com/paragon_software/engine/rx/preloadedwords/PreloadedWordsNativeCallback" );
    m_jPreloadedWordsCallbackClass = (jclass) env->NewGlobalRef( jClass );
    env->DeleteLocalRef( jClass );
    m_jAddNewChildDirectory = env->GetMethodID( m_jPreloadedWordsCallbackClass, "addNewChildDirectory", "(Ljava/lang/String;)V" );
    m_jNavigateToUpDirectory = env->GetMethodID( m_jPreloadedWordsCallbackClass, "navigateToUpDirectory", "()V" );
    m_jAddArticleItem = env->GetMethodID( m_jPreloadedWordsCallbackClass, "addArticleItem", "(II)V" );
  }
}

void Uninit( JNIEnv* env )
{
  env->DeleteGlobalRef( m_jPreloadedWordsCallbackClass );
}

void AddNewChildDirectory( JNIEnv* env, jobject jCallback, jstring jName )
{
  env->CallVoidMethod( jCallback, m_jAddNewChildDirectory, jName );
}

void NavigateToUpDirectory( JNIEnv* env, jobject jCallback )
{
  env->CallVoidMethod( jCallback, m_jNavigateToUpDirectory );
}

void AddArticleItem( JNIEnv* env, jobject jCallback, Int32 listIndex, Int32 globalIndex )
{
  env->CallVoidMethod( jCallback, m_jAddArticleItem, listIndex, globalIndex );
}

}