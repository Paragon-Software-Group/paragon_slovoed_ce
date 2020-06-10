package com.paragon_software.settings_manager;

import androidx.annotation.NonNull;

public interface OnApplicationSettingsSaveListener extends SettingsManagerAPI.Notifier
{
  void onApplicationSettingsSaved( @NonNull ApplicationSettings applicationSettings );
}
