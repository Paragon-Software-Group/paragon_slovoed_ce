package com.paragon_software.hint_manager;

import androidx.annotation.Nullable;

public class HintManagerHolder
{
  @Nullable
  private static HintManagerAPI mManager = null;

  @Nullable
  static HintManagerAPI getManager()
  {
    return mManager;
  }

  public static void setManager( @Nullable HintManagerAPI manager )
  {
    mManager = manager;
  }
}
