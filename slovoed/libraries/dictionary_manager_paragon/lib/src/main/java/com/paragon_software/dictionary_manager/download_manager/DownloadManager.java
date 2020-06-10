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

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import com.paragon_software.dictionary_manager.download_manager.exceptions.DownloadManagerException;
import com.paragon_software.dictionary_manager.download_manager.exceptions.NoSuchCallbackException;
import com.paragon_software.dictionary_manager.download_manager.exceptions.NoSuchDownloadTaskException;

@SuppressWarnings( "unused" )
public final class DownloadManager
{
  private String mUserAgent;

  public interface IDownloadStatus
  {
    /**
     * Called when another part of file was downloaded
     *
     * @param currentSize how many bytes downloaded for now
     * @param totalSize   total downloading file size
     */
    void onProgress( final long currentSize, final long totalSize );

    /**
     * called when download finished successfully
     */
    void onFinished();

    /**
     * On status changed. IN_PROCESS <-> PAUSED
     */
    void onDownloadStatusChanged();

    /**
     * called when download process failed
     *
     * @param reason contains exception that has been the fail reason
     */
    void onFailed( @NonNull final DownloadManagerException reason );
  }

  @NonNull
  private final HashMap< DownloadTaskId, DownloadTask > mDownloads = new HashMap<>();

  @NonNull
  private final HashMap< DownloadTaskId, ObservableCallback > mCallbacks = new HashMap<>();


  @NonNull
  private final DownloadQueue mDownloadQueue = new DownloadQueue();

  @NonNull
  public DownloadTaskId downloadNow( @NonNull final IDownloadSource source,
                                     @NonNull final IDownloadDestination dest,
                                     @NonNull final IDownloadStatus downloadStatus)
  {
    final DownloadTaskId taskId = new DownloadTaskId();
    ObservableCallback callback = new ObservableCallback();
    DownloadTask task = prepareDownloadTask(source, dest, callback);
    putDownloadTask(taskId, task);
    putCallback(taskId, callback);
    registerObserverSilent(downloadStatus, taskId);
    new Thread(task).start();
    return taskId;
  }

  private void registerObserverSilent( @NonNull IDownloadStatus downloadStatus, DownloadTaskId taskId )
  {
    try
    {
      registerObserver(taskId, downloadStatus);
    }
    catch ( NoSuchCallbackException e )
    {
      e.printStackTrace();
    }
  }

  private void putCallback( @NonNull DownloadTaskId taskId, @NonNull ObservableCallback callback )
  {
    synchronized ( mCallbacks )
    {
      mCallbacks.put(taskId, callback);
    }
  }

  @NonNull
  private DownloadTask prepareDownloadTask( @NonNull final IDownloadSource source,
                                            @NonNull final IDownloadDestination dest,
                                            @NonNull final IDownloadStatus statusCallback )
  {
    return new DownloadTask(source, dest, statusCallback);
  }

  private void putDownloadTask( @NonNull final DownloadTaskId taskId, @NonNull final DownloadTask task )
  {
    synchronized ( mDownloads )
    {
      mDownloads.put(taskId, task);
      task.setOnEndTaskNotifier(new DownloadTask.IEndTaskNotifier()
      {
        @Override
        public void onTaskEnded()
        {
          synchronized ( mDownloads )
          {
            mDownloads.remove(taskId);
          }
          synchronized ( mCallbacks )
          {
            ObservableCallback remove = mCallbacks.remove(taskId);
            if (null != remove)
            {
              remove.unregisterAllObservers();
            }
          }
        }
      });
    }
  }

  @NonNull
  public DownloadTaskId addToQueue( @NonNull final IDownloadSource source,
                                    @NonNull final IDownloadDestination dest,
                                    @NonNull final IDownloadStatus statusCallback )
  {
    final DownloadTaskId taskId = new DownloadTaskId();
    DownloadTask task = prepareDownloadTask(source, dest, statusCallback);
    putDownloadTask(taskId, task);
    mDownloadQueue.addToQueue(task);
    return taskId;
  }

  @NonNull
  private DownloadTask getDownloadTaskById( @NonNull final DownloadTaskId id ) throws NoSuchDownloadTaskException
  {
    synchronized ( mDownloads )
    {
      DownloadTask task = mDownloads.get(id);
      if ( null == task )
      {
        throw new NoSuchDownloadTaskException();
      }
      return task;
    }
  }

  private ObservableCallback getCallbackById( DownloadTaskId taskId ) throws NoSuchCallbackException
  {
    synchronized ( mCallbacks )
    {
      ObservableCallback observableCallback = mCallbacks.get(taskId);
      if (null == observableCallback)
      {
        throw new NoSuchCallbackException();
      }
      return observableCallback;
    }
  }

  /**
   * All observers automatically unregister after download task complete
   */
  public void registerObserver (DownloadTaskId taskId, IDownloadStatus downloadStatus) throws NoSuchCallbackException
  {
    ObservableCallback callback = getCallbackById(taskId);
    callback.registerObserver(downloadStatus);
  }

  /**
   * This method is optional to use.
   * All observers automatically unregister after download task complete
   */
  public void unregisterObserver (DownloadTaskId taskId, IDownloadStatus downloadStatus) throws NoSuchCallbackException
  {
    ObservableCallback callback = getCallbackById(taskId);
    callback.unregisterObserver(downloadStatus);
  }

  @NonNull
  public DownloadTaskStatus getDownloadTaskStatus( @NonNull final DownloadTaskId id ) throws NoSuchDownloadTaskException
  {
    return getDownloadTaskById(id).getStatus();
  }

  public void cancelDownloadTask( @NonNull final DownloadTaskId id ) throws NoSuchDownloadTaskException
  {
    getDownloadTaskById(id).cancel();
  }

  public void pauseDownloadTask( @NonNull final DownloadTaskId id ) throws NoSuchDownloadTaskException
  {
    getDownloadTaskById(id).pause();
  }

  public void resumeDownloadTask( @NonNull final DownloadTaskId id ) throws NoSuchDownloadTaskException
  {
    getDownloadTaskById(id).resume();
  }

  public void setUserAgent( @NonNull String userAgent )
  {
    mUserAgent = userAgent;
  }

  public String getUserAgent()
  {
    return mUserAgent;
  }

  private static class ObservableCallback implements IDownloadStatus
  {
    List< IDownloadStatus > observers = new ArrayList<>();

    @Override
    public void onProgress( long currentSize, long totalSize )
    {
      for ( IDownloadStatus observer : new ArrayList<>(observers) )
      {
        observer.onProgress(currentSize, totalSize);
      }
    }

    @Override
    public void onFinished()
    {
      for ( IDownloadStatus observer : new ArrayList<>(observers) )
      {
        observer.onFinished();
      }
    }

    @Override
    public void onDownloadStatusChanged()
    {
      for ( IDownloadStatus observer : new ArrayList<>(observers) )
      {
        observer.onDownloadStatusChanged();
      }
    }

    @Override
    public void onFailed( @NonNull DownloadManagerException reason )
    {
      for ( IDownloadStatus observer : new ArrayList<>(observers) )
      {
        observer.onFailed(reason);
      }
    }

    void registerObserver( @NonNull IDownloadStatus downloadStatus )
    {
      observers.add(downloadStatus);
    }

    void unregisterObserver( @NonNull IDownloadStatus downloadStatus )
    {
      observers.remove(downloadStatus);
    }

    public void unregisterAllObservers()
    {
      observers.clear();
    }
  }
}
