//
// Created by borbat on 20.09.2018.
//

#ifndef NATIVE_ENGINE_DELETEALLSEARCHLISTS_H
#define NATIVE_ENGINE_DELETEALLSEARCHLISTS_H

#include "../dictionary_function.h"

class DeleteAllSearchLists : public CDictionaryFunction
{
  protected:
    jobject
    native( JNIEnv* env, CSldDictionary* dictionary ) const noexcept override;
};

#endif //NATIVE_ENGINE_DELETEALLSEARCHLISTS_H
