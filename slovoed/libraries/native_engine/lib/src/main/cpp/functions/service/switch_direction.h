//
// Created by kovtunov on 25.12.18.
//

#ifndef SLOVOED_SWITCH_DIRECTION_H
#define SLOVOED_SWITCH_DIRECTION_H

#include <functions/dictionary_function.h>

class SwitchDirection : public CDictionaryFunction {
protected:
    jobject
    native(
            JNIEnv* env,
            CSldDictionary* dictionary,
            jobject _listIndex,
            jobject _searchQuery
    ) const noexcept override;
};


#endif //SLOVOED_SWITCH_DIRECTION_H
