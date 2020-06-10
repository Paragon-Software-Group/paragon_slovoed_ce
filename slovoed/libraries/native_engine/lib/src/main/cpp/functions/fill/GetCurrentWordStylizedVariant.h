//
// Created by popov on 05.09.2019.
//

#ifndef NATIVE_ENGINE_GETCURRENTWORDSTYLIZEDVARIANT_H
#define NATIVE_ENGINE_GETCURRENTWORDSTYLIZEDVARIANT_H

#include "../dictionary_function.h"

class GetCurrentWordStylizedVariant : public CDictionaryFunction
{
  protected:
    jobject native(
        JNIEnv* env,
        CSldDictionary* dictionary,
        jobject aVariantIndex
    ) const noexcept override;
};

#endif //NATIVE_ENGINE_GETCURRENTWORDSTYLIZEDVARIANT_H
