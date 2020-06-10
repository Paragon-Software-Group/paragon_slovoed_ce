//
// Created by borbat on 11.09.2018.
//

#ifndef NATIVE_ENGINE_GETWORDCOUNT_H
#define NATIVE_ENGINE_GETWORDCOUNT_H

#include "../dictionary_function.h"

class GetWordCount : public CDictionaryFunction
{
  protected:
    jobject native( JNIEnv* env, CSldDictionary* dictionary, jobject listIndex ) const noexcept override;
};

#endif //NATIVE_ENGINE_GETWORDCOUNT_H
