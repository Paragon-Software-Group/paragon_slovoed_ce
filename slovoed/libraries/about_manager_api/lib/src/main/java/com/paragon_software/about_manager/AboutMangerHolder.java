package com.paragon_software.about_manager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

public class AboutMangerHolder
{
  @Nullable
  private static AboutManagerAPI mManager = null;

  @NonNull
  static AboutManagerAPI getManager() throws IllegalStateException
  {
    if ( null == mManager )
    {
      throw new IllegalStateException("AboutManagerHolder uninitialized");
    }
    return mManager;
  }

  public static void setManager( @NonNull AboutManagerAPI manager )
  {
    mManager = manager;
  }
}
