//
// Created by borbat on 11.09.2018.
//

#ifndef NATIVE_ENGINE_GETLISTLANGUAGES_H
#define NATIVE_ENGINE_GETLISTLANGUAGES_H

#include "../dictionary_function.h"

class GetListLanguages : public CDictionaryFunction
{
  protected:
    jobject native( JNIEnv* env, CSldDictionary* dictionary, jobject listIndex ) const noexcept override;
};

#endif //NATIVE_ENGINE_GETLISTLANGUAGES_H
