//
// Created by borbat on 10.09.2018.
//

#ifndef NATIVE_ENGINE_GETDICTIONARYLOCALIZEDSTRINGS_H
#define NATIVE_ENGINE_GETDICTIONARYLOCALIZEDSTRINGS_H

#include "../dictionary_function.h"

class GetDictionaryLocalizedStrings : public CDictionaryFunction
{
  public:
    virtual jobject native_a( JNIEnv* env, CSldDictionary* dictionary, jobjectArray callbacks ) const noexcept override;
};

#endif //NATIVE_ENGINE_GETDICTIONARYLOCALIZEDSTRINGS_H
