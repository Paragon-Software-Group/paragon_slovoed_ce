//
// Created by Ivan M. Zhdanov on 18.06.18.
//

#ifndef SEARCH_ALL_DICTIONARY_LAYERACCESS_H
#define SEARCH_ALL_DICTIONARY_LAYERACCESS_H

#include <SldHTMLBuilder.h>
#include <java/java_objects.h>
#include "../env_aware.h"
#include "HtmlBuilder.h"
#include "SoundBuilder.h"


class LayerAccess final : public CEnvAware, public ISldLayerAccess
{
  private:
    UInt16 m_PlatformID[4];
    UInt8* m_SerialData;
    UInt32 m_SerialDataSize;
    UInt32 m_SerialDictID;
    JavaObjects::string cacheDir;

    DataAccess* m_pData;
    CSldMetadataParser* m_pParser;
    HtmlBuilder* m_pBuilder;
    SoundBuilder* m_pSoundBuilder;

  public:
    explicit LayerAccess(const JavaObjects::string &cacheDir);

    ~LayerAccess();

    void InitTranslation( CSldDictionary* dictionary, JNIEnv* env, jobject _htmlParams );

    const UInt16* GetTranslationHtml();

    const SoundBuilder* GetSoundBuilder();

    ESldError BuildTranslationRight(
        const CSldDictionary* aDictionary,
        const UInt16* aText,
        ESldTranslationModeType aTree
    ) override;

    ESldError BuildTranslationWrong(
        const CSldDictionary* aDictionary,
        const UInt16* aText,
        ESldTranslationModeType aTree
    ) override;

    ESldError WordFound( const ESldWordFoundCallbackType aCallbackType, const UInt32 aIndex ) override;

    const UInt16* GetPlatformID() override;

    ESldError SaveSerialData( UInt32 aDictID, const UInt8* aData, UInt32 aSize ) override;

    ESldError LoadSerialData( UInt32 aDictID, UInt8* aData, UInt32* aSize ) override;

    ESldError BuildSoundRight(
        const UInt8* aBlockPtr,
        UInt32 aBlockSize,
        UInt32 aPreviousSize,
        UInt32 aFrequency,
        UInt32 aFinishFlag
    ) override;

    ESldError BuildSoundWrong(
        const UInt8* aBlockPtr,
        UInt32 aBlockSize,
        UInt32 aPreviousSize,
        UInt32 aFrequency,
        UInt32 aFinishFlag
    ) override;

    ESldError LoadSoundByIndex( Int32 aSoundIndex, const UInt8** aDataPtr, UInt32* aDataSize ) override;

    ESldError LoadImageByIndex( Int32 aImageIndex, const UInt8** aDataPtr, UInt32* aDataSize ) override;
};

#endif //SEARCH_ALL_DICTIONARY_LAYERACCESS_H
