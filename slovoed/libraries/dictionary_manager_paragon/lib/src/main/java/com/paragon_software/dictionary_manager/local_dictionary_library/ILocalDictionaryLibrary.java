package com.paragon_software.dictionary_manager.local_dictionary_library;

import androidx.annotation.NonNull;

import java.util.Collection;

import com.paragon_software.dictionary_manager.file_operations.exceptions.CantDeleteFileException;
import com.paragon_software.dictionary_manager.local_dictionary_library.exceptions.WriteToDictionaryLibraryException;

public interface ILocalDictionaryLibrary
{
  @NonNull
  Collection< String > getLocalDictionariesList();

  boolean isDictionaryPresent( @NonNull final String dictionaryName );

  @NonNull
  String getDictionaryContainerName();

  IDictionaryWriter getDictionaryWriter( @NonNull final String dictionaryName, boolean overwrite )
      throws WriteToDictionaryLibraryException;

  IDictionaryWriter getDictionaryWriter( @NonNull final String dictionaryName )
      throws WriteToDictionaryLibraryException;

  String getPath( String componentFileName );

  boolean remove( String componentFileName ) throws CantDeleteFileException;

  @NonNull
  String getDirectoryPath();

  void rescanDictionaries();

  boolean rename( String from, String to );
}
