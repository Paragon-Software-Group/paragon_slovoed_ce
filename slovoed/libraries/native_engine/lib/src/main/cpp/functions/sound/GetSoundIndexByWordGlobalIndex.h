//
// Created by mamedov on 14.02.2019.
//

#ifndef NATIVE_ENGINE_GETSOUNDINDEXBYWORDLOCALINDEX_H
#define NATIVE_ENGINE_GETSOUNDINDEXBYWORDLOCALINDEX_H

#include "../dictionary_function.h"

class GetSoundIndexByWordGlobalIndex : public CDictionaryFunction
{
  protected:
    jobject native(
        JNIEnv* env,
        CSldDictionary* dictionary,
        jobject _listIndex,
        jobject _globalIndex
    ) const noexcept override;

};

#endif //NATIVE_ENGINE_GETSOUNDINDEXBYWORDLOCALINDEX_H
