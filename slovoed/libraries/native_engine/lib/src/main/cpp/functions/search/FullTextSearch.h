//
// Created by borbat on 14.09.2018.
//

#ifndef NATIVE_ENGINE_FULLTEXTSEARCH_H
#define NATIVE_ENGINE_FULLTEXTSEARCH_H

#include "../dictionary_context_function.h"

class FullTextSearch : public CDictionaryContextFunction
{
  protected:
    jobject
    native(
        JNIEnv* env,
        CDictionaryContext* context,
        jobject _listIndex,
        jobject _word,
        jobject _maximumWords,
        jobject _sortType
    ) const noexcept override;
};

#endif //NATIVE_ENGINE_FULLTEXTSEARCH_H
