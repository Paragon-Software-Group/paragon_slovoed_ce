//
// Created by IMZhdanov on 09.08.2018.
//
#include <jni.h>
#include <android/log.h>
#include <functions/sound/ConvertSpx.h>

#include "java/java_objects.h"
#include "java/java_callback.h"
#include "functions.h"

#include "functions/all_functions.h"

namespace
{
const char class_path_name[] = "com/paragon_software/engine/nativewrapper/NativeFunctions";

std::vector< INativeFunction< jbyteArray >* > LoadAllFunctions()
{
  return {
      //service private
      new GetNativeContextPointerSize,
      new GetEngineVersion,
      new OpenSdcViaJavaReader,
      new OpenSdcViaDescriptor,
      new CloseSdc,
      new AddMorphoViaJavaReader,
      new AddMorphoViaDescriptor,

      //service public
      new SwitchDirection,

      //info
      new GetDictionaryVersion,
      new GetListCount,
      new GetDictionaryLocalizedStrings,
      new GetListType,
      new GetListLanguages,
      new GetListLocalizedStrings,
      new GetListVariants,

      //fill
      new GetWordCount,
      new GetWordByLocalIndex,
      new GetRealWordLocation,
      new GetPathFromGlobalIndex,
      new GetPreloadedFavorites,
      new GetCurrentWordStylizedVariant,
      new GetExternalImage,

      //search
      new ResetList,
      new GetWordByText,
      new FullTextSearch,
      new SpellingSearch,
      new WildCardSearch,
      new AnagramSearch,
      new FuzzySearch,
      new DeleteAllSearchLists,
      new GetBaseForms,
      new GetWordReferenceInList,
      new GetArticleLinks,
      new GetQueryReferenceInPhrase,
      new GetWordReference,
      new GetHighlightWordReferenceIncludingWhitespace,
      new GetQueryHighlightData,

      //article
      new Translate,
      new GetHistoryElementByGlobalIndex,
      new GetArticleIndexesByHistoryElement,
      new Swipe,
      new StartsWith,

      //sound
      new GetSoundIndexByWordGlobalIndex,
      new GetSoundIndexByText,
      new PlaySoundByIndex,
      new ConvertSpx
  };
}

jclass gClass = nullptr;
jclass gNativeArguments = nullptr;

static jobject NativeDictionaryCall(
    JNIEnv* env,
    jclass thisClass,
    jint id,
    jbyteArray nativeContextPointer,
    jobjectArray args
) noexcept
{
  if ( static_cast<unsigned int>(id) >= NativeFunctions::GetCount() ||
      JNI_TRUE != env->IsInstanceOf( args, gNativeArguments ) )
  {
    __android_log_print( ANDROID_LOG_ERROR, "NativeEngine", "Unknown native" );
    return nullptr;
  }
  return NativeFunctions::Get( id )->call( env, nativeContextPointer, args );
}

static JNINativeMethod methods[] = {
    {"ncall", "(I[B[Ljava/lang/Object;)Ljava/lang/Object;", (void*) NativeDictionaryCall}
};

}

jobject GetClassReference( JNIEnv* env, const char* name )
{
  jobject res = nullptr;
  jclass theClass = env->FindClass( name );
  if ( nullptr != theClass )
  {
    res = env->NewGlobalRef( theClass );
  }
  return res;
}

void RemoveReference( JNIEnv* env, jobject reference )
{
  if ( reference )
  {
    env->DeleteGlobalRef( reference );
  }
}

jint JNI_OnLoad( JavaVM* vm, void* )
{
  JNIEnv* env = nullptr;
  if ( vm->GetEnv( reinterpret_cast<void**>(&env), JNI_VERSION_1_6 ) != JNI_OK )
  {
    return -1;
  }
  if ( env->RegisterNatives( env->FindClass( class_path_name ), methods, sizeof( methods ) / sizeof( methods[0] ) )
      < 0 )
  {
    __android_log_print( ANDROID_LOG_ERROR, "NativeEngine", "Failed to register native methods" );
  }

  gClass = (jclass) GetClassReference( env, class_path_name );
  gNativeArguments = (jclass) GetClassReference( env, "[Ljava/lang/Object;" );

  JavaObjects::Init( env );
  JavaCallback::Init( env, class_path_name, "ncallback", "(Ljava/lang/Object;[Ljava/lang/Object;)Ljava/lang/Object;" );
  NativeFunctions::Init( LoadAllFunctions() );

  return JNI_VERSION_1_6;
}

void JNI_OnUnload( JavaVM* vm, void* )
{
  JNIEnv* env = nullptr;
  if ( vm->GetEnv( reinterpret_cast<void**>(&env), JNI_VERSION_1_6 ) == JNI_OK )
  {
    RemoveReference( env, gClass );
    RemoveReference( env, gNativeArguments );
    gClass = gNativeArguments = nullptr;
    JavaCallback::Uninit( env );
    JavaObjects::Uninit( env );
    NativeFunctions::Uninit();
  }
}
