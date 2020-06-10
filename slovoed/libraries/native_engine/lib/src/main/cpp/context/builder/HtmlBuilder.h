//
// Created by borbat on 01.10.2018.
//

#ifndef NATIVE_ENGINE_HTMLBUILDER_H
#define NATIVE_ENGINE_HTMLBUILDER_H

#include <java/java_objects.h>
#include <map>
#include "SldHTMLBuilder.h"
#include "DataAccess.h"
#include "vector"

class HtmlBuilder : public CSldBasicHTMLBuilder
{
  public:
    HtmlBuilder( DataAccess* pData );

    void StartBuilding();

    void EndBuilding();

    const UInt16* GetHtml();

    CSldCSSUrlResolver* GetUrlResolver();

    void setScale( jfloat scale );

    void setMyViewSettings( bool hidePhonetics, bool hideExamples, bool hideImages,
                            bool hideIdioms, bool hidePhrase );

    void setHorizontalPadding( jfloat scale );

    void setRemoveBodyMargin( bool removeBodyMargin );

    void initFontFaces( JNIEnv* env );

    void setVisibilitySentencesSoundButton(JNIEnv* env, jobject _htmlParams );

    void SetLanguageCode(const ESldLanguage aLangCode) { m_LangCode = aLangCode; }

    void BuildSound(SldU16StringRef aLang, SldU16StringRef aExtKey, SldU16StringRef aDictId, std::string soundId, bool isClosing);

protected:
    virtual void append( SldU16StringRef aString ) override;
    virtual void addText(UInt32, SldU16StringRef) override;
    virtual void addPhonetics(UInt32, SldU16StringRef) override;
    virtual void addBlock(const CSldMetadataProxy<eMetaUiElement> &aUiElem) override;

private:
    static const char* mScript;
    JavaObjects::string mHtmlBuffer, mBodyBuffer, mFontFacesBuffer;
    DataAccess* mData;
    jfloat mScale;
    jfloat mHorizontalPadding;
    bool mRemoveBodyMargin;
    ESldLanguage			m_LangCode;

    JavaObjects::string m_CurSoundBlockLang;
    std::vector<JavaObjects::string> mHiddenSoundIcons;
    bool mIsPreviousIconHide = false;
    size_t m_SoundBlockStartPos;
    std::map<JavaObjects::string, JavaObjects::string> m_SoundBlockLangToBaseImgMap;

    bool isInLink() const { return isInsideMetaBlock(eMetaLink) ||
                                   isInsideMetaBlock(eMetaUrl) ||
                                   isInsideMetaBlock(eMetaPopupArticle); }
    void addTextHelper(UInt32, SldU16StringRef, bool);

    JavaObjects::string createSoundExtImgTag();
};

#endif //NATIVE_ENGINE_HTMLBUILDER_H
