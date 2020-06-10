#include "LayerAccess.h"
#include "SldCompare.h"
#include "SldDictionary.h"

LayerAccess::LayerAccess(const JavaObjects::string& _cacheDir) :
        m_pData( nullptr ), m_pParser( nullptr ), m_pBuilder( nullptr ), m_pSoundBuilder( nullptr )
        {
  CSldCompare::StrUTF8_2_UTF16( m_PlatformID, (const UInt8*) "A1" );
  m_pSoundBuilder = new SoundBuilder();
  m_SerialData = NULL;
  m_SerialDataSize = 0;
  m_SerialDictID = 0;
  cacheDir = _cacheDir;
}

LayerAccess::~LayerAccess()
{
  if ( m_SerialData )
  {
    sldMemFree( m_SerialData );
  }
  m_SerialDataSize = 0;
  m_SerialDictID = 0;

  if ( m_pBuilder )
  {
    delete m_pBuilder;
  }
  if ( m_pParser )
  {
    delete m_pParser;
  }
  if ( m_pData )
  {
    delete m_pData;
  }
  if ( m_pSoundBuilder )
  {
    delete m_pSoundBuilder;
  }
}

void LayerAccess::InitTranslation( CSldDictionary* dictionary, JNIEnv* env, jobject _htmlParams )
{
  if ( m_pBuilder == nullptr )
  {
    m_pData = new DataAccess( dictionary,  cacheDir);
    m_pParser = new CSldMetadataParser( dictionary, m_pData );
    m_pBuilder = new HtmlBuilder( m_pData );
    m_pBuilder->initFontFaces(env);
  }
  m_pBuilder->setVisibilitySentencesSoundButton(env, _htmlParams);
  m_pBuilder->setScale(JavaHtmlBuilderParams::GetScale( env, _htmlParams ));
  m_pBuilder->setMyViewSettings(JavaHtmlBuilderParams::IsHidePhonetics( env, _htmlParams ),
                                JavaHtmlBuilderParams::IsHideExamples( env, _htmlParams ),
                                JavaHtmlBuilderParams::IsHideImages( env, _htmlParams ),
                                JavaHtmlBuilderParams::IsHideIdioms( env, _htmlParams ),
                                JavaHtmlBuilderParams::IsHidePhrase( env, _htmlParams ));
  m_pBuilder->setHorizontalPadding(JavaHtmlBuilderParams::GetHorizontalPadding( env, _htmlParams ));
  m_pBuilder->setRemoveBodyMargin(JavaHtmlBuilderParams::IsRemoveBodyMargin( env, _htmlParams ));
}

const UInt16* LayerAccess::GetTranslationHtml()
{
  return m_pBuilder->GetHtml();
}

const SoundBuilder* LayerAccess::GetSoundBuilder()
{
  return m_pSoundBuilder;
}

ESldError LayerAccess::BuildTranslationRight(
    const CSldDictionary* aDictionary,
    const UInt16* aText,
    ESldTranslationModeType aTree
)
{
  const CSldStyleInfo* StyleInfo = nullptr;
  ESldError error = eOK;
  switch ( (int) aTree )
  {
    case eTranslationStart:
      m_pBuilder->StartBuilding();
      break;

    case eTranslationContinue:
      break;

    case eTranslationStop:
      m_pBuilder->EndBuilding();
      break;

    default:
      error = aDictionary->GetStyleInfo( aTree, &StyleInfo );
      if ( error == eOK )
      {
        if ( StyleInfo->IsVisible() )
        {
          ESldStyleMetaTypeEnum TextType = StyleInfo->GetStyleMetaType();
          if ( TextType == eMetaSound )
          {
            const auto sound = m_pParser->GetMetadata<eMetaSound>(aText);
            error = sound.error();
            if ( error == eOK )
            {
              const UInt16* Lang = sound.c_str( sound->Lang );
              const UInt16* ExtKey = sound.c_str( sound->ExtKey.Key );
              const UInt16* DictId = sound.c_str( sound->ExtKey.DictId );
              std::string soundId = std::to_string(sound->Index);
                m_pBuilder->BuildSound(Lang, ExtKey, DictId, soundId ,sound.isClosing() );
            }
          }
          else if ( TextType == eMetaText )
          {
            m_pBuilder->SetLanguageCode(SldLanguage::fromCode(StyleInfo->GetLanguage()));
          }

          if ( error == eOK )
          {
            error = m_pBuilder->Translate( *m_pParser, StyleInfo->GetStyleMetaType(), StyleInfo->GetGlobalIndex(), aText );
          }
        }
      }
      break;
  }
  return error;
}

ESldError LayerAccess::BuildTranslationWrong(
    const CSldDictionary* aDictionary,
    const UInt16* aText,
    ESldTranslationModeType aTree
)
{
  return BuildTranslationRight( aDictionary, aText, aTree );
}

ESldError LayerAccess::WordFound( const ESldWordFoundCallbackType aCallbackType, const UInt32 aIndex )
{
  return eOK;
}

const UInt16* LayerAccess::GetPlatformID()
{
  return m_PlatformID;
}

ESldError LayerAccess::SaveSerialData( UInt32 aDictID, const UInt8* aData, UInt32 aSize )
{
  if ( !aData )
  {
    return eMemoryNullPointer;
  }
  if ( m_SerialData )
  {
    sldMemFree( m_SerialData );
    m_SerialDataSize = 0;
    m_SerialDictID = 0;
  }

  m_SerialData = (UInt8*) sldMemNew( aSize );
  if ( !m_SerialData )
  {
    return eMemoryNotEnoughMemory;
  }

  sldMemMove( m_SerialData, aData, aSize );
  m_SerialDataSize = aSize;
  m_SerialDictID = aDictID;

  return eOK;
}

ESldError LayerAccess::LoadSerialData( UInt32 aDictID, UInt8* aData, UInt32* aSize )
{
  ESldError error;
  if ( m_SerialDictID != aDictID || !m_SerialDictID || !m_SerialData || !m_SerialDataSize )
  {
    return eSNDictIDNotFound;
  }

  if ( !aSize )
  {
    return eMemoryNullPointer;
  }

  if ( !aData )
  {
    *aSize = m_SerialDataSize;
    return eOK;
  }

  if ( *aSize < m_SerialDataSize )
  {
    *aSize = m_SerialDataSize;
    return eMemoryNotEnoughMemory;
  }

  sldMemMove( aData, m_SerialData, m_SerialDataSize );

  return eOK;
}

ESldError LayerAccess::BuildSoundRight(
    const UInt8* aBlockPtr,
    UInt32 aBlockSize,
    UInt32 aPreviousSize,
    UInt32 aFrequency,
    UInt32 aFinishFlag
)
{
  if ( m_pSoundBuilder )
  {
    if (aFinishFlag == SLD_SOUND_FLAG_START)
    {
      return m_pSoundBuilder->StartBuilding(aBlockPtr);
    }
    else if (aFinishFlag == SLD_SOUND_FLAG_CONTINUE)
    {
      return m_pSoundBuilder->ContinueBuilding(aBlockPtr, aBlockSize);
    }
    else if (aFinishFlag == SLD_SOUND_FLAG_FINISH)
    {
      return m_pSoundBuilder->FinishBuilding(aFrequency);
    }
  }
  return eOK;
}

ESldError LayerAccess::BuildSoundWrong(
    const UInt8* aBlockPtr,
    UInt32 aBlockSize,
    UInt32 aPreviousSize,
    UInt32 aFrequency,
    UInt32 aFinishFlag
)
{
  return BuildSoundRight( aBlockPtr, aBlockSize, aPreviousSize, aFrequency, aFinishFlag );
}

ESldError LayerAccess::LoadSoundByIndex( Int32 aSoundIndex, const UInt8** aDataPtr, UInt32* aDataSize )
{
  return eOK;
}

ESldError LayerAccess::LoadImageByIndex( Int32 aImageIndex, const UInt8** aDataPtr, UInt32* aDataSize )
{
  return eOK;
}
