package com.paragon_software.search_manager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

public class SearchManagerHolder
{
  @Nullable
  private static SearchManagerAPI mManager = null;

  @Nullable
  static SearchManagerAPI getManager()
  {
    return mManager;
  }

  public static void setManager( @NonNull SearchManagerAPI manager )
  {
    mManager = manager;
  }
}
