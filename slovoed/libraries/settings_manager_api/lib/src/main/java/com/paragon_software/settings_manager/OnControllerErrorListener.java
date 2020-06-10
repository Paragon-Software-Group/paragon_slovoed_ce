package com.paragon_software.settings_manager;

interface OnControllerErrorListener extends SettingsControllerAPI.Notifier
{
  void onControllerError( Exception exception );
}
