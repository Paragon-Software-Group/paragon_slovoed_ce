//
// Created by Ivan M. Zhdanov on 18.06.18.
//

#ifndef SEARCH_ALL_DICTIONARY_DICTIONARY_HOLDER_H
#define SEARCH_ALL_DICTIONARY_DICTIONARY_HOLDER_H

#include <SldDictionary.h>
#include <mutex>
#include "descriptor_reader.h"
#include "LayerAccess.h"

template < class TRegisterData >
class DictionaryHolder final
{
    mutable std::mutex mtx;
    mutable std::atomic_bool m_needStop{false};
    JNIEnv* m_env;
    std::unique_ptr< ISDCFile > m_reader;
    LayerAccess< TRegisterData > m_access;
    CSldDictionary m_dictionary;
  public:
    virtual ~DictionaryHolder()
    {
      m_dictionary.Close();
    }

    std::mutex& getMutex() const noexcept
    {
      return mtx;
    }

    std::atomic_bool& getNeedStop() const noexcept
    {
      return m_needStop;
    }

    DictionaryHolder(
        TRegisterData& dataStorage, const JavaObjectFactory& objectFactory,
        JNIEnv* env, const jobject arg1, const JavaFunction& readerFunction
    ) noexcept
        : m_env( env ), m_access( dataStorage )
    {
      if ( objectFactory.IsInstanceOfInteger( env, arg1 ) )
      {
        m_reader.reset( new DescriptorReader( objectFactory.getInteger( m_env, arg1 ) ) );
      }
      else
      {
        m_reader.reset( new InputStreamReader( m_env, 0, readerFunction, arg1 ) );
      }
    }

    void setEnv( JNIEnv* env ) noexcept
    {
      m_env = env;
    }

    bool init() noexcept
    {
      return ESldError::eOK == m_dictionary.Open( &( *m_reader ), &m_access );
    }

    const CSldDictionary& getDictionary() const noexcept
    {
      return m_dictionary;
    }

    CSldDictionary& getDictionary() noexcept
    {
      return m_dictionary;
    }

    const LayerAccess< TRegisterData >& getAccess() const noexcept
    {
      return m_access;
    }

    LayerAccess< TRegisterData >& getAccess()noexcept
    {
      return m_access;
    }
};

#endif //SEARCH_ALL_DICTIONARY_DICTIONARY_HOLDER_H
