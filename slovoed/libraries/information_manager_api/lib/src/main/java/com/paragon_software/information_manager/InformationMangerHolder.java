package com.paragon_software.information_manager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

public class InformationMangerHolder
{
  @Nullable
  private static InformationManagerAPI mManager = null;

  @NonNull
  public static InformationManagerAPI getManager()
  {
    if ( null != mManager )
    {
      return mManager;
    }
    else
    {
      throw new IllegalStateException("Uninitialized information manager holder");
    }
  }

  public static void setManager( @NonNull InformationManagerAPI manager )
  {
    mManager = manager;
  }
}
