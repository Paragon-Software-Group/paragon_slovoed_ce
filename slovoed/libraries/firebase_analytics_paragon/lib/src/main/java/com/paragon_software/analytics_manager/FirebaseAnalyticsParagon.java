package com.paragon_software.analytics_manager;

import android.content.Context;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.google.firebase.analytics.FirebaseAnalytics;
import com.paragon_software.settings_manager.ApplicationSettings;
import com.paragon_software.settings_manager.OnApplicationSettingsSaveListener;
import com.paragon_software.settings_manager.SettingsManagerAPI;

public class FirebaseAnalyticsParagon extends AnalyticsManagerAPI implements OnApplicationSettingsSaveListener
{
  @Nullable
  private Context appContext;
  @Nullable
  private Boolean enabled;

  @Override
  void initializeInternal( Context appContext, SettingsManagerAPI settingsManager )
  {
    if ( null == appContext || null == settingsManager )
    {
      throw new IllegalArgumentException("Cant initalize " + FirebaseAnalyticsParagon.class.getCanonicalName()
                                             + ". Context or settings manager is null");
    }
    this.appContext = appContext;
    settingsManager.registerNotifier(this);
    setAnalyticsEnabled(settingsManager.loadApplicationSettings().isStatisticsEnabled());
  }

  @Override
  public void logEvent( @NonNull Event event )
  {
    if ( null != enabled && enabled && null != appContext )
    {
      FirebaseAnalytics.getInstance(appContext).logEvent(event.getName(), event.getBundle());
    }
  }

  private void setAnalyticsEnabled( boolean enabled )
  {
    if ( appContext != null && ( null == this.enabled || this.enabled != enabled ) )
    {
      FirebaseAnalytics.getInstance(appContext).setAnalyticsCollectionEnabled(enabled);
      this.enabled = enabled;
    }
  }

  @Override
  public void onApplicationSettingsSaved( @NonNull ApplicationSettings applicationSettings )
  {
    setAnalyticsEnabled(applicationSettings.isStatisticsEnabled());
  }
}
