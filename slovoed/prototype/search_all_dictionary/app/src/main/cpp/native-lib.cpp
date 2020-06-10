#include <jni.h>
#include <android/log.h>

#include <memory>
#include <unordered_map>

#include <SldDictionary.h>

#include "functions.h"
#include "load_dictionary.h"
#include "close_dictionary.h"
#include "search_full_text.h"
#include "java_objects.h"
#include "dictionary_holder.h"


//Dictionary objects
namespace
{
typedef std::unordered_map< jlong, std::pair<std::unique_ptr< UInt8 >, size_t> > TRegisterData;
typedef std::unordered_map< jlong, std::unique_ptr< DictionaryHolder<TRegisterData> > > TDictionaryStorage;
JavaFunction g_searchResult;
JavaFunction g_dictionaryOptions;
JavaFunction g_readerFunction;
JavaObjectFactory g_objectFactory;
TDictionaryStorage g_dictionaries;
TRegisterData g_registerData;
}

//Functions
namespace
{
const char classPathName[] = "com/paragon_software/engine/NativeFunctions";

constexpr IFunctions empty;
constexpr LoadDictionary< TDictionaryStorage, TRegisterData >
    loadDictionary( g_dictionaries, g_registerData, g_objectFactory, g_dictionaryOptions, g_readerFunction );
constexpr CloseDictionary< TDictionaryStorage > closeDictionary( g_dictionaries, g_objectFactory );

constexpr SearchFullText< TDictionaryStorage > searchFullText( g_dictionaries, g_objectFactory, g_searchResult );

constexpr IFunctions const* const functions[]{&empty, &loadDictionary, &closeDictionary, &searchFullText, &empty};

jobject native_1( JNIEnv* env, jclass, jint id, const jobject arg1 ) noexcept
{
  if ( id < 0 || id > sizeof( functions ) / sizeof( functions[0] ) )
  {
    id = 0;
  }
  return functions[id]->native1( env, arg1 );
}

jobject native_2( JNIEnv* env, jclass, jint id, const jlong arg1, const jobject arg2, const jobject arg3 ) noexcept
{
  if ( id < 0 || id > sizeof( functions ) / sizeof( functions[0] ) )
  {
    id = 0;
  }
  return functions[id]->native2( env, arg1, arg2, arg3 );
}

static JNINativeMethod methods[] = {
    {"native_1", "(ILjava/lang/Object;)Ljava/lang/Object;", (void*) native_1},
    {"native_2", "(IJLjava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;", (void*) native_2}
};

}

jint JNI_OnLoad( JavaVM* vm, void* )
{
  JNIEnv* env = nullptr;
  if ( vm->GetEnv( reinterpret_cast<void**>(&env), JNI_VERSION_1_6 ) != JNI_OK )
  {
    return -1;
  }
  if ( env->RegisterNatives( env->FindClass( classPathName ), methods, sizeof( methods ) / sizeof( methods[0] ) ) < 0 )
  {
    __android_log_print( ANDROID_LOG_ERROR, "NativeEngine", "Failed to register native methods" );
  }

  g_objectFactory.init( env );
  g_searchResult.init( env, classPathName, "searchResult", "(Ljava/lang/Object;Ljava/lang/String;)V", true );
  g_dictionaryOptions.init(
      env,
      classPathName,
      "initDictionaryOptions",
      "(JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/Object;",
      true
  );
  g_readerFunction.init( env, classPathName, "readerFunction", "(Ljava/lang/Object;II)Lcom/paragon_software/engine/NativeFunctions$ReadResult;", true );

  return JNI_VERSION_1_6;
}

void JNI_OnUnload( JavaVM* vm, void* )
{
  JNIEnv* env = nullptr;
  if ( vm->GetEnv( reinterpret_cast<void**>(&env), JNI_VERSION_1_6 ) == JNI_OK )
  {
    g_objectFactory.clean( env );
    g_searchResult.clean( env );
  }
}
