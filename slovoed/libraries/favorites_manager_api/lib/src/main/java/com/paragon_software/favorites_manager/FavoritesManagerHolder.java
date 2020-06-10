package com.paragon_software.favorites_manager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

public class FavoritesManagerHolder
{
  @Nullable
  private static FavoritesManagerAPI mManager = null;

  @Nullable
  static FavoritesManagerAPI getManager()
  {
    return mManager;
  }

  public static void setManager( @NonNull FavoritesManagerAPI manager )
  {
    mManager = manager;
  }
}
