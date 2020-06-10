package com.paragon_software.settings_manager;

/**
 * Interface for notifiers to notify observers about {@link SettingsControllerAPI} controller application settings
 * state changes.
 */
interface OnControllerApplicationSettingsChangeListener extends SettingsControllerAPI.Notifier
{
  void onSettingsChanged();
}
