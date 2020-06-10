//
// Created by borbat on 10.09.2018.
//

#ifndef NATIVE_ENGINE_GETDICTIONARYVERSION_H
#define NATIVE_ENGINE_GETDICTIONARYVERSION_H

#include "../dictionary_function.h"

class GetDictionaryVersion : public CDictionaryFunction
{
  protected:
    jobject native( JNIEnv* env, CSldDictionary* dictionary ) const noexcept override;

};

#endif //NATIVE_ENGINE_GETDICTIONARYVERSION_H
