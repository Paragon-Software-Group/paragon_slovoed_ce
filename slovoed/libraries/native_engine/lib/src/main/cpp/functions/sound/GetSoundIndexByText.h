//
// Created by mamedov on 25.01.2019.
//

#ifndef NATIVE_ENGINE_GETSOUNDINDEXBYTEXT_H
#define NATIVE_ENGINE_GETSOUNDINDEXBYTEXT_H

#include "../dictionary_function.h"

class GetSoundIndexByText : public CDictionaryFunction
{
  protected:
    jobject native(
        JNIEnv* env,
        CSldDictionary* dictionary,
        jobject _text
    ) const noexcept override;

};

#endif //NATIVE_ENGINE_GETSOUNDINDEXBYTEXT_H
