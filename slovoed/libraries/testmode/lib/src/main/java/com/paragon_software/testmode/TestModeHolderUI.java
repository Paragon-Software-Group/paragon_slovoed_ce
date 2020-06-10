package com.paragon_software.testmode;

import androidx.annotation.NonNull;

public class TestModeHolderUI
{
  private static TestModeAPI mTestMode = null;

  static void set( @NonNull TestModeAPI testMode )
  {
    mTestMode = testMode;
  }

  @NonNull
  public static TestModeAPI get()
  {
    if ( mTestMode == null )
    {
      throw new IllegalStateException("Test Mode must be initialized at this stage.");
    }
    return mTestMode;
  }
}
