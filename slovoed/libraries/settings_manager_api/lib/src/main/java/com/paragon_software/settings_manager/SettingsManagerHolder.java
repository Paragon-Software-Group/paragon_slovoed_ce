package com.paragon_software.settings_manager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

public class SettingsManagerHolder
{
  @Nullable
  private static SettingsManagerAPI mManager = null;

  public static void setManager( @NonNull SettingsManagerAPI manager )
  {
    mManager = manager;
  }

  @Nullable
  public static SettingsManagerAPI getManager()
  {
    return mManager;
  }
}
