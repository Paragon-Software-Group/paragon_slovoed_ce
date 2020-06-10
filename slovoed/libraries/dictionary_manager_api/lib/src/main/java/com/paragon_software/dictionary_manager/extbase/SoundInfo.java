package com.paragon_software.dictionary_manager.extbase;

import androidx.annotation.NonNull;

import com.paragon_software.dictionary_manager.location.DictionaryLocation;

// NOTE This class should be immutable otherwise don't forget to implement cloning method and use it in Dictionary objects cloning
public class SoundInfo extends ExternalBaseInfo
{
  private final String mSdcId;

  public SoundInfo( int langFrom, @NonNull DictionaryLocation location, @NonNull String sdcId)
  {
    super(langFrom, location);
    mSdcId = sdcId;
  }

  public String getSdcId()
  {
    return mSdcId;
  }

  @Override
  public String toString()
  {
    // NOTE When modify this method consider what it is used to create ParagonDictionaryManager object state trace
    return "SoundInfo{" + "langFrom=" + langFrom + ", location=" + location + ", sdcId=" + mSdcId + '}';
  }
}
