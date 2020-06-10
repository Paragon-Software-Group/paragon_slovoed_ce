package com.paragon_software.dictionary_manager.local_dictionary_library;

import androidx.annotation.NonNull;

import com.paragon_software.dictionary_manager.file_operations.IFileOperations;
import com.paragon_software.dictionary_manager.file_operations.IFileStreamWriter;
import com.paragon_software.dictionary_manager.file_operations.exceptions.CantDeleteFileException;
import com.paragon_software.dictionary_manager.file_operations.exceptions.FileWriteException;
import com.paragon_software.dictionary_manager.local_dictionary_library.exceptions.WriteToDictionaryLibraryException;

public class LocalDictionaryWriter implements IDictionaryWriter
{
  private static final String TMP_FILE_SUFFIX = ".tmp";
  @NonNull
  final private LocalDictionaryLibrary mLibrary;
  @NonNull
  private final IFileStreamWriter      mFileStreamWriter;
  @NonNull
  private final String                 mDictionaryPath;
  @NonNull
  private final String                 mDictionaryPathTmp;
  @NonNull
  private final IFileOperations        mFileOperations;

  public LocalDictionaryWriter( @NonNull final LocalDictionaryLibrary library, @NonNull final String dictionaryPath,
                                @NonNull final IFileOperations fileOperations ) throws WriteToDictionaryLibraryException
  {
    this(library, dictionaryPath, fileOperations, false);
  }

  public LocalDictionaryWriter( @NonNull final LocalDictionaryLibrary library, @NonNull final String dictionaryPath,
                                @NonNull final IFileOperations fileOperations, final boolean overwrite )
      throws WriteToDictionaryLibraryException
  {

    try
    {
      if ( fileOperations.isExist(dictionaryPath) )
      {
        if ( overwrite )
        {
          fileOperations.delete(dictionaryPath);
        }
        else
        {
          throw new WriteToDictionaryLibraryException("Already exist");
        }

      }
      this.mLibrary = library;
      this.mDictionaryPath = dictionaryPath;
      this.mDictionaryPathTmp = dictionaryPath + TMP_FILE_SUFFIX;
      this.mFileOperations = fileOperations;

      if ( fileOperations.isExist(mDictionaryPathTmp) )
      {
        fileOperations.delete(mDictionaryPathTmp);
      }

      mFileStreamWriter = fileOperations.getFileWriter(mDictionaryPathTmp, true);
    }
    catch ( FileWriteException | CantDeleteFileException e )
    {
      throw new WriteToDictionaryLibraryException(e);
    }
  }

  @Override
  public void write( @NonNull final byte[] data, final int len ) throws WriteToDictionaryLibraryException
  {
    try
    {
      mFileStreamWriter.write(data, len);
    }
    catch ( FileWriteException e )
    {
      throw new WriteToDictionaryLibraryException(e);
    }
  }

  @Override
  public void finish() throws WriteToDictionaryLibraryException
  {
    mFileStreamWriter.finish();
    if ( !mFileOperations.rename(mDictionaryPathTmp, mDictionaryPath) )
    {
      throw new WriteToDictionaryLibraryException("");
    }
    mLibrary.rescanDictionaries();
  }

  @Override
  public void remove() throws WriteToDictionaryLibraryException
  {
    try
    {
      mFileStreamWriter.remove();
    }
    catch ( CantDeleteFileException e )
    {
      throw new WriteToDictionaryLibraryException(e);
    }
  }
}
