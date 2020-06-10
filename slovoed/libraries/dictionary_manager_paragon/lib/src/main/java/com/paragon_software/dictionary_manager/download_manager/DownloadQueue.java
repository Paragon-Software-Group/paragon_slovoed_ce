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

import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;

class DownloadQueue
{
  public final class QueuePosition
  {
    final private long mCurrentPosition;
    final private long mTotal;

    QueuePosition( long currentPosition, long total )
    {
      this.mCurrentPosition = currentPosition;
      this.mTotal = total;
    }

    public long getCurrentPosition()
    {
      return mCurrentPosition;
    }

    public long getTotal()
    {
      return mTotal;
    }
  }

  @NonNull
  private final ThreadPoolExecutor mDownloadThreadPool =
      new ThreadPoolExecutor(1, 1, 60, TimeUnit.SECONDS, new LinkedBlockingQueue< Runnable >());

  void addToQueue( @NonNull final Runnable task )
  {
    mDownloadThreadPool.execute(task);
  }

  void remove( @NonNull final Runnable task )
  {
    mDownloadThreadPool.remove(task);
  }

  @NonNull
  QueuePosition getQueuePosition( @NonNull final Runnable task )
  {
    //todo fix it
    return new QueuePosition(0, 0);
  }

}
