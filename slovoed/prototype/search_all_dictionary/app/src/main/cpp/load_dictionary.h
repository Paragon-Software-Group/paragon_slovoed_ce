//
// Created by Ivan M. Zhdanov on 16.06.18.
//

#ifndef SEARCH_ALL_DICTIONARY_LOAD_DICTIONARY_H
#define SEARCH_ALL_DICTIONARY_LOAD_DICTIONARY_H

#include "SldDefines.h"
#include "functions.h"
#include "java_objects.h"
#include "descriptor_reader.h"
#include "LayerAccess.h"
#include "dictionary_holder.h"

namespace
{
template < class TDictionaryStorage, class TRegisterData >
class LoadDictionary final : public IFunctions
{

    TDictionaryStorage& m_storage;
    TRegisterData& m_registerData;
    const JavaObjectFactory& m_objectFactory;
    const JavaFunction& m_dictionaryOptions;
    const JavaFunction& m_readerFunction;
  public:
    constexpr LoadDictionary(
        TDictionaryStorage& storage,
        TRegisterData& registerData,
        const JavaObjectFactory& objectFactory,
        const JavaFunction& dictionaryOptions,
        const JavaFunction& readerFunction
    )noexcept
        : m_storage( storage ),
          m_registerData( registerData ),
          m_objectFactory( objectFactory ),
          m_dictionaryOptions( dictionaryOptions ),
          m_readerFunction( readerFunction )
    {}

    jobject native1( JNIEnv* env, const jobject arg1 ) const noexcept override
    {
      std::unique_ptr< DictionaryHolder< TRegisterData > >
          dictionary
          {new DictionaryHolder< TRegisterData >( m_registerData, m_objectFactory, env, arg1, m_readerFunction )};
      if ( dictionary && dictionary->init() )
      {
        UInt32 dictID = ~0u;
        const CSldLocalizedString* strings = nullptr;
        if ( ESldError::eOK == dictionary->getDictionary().GetDictionaryID( &dictID, 0 )
            && ESldError::eOK == dictionary->getDictionary().GetLocalizedStrings( &strings ) )
        {
          const jlong id = static_cast<jlong>(dictID);
          const auto it = m_storage.find( id );
          if ( it != m_storage.end() )
          {
            dictionary->getDictionary().Close();
            return nullptr;
          }
          JavaObject productName;
          JavaObject dictionaryName;
          JavaObject dictionaryNameShort;
          JavaObject dictionaryClass;
          JavaObject dictionaryLanguagePair;
          JavaObject dictionaryLanguagePairShort;
          JavaObject authorName;

          const UInt32 langIndex = SLD_LANGUAGE_CODE('engl');

          const UInt16* buffer = nullptr;

          if ( eOK == strings->GetProductName( langIndex, &buffer ) )
          {
            productName = std::move( m_objectFactory.getString( env, buffer ) );
          }

          if ( eOK == strings->GetDictionaryName( langIndex, &buffer ) )
          {
            dictionaryName = std::move( m_objectFactory.getString( env, buffer ) );
          }

          if ( eOK == strings->GetDictionaryNameShort( langIndex, &buffer ) )
          {
            dictionaryNameShort = std::move( m_objectFactory.getString( env, buffer ) );
          }

          if ( eOK == strings->GetDictionaryClass( langIndex, &buffer ) )
          {
            dictionaryClass = std::move( m_objectFactory.getString( env, buffer ) );
          }

          if ( eOK == strings->GetDictionaryLanguagePair( langIndex, &buffer ) )
          {
            dictionaryLanguagePair = std::move( m_objectFactory.getString( env, buffer ) );
          }

          if ( eOK == strings->GetDictionaryLanguagePairShort( langIndex, &buffer ) )
          {
            dictionaryLanguagePairShort = std::move( m_objectFactory.getString( env, buffer ) );
          }
          if ( eOK == strings->GetAuthorName( langIndex, &buffer ) )
          {
            authorName = std::move( m_objectFactory.getString( env, buffer ) );
          }
          m_storage.emplace( id, std::move( dictionary ) );
          return m_dictionaryOptions.functionO(
              env, id, productName.release(),
              dictionaryName.release(),
              dictionaryNameShort.release(),
              dictionaryClass.release(),
              dictionaryLanguagePair.release(),
              dictionaryLanguagePairShort.release(),
              authorName.release()
          );
        }
        dictionary->getDictionary().Close();
      }
      return nullptr;
    }
};
}

#endif //SEARCH_ALL_DICTIONARY_LOAD_DICTIONARY_H
