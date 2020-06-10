package com.paragon_software.dictionary_manager.extbase;

import androidx.annotation.NonNull;

import com.paragon_software.dictionary_manager.location.DictionaryLocation;

// NOTE This class should be immutable otherwise don't forget to implement cloning method and use it in Dictionary objects cloning
public class MorphoInfo extends ExternalBaseInfo
{

  public MorphoInfo( int langFrom, @NonNull DictionaryLocation location )
  {
    super(langFrom, location);
  }

  @Override
  public String toString()
  {
    // NOTE When modify this method consider what it is used to create ParagonDictionaryManager object state trace
    return "MorphoInfo{" + "langFrom=" + langFrom + ", location=" + location + '}';
  }
}
