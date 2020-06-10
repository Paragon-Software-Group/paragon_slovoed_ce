/*
 * file operations
 *
 *  Created on: 06.04.18
 *      Author: Mikhail Belyshov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

package com.paragon_software.dictionary_manager.file_operations;

import android.content.Context;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.dictionary_manager.file_operations.exceptions.CantCreateFileException;
import com.paragon_software.dictionary_manager.file_operations.exceptions.CantDeleteFileException;
import com.paragon_software.dictionary_manager.file_operations.exceptions.FileReadException;
import com.paragon_software.dictionary_manager.file_operations.exceptions.FileWriteException;

public class DefaultFileOperations implements IFileOperations
{
  @Override
  public void delete( @NonNull String fileName ) throws CantDeleteFileException
  {
    FileOperations.delete(fileName);
  }

  @Override
  public void createDir( @NonNull String path ) throws CantCreateFileException
  {
    FileOperations.createDir(path);
  }

  @Override
  public void createDir( @NonNull String parentPath, @NonNull String name ) throws CantCreateFileException
  {
    FileOperations.createDir(parentPath, name);
  }

  @Override
  public boolean isExistInDir( @NonNull String dirPath, @NonNull String fileName )
  {
    return FileOperations.isExistInDir(dirPath, fileName);
  }

  @Override
  public boolean isExist( @NonNull String path )
  {
    return FileOperations.isExist(path);
  }

  @Override
  public boolean isFile( @NonNull String path )
  {
    return FileOperations.isFile(path);
  }

  @Override
  public boolean canRead( @NonNull String path )
  {
    return FileOperations.canRead(path);
  }

  @Override
  public boolean isDir( @NonNull String path )
  {
    return FileOperations.isDir(path);
  }

  @NonNull
  @Override
  public String getRootPathForContext( @NonNull Context context )
  {
    return FileOperations.getRootPathForContext(context);
  }

  @NonNull
  @Override
  public String buildPath( @NonNull String dirPath, @NonNull String fileName )
  {
    return FileOperations.buildPath(dirPath, fileName);
  }

  @NonNull
  @Override
  public String basename( @NonNull String path )
  {
    return FileOperations.basename(path);
  }

  @NonNull
  @Override
  public String[] fileList( @NonNull String path )
  {
    return FileOperations.fileList(path);
  }

  @Override
  public IFileStreamWriter getFileWriter( @NonNull String path, boolean overwrite ) throws FileWriteException
  {
    return FileOperations.getFileWriter(path, overwrite);
  }

  @Override
  public IFileStreamReader getFileReader( @NonNull String path ) throws FileReadException
  {
    return FileOperations.getFileReader(path);
  }

  @Override
  public void copy( @NonNull final IFileStreamReader reader, @NonNull final IFileStreamWriter writer )
      throws FileReadException, FileWriteException

  {
    copy(reader, writer, null, null);
  }

  @Override
  public void copy( @NonNull final IFileStreamReader reader, @NonNull final IFileStreamWriter writer,
                    @NonNull final CopyControl copyControl ) throws FileReadException, FileWriteException
  {
    copy(reader, writer, null, copyControl);
  }

  @Override
  public void copy( @NonNull final IFileStreamReader reader, @NonNull final IFileStreamWriter writer,
                    @NonNull final ICopyProgress copyStatusCallback ) throws FileReadException, FileWriteException
  {
    copy(reader, writer, copyStatusCallback, null);
  }

  @Override
  public void copy( @NonNull final IFileStreamReader reader, @NonNull final IFileStreamWriter writer,
                    @Nullable final ICopyProgress copyStatusCallback, @Nullable final CopyControl copyControl )
      throws FileReadException, FileWriteException
  {
    FileOperations.copy(reader, writer, copyStatusCallback, copyControl);
  }

  @Override
  public boolean rename( String pathFrom, String pathTo )
  {
    return FileOperations.rename(pathFrom, pathTo);
  }
}
