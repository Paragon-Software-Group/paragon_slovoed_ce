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

import androidx.annotation.VisibleForTesting;

import java.util.concurrent.atomic.AtomicLong;

public class DownloadTaskId
{
  private final long mId;
  private static volatile AtomicLong ID_GLOBAL_COUNTER = new AtomicLong(0);

  @VisibleForTesting
  DownloadTaskId(long id)
  {
    mId = id;
  }

  DownloadTaskId()
  {
    mId = ID_GLOBAL_COUNTER.getAndIncrement();
  }

  @Override
  public int hashCode()
  {
    return Long.valueOf(mId).hashCode();
  }

  @SuppressWarnings( "SimplifiableIfStatement" )
  @Override
  public boolean equals( Object obj )
  {
    if ( this == obj )
    {
      return true;
    }
    if ( null == obj )
    {
      return false;
    }
    if ( getClass() != obj.getClass() )
    {
      return false;
    }
    return mId == ( (DownloadTaskId) obj ).mId;
  }

  @Override
  protected Object clone() throws CloneNotSupportedException
  {
    throw new CloneNotSupportedException();
  }

  @Override
  public String toString()
  {
    return String.valueOf(mId);
  }
}
