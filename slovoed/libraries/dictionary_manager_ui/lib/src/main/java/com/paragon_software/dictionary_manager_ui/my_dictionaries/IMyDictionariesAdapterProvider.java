package com.paragon_software.dictionary_manager_ui.my_dictionaries;


import androidx.annotation.NonNull;

import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.dictionary_manager.components.DictionaryComponent;

import java.util.List;

public interface IMyDictionariesAdapterProvider
{
  boolean isDownloaded( DictionaryComponent component );

  boolean isInProgress( DictionaryComponent component );

  int getDownloadProgress( DictionaryComponent component );

  int getTrialLengthInMinutes( @NonNull Dictionary.DictionaryId dictionaryId );

  List<Dictionary> getEnabledDictionaries();

  long getTotalSize( @NonNull Dictionary dictionary );

  void unregisterComponentProgressObserver( @NonNull DictionaryManagerAPI.IComponentDownloadProgressObserver observer );

  void registerComponentProgressObserver( @NonNull DictionaryComponent component,
                                          @NonNull DictionaryManagerAPI.IComponentDownloadProgressObserver observer );

  void download( @NonNull DictionaryComponent component, @NonNull Dictionary dictionary );

  void remove( @NonNull DictionaryComponent component );

  void pause( DictionaryComponent component, Dictionary dictionary );
}
