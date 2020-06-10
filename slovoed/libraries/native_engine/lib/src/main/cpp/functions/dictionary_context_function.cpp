#include "dictionary_context_function.h"

CDictionaryContext* CDictionaryContextFunction::LoadContext( JNIEnv* env, jbyteArray array )
{
  CDictionaryContext* pDictionaryContext;
  env->GetByteArrayRegion( array, 0, sizeof( pDictionaryContext ), (jbyte*) &pDictionaryContext );
  pDictionaryContext->SetEnv( env );
  return pDictionaryContext;
}

jobject CDictionaryContextFunction::call( JNIEnv* env, jbyteArray array, jobjectArray args ) const noexcept
{
  return native_a( env, LoadContext( env, array ), args );
}
