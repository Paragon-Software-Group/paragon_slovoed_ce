package com.paragon_software.settings_manager;

import android.content.Context;

public interface MigrationHelper {
  void migrateSettings(SettingsManagerAPI settingsManager, Context context);
}
