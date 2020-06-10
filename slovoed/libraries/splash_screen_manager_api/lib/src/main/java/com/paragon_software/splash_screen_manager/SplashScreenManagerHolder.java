package com.paragon_software.splash_screen_manager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;


public class SplashScreenManagerHolder
{
  @Nullable
  private static ISplashScreenManager mManager = null;

  @Nullable
  static ISplashScreenManager getManager()
  {
    return mManager;
  }

  public static void setManager( @NonNull ISplashScreenManager manager )
  {
    mManager = manager;
  }
}
