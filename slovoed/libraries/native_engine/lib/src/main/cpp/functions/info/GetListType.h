//
// Created by borbat on 13.09.2018.
//

#ifndef NATIVE_ENGINE_GETLISTTYPE_H
#define NATIVE_ENGINE_GETLISTTYPE_H

#include "../dictionary_function.h"

class GetListType : public CDictionaryFunction
{
  protected:
    jobject
    native( JNIEnv* env, CSldDictionary* dictionary, jobject listNumber ) const noexcept override;
};

#endif //NATIVE_ENGINE_GETLISTTYPE_H
