//
// Created by borbat on 11.09.2018.
//

#ifndef NATIVE_ENGINE_GETLISTVARIANTS_H
#define NATIVE_ENGINE_GETLISTVARIANTS_H

#include "../dictionary_function.h"

class GetListVariants : public CDictionaryFunction
{
  protected:
    jobject
    native( JNIEnv* env, CSldDictionary* dictionary, jobject listNumber ) const noexcept override;
};

#endif //NATIVE_ENGINE_GETLISTVARIANTS_H
