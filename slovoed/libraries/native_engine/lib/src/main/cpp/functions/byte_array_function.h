#ifndef BYTE_ARRAY_FUNCTION_H
#define BYTE_ARRAY_FUNCTION_H

#include "i_native_function.h"
#include "native_function.h"

class CByteArrayFunction : public CNativeFunction<jbyteArray>, public INativeFunction<jbyteArray>
{
  public:
    virtual jobject call( JNIEnv* env, jbyteArray array, jobjectArray args ) const noexcept;
};

#endif //BYTE_ARRAY_FUNCTION_H
