package com.paragon_software.settings_manager;

import androidx.annotation.NonNull;

import com.paragon_software.screen_opener_api.ScreenType;

abstract class SettingsControllerAPI
{

  abstract void registerNotifier( @NonNull Notifier notifier );

  abstract void unregisterNotifier( @NonNull Notifier notifier );

  abstract void saveNewApplicationSettings( ApplicationSettings newApplicationSettings );

  abstract ApplicationSettings getApplicationSettings();

  abstract void openScreen(ScreenType screenType);

  /**
   * Base interface of {@link SettingsControllerAPI} notifiers.
   */
  interface Notifier
  {
  }
}
