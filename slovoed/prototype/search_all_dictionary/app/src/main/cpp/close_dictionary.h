//
// Created by Ivan M. Zhdanov on 17.06.18.
//

#ifndef SEARCH_ALL_DICTIONARY_CLOSE_DICTIONARY_H
#define SEARCH_ALL_DICTIONARY_CLOSE_DICTIONARY_H

#include "functions.h"
#include "java_objects.h"

namespace
{
template < class TDictionaryStorage >
class CloseDictionary final : public IFunctions
{
    TDictionaryStorage& m_storage;
    const JavaObjectFactory& m_objectFactory;
  public:
    constexpr CloseDictionary(
        TDictionaryStorage& storage,
        const JavaObjectFactory& objectFactory
    ) noexcept
        : m_storage( storage ), m_objectFactory( objectFactory )
    {}

    jobject native1( JNIEnv* env, const jobject arg1 ) const noexcept override
    {
      m_storage.erase( (size_t) m_objectFactory.getLong( env, arg1 ) );
      return nullptr;
    }
};
}

#endif //SEARCH_ALL_DICTIONARY_CLOSE_DICTIONARY_H
