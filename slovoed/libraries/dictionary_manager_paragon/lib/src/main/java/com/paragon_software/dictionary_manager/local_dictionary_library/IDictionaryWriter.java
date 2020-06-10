package com.paragon_software.dictionary_manager.local_dictionary_library;

import androidx.annotation.NonNull;

import com.paragon_software.dictionary_manager.local_dictionary_library.exceptions.WriteToDictionaryLibraryException;

public interface IDictionaryWriter
{
  void write( @NonNull final byte[] data, final int len ) throws WriteToDictionaryLibraryException;

  void remove() throws WriteToDictionaryLibraryException;

  void finish() throws WriteToDictionaryLibraryException;
}
