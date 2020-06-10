//
// Created by mamedov on 02.11.2018.
//

#ifndef NATIVE_ENGINE_GETGLOBALINDEXBYHISTORYELEMENT_H
#define NATIVE_ENGINE_GETGLOBALINDEXBYHISTORYELEMENT_H

#include "functions/dictionary_function.h"

class GetArticleIndexesByHistoryElement : public CDictionaryFunction
{
  protected:
    jobject native(
        JNIEnv* env,
        CSldDictionary* dictionary,
        jobject _historyElement
    ) const noexcept override;
};

#endif //NATIVE_ENGINE_GETGLOBALINDEXBYHISTORYELEMENT_H
