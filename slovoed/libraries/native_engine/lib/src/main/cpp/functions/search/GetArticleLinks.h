//
// Created by popov on 06.04.2020.
//

#ifndef NATIVE_ENGINE_GETARTICLELINKS_H
#define NATIVE_ENGINE_GETARTICLELINKS_H

#include "../dictionary_function.h"

class GetArticleLinks : public CDictionaryFunction
{
  protected:
  jobjectArray native(
        JNIEnv* env,
        CSldDictionary* dictionary,
        jobject listIndex,
        jobject wordIndex
    ) const noexcept override;
};

#endif //NATIVE_ENGINE_GETARTICLELINKS_H
