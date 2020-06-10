//
// Created by mamedov on 16.05.2019.
//

#ifndef SLOVOED_GETQUERYREFERENCEINPHRASE_H
#define SLOVOED_GETQUERYREFERENCEINPHRASE_H

#include "../dictionary_context_function.h"

class GetQueryReferenceInPhrase : public CDictionaryContextFunction
{
  protected:
    jobject
    native(
        JNIEnv* env,
        CDictionaryContext* context,
        jobject _listIndex,
        jobject _phrase,
        jobject _query
    ) const noexcept override;
};

#endif //SLOVOED_GETQUERYREFERENCEINPHRASE_H
