//
// Created by popov on 31.05.19.
//

#ifndef SLOVOED_GET_WORD_REFERENCE_H
#define SLOVOED_GET_WORD_REFERENCE_H

#include <functions/dictionary_context_function.h>

class GetWordReference : public CDictionaryContextFunction {
protected:
    jobject
    native(
            JNIEnv* env,
            CDictionaryContext* context,
            jobject _listIndex,
            jobject _phrase

    ) const noexcept override;
};


#endif //SLOVOED_GET_WORD_REFERENCE_H
