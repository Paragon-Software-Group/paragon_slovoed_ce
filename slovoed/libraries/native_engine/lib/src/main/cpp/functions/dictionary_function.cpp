#include "dictionary_function.h"
#include "dictionary_context_function.h"

jobject CDictionaryFunction::call( JNIEnv* env, jbyteArray array, jobjectArray args ) const noexcept
{
  return native_a( env, CDictionaryContextFunction::LoadContext( env, array )->GetDictionary(), args );
}
