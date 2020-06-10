#ifndef NATIVE_ENGINE_ANAGRAMSEARCH_H
#define NATIVE_ENGINE_ANAGRAMSEARCH_H

#include "../dictionary_context_function.h"

class AnagramSearch : public CDictionaryContextFunction
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

#endif //NATIVE_ENGINE_ANAGRAMSEARCH_H
