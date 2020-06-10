package com.paragon_software.dictionary_manager.download_manager;

import androidx.annotation.NonNull;

import com.paragon_software.dictionary_manager.download_manager.exceptions.DownloadFailedException;

public interface IDownloadSource
{
  long length();

  int read( @NonNull byte[] data ) throws DownloadFailedException;

  void finish();

  void open() throws DownloadFailedException;
}
