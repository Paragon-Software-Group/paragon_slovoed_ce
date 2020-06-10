/*
 * download manager
 *
 *  Created on: 03.04.18
 *      Author: Mikhail Belyshov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

package com.paragon_software.dictionary_manager.download_manager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.util.concurrent.atomic.AtomicBoolean;

import com.paragon_software.dictionary_manager.download_manager.exceptions.DownloadFailedException;
import com.paragon_software.dictionary_manager.download_manager.exceptions.DownloadManagerException;
import com.paragon_software.dictionary_manager.download_manager.exceptions.WriteToDestinationException;

class DownloadTask implements Runnable
{
  @NonNull
  private final IDownloadSource                 mSource;
  @NonNull
  private final IDownloadDestination            mDestination;
  @NonNull
  private final DownloadManager.IDownloadStatus mStatusCallback;
  @NonNull
  private final DownloadTaskStatus              mTaskStatus;
  @Nullable
  private       IEndTaskNotifier mEndTaskNotifier = null;
  @NonNull
  private final AtomicBoolean    mIsPaused        = new AtomicBoolean(false);
  private final AtomicBoolean    mNeedPause       = new AtomicBoolean(false);
  private final AtomicBoolean    mNeedCancel      = new AtomicBoolean(false);

  @SuppressWarnings( "FieldCanBeLocal" )
  private static int TRANSFER_BUFFER_SIZE = 4096;

  DownloadTask( @NonNull IDownloadSource source, @NonNull IDownloadDestination destination,
                @NonNull DownloadManager.IDownloadStatus statusCallback )
  {
    this.mSource = source;
    this.mDestination = destination;
    this.mStatusCallback = statusCallback;
    this.mTaskStatus = new DownloadTaskStatus();
  }

  void doDownload() throws DownloadFailedException, WriteToDestinationException
  {
    mSource.open();
    mTaskStatus.setStatus(DownloadTaskStatus.DOWNLOAD_TASK_STATUS.IN_PROCESS);
    mStatusCallback.onDownloadStatusChanged();
    final long length = mSource.length();
    mTaskStatus.setTotalSize(length);
    final byte[] buffer = new byte[TRANSFER_BUFFER_SIZE];

    long transferred = 0;
    mDestination.create();
    int read;
    while ( transferred != length )
    {
      read = mSource.read(buffer);
      if ( -1 == read )
      {
        //unexpected end occured
        throw new DownloadFailedException("End of file reached unexpectedly");
      }
      mDestination.write(buffer, read);
      transferred += read;
      mTaskStatus.setCurrentSize(transferred);
      mStatusCallback.onProgress(transferred, length);
      checkPaused();
      if ( mNeedCancel.get() )
      {
        return;
      }
      checkStatusInProgress();
    }
    mSource.finish();
    mDestination.finish();
    mStatusCallback.onFinished();
  }

  private void checkStatusInProgress()
  {
    if ( !mTaskStatus.getStatus().equals(DownloadTaskStatus.DOWNLOAD_TASK_STATUS.IN_PROCESS) )
    {
      mTaskStatus.setStatus(DownloadTaskStatus.DOWNLOAD_TASK_STATUS.IN_PROCESS);
      mStatusCallback.onDownloadStatusChanged();
    }
  }

  @Override
  public void run()
  {
    try
    {
      doDownload();

    }
    catch ( DownloadManagerException e )
    {
      try
      {
        mSource.finish();
        mDestination.remove();
      }
      catch ( WriteToDestinationException ignore )
      {

      }
      mStatusCallback.onFailed(e);
    }
    finally
    {
      mTaskStatus.setStatus(DownloadTaskStatus.DOWNLOAD_TASK_STATUS.FINISHED);
      notifyEnd();
    }

  }

  private void checkPaused()
  {
    synchronized ( mNeedPause )
    {
      //already paused
      if ( mIsPaused.get() )
      {
        return;
      }

      while ( mNeedPause.get() )
      {
        try
        {
          mTaskStatus.setStatus(DownloadTaskStatus.DOWNLOAD_TASK_STATUS.PAUSED);
          mIsPaused.set(true);
          mStatusCallback.onDownloadStatusChanged();
          mNeedPause.wait();
        }
        catch ( InterruptedException e )
        {
          e.printStackTrace();
        }
      }
      mIsPaused.set(false);
    }
  }

  public void cancel()
  {
    mNeedCancel.set(true);
    resume();
  }

  public void resume()
  {
    synchronized ( mNeedPause )
    {
      if ( mNeedPause.compareAndSet(true, false) )
      {
        mNeedPause.notify();
      }

    }
  }

  public void pause()
  {
    mNeedPause.set(true);
  }

  private void notifyEnd()
  {
    if ( null != mEndTaskNotifier )
    {
      mEndTaskNotifier.onTaskEnded();
    }
  }

  @NonNull
  public DownloadTaskStatus getStatus()
  {
    return mTaskStatus;
  }

  public void setOnEndTaskNotifier( @NonNull final IEndTaskNotifier notifier )
  {
    mEndTaskNotifier = notifier;
  }

  interface IEndTaskNotifier
  {
    void onTaskEnded();
  }
}
