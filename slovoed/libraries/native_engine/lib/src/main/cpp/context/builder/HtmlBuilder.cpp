//
// Created by borbat on 01.10.2018.
//

#include <sstream>
#include <utility>
#include "HtmlBuilder.h"

HtmlBuilder::HtmlBuilder( DataAccess* pData ) : CSldBasicHTMLBuilder( *pData )
{
  mData = pData;
  mScale = 1.0f;
  mHorizontalPadding = 0.0f;
  m_LangCode = SldLanguage::Unknown;
}

static void AppendStringHelper( JavaObjects::string& buffer, const char* toAdd )
{
    while ( *toAdd )
    {
        buffer.append( 1u, (UInt16) (unsigned char) *toAdd );
        toAdd++;
    }
}

static void AppendWStringHelper( JavaObjects::string& buffer, const wchar_t* toAdd )
{
    while ( *toAdd )
    {
        buffer.append( 1u, (UInt16)  *toAdd );
        toAdd++;
    }
}

static void toHtmlEscaped(std::wstring &data) {
    std::wstring buffer;
    buffer.reserve(data.size());
    for(size_t pos = 0; pos != data.size(); ++pos) {
        switch(data[pos]) {
            case '&':  buffer.append(L"&amp;");  break;
            case '\"': buffer.append(L"&quot;"); break;
            case '\'': buffer.append(L"&apos;"); break;
            case '<':  buffer.append(L"&lt;");   break;
            case '>':  buffer.append(L"&gt;");   break;
            default:   buffer.append(&data[pos], 1); break;
        }
    }
    data.swap(buffer);
}



static void replaceAll(std::wstring& str, const std::wstring& from, const std::wstring& to) {
    if(from.empty())
        return;
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
}

static void U16toWchar(const UInt16 *from, wchar_t *to, UInt32 size)
{
    for (int i = 0; i < size; ++i)
    {
        to[i] = from[i];
    }
}

void HtmlBuilder::append( SldU16StringRef aString )
{
  mBodyBuffer.append( aString.data(), aString.size() );
}


// std::to_string doesn't supported
template <typename T>
std::string to_string(T value)
{
    std::ostringstream os ;
    os << value ;
    return os.str() ;
}

void HtmlBuilder::addTextHelper(UInt32 aStyle, SldU16StringRef aText, bool phonetics)
{
    const bool makeCrossRefs = !isInLink() && !isInsideMetaBlock(eMetaHideControl) &&
                               !isInsideMetaBlock(eMetaSwitchControl) &&
                               !isInsideMetaBlock(eMetaPopupImage) &&
                               !isInsideMetaBlock(eMetaSound);

    const bool preNeeded = isInsideMetaBlock(eMetaNoBrText) || isInsideMetaBlock(eMetaPhonetics) || phonetics;

    AppendStringHelper(mBodyBuffer,"<span");

    if (makeCrossRefs)
    {
        AppendStringHelper(mBodyBuffer," sld-ref");
    }
    AppendStringHelper(mBodyBuffer," lang=\"");

    char temp[5] = {};
    memcpy(&temp[0],&m_LangCode,4u);
    AppendStringHelper(mBodyBuffer,temp);
    AppendStringHelper(mBodyBuffer,"\"");
    AppendStringHelper(mBodyBuffer," class='S");
    AppendStringHelper(mBodyBuffer,to_string(aStyle).c_str());
    AppendStringHelper(mBodyBuffer,"'");
    if (preNeeded)
        AppendStringHelper(mBodyBuffer," style='white-space:pre !important'");
    AppendStringHelper(mBodyBuffer,">");

    wchar_t wBuffer[aText.size()];
    U16toWchar(aText.data(), wBuffer, aText.size());

    std::wstring buffer(wBuffer,aText.size());
    toHtmlEscaped(buffer);

    replaceAll(buffer,std::wstring(L"\n"),std::wstring(L"<br>"));
    AppendWStringHelper(mBodyBuffer,buffer.data());
    AppendStringHelper(mBodyBuffer,"</span>");
}

void HtmlBuilder::addText(UInt32 aStyle, SldU16StringRef aText)
{
    if (!aText.empty())
        addTextHelper(aStyle, aText, false);
}

void HtmlBuilder::addPhonetics(UInt32 aStyle, SldU16StringRef aText)
{
    if (!aText.empty())
    {
        AppendStringHelper(mBodyBuffer, "<wbr/>");
        addTextHelper(aStyle, aText, true);
        AppendStringHelper(mBodyBuffer, "<wbr/>");
    }
}

void HtmlBuilder::StartBuilding()
{
  Clear();
  mData->clear();
  mBodyBuffer.clear();
}

static void AddStringHelper( JavaObjects::string& buffer, const char* toAdd )
{
  while ( *toAdd )
  {
    buffer.append( 1u, (UInt16) (unsigned char) *toAdd );
    toAdd++;
  }
  buffer.append( 1u, (UInt16) (unsigned char) '\n' );
}

static void AddSldStringHelper( JavaObjects::string& buffer, SldU16StringRef toAdd )
{
  buffer.append( toAdd.data(), toAdd.size() );
  buffer.append( 1u, (UInt16) (unsigned char) '\n' );
}

const char* GetSoundAnimStyle() {
    return "soundmarker {\n"
           "    transition: background-image 0.2s linear 0s;\n"
           "    background-size: contain;\n"
           "    display: block;\n"
           "    backface-visibility: hidden;\n"
           "    background-repeat: no-repeat;\n"
           "}";
}

const char* GetSoundAnimScript() {
    return "var playSldSoundMarker;\n"
           "var previousPlaySldSoundImg;\n"
           "function activateSoundImageElement(event)\n"
           "{\n"
           "    var img = event.target||event.srcElement;\n"
           "    if(img.tagName == \"SOUNDMARKER\") {\n"
           "      img = img.lastChild;\n"
           "    }\n"
           "    if(isSoundElement(img)) { \n"
           "      var soundMarker;\n"
           "      if(img.parentNode.tagName != \"SOUNDMARKER\") {\n"
           "        soundMarker = wrapSoundElement(img);\n"
           "      } else {\n"
           "        soundMarker = img.parentNode;\n"
           "      }\n"
           "      img.style.visibility = 'hidden';\n"
           "      soundMarker.parentNode.style.webkitTapHighlightColor = \"rgba(0, 0, 0, 0);\";\n"
           "      soundMarker.style.backgroundImage = 'url(' + img.src + ')';\n"
           "      soundMarker.style.width = img.style.width;\n"
           "      soundMarker.style.height = img.style.height;\n"
           "      if(playSldSoundMarker && previousPlaySldSoundImg) \n"
           "      {\n"
           "         returnSoundImageIcon();\n"
           "      }\n"
           "        playSldSoundMarker = soundMarker;\n"
           "        previousPlaySldSoundImg = img;\n"
           "    }\n"
           " }\n"
           "\n"
           " function wrapSoundElement(img) {\n"
           "     var soundMarker = document.createElement(\"soundmarker\");\n"
           "    img.parentNode.insertBefore(soundMarker, img);\n"
           "    soundMarker.appendChild(img);\n"
           "    return soundMarker;\n"
           " }\n"
           "\n"
           " function isSoundElement(element) {\n"
           "   var href;\n"
           "    if(element.parentNode.tagName != \"SOUNDMARKER\") {\n"
           "      href = element.parentNode.href;\n"
           "    } else {\n"
           "      href = element.parentNode.parentNode.href;\n"
           "    }\n"
           "\n"
           "    return href.split(':')[0]==\"sld-sound\";;\n"
           " }\n"
           "\n"
           "function setActiveSoundImageIcon(src)\n"
           "{\n"
           "  if(playSldSoundMarker == undefined)\n"
           "    return;\n"
           "  playSldSoundMarker.style.backgroundImage = 'url(' + src + ')';\n"
           "}\n"
           "\n"
           "function returnSoundImageIcon()\n"
           "{\n"
           "  if(playSldSoundMarker == undefined || previousPlaySldSoundImg == undefined)\n"
           "    return;\n"
           "  playSldSoundMarker.style.backgroundImage = 'url(' + previousPlaySldSoundImg.src + ')';\n"
           "  playSldSoundMarker = null;\n"
           "  previousPlaySldSoundImg.visibility = 'visible';\n"
           "  previousPlaySldSoundImg = null;\n"
           "}\n"
           "\n"
           //This method should be used when it is necessary to clear the animation state for all sld-sound icons in the article.
           //If this method will be renamed - do not forget to change it in java code
           "function clearSoundImageStates() \n"
           "{\n"
           "  var soundMarkers = document.getElementsByTagName(\"soundmarker\");\n"
           "  for(var i = 0; i < soundMarkers.length; i++) {\n"
           "    var element = soundMarkers[i];\n"
           "    element.style.backgroundImage = 'url(' + element.lastChild.src + ')';\n"
           "  }\n"
           "}\n"
           "\n"
           "function soundAnim_onInit(){\n"
           "    document.addEventListener('click', activateSoundImageElement, false);\n"
           "}";
}

const char* GetInitScript() {
    return "function Init(event){\n"
           "    soundAnim_onInit(event);\n"
           "    sld2_crossRef_onInit(event);\n"
           "}\n";
}

const char* GetPractisePronunciationButtonsScript()
{
    return "function appendInteractiveButtons(recordPath, playPath, stopPath)\n"
           " {\n"
           "  var uiElementsMarkers = document.getElementsByTagName(\"uielementmarker\");\n"
           "  \n"
           "  for (var i = 0; i < uiElementsMarkers.length; ++i) \n"
           "  {\n"
           "    var uiElement = uiElementsMarkers[i];\n"
           "    uiElement.appendChild(appendInteractiveButton(\"record:\" + i, recordPath, 35, false));\n"
           "    uiElement.appendChild(appendInteractiveButton(\"play:\" + i, playPath, 35, true));\n"
           "    uiElement.appendChild(appendInteractiveButton(\"stop:\" + i, stopPath, 35, true));\n"
           "  }\n"
           "}\n"
           "function appendInteractiveButton(id, path, size, hidden)\n"
           "{\n"
           "    var elem = document.createElement(\"img\");\n"
           "    elem.setAttribute(\"id\", id);\n"
           "    elem.setAttribute(\"src\", path);\n"
           "    elem.setAttribute(\"height\", size);\n"
           "    elem.setAttribute(\"width\", size);\n"
           "    if(hidden)\n"
           "    {\n"
           "        elem.style.display = 'none'\n"
           "    }\n"
           "    elem.addEventListener(\n"
           "        \"click\", \n"
           "        function()\n"
           "        { \n"
           "            if(window.Interface != null)\n"
           "            {\n"
           "                window.Interface.onClick(this.id);\n"
           "            }\n"
           "        })\n"
           "    return elem;\n"
           "}\n"
           "function showRecordButton(aId)\n"
           "{\n"
           "    var recordButton = document.getElementById(\"record:\"+aId.toString());\n"
           "    recordButton.style.display = 'inline';\n"
           "    var playButton = document.getElementById(\"play:\"+aId.toString());\n"
           "    playButton.style.display = 'none';\n"
           "    var stopButton = document.getElementById(\"stop:\"+aId.toString());\n"
           "    stopButton.style.display = 'none';\n"
           "}\n"
           "function showStopButton(aId)\n"
           "{\n"
           "    var recordButton = document.getElementById(\"record:\"+aId.toString());\n"
           "    recordButton.style.display = 'none';\n"
           "    var stopButton = document.getElementById(\"stop:\"+aId.toString());\n"
           "    stopButton.style.display = 'inline';\n"
           "    var playButton = document.getElementById(\"play:\"+aId.toString());\n"
           "    playButton.style.display = 'none';\n"
           "}\n"
           "function showPlayButton(aId)\n"
           "{\n"
           "    var recordButton = document.getElementById(\"record:\"+aId.toString());\n"
           "    recordButton.style.display = 'none';\n"
           "    var stopButton = document.getElementById(\"stop:\"+aId.toString());\n"
           "    stopButton.style.display = 'none';\n"
           "    var playButton = document.getElementById(\"play:\"+aId.toString());\n"
           "    playButton.style.display = 'inline';\n"
           "}\n";
}

void HtmlBuilder::EndBuilding()
{
  mHtmlBuffer.clear();
  AddStringHelper( mHtmlBuffer, "<HTML>" );
  AddStringHelper( mHtmlBuffer, "<HEAD>" );
  AddStringHelper( mHtmlBuffer, "<meta http-equiv='Content-Type' content='text/html; charset=UTF-8'>" );
  // Viewport 'width' parameter actually works like minimum supported width of page content layout.
  // If current WebView width is bigger when viewport width then the page content width would be stretched.
  // Setting small viewport width (like 1px) works well for article pages.
  std::string data = "<meta name='viewport' content='width=1, initial-scale=" + to_string(mScale) + ", maximum-scale=10.0, minimum-scale=0.1'>";
  AddStringHelper( mHtmlBuffer, data.c_str() );
  AddStringHelper( mHtmlBuffer, "<style type=\"text/css\" media=\"screen\">" );
  AddStringHelper( mHtmlBuffer, "a { text-decoration: none; }" );
  for ( UInt32 id : mData->usedStyles )
  {
    AddSldStringHelper( mHtmlBuffer, mData->GetStyleString( id ) );
  }
  for ( UInt32 id : m_state.usedStyles )
  {
    AddSldStringHelper( mHtmlBuffer, mData->GetStyleString( id ) );
  }
  for ( UInt32 id: m_state.usedCSSStyles )
  {
    AddSldStringHelper( mHtmlBuffer, mData->GetCSSStyleString( id ) );
  }
  mHtmlBuffer.append(mFontFacesBuffer);
  AddStringHelper( mHtmlBuffer, GetSoundAnimStyle() );
  AddStringHelper( mHtmlBuffer, "</style>" );
  AddStringHelper( mHtmlBuffer, "</HEAD>" );
  AddStringHelper( mHtmlBuffer, "<script type=\"text/javascript\">\n" );
  AddStringHelper( mHtmlBuffer, GetPractisePronunciationButtonsScript() );
  AddStringHelper( mHtmlBuffer, mScript );
  AddStringHelper( mHtmlBuffer, GetSoundAnimScript());
  AddSldStringHelper( mHtmlBuffer, GetCrossRefInitScript() );
  AddSldStringHelper( mHtmlBuffer, GetSearchHighlightScript() );
  AddSldStringHelper( mHtmlBuffer, GetSwitchUncoverScript() );
  AddSldStringHelper( mHtmlBuffer, GetHideControlScript() );
  AddSldStringHelper( mHtmlBuffer, GetSwitchControlScript() );
  AddSldStringHelper( mHtmlBuffer, GetCrossRefScript() );
    AddStringHelper( mHtmlBuffer, GetInitScript() );
  AddStringHelper( mHtmlBuffer, "</script>" );
  data = "<BODY  onload=\"Init(event);\" style= \"" + (mRemoveBodyMargin ? "margin:0;"  : ("padding-left:" + to_string(mHorizontalPadding) + "px; padding-right:" + to_string(mHorizontalPadding) + "px;")) + "\">";
  AddStringHelper( mHtmlBuffer, data.c_str());
  mHtmlBuffer.append( mBodyBuffer );
  AddStringHelper( mHtmlBuffer, "" );
  AddStringHelper( mHtmlBuffer, "</BODY>" );
  AddStringHelper( mHtmlBuffer, "</HTML>" );
}

const UInt16* HtmlBuilder::GetHtml()
{
  return mHtmlBuffer.c_str();
}

CSldCSSUrlResolver* HtmlBuilder::GetUrlResolver()
{
  return &m_data;
}

void HtmlBuilder::setScale( jfloat scale )
{
  mScale = scale;
}

void HtmlBuilder::setMyViewSettings(
    bool hidePhonetics,
    bool hideExamples,
    bool hideImages,
    bool hideIdioms,
    bool hidePhrase
)
{
  mData->setMyViewSettings(hidePhonetics, hideExamples, hideImages, hideIdioms, hidePhrase);
}

void HtmlBuilder::setHorizontalPadding( jfloat horizontalPadding )
{
    mHorizontalPadding = horizontalPadding;
}

void HtmlBuilder::setRemoveBodyMargin(bool removeBodyMargin)
{
    mRemoveBodyMargin = removeBodyMargin;
}

void HtmlBuilder::initFontFaces( JNIEnv* env )
{
  jobjectArray fonts = JavaFontUtils::GetAvailableFonts( env );
  jsize len = env->GetArrayLength(fonts);
  for ( int i = 0; i < len; i++) {
    jobject font = env->GetObjectArrayElement(fonts, i);
    const char* filePath = JavaFontUtils::GetFontFilePath(env, font);
    const char* family = JavaFontUtils::GetFontFamily(env, font);
    bool isBold = JavaFontUtils::IsBold(env, font);
    bool isSemiBold = JavaFontUtils::IsSemiBold(env, font);
    bool isItalic = JavaFontUtils::IsItalic(env, font);
    AppendStringHelper(mFontFacesBuffer, "@font-face {\tfont-family: '");
    AppendStringHelper(mFontFacesBuffer, family);
    AppendStringHelper(mFontFacesBuffer, "';\tsrc: url('file:///android_asset/");
    AppendStringHelper(mFontFacesBuffer, filePath);
    AppendStringHelper(mFontFacesBuffer, "')");
    if ( isSemiBold || isBold) {
      AppendStringHelper( mFontFacesBuffer, ";\tfont-weight: " );
      if ( isSemiBold ) {
        AppendStringHelper( mFontFacesBuffer, "600" );
      }
      else {
        AppendStringHelper( mFontFacesBuffer, "bold" );
      }
    }
    if (isItalic) {
      AppendStringHelper(mFontFacesBuffer, ";\tfont-style: ");
      AppendStringHelper(mFontFacesBuffer, "italic");
    }
    AppendStringHelper(mFontFacesBuffer, ";\tfont-variant: normal");
    AddStringHelper(mFontFacesBuffer, "}");
  }
}

void HtmlBuilder::setVisibilitySentencesSoundButton(JNIEnv *env, jobject _htmlParams) {
    jobjectArray HiddenSoundIcons = JavaHtmlBuilderParams::HiddenSoundIcons(env, _htmlParams);
    jsize len = env->GetArrayLength(HiddenSoundIcons);
    mHiddenSoundIcons.clear();
    for (int i = 0; i < len; i++) {
        mHiddenSoundIcons.push_back(
                JavaObjects::GetString(env, ((jobject) env->GetObjectArrayElement(HiddenSoundIcons, i))));
    }
}
void HtmlBuilder::BuildSound(SldU16StringRef aLang, SldU16StringRef aExtKey, SldU16StringRef aDictId, std::string soundId, bool isClosing)
{
  if ( isClosing )
  {
    std::map<JavaObjects::string, JavaObjects::string>::iterator it = m_SoundBlockLangToBaseImgMap.find(m_CurSoundBlockLang);
    JavaObjects::string imgBlockBuffer = mBodyBuffer.substr(m_SoundBlockStartPos);
    JavaObjects::string lang;
    if ( imgBlockBuffer.empty() )
    {
      if ( it != m_SoundBlockLangToBaseImgMap.end() && !it->second.empty() )
      {

          if (std::find(mHiddenSoundIcons.begin(), mHiddenSoundIcons.end(), it->first) == mHiddenSoundIcons.end()) {
              mBodyBuffer.append(it->second);
          } else {
              AppendStringHelper(lang, "enUK");
              mIsPreviousIconHide = it->first == lang;
          }
      }
      else
      {
        mBodyBuffer.append( createSoundExtImgTag() );
      }
    }
    else
    {
      if ( it == m_SoundBlockLangToBaseImgMap.end() || it->second.empty() )
      {
        m_SoundBlockLangToBaseImgMap[m_CurSoundBlockLang] = imgBlockBuffer;
      }
    }
    AppendStringHelper( mBodyBuffer, "</a>" );
    m_SoundBlockStartPos = 0;
    m_CurSoundBlockLang.clear();
  }
  else
  {
    AppendStringHelper( mBodyBuffer, "<a href=\"sld-sound:" );

    if(!aExtKey.empty()) {
        mBodyBuffer.append(aExtKey.data(), aExtKey.size());
    } else{
        AppendStringHelper( mBodyBuffer, " ");
    }

    AppendStringHelper( mBodyBuffer, ":");
    if(!aDictId.empty()) {
        mBodyBuffer.append(aDictId.data(), aDictId.size());
    } else {
        AppendStringHelper(mBodyBuffer, "0000");
    }

    AppendStringHelper( mBodyBuffer, ":");
    AppendStringHelper(mBodyBuffer, soundId.data());

    AppendStringHelper( mBodyBuffer, ":");
    if(!aLang.empty()) {
      mBodyBuffer.append(aLang.data(), aLang.size());
    } else {
      AppendStringHelper(mBodyBuffer, "0");
    }

      if (mIsPreviousIconHide) {
          AppendStringHelper(mBodyBuffer, "\" style= \"position: relative;left: -8px;\"");
          mIsPreviousIconHide = false;
      }
    AppendStringHelper( mBodyBuffer, "\">");
    m_CurSoundBlockLang.assign( aLang.data(), aLang.size() );
    m_SoundBlockStartPos = mBodyBuffer.size();
  }
}

void HtmlBuilder::addBlock(const CSldMetadataProxy<eMetaUiElement> &aUiElem)
{
    if (aUiElem.isClosing())
        return;

    AddStringHelper( mBodyBuffer, "<uielementmarker></uielementmarker>");
}

JavaObjects::string HtmlBuilder::createSoundExtImgTag()
{
  JavaObjects::string imgBlockBuffer;
  // TODO Create img tag using img from app resources
  // For now only stub implementation
  AppendStringHelper( imgBlockBuffer, "SOUND " );
  imgBlockBuffer.append( m_CurSoundBlockLang );
  return imgBlockBuffer;
}

const char* HtmlBuilder::mScript =
                                   "function scrollToName(name)\n"
                                   "{\n"
                                   "    var tag = document.getElementsByName(name)[0];\n"
                                   "    sld2_expandParentSwitchBlock(tag);\n"
                                   "    tag.scrollIntoView();\n"
                                   "}\n"
                                   // Viewport 'width' parameter actually works like minimum supported width of page content layout.
                                   // If current WebView width is bigger when viewport width then the page content width would be stretched.
                                   // Setting small viewport width (like 1px) works well for article pages.
                                   "function updateViewport(initialScale)\n"
                                   "{\n"
                                   "    viewport = document.querySelector('meta[name=viewport]');\n"
                                   "    viewport.setAttribute('content', 'width=1, initial-scale=' + initialScale + ', maximum-scale=10.0, minimum-scale=0.1');\n"
                                   "}\n"
                                   "function nonResizableElement(initialScale, href){\n"
                                   "    var els = document.querySelectorAll('a[href^=\"'+href+'\"]');\n"
                                   "    for (var i = 0, l = els.length; i < l; i++) {\n"
                                   "        els[i].childNodes[0].style.zoom = 1/initialScale;\n"
                                   "    }\n"
                                   "}\n";
