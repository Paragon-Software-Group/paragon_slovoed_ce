//
// Created by Ivan M. Zhdanov on 17.06.18.
//

#ifndef SEARCH_ALL_DICTIONARY_SEARCH_FULL_TEXT_H
#define SEARCH_ALL_DICTIONARY_SEARCH_FULL_TEXT_H

#include <mutex>

#include "functions.h"

namespace
{
template < class TDictionaryStorage >
class SearchFullText final : public IFunctions
{
    const JavaObjectFactory& m_objectFactory;
    const JavaFunction& m_searchResult;
    const TDictionaryStorage& m_dictionaryStorage;
  public:
    constexpr SearchFullText(
        const TDictionaryStorage& dictionaryStorage,
        const JavaObjectFactory& objectFactory,
        const JavaFunction& searchResult
    ) noexcept
        : m_objectFactory( objectFactory ), m_searchResult( searchResult ), m_dictionaryStorage( dictionaryStorage )
    {}

    jobject native2( JNIEnv* env, const jlong arg1, const jobject arg2, const jobject arg3 ) const noexcept override
    {
      if ( arg1 < 0 )
      {
        return nullptr;
      }
      const auto it = m_dictionaryStorage.find( (size_t) arg1 );
      if ( it != m_dictionaryStorage.end() )
      {
        auto string = std::move( m_objectFactory.getString( env, arg2 ) );
        std::unique_ptr< UInt16 > buffer( new UInt16[( string.size() + 1 ) * sizeof( UInt16 )] );
        CSldCompare::StrUTF8_2_UTF16( buffer.get(), (const UInt8*) string.data() );
        bool bExit = false;
        {
          it->second->getNeedStop().store( true );
          std::lock_guard< std::mutex > lck( it->second->getMutex() );
          it->second->getNeedStop().store( false );
          auto& dictionary = it->second->getDictionary();
          it->second->setEnv( env );
          it->second->getAccess().setOnWordFound(
              [this, &it, &env, &dictionary, &arg3, &bExit](
                  const ESldWordFoundCallbackType callbackType,
                  const UInt32 index
              ) -> ESldError
              {
                if ( eWordFoundCallbackFound == callbackType || eWordFoundCallbackInterleave == callbackType )
                {
                  if ( ESldError::eOK == dictionary.GetWordByIndex( index ) )
                  {
                    UInt16* wordText = nullptr;
                    if ( ESldError::eOK == dictionary.GetCurrentWord( 0, &wordText ) )
                    {
                      m_searchResult.function( env, arg3, m_objectFactory.getString( env, wordText ).release() );
                    }
                  }
                }
                return it->second->getNeedStop().load() ? ESldError::eExceptionSearchStop : ESldError::eOK;
              }
          );
          Int32 maxId = 0;
          if ( eOK == dictionary.GetNumberOfLists( &maxId ) )
          {
            for ( Int32 id = 0 ; id < maxId && !it->second->getNeedStop().load() ; ++id )
            {
              dictionary.DoFullTextSearch( id, buffer.get(), 1000000 );
            }
          }
        }
      }
      return nullptr;
    }
};
}

#endif //SEARCH_ALL_DICTIONARY_SEARCH_FULL_TEXT_H
