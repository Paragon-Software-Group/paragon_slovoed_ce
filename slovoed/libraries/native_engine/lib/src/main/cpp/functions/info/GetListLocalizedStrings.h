//
// Created by borbat on 11.09.2018.
//

#ifndef NATIVE_ENGINE_GETLISTLOCALIZEDSTRINGS_H
#define NATIVE_ENGINE_GETLISTLOCALIZEDSTRINGS_H

#include "../dictionary_function.h"

class GetListLocalizedStrings : public CDictionaryFunction
{
  protected:
    jobject
    native( JNIEnv* env, CSldDictionary* dictionary, jobject listNumber, jobject callbacks ) const noexcept override;
};

#endif //NATIVE_ENGINE_GETLISTLOCALIZEDSTRINGS_H
