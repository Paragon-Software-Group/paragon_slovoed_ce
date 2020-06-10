//
// Created by Ivan M. Zhdanov on 18.06.18.
//

#ifndef SEARCH_ALL_DICTIONARY_LAYERACCESS_H
#define SEARCH_ALL_DICTIONARY_LAYERACCESS_H

#include <ISldLayerAccess.h>

template < class TRegisterData >
class LayerAccess final : public ISldLayerAccess
{
    TRegisterData& m_registerData;
    UInt16 m_id = 0;
    std::function< ESldError( const ESldWordFoundCallbackType, const UInt32 ) > m_onWordFound;
  public:
    LayerAccess( TRegisterData& registerData )
        : m_registerData( registerData )
    {}

    ESldError BuildTranslationRight(
        const CSldDictionary* aDictionary,
        const UInt16* aText,
        ESldTranslationModeType aTree
    ) override
    {
      UInt8 result[4096] = {0};
      CSldCompare::StrUTF16_2_UTF8( result, aText );
      __android_log_print(
          ANDROID_LOG_INFO,
          "LayerAccess",
          "BuildTranslationRight( %p, '%s', %d );",
          aDictionary,
          (const char*) result,
          aTree
      );
      return eOK;
    }

    ESldError BuildTranslationWrong(
        const CSldDictionary* aDictionary,
        const UInt16* aText,
        ESldTranslationModeType aTree
    ) override
    {
      UInt8 result[4096] = {0};
      CSldCompare::StrUTF16_2_UTF8( result, aText );
      __android_log_print(
          ANDROID_LOG_INFO,
          "LayerAccess",
          "BuildTranslationWrong( %p, '%s', %d );",
          aDictionary,
          (const char*) result,
          aTree
      );
      return eOK;
    }

    void setOnWordFound(
        const std::function< ESldError( const ESldWordFoundCallbackType, const UInt32 ) >& onWordFound
    )noexcept
    {
      m_onWordFound = onWordFound;
    }

    ESldError WordFound( const ESldWordFoundCallbackType aCallbackType, const UInt32 aIndex ) override
    {
//      __android_log_print( ANDROID_LOG_INFO, "LayerAccess", "WordFound( %d, 0x%X );", aCallbackType, aIndex );
      if ( m_onWordFound )
      {
        return m_onWordFound( aCallbackType, aIndex );
      }
      return eOK;
    }

    const UInt16* GetPlatformID() override
    {
      __android_log_print( ANDROID_LOG_INFO, "LayerAccess", "GetPlatformID();" );
      return &m_id;
    }

    ESldError SaveSerialData( UInt32 aDictID, const UInt8* aData, UInt32 aSize ) override
    {
      auto it = m_registerData.find( aDictID );
      if ( it != m_registerData.end() )
      {
        m_registerData.erase( it );
      }
      const auto result = m_registerData.emplace(
          aDictID,
          std::pair< std::unique_ptr< UInt8 >, size_t >{new UInt8[aSize], aSize}
      );
      __builtin_memcpy( &( *result.first->second.first ), aData, result.first->second.second );
//      __android_log_print( ANDROID_LOG_INFO, "LayerAccess", "SaveSerialData(0x%X, '%s', 0x%X);", aDictID, (const char*)aData, aSize);
      return eOK;
    }

    ESldError LoadSerialData( UInt32 aDictID, UInt8* aData, UInt32* aSize ) override
    {
//      __android_log_print( ANDROID_LOG_INFO, "LayerAccess", "LoadSerialData(0x%X, '%s', 0x%X);", aDictID, (const char*)aData, *aSize);
      auto it = m_registerData.find( aDictID );
      if ( it != m_registerData.end() )
      {
        if ( nullptr != aData )
        {
          __builtin_memcpy( aData, &( *it->second.first ), it->second.second );
        }
        *aSize = it->second.second;
        return eOK;
      }
      return eSNWrongLoadedData;
    }

    ESldError BuildSoundRight(
        const UInt8* aBlockPtr,
        UInt32 aBlockSize,
        UInt32 aPreviousSize,
        UInt32 aFrequency,
        UInt32 aFinishFlag
    ) override
    {
      __android_log_print( ANDROID_LOG_INFO, "LayerAccess", "BuildSoundRight();" );
      return eOK;
    }

    ESldError BuildSoundWrong(
        const UInt8* aBlockPtr,
        UInt32 aBlockSize,
        UInt32 aPreviousSize,
        UInt32 aFrequency,
        UInt32 aFinishFlag
    ) override
    {
      __android_log_print( ANDROID_LOG_INFO, "LayerAccess", "BuildSoundWrong();" );
      return eOK;
    }

    ESldError LoadSoundByIndex( Int32 aSoundIndex, const UInt8** aDataPtr, UInt32* aDataSize ) override
    {
      __android_log_print( ANDROID_LOG_INFO, "LayerAccess", "LoadSoundByIndex();" );
      return eOK;
    }

    ESldError LoadImageByIndex( Int32 aImageIndex, const UInt8** aDataPtr, UInt32* aDataSize ) override
    {
      __android_log_print( ANDROID_LOG_INFO, "LayerAccess", "LoadImageByIndex();" );
      return eOK;
    }
};

#endif //SEARCH_ALL_DICTIONARY_LAYERACCESS_H
