//
// Created by mamedov on 18.10.2019.
//

#ifndef SLOVOED_STARTSWITH_H
#define SLOVOED_STARTSWITH_H

#include "functions/dictionary_function.h"

class StartsWith : public CDictionaryFunction
{
  protected:
    jobject native(
        JNIEnv* env,
        CSldDictionary* dictionary,
        jobject _listIndex,
        jobject _globalIndex,
        jobject _text
    ) const noexcept override;
};

#endif //SLOVOED_STARTSWITH_H
