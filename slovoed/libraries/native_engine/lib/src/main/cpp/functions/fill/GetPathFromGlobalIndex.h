//
// Created by borbat on 19.09.2018.
//

#ifndef NATIVE_ENGINE_GETPATHFROMGLOBALINDEX_H
#define NATIVE_ENGINE_GETPATHFROMGLOBALINDEX_H

#include "../dictionary_function.h"

class GetPathFromGlobalIndex : public CDictionaryFunction
{
  protected:
    jobject native(
        JNIEnv* env,
        CSldDictionary* dictionary,
        jobject _listIndex,
        jobject _globalIndex
    ) const noexcept override;
};

#endif //NATIVE_ENGINE_GETPATHFROMGLOBALINDEX_H
