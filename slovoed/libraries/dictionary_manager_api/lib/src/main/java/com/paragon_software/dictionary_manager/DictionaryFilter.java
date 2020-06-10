package com.paragon_software.dictionary_manager;

import androidx.annotation.NonNull;

public interface DictionaryFilter
{
  boolean apply( @NonNull Dictionary.DictionaryId dictionaryId );

  void saveState();

  void registerListener( @NonNull FilterStateChangedListener listener );

  void unregisterListener( @NonNull FilterStateChangedListener listener );

  interface FilterStateChangedListener
  {
    void onStateChanged();
  }
}
