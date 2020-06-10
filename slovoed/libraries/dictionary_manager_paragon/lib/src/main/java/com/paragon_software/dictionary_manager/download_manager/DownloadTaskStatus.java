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

@SuppressWarnings( "unused" )
public final class DownloadTaskStatus
{
  public enum DOWNLOAD_TASK_STATUS
  {
    IN_QUEUE,
    IN_PROCESS,
    PAUSED,
    FINISHED
  }

  @NonNull
  private DOWNLOAD_TASK_STATUS        mStatus        = DOWNLOAD_TASK_STATUS.IN_QUEUE;
  @Nullable
  private DownloadQueue.QueuePosition mQueuePosition = null;
  private long                        mCurrentSize   = 0;
  private long                        mTotalSize     = 0;
  private long                        mDownloadTime  = 0;

  @NonNull
  public DOWNLOAD_TASK_STATUS getStatus()
  {
    return mStatus;
  }

  @Nullable
  public DownloadQueue.QueuePosition getQueuePosition()
  {
    return mQueuePosition;
  }

  public long getCurrentSize()
  {
    return mCurrentSize;
  }

  public long getTotalSize()
  {
    return mTotalSize;
  }

  public long getDownloadTime()
  {
    return mDownloadTime;
  }

  public void setStatus( @NonNull DOWNLOAD_TASK_STATUS status )
  {
    this.mStatus = status;
  }

  void setQueuePosition( @Nullable DownloadQueue.QueuePosition mQueuePosition )
  {
    this.mQueuePosition = mQueuePosition;
  }

  void setCurrentSize( long currentSize )
  {
    this.mCurrentSize = currentSize;
  }

  void setTotalSize( long totalSize )
  {
    this.mTotalSize = totalSize;
  }

  void setDownloadTime( long downloadTime )
  {
    this.mDownloadTime = downloadTime;
  }
}
