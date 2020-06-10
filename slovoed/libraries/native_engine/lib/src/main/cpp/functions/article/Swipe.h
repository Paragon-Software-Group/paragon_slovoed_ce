
#ifndef NATIVE_ENGINE_SWIPE_H
#define NATIVE_ENGINE_SWIPE_H

#include "../dictionary_function.h"

class Swipe : public CDictionaryFunction
{
  protected:
    jobject
    native(
        JNIEnv* env,
        CSldDictionary* dictionary,
        jobject _listIndex,
        jobject _globalIndex,
        jobject _swipingDirection,
        jobject _swipingMode
    ) const noexcept override;
};

#endif //NATIVE_ENGINE_SWIPE_H
