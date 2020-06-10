//
// Created by kovtunov on 14.02.19.
//

#ifndef SLOVOED_DIDYOUMEANSEARCH_H
#define SLOVOED_DIDYOUMEANSEARCH_H

#include <functions/dictionary_function.h>

class SpellingSearch : public CDictionaryFunction
{
protected:
    jobject
    native(
            JNIEnv* env,
            CSldDictionary* dictionary,
            jobject _listIndex,
            jobject _word,
            jobject _maximumWords
    ) const noexcept override;
};


#endif //SLOVOED_DIDYOUMEANSEARCH_H
