#ifndef NATIVE_ENGINE_FUZZYSEARCH_H
#define NATIVE_ENGINE_FUZZYSEARCH_H

#include "../dictionary_context_function.h"

class FuzzySearch : public CDictionaryContextFunction
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

#endif //NATIVE_ENGINE_FUZZYSEARCH_H
