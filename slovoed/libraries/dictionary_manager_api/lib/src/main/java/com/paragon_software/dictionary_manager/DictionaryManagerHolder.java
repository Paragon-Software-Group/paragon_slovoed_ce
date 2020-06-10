/*
 * dictionary mManager api
 *
 *  Created on: 09.04.18
 *      Author: Mikhail Belyshov
 * Last Modify: $Idc$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

package com.paragon_software.dictionary_manager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

public final class DictionaryManagerHolder
{
  @Nullable
  private static DictionaryManagerAPI mManager = null;

  public static void setManager( @NonNull DictionaryManagerAPI dictionaryManager )
  {
    mManager = dictionaryManager;
  }

  @NonNull
  public static DictionaryManagerAPI getManager()
  {
    if ( mManager != null )
      return mManager;
    else
      throw new IllegalStateException("Dictionary manager must be initialized at this stage");
  }
}
