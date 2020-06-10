#ifndef GET_NATIVE_CONTEXT_POINTER_SIZE_H
#define GET_NATIVE_CONTEXT_POINTER_SIZE_H

#include "../byte_array_function.h"

class GetNativeContextPointerSize : public CByteArrayFunction
{
  protected:
    jobject native( JNIEnv* env, jbyteArray array ) const noexcept override;
};

#endif //GET_NATIVE_CONTEXT_POINTER_SIZE_H
