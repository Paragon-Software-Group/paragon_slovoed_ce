//
// Created by mamedov on 01.11.2018.
//

#ifndef NATIVE_ENGINE_GETHISTORYELEMENT_H
#define NATIVE_ENGINE_GETHISTORYELEMENT_H

#include "functions/dictionary_function.h"

class GetHistoryElementByGlobalIndex : public CDictionaryFunction
{
    protected:
    jobject native(
        JNIEnv* env,
        CSldDictionary* dictionary,
        jobject _listIndex,
        jobject _globalIndex
    ) const noexcept override;
};

#endif //NATIVE_ENGINE_GETHISTORYELEMENT_H
