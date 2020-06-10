package com.paragon_software.user_core_manager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

public class UserCoreManagerHolder
{
  @Nullable
  private static UserCoreManagerAPI mManager = null;

  @NonNull
  static UserCoreManagerAPI getManager() throws IllegalStateException
  {
    if ( null == mManager )
    {
      throw new IllegalStateException("UserCoreManagerHolder uninitialized");
    }
    return mManager;
  }

  public static void setManager( @Nullable UserCoreManagerAPI manager )
  {
    mManager = manager;
  }
}
