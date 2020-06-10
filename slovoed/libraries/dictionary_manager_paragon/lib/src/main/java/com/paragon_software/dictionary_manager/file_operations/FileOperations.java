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

import java.io.*;

import com.paragon_software.dictionary_manager.file_operations.exceptions.CantCreateFileException;
import com.paragon_software.dictionary_manager.file_operations.exceptions.CantDeleteFileException;
import com.paragon_software.dictionary_manager.file_operations.exceptions.FileReadException;
import com.paragon_software.dictionary_manager.file_operations.exceptions.FileWriteException;

final class FileOperations
{
  private static void delete( @NonNull final File file ) throws CantDeleteFileException
  {
    if ( !file.exists() )
    {
      return;
    }
    final File[] contents = file.listFiles();
    if ( contents != null )
    {
      for ( File f : contents )
      {
        delete(f);
      }
    }
    if ( !file.delete() )
    {
      throw new CantDeleteFileException(file.getName());
    }
  }

  static void delete( @NonNull final String fileName ) throws CantDeleteFileException
  {
    delete(new File(fileName));
  }

  static void createDir( @NonNull final String path ) throws CantCreateFileException
  {
    if ( !new File(path).mkdirs() )
    {
      throw new CantCreateFileException(path);
    }
  }

  static void createDir( @NonNull final String parentPath, @NonNull final String name ) throws CantCreateFileException
  {
    if ( !new File(parentPath, name).mkdirs() )
    {
      throw new CantCreateFileException(name + " in " + parentPath);
    }
  }

  static boolean isExistInDir( @NonNull final String dirPath, @NonNull final String fileName )
  {
    return new File(dirPath, fileName).exists();
  }

  static boolean isExist( @NonNull final String path )
  {
    return new File(path).exists();
  }

  static boolean isFile( @NonNull final String path )
  {
    return new File(path).isFile();
  }

  static boolean canRead( @NonNull final String path )
  {
    return new File(path).canRead();
  }

  static boolean isDir( @NonNull final String path )
  {
    return new File(path).isDirectory();
  }

  @NonNull
  static String getRootPathForContext( @NonNull final Context context )
  {
    return context.getFilesDir().getAbsolutePath();
  }

  @NonNull
  static String buildPath( @NonNull final String dirPath, @NonNull final String fileName )
  {
    return new File(dirPath, fileName).getAbsolutePath();
  }

  @NonNull
  static String basename( @NonNull final String path )
  {
    return new File(path).getName();
  }

  @NonNull
  static String[] fileList( @NonNull final String path )
  {
    String[] fileList = new File(path).list();
    if ( null == fileList )
    {
      return new String[0];
    }
    return fileList;
  }

  public static boolean rename( String pathFrom, String pathTo )
  {
    final File file = new File(pathFrom);
    if ( file.exists() )
    {
      return file.renameTo(new File(pathTo));
    }
    return false;
  }

  private static final class StreamWriter implements IFileStreamWriter
  {
    @NonNull
    private final BufferedOutputStream mBos;
    private final static int BUFFER_SIZE = 4096 * 16;
    @NonNull
    private final File mFile;

    StreamWriter( @NonNull final File file ) throws FileWriteException
    {
      this(file, BUFFER_SIZE);
    }

    StreamWriter( @NonNull final File file, final int bufferSize ) throws FileWriteException
    {
      if ( file.isDirectory() )
      {
        throw new FileWriteException("Is Directory");
      }
      this.mFile = file;
      try
      {
        mBos = new BufferedOutputStream(new FileOutputStream(file), bufferSize);
      }
      catch ( IOException e )
      {
        throw new FileWriteException(e);
      }
    }

    @Override
    public void write( @NonNull byte[] data, int len ) throws FileWriteException
    {
      try
      {
        mBos.write(data, 0, len);
      }
      catch ( IOException e )
      {
        throw new FileWriteException(e);
      }
    }

    private void closeStream()
    {
      try
      {
        mBos.flush();
        mBos.close();
      }
      catch ( IOException ignore )
      {
      }
    }

    @Override
    public void finish()
    {
      closeStream();
    }

    @Override
    public void remove() throws CantDeleteFileException
    {
      closeStream();
      FileOperations.delete(mFile);
    }
  }

  private static final class StreamReader implements IFileStreamReader
  {
    @NonNull
    private final BufferedInputStream mBis;
    private final static int BUFFER_SIZE = 4096 * 16;
    private final long mLength;

    StreamReader( @NonNull final File file ) throws FileReadException
    {
      this(file, BUFFER_SIZE);
    }

    StreamReader( @NonNull final File file, final int bufferSize ) throws FileReadException
    {
      if ( file.isDirectory() )
      {
        throw new FileReadException("Is Directory");
      }
      try
      {
        this.mBis = new BufferedInputStream(new FileInputStream(file), bufferSize);
        this.mLength = file.length();
      }
      catch ( IOException e )
      {
        throw new FileReadException(e);
      }
    }

    @Override
    public long length()
    {
      return mLength;
    }

    @Override
    public int read( @NonNull byte[] data, int len ) throws FileReadException
    {
      try
      {
        return mBis.read(data, 0, len);
      }
      catch ( IOException e )
      {
        throw new FileReadException(e);
      }
    }

    @Override
    public void finish()
    {
      try
      {
        mBis.close();
      }
      catch ( IOException ignore )
      {

      }
    }
  }

  static IFileStreamWriter getFileWriter( @NonNull final String path, boolean overwrite ) throws FileWriteException
  {
    final File file = new File(path);
    if ( file.exists() )
    {
      if ( !overwrite )
      {
        throw new FileWriteException(path + " Already exist");
      }
    }
    return new StreamWriter(file);
  }

  static IFileStreamReader getFileReader( @NonNull final String path ) throws FileReadException
  {
    final File file = new File(path);
    if ( !file.exists() )
    {
      throw new FileReadException(path + " Not exist");
    }
    return new StreamReader(file);
  }

  static void copy( @NonNull final IFileStreamReader reader, @NonNull final IFileStreamWriter writer,
                    @Nullable final IFileOperations.ICopyProgress copyStatusCallback,
                    @Nullable final IFileOperations.CopyControl copyControl )
      throws FileReadException, FileWriteException
  {

    int bufferSize = 4096 * 8;
    byte[] buffer = new byte[bufferSize];
    long length = reader.length();
    long completed = 0;
    while ( completed != length )
    {
      int toRead = Math.min(bufferSize, (int) ( length - completed ));
      int read = reader.read(buffer, toRead);
      writer.write(buffer, read);
      completed += read;
      if ( null != copyStatusCallback )
      {
        copyStatusCallback.onProgress(completed, length);
      }
      if ( null != copyControl && copyControl.needCancel() )
      {
        break;
      }
    }
    reader.finish();
    writer.finish();

  }

}
