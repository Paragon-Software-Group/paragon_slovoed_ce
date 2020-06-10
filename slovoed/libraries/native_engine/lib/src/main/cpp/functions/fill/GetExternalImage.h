//
// Created by popov on 17.09.2019.
//

#ifndef NATIVE_ENGINE_GETEXTERNALIMAGE_H
#define NATIVE_ENGINE_GETEXTERNALIMAGE_H

#include "../dictionary_function.h"

class GetExternalImage : public CDictionaryFunction
{
  protected:
    jobject native(
        JNIEnv* env,
        CSldDictionary* dictionary,
        jobject _listIndex,
        jobject _imgKey
    ) const noexcept override;
};

#endif //NATIVE_ENGINE_GETEXTERNALIMAGE_H
