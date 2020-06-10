package com.paragon_software.dictionary_manager;

import com.paragon_software.dictionary_manager.exceptions.DownloadDictionaryException;

@SuppressWarnings( "unused" )
public interface IDictionaryDownloadProcessControl
{
  void pause() throws DownloadDictionaryException;

  void resume() throws DownloadDictionaryException;

  void cancel() throws DownloadDictionaryException;

  boolean isPaused() ;
}
