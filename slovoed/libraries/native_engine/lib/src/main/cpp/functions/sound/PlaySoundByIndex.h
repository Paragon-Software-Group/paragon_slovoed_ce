//
// Created by mamedov on 30.01.2019.
//

#ifndef NATIVE_ENGINE_PLAYSOUNDBYINDEX_H
#define NATIVE_ENGINE_PLAYSOUNDBYINDEX_H

#include "../dictionary_function.h"
#include "../dictionary_context_function.h"

class PlaySoundByIndex : public CDictionaryContextFunction
{
  protected:
    jobject native(
        JNIEnv* env,
        CDictionaryContext* context,
        jobject _soundIndex
    ) const noexcept override;

};

#endif //NATIVE_ENGINE_PLAYSOUNDBYINDEX_H
