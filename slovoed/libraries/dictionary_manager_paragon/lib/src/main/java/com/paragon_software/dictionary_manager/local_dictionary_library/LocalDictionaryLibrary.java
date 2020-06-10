package com.paragon_software.dictionary_manager.local_dictionary_library;

import android.content.Context;
import androidx.annotation.NonNull;
import androidx.annotation.VisibleForTesting;

import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
import java.util.Set;

import com.paragon_software.dictionary_manager.file_operations.DefaultFileOperations;
import com.paragon_software.dictionary_manager.file_operations.IFileOperations;
import com.paragon_software.dictionary_manager.file_operations.exceptions.CantDeleteFileException;
import com.paragon_software.dictionary_manager.file_operations.exceptions.FileOperationsException;
import com.paragon_software.dictionary_manager.local_dictionary_library.exceptions.InitDictionaryLibraryException;
import com.paragon_software.dictionary_manager.local_dictionary_library.exceptions.WriteToDictionaryLibraryException;

@SuppressWarnings( "WeakerAccess" )
public final class LocalDictionaryLibrary implements ILocalDictionaryLibrary
{
  @NonNull
  private static final String DEFAULT_DICTIONARY_DIRECTORY_NAME = "DICTIONARIES";
  @NonNull
  private final String mDictionariesDirectoryPath;
  @NonNull
  private Set< String > mDictionaries = new HashSet<>();

  @NonNull
  private final IFileOperations mFileOperations;

  @Override
  @NonNull
  public Collection< String > getLocalDictionariesList()
  {
    return Collections.unmodifiableCollection(mDictionaries);
  }

  @Override
  public boolean isDictionaryPresent( @NonNull final String dictionaryName )
  {
    return mDictionaries.contains(dictionaryName);
  }

  public LocalDictionaryLibrary( @NonNull final Context context, @NonNull final String dictionaryContainerName )
      throws InitDictionaryLibraryException
  {
    this(context, dictionaryContainerName, new DefaultFileOperations());
  }

  public LocalDictionaryLibrary( @NonNull final Context context ) throws InitDictionaryLibraryException
  {
    this(context, DEFAULT_DICTIONARY_DIRECTORY_NAME);
  }

  @VisibleForTesting
  LocalDictionaryLibrary( @NonNull final Context context, @NonNull final String dictionaryContainerName,
                          @NonNull final IFileOperations operations ) throws InitDictionaryLibraryException
  {
    mFileOperations = operations;
    mDictionariesDirectoryPath =
        mFileOperations.buildPath(mFileOperations.getRootPathForContext(context), dictionaryContainerName);

    if ( !mFileOperations.isExist(mDictionariesDirectoryPath) )
    {
      createDictionaryDirectory();
    }
    else if ( mFileOperations.isFile(mDictionariesDirectoryPath) )
    {
      try
      {
        mFileOperations.delete(mDictionariesDirectoryPath);
      }
      catch ( FileOperationsException e )
      {
        throw new InitDictionaryLibraryException(e);
      }
      createDictionaryDirectory();
    }
    rescanDictionaries();
  }

  @NonNull
  public static String getDefaultDictionaryDirectoryName()
  {
    return DEFAULT_DICTIONARY_DIRECTORY_NAME;
  }

  @Override
  @NonNull
  public final String getDictionaryContainerName()
  {
    return mFileOperations.basename(mDictionariesDirectoryPath);
  }

  private void createDictionaryDirectory() throws InitDictionaryLibraryException
  {
    try
    {
      mFileOperations.createDir(mDictionariesDirectoryPath);
    }
    catch ( FileOperationsException e )
    {
      throw new InitDictionaryLibraryException(e);
    }
  }

  public void rescanDictionaries()
  {
    Set< String > dictionaries = new HashSet<>();
    final String[] fileList = mFileOperations.fileList(mDictionariesDirectoryPath);
    for ( String dict : fileList )
    {
      final String dictPath = mFileOperations.buildPath(mDictionariesDirectoryPath, dict);
      if ( mFileOperations.isFile(dictPath) && mFileOperations.canRead(dictPath) )
      {
        dictionaries.add(dict);
      }
    }
    mDictionaries = dictionaries;
  }

  @Override
  public boolean rename( String fromFileName, String toFilename )
  {
    String pathFrom = mFileOperations.buildPath(mDictionariesDirectoryPath, fromFileName);
    String pathTo = mFileOperations.buildPath(mDictionariesDirectoryPath, toFilename);
    return mFileOperations.rename(pathFrom, pathTo);
  }

  @Override
  public IDictionaryWriter getDictionaryWriter( @NonNull final String fileName, boolean overwrite )
      throws WriteToDictionaryLibraryException
  {
    return new LocalDictionaryWriter(this, mFileOperations.buildPath(mDictionariesDirectoryPath, fileName),
                                     mFileOperations, overwrite);
  }

  @Override
  public IDictionaryWriter getDictionaryWriter( @NonNull final String dictionaryName )
      throws WriteToDictionaryLibraryException
  {
    return getDictionaryWriter(dictionaryName, false);
  }

  @Override
  public String getPath( String componentFileName )
  {
    if ( isDictionaryPresent(componentFileName) )
    {
      return mFileOperations.buildPath(mDictionariesDirectoryPath, componentFileName);
    }
    return null;
  }

  @Override
  public boolean remove( String componentFileName ) throws CantDeleteFileException
  {
    String path;
    if ( (path = getPath(componentFileName)) != null )
    {
      mFileOperations.delete(path);
      rescanDictionaries();
      return true;
    }
    else
    {
      return false;
    }
  }

  @Override
  @NonNull
  public String getDirectoryPath()
  {
    return mDictionariesDirectoryPath;
  }
}
