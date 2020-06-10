//
// Created by borbat on 01.10.2018.
//

#ifndef NATIVE_ENGINE_TRANSLATE_H
#define NATIVE_ENGINE_TRANSLATE_H

#include "../dictionary_function.h"
#include "../dictionary_context_function.h"

class Translate : public CDictionaryContextFunction
{
  protected:
    jobject native(
        JNIEnv* env,
        CDictionaryContext* context,
        jobject _listIndex,
        jobject _localIndex,
        jobject _htmlParams
    ) const noexcept override;
};

#endif //NATIVE_ENGINE_TRANSLATE_H
