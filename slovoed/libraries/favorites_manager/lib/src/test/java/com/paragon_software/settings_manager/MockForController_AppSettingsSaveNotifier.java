package com.paragon_software.settings_manager;

import java.util.ArrayList;

public abstract class MockForController_AppSettingsSaveNotifier
{

  private final ArrayList<OnApplicationSettingsSaveListener> mOnApplicationSettingsSaveListeners = new ArrayList<>();

  public void registerNotifier( OnApplicationSettingsSaveListener notifier )
  {
    if (!mOnApplicationSettingsSaveListeners.contains(notifier))
    {
      mOnApplicationSettingsSaveListeners.add(notifier);
    }
  }

  public void onApplicationSettingsSave( ApplicationSettings applicationSettings )
  {
    for (final OnApplicationSettingsSaveListener listener : mOnApplicationSettingsSaveListeners )
    {
      listener.onApplicationSettingsSaved(applicationSettings);
    }
  }
}