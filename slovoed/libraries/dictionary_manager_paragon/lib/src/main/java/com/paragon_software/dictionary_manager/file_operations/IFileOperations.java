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

import com.paragon_software.dictionary_manager.file_operations.exceptions.CantCreateFileException;
import com.paragon_software.dictionary_manager.file_operations.exceptions.CantDeleteFileException;
import com.paragon_software.dictionary_manager.file_operations.exceptions.FileReadException;
import com.paragon_software.dictionary_manager.file_operations.exceptions.FileWriteException;

@SuppressWarnings( "unused" )
public interface IFileOperations
{

  void delete( @NonNull final String fileName ) throws CantDeleteFileException;

  void createDir( @NonNull final String path ) throws CantCreateFileException;

  void createDir( @NonNull final String parentPath, @NonNull final String name ) throws CantCreateFileException;

  boolean isExistInDir( @NonNull final String dirPath, @NonNull final String fileName );

  boolean isExist( @NonNull final String path );

  boolean isFile( @NonNull final String path );

  boolean canRead( @NonNull final String path );

  boolean isDir( @NonNull final String path );

  @NonNull
  String getRootPathForContext( @NonNull final Context context );

  @NonNull
  String buildPath( @NonNull final String dirPath, @NonNull final String fileName );

  @NonNull
  String basename( @NonNull final String path );

  @NonNull
  String[] fileList( @NonNull final String path );

  IFileStreamWriter getFileWriter( @NonNull final String path, boolean overwrite ) throws FileWriteException;

  IFileStreamReader getFileReader( @NonNull final String path ) throws FileReadException;

  void copy( @NonNull final IFileStreamReader reader, @NonNull final IFileStreamWriter writer )
      throws FileReadException, FileWriteException;

  void copy( @NonNull final IFileStreamReader reader, @NonNull final IFileStreamWriter writer,
             @NonNull final CopyControl copyControl ) throws FileReadException, FileWriteException;

  void copy( @NonNull final IFileStreamReader reader, @NonNull final IFileStreamWriter writer,
             @NonNull final ICopyProgress copyStatusCallback ) throws FileReadException, FileWriteException;

  void copy( @NonNull final IFileStreamReader reader, @NonNull final IFileStreamWriter writer,
             @NonNull final ICopyProgress copyStatusCallback, @NonNull final CopyControl copyControl )
      throws FileReadException, FileWriteException;

  boolean rename( String pathFrom, String pathTo );

  interface ICopyProgress
  {
    void onProgress( final long current, final long total );
  }

  class CopyControl
  {
    private boolean mNeedCancel = false;

    public void cancel()
    {
      mNeedCancel = true;
    }

    boolean needCancel()
    {
      return mNeedCancel;
    }
  }

  interface IFileWriteFinishCallback
  {
    void onFinish();
  }

  interface IFileReaderFinishCallback
  {
    void onFinish();
  }
}
