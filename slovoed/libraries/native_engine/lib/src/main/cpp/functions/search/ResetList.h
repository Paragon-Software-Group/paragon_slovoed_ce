//
// Created by borbat on 20.09.2018.
//

#ifndef NATIVE_ENGINE_RESETLIST_H
#define NATIVE_ENGINE_RESETLIST_H

#include "../dictionary_function.h"

class ResetList : public CDictionaryFunction
{
  protected:
    jobject
    native(
        JNIEnv* env,
        CSldDictionary* dictionary,
        jobject _listIndex,
        jobject _hierarchyPath,
        jobject _pathLength
    ) const noexcept override;
};

#endif //NATIVE_ENGINE_RESETLIST_H
