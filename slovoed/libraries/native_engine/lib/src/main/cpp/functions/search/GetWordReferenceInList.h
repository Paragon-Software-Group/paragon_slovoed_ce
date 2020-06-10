//
// Created by ulyanov on 13.03.2019.
//

#ifndef SLOVOED_GETWORDREFERENCEINLIST_H
#define SLOVOED_GETWORDREFERENCEINLIST_H

#include "../dictionary_context_function.h"

class GetWordReferenceInList : public CDictionaryContextFunction
{
  protected:
    jobject
    native(
        JNIEnv* env,
        CDictionaryContext* context,
        jobject _listIndex,
        jobject _query
    ) const noexcept override;
};

#endif //SLOVOED_GETWORDREFERENCEINLIST_H
