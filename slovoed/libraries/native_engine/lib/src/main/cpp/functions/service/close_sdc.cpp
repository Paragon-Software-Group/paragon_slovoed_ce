#include "close_sdc.h"

#include "../dictionary_context_function.h"
#include "java/java_objects.h"

jobject CloseSdc::native( JNIEnv* env, jbyteArray array ) const noexcept
{
  CDictionaryContext* pDictionaryContext = CDictionaryContextFunction::LoadContext( env, array );
  jint error = pDictionaryContext->Close();
  delete pDictionaryContext;
  pDictionaryContext = nullptr;
  env->SetByteArrayRegion( array, 0, sizeof( pDictionaryContext ), (jbyte*) &pDictionaryContext );
  return JavaObjects::GetInteger( env, error );
}
