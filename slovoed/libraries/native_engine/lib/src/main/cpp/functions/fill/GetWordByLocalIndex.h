//
// Created by borbat on 12.09.2018.
//

#ifndef NATIVE_ENGINE_GETWORDBYLOCALINDEX_H
#define NATIVE_ENGINE_GETWORDBYLOCALINDEX_H

#include "../dictionary_function.h"

class GetWordByLocalIndex : public CDictionaryFunction
{
  protected:
    jobject native(
        JNIEnv* env,
        CSldDictionary* dictionary,
        jobject listIndex,
        jobject wordIndex,
        jobject variantIndexArray,
        jobject wordArray
    ) const noexcept override;
};

#endif //NATIVE_ENGINE_GETWORDBYLOCALINDEX_H
