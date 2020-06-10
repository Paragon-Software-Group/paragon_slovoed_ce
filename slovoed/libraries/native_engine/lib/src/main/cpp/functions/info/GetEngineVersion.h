//
// Created by borbat on 11.09.2018.
//

#ifndef NATIVE_ENGINE_GETENGINEVERSION_H
#define NATIVE_ENGINE_GETENGINEVERSION_H

#include "../byte_array_function.h"

class GetEngineVersion : public CByteArrayFunction
{
  protected:
    jobject native( JNIEnv* env, jbyteArray array ) const noexcept override;
};

#endif //NATIVE_ENGINE_GETENGINEVERSION_H
