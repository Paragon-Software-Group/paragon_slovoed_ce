//
// Created by kovtunov on 26.11.18.
//

#ifndef SLOVOED_MORPHOTEST_H
#define SLOVOED_MORPHOTEST_H

#include "../dictionary_context_function.h"


class GetBaseForms  : public CDictionaryContextFunction
{
protected:
    jobject
    native(
            JNIEnv* env,
            CDictionaryContext* dictionaryContext,
            jobject _direction,
            jobject _searchQuery
    ) const noexcept override;
};


#endif //SLOVOED_MORPHOTEST_H
