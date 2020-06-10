package com.paragon_software.about_manager;

import androidx.annotation.Nullable;

import com.paragon_software.utils_slovoed.text.LocalizedString;

import java.util.Collection;

class AboutSpecs
{
  @Nullable
  private String                      engineVersion;
  @Nullable
  private Collection<LocalizedString> soundInfo;
  @Nullable
  private Collection<LocalizedString> morphoInfo;
  @Nullable
  private String                      numberOfWords;
  @Nullable
  private String                      baseVersion;
  @Nullable
  private LocalizedString             productName;
  @Nullable
  private String                      dictionaryId;

  @Nullable
  String getEngineVersion()
  {
    return engineVersion;
  }

  void setEngineVersion( @Nullable String engineVersion )
  {
    this.engineVersion = engineVersion;
  }

  @Nullable
  Collection< LocalizedString > getMorphoInfo()
  {
    return morphoInfo;
  }

  void setMorphoInfo( @Nullable Collection< LocalizedString > morphoInfo )
  {
    this.morphoInfo = morphoInfo;
  }

  @Nullable
  String getNumberOfWords()
  {
    return numberOfWords;
  }

  void setNumberOfWords( @Nullable String numberOfWords )
  {
    this.numberOfWords = numberOfWords;
  }

  @Nullable
  String getBaseVersion()
  {
    return baseVersion;
  }

  void setBaseVersion( @Nullable String baseVersion )
  {
    this.baseVersion = baseVersion;
  }

  @Nullable
  LocalizedString getProductName()
  {
    return productName;
  }

  void setProductName( @Nullable LocalizedString productName )
  {
    this.productName = productName;
  }

  @Nullable
  String getDictionaryId()
  {
    return dictionaryId;
  }

  void setDictionaryId( @Nullable String dictionaryId )
  {
    this.dictionaryId = dictionaryId;
  }

  @Nullable
  Collection< LocalizedString > getSoundInfo()
  {
    return soundInfo;
  }

  void setSoundInfo( @Nullable Collection< LocalizedString > soundInfo )
  {
    this.soundInfo = soundInfo;
  }
}
