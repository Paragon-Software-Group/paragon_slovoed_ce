package com.paragon_software.dictionary_manager.download_manager;

import androidx.annotation.NonNull;

import com.paragon_software.dictionary_manager.download_manager.exceptions.DownloadFailedException;
import com.paragon_software.dictionary_manager.download_manager.exceptions.DownloadManagerException;
import com.paragon_software.dictionary_manager.download_manager.exceptions.WriteToDestinationException;

import org.mockito.Mockito;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.util.Arrays;
import java.util.List;

class Utils
{
  static DownloadManager.IDownloadStatus newEmptyDownloadStatusObserver()
  {
    return Mockito.mock(DownloadManager.IDownloadStatus.class);
  }

  static byte[] getData()
  {
    byte[] bytes = new byte[1024 * 15];
    Arrays.fill(bytes, Byte.MIN_VALUE);
    return bytes;
  }

  static boolean equalsBytes(byte[] left, byte[] right)
  {
    if ( left.length != right.length )
    {
      return false;
    }

    for ( int i = 0 ; i < left.length ; i++ )
    {
      if ( right[i] != left[i] )
      {
        return false;
      }
    }
    return true;
  }

  static boolean equalsDownloadStatuses(@NonNull List<DownloadTaskStatus.DOWNLOAD_TASK_STATUS> left,
                                        @NonNull List<DownloadTaskStatus.DOWNLOAD_TASK_STATUS> right)
  {
    if ( left.size() != right.size() )
    {
      return false;
    }
    for ( int i = 0 ; i < left.size() ; i++ )
    {
      if ( !left.get(i).equals(right.get(i)) )
      {
        return false;
      }
    }
    return true;
  }

  static void sleep(int millis)
  {
    try
    {
      Thread.sleep(millis);
    }
    catch ( InterruptedException e )
    {
      e.printStackTrace();
    }
  }

  public static abstract class MockSource implements IDownloadSource
  {
    ByteArrayInputStream bais;
    private byte[] data;
    private int    sleepMillis;
    private DownloadFailedException exception;

    MockSource(byte[] data, int sleepMillis)
    {
      this.data = data;
      this.sleepMillis = sleepMillis;
    }

    @Override
    public long length()
    {
      return data.length;
    }

    @Override
    public int read( @NonNull byte[] readBuffer ) throws DownloadFailedException
    {
      if (null != getException())
        throw getException();

      int read;
      try
      {
        sleep(sleepMillis);
        read = bais.read(readBuffer, 0, 1024);
      }
      catch ( Exception e )
      {
        e.printStackTrace();
        throw new DownloadFailedException(e);
      }
      return read;
    }

    private void sleep( int sleepMillis )
    {
      try
      {
        Thread.sleep(sleepMillis);
      }
      catch ( InterruptedException e )
      {
        e.printStackTrace();
      }
    }

    @Override
    public void open() throws DownloadFailedException
    {
      bais = new ByteArrayInputStream(data);
    }

    public void setException( DownloadFailedException exception )
    {
      this.exception = exception;
    }

    public DownloadFailedException getException()
    {
      return exception;
    }
  }

  public static abstract class MockDestination implements IDownloadDestination
  {
    ByteArrayOutputStream baos;
    boolean finished = false;

    @Override
    public void create() throws WriteToDestinationException
    {
      baos = new ByteArrayOutputStream();
    }

    @Override
    public void write( byte[] data, int size ) throws WriteToDestinationException
    {
      baos.write(data, 0, size);
    }

    @Override
    public void remove() throws WriteToDestinationException
    {
      baos.reset();
    }

    @Override
    public void finish() throws WriteToDestinationException
    {
      finished = true;
    }

    byte[] getBytes()
    {
      return baos.toByteArray();
    }
  }

  public static abstract class DownloadCallback implements DownloadManager.IDownloadStatus
  {
    private DownloadTaskId taskId;
    private DownloadManager downloadManager;
    private DownloadManagerException exception;

    public DownloadCallback( DownloadManager downloadManager )
    {
      this.downloadManager = downloadManager;
    }

    @Override
    public void onFailed( @NonNull DownloadManagerException reason )
    {
      setException(reason);
    }

    public DownloadTaskId getTaskId()
    {
      return taskId;
    }

    public void setTaskId( DownloadTaskId taskId )
    {
      this.taskId = taskId;
    }

    public DownloadManager getDownloadManager()
    {
      return downloadManager;
    }

    public DownloadManagerException getException()
    {
      return exception;
    }

    public void setException( DownloadManagerException exception )
    {
      this.exception = exception;
    }
  }
}
