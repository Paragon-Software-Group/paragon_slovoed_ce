package com.paragon_software.dictionary_manager;

import androidx.annotation.NonNull;

import com.paragon_software.dictionary_manager.components.DictionaryComponent;

interface IDownloadController
{
  void download( DictionaryComponent component, Dictionary dictionary );

  boolean isDownloaded( DictionaryComponent component );

  @SuppressWarnings( "SimplifiableIfStatement" )
  boolean isInProgress( DictionaryComponent component );

  long getTransferredBytes( DictionaryComponent component );

  long getSizeBytes( DictionaryComponent component );

  void unregisterComponentProgressObserver( @NonNull DictionaryManagerAPI.IComponentDownloadProgressObserver observer );

  void registerComponentProgressObserver( @NonNull DictionaryComponent component,
                                          @NonNull DictionaryManagerAPI.IComponentDownloadProgressObserver observer );

  void registerDownloadFailureObserver( DictionaryManagerAPI.IOnDownloadFailureObserver observer );

  void unregisterDownloadFailureObserver( DictionaryManagerAPI.IOnDownloadFailureObserver observer );

  void remove( @NonNull DictionaryComponent component );

  boolean isAnyDictionaryDownloaded();

  void removeAll();

  void pauseDownload( DictionaryComponent component, Dictionary dictionary );
}
