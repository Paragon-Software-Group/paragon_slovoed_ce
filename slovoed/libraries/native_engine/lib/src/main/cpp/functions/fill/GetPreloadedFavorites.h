//
// Created by mamedov on 04.09.2019.
//

#ifndef SLOVOED_GETPRELOADEDFAVORITES_H
#define SLOVOED_GETPRELOADEDFAVORITES_H

#include "../dictionary_function.h"

class GetPreloadedFavorites : public CDictionaryFunction
{
  protected:
    jobject native(
        JNIEnv* env,
        CSldDictionary* dictionary,
        jobject _listIndex,
        jobject _callback
    ) const noexcept override;

    ESldError ReadWordsFromHierarchyList(
        JNIEnv* env,
        CSldDictionary &dict,
        jobject callback) const;
};

#endif //SLOVOED_GETPRELOADEDFAVORITES_H
