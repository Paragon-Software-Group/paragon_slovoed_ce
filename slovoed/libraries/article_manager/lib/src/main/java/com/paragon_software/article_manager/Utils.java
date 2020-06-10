package com.paragon_software.article_manager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.io.Serializable;

import com.paragon_software.settings_manager.SettingsManagerAPI;
import com.paragon_software.settings_manager.exceptions.LocalResourceUnavailableException;
import com.paragon_software.settings_manager.exceptions.ManagerInitException;
import com.paragon_software.settings_manager.exceptions.WrongTypeException;

final class Utils
{

  private Utils() { }

  static void saveSetting( @Nullable SettingsManagerAPI settingsManager, @NonNull String name, @NonNull Serializable value )
  {
    if ( settingsManager != null )
    {
      try
      {
        settingsManager.save(name, value, true);
      }
      catch ( LocalResourceUnavailableException | ManagerInitException ignore )
      {

      }
    }
  }

  @NonNull
  static < T > T loadSetting( @Nullable SettingsManagerAPI settingsManager, @NonNull String name, @NonNull T defaultValue )
  {
    T res = null;
    if ( settingsManager != null )
    {
      try
      {
        res = settingsManager.load(name, defaultValue);
      }
      catch ( WrongTypeException | ManagerInitException ignore )
      {

      }
    }
    if ( res == null )
    {
      res = defaultValue;
    }
    return res;
  }
}
