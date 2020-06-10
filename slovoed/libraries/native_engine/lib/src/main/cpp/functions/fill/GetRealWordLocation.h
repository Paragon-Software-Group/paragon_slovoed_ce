//
// Created by borbat on 19.09.2018.
//

#ifndef NATIVE_ENGINE_GETREALWORDLOCATION_H
#define NATIVE_ENGINE_GETREALWORDLOCATION_H

#include "../dictionary_function.h"

class GetRealWordLocation : public CDictionaryFunction
{
  protected:
    jobject native(
        JNIEnv* env,
        CSldDictionary* dictionary,
        jobject _listIndex,
        jobject _localIndex,
        jobject _outputArray
    ) const noexcept override;
};

#endif //NATIVE_ENGINE_GETREALWORDLOCATION_H
