package com.paragon_software.analytics_manager;

import android.content.Context;
import androidx.annotation.NonNull;
import androidx.annotation.VisibleForTesting;

import com.paragon_software.settings_manager.SettingsManagerAPI;

public abstract class AnalyticsManagerAPI
{
  private static AnalyticsManagerAPI instance = null;

  private static void setInstance( @NonNull AnalyticsManagerAPI analyticsManager ) throws IllegalStateException
  {
    if ( null != instance )
    {
      throw new IllegalStateException("Analytics already initialized. Multiple instance or initialization unavailable");
    }
    instance = analyticsManager;
  }

  public final void initialize( Context appContext, SettingsManagerAPI settingsManager ) throws IllegalStateException
  {
    initializeInternal(appContext, settingsManager);
    setInstance(this);
  }

  @NonNull
  public static AnalyticsManagerAPI get() throws IllegalStateException
  {
    if ( null == instance )
    {
      throw new IllegalStateException("Analytics uninitialized yet");
    }
    return instance;
  }

  abstract void initializeInternal( Context appContext, SettingsManagerAPI settingsManager );

  public abstract void logEvent( @NonNull Event event );

  @VisibleForTesting
  public static void freeInstance()
  {
    instance = null;
  }
}
