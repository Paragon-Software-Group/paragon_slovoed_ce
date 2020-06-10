//
// Created by popov on 15.07.2019.
//

#ifndef SLOVOED_GETQUERYHIGHLIGHTDATA_H
#define SLOVOED_GETQUERYHIGHLIGHTDATA_H

#include "functions/dictionary_context_function.h"

class GetQueryHighlightData : public CDictionaryContextFunction
{
  protected:
    jobject
    native(
        JNIEnv* env,
        CDictionaryContext* context,
        jobject _listIndex,
        jobject _articleText,
        jobject _headWord,
        jobject _phrase,
        jobject _query

    ) const noexcept override;
};

#endif //SLOVOED_GETQUERYHIGHLIGHTDATA_H
