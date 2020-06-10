//
// Created by borbat on 01.10.2018.
//

#ifndef NATIVE_ENGINE_DATAACCESS_H
#define NATIVE_ENGINE_DATAACCESS_H

#include <algorithm>
#include <unordered_map>
#include <set>

#include "SldHTMLBuilder.h"
#include "SldDictionary.h"

class DataAccess : public CSldBasicHTMLBuilder::IDataAccess
{
  public:
    std::set< UInt32 > usedStyles;
    CSldDictionary *dictionary;
    JavaObjects::string cacheDir;

    DataAccess( const CSldDictionary* pDictionary, const JavaObjects::string& cacheDir );

    void clear();

    SldU16StringRef GetStyleString( UInt32 styleId ) const;

    SldU16StringRef GetCSSStyleString( UInt32 styleId ) const;

    virtual SldU16String ResolveImageUrl( UInt32 aIndex ) override;

    virtual SldU16String ResolveImageUrl( SldU16StringRef aDictId, UInt32 aListId, SldU16StringRef aKey ) override;

    virtual SldU16StringRef StyleName( UInt32 aStyleId ) override;

    virtual void InjectCSSStyle( SldU16StringRef aStyleString ) override;

    virtual SldU16StringRef CSSClassName( const CSldMetadataProxyBase& aData ) override;

    virtual SldU16StringRef CSSClassName( const CSldMetadataProxy< eMetaMediaContainer >& aContainer ) override;

    virtual SldU16StringRef CSSClassName( const CSldMetadataProxy< eMetaBackgroundImage >& aImage ) override;

    virtual UInt32 getDefaultSwitchState( const sld2::html::StateTracker& state, ESldMetaSwitchThematicTypeEnum thematic ) override;

    void setMyViewSettings( bool hidePhonetics, bool hideExamples, bool hideImages,
                            bool hideIdioms, bool hidePhrase );

  private:
    SldU16String m_scratchString;
    std::unordered_map< UInt32, SldU16String > m_cssStyles;
    std::unordered_map< UInt32, SldU16String > m_htmlStyles;
    std::unordered_map< Int32, UInt32 > m_bgImageStyles;

    bool mHidePhonetics, mHideExamples, mHideImages, mHideIdioms, mHidePhrase;

    JavaObjects::string createImageFile(const TImageElement *aImageElement, UInt32 aExtension);
};

#endif //NATIVE_ENGINE_DATAACCESS_H
