package com.paragon_software.toolbar_manager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

public class ToolbarManagerHolder
{
  @Nullable
  private static ToolbarManager mManager = null;

  @Nullable
  static ToolbarManager getManager()
  {
    return mManager;
  }

  public static void setManager( @NonNull ToolbarManager manager )
  {
    mManager = manager;
  }
}
