//
// Created by popov on 14.10.2019.
//

#ifndef NATIVE_ENGINE_CONVERTSPX_H
#define NATIVE_ENGINE_CONVERTSPX_H

#include "../dictionary_function.h"
#include "../dictionary_context_function.h"

class ConvertSpx : public CDictionaryContextFunction
{
  protected:
    jobject native(
        JNIEnv* env,
        CDictionaryContext* context,
        jobject _byteArray
    ) const noexcept override;

};

#endif //NATIVE_ENGINE_CONVERTSPX_H
