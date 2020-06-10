//
// Created by borbat on 16.10.2018.
//

#ifndef NATIVE_ENGINE_GETWORDBYTEXT_H
#define NATIVE_ENGINE_GETWORDBYTEXT_H

#include "../dictionary_function.h"

class GetWordByText : public CDictionaryFunction
{
  protected:
    jobject
    native(
            JNIEnv* env,
            CSldDictionary* dictionary,
            jobject _listIndex,
            jobject _searchQuery,
            jobject _needExact,
            jobject _listVariant
    ) const noexcept override;
};

#endif //NATIVE_ENGINE_GETWORDBYTEXT_H
