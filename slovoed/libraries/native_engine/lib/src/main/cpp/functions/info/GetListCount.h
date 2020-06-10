//
// Created by borbat on 10.09.2018.
//

#ifndef NATIVE_ENGINE_GET_LIST_COUNT_H
#define NATIVE_ENGINE_GET_LIST_COUNT_H

#include "../dictionary_function.h"

class GetListCount : public CDictionaryFunction
{
  protected:
    jobject native( JNIEnv* env, CSldDictionary* dictionary ) const noexcept override;
};

#endif //NATIVE_ENGINE_GET_LIST_COUNT_H
