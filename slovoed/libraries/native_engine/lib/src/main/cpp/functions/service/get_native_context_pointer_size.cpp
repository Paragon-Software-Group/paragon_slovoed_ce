#include "get_native_context_pointer_size.h"

#include "context/dictionary_context.h"
#include "java/java_objects.h"

jobject GetNativeContextPointerSize::native( JNIEnv* env, jbyteArray array ) const noexcept
{
  return JavaObjects::GetInteger( env, (int) sizeof( CDictionaryContext* ) );
}
