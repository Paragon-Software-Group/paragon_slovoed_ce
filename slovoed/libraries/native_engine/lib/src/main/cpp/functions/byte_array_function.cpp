#include "byte_array_function.h"

jobject CByteArrayFunction::call( JNIEnv* env, jbyteArray array, jobjectArray args ) const noexcept
{
  return native_a(env, array, args);
}
