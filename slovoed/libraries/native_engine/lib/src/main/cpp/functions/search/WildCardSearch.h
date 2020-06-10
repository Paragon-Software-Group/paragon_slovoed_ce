//
// Created by popov on 12.02.2019.
//

#ifndef NATIVE_ENGINE_WILDCARDSEARCH_H
#define NATIVE_ENGINE_WILDCARDSEARCH_H

#include "../dictionary_context_function.h"

class WildCardSearch : public CDictionaryContextFunction
{
  protected:
    jobject
    native(
        JNIEnv* env,
        CDictionaryContext* context,
        jobject _listIndex,
        jobject _word,
        jobject _maximumWords
    ) const noexcept override;
};

#endif //NATIVE_ENGINE_WILDCARDSEARCH_H
