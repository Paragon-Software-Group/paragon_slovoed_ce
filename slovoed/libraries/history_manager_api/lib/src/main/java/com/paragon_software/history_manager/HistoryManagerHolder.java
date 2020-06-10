package com.paragon_software.history_manager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

public class HistoryManagerHolder
{
  @Nullable
  private static HistoryManagerAPI mManager = null;

  @Nullable
  static HistoryManagerAPI getManager()
  {
    return mManager;
  }

  public static void setManager( @NonNull HistoryManagerAPI manager )
  {
    mManager = manager;
  }
}
