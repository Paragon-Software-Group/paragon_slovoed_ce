package com.paragon_software.about_manager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.components.DictionaryComponent;
import com.paragon_software.dictionary_manager.components.MorphoComponent;
import com.paragon_software.dictionary_manager.components.WordBaseComponent;
import com.paragon_software.native_engine.EngineVersion;
import com.paragon_software.utils_slovoed.text.LocalizedString;

import java.util.ArrayList;
import java.util.Collection;

class AboutController extends AboutControllerAPI
{
  @NonNull
  private final AboutManagerAPI aboutManagerAPI;

  @Nullable
  private Dictionary.DictionaryId dictionaryId;

  AboutController( @NonNull AboutManagerAPI aboutManagerAPI )
  {
    this.aboutManagerAPI = aboutManagerAPI;
  }

  @Override
  public void setDictionaryId( @Nullable Dictionary.DictionaryId dictionaryId )
  {

    this.dictionaryId = dictionaryId;
  }

  @NonNull
  @Override
  public AboutSpecs getAboutSpecs()
  {
    Dictionary dictionary = null;
    if ( null != dictionaryId )
    {
      dictionary = aboutManagerAPI.getDictionary(dictionaryId);
    }
    AboutSpecs aboutSpecs = new AboutSpecs();
    aboutSpecs.setBaseVersion(createBaseVersionString(dictionary));
    aboutSpecs.setDictionaryId(createDictionaryIdString());
    aboutSpecs.setEngineVersion(createEngineVersionString());
    aboutSpecs.setNumberOfWords(createNumberWordsString(dictionary));
    aboutSpecs.setProductName(createProductNameString(dictionary));
    aboutSpecs.setSoundInfo(createSoundInfoStrings());
    aboutSpecs.setMorphoInfo(createMorphoInfoStrings(dictionary));
    return aboutSpecs;
  }

  private Collection< LocalizedString > createSoundInfoStrings()
  {
    ArrayList< LocalizedString > result = new ArrayList<>();
    if ( null != dictionaryId )
    {
      for ( AboutSoundInfo info : aboutManagerAPI.getSoundInfo(dictionaryId) )
      {
        result.add(info.getProductName());
      }
    }
    return result;
  }

  private Collection< LocalizedString > createMorphoInfoStrings( @Nullable Dictionary dictionary )
  {
    ArrayList< LocalizedString > result = new ArrayList<>();
    if ( null != dictionary )
    {
      for ( DictionaryComponent morphoComponent : dictionary.getDictionaryComponents() )
      {
        if ( DictionaryComponent.Type.MORPHO.equals(morphoComponent.getType()) )
        {
          result.add(LocalizedString.from(( (MorphoComponent) morphoComponent).getId()));
        }
      }
    }
    return result;
  }

  private String createEngineVersionString()
  {
    EngineVersion engineVersion = aboutManagerAPI.getEngineVersion();
    return engineVersion.getVersion() + "." + engineVersion.getBuild();
  }

  private String createDictionaryIdString()
  {
    return null != dictionaryId ? dictionaryId.toString() : "";
  }

  private String createBaseVersionString( @Nullable Dictionary dictionary )
  {
    if ( dictionary != null )
    {
      for ( DictionaryComponent wordBase : dictionary.getDictionaryComponents() )
      {
        if ( DictionaryComponent.Type.WORD_BASE.equals(wordBase.getType()) && !wordBase.isDemo() )
        {
          return wordBase.getVersion();
        }
      }
    }
    return "";
  }

  private String createNumberWordsString( @Nullable Dictionary dictionary )
  {
    if ( dictionary != null )
    {
      for ( DictionaryComponent wordBase : dictionary.getDictionaryComponents() )
      {
        if ( DictionaryComponent.Type.WORD_BASE.equals(wordBase.getType()) && !wordBase.isDemo() )
        {
          return ( (WordBaseComponent) wordBase ).getWordsCount();
        }
      }
    }
    return "";
  }

  private LocalizedString createProductNameString(@Nullable Dictionary dictionary )
  {
    return dictionary != null ? dictionary.getTitle() : LocalizedString.empty();
  }
}
