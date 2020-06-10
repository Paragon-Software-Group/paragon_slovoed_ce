package com.paragon_software.settings_manager;

import java.io.Serializable;

public interface OnSettingsSaveListener extends SettingsManagerAPI.Notifier
{
  void onSettingsSaved( String name, Serializable data );
}
