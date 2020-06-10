package com.paragon_software.analytics_manager;

import android.content.Context;
import androidx.annotation.NonNull;

import com.paragon_software.settings_manager.SettingsManagerAPI;

public final class EmptyAnalyticsManger extends AnalyticsManagerAPI
{
  @Override
  void initializeInternal( Context appContext, SettingsManagerAPI settingsManager )
  {
  }

  @Override
  public void logEvent( @NonNull Event event )
  {
  }
}
