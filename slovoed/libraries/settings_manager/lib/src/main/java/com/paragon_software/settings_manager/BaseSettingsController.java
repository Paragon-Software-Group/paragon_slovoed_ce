package com.paragon_software.settings_manager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.util.ArrayList;
import java.util.List;

import com.paragon_software.screen_opener_api.ScreenOpenerAPI;
import com.paragon_software.screen_opener_api.ScreenType;
import com.paragon_software.settings_manager.exceptions.LocalResourceUnavailableException;
import com.paragon_software.settings_manager.exceptions.ManagerInitException;

public class BaseSettingsController extends SettingsControllerAPI implements OnApplicationSettingsSaveListener
{
  private final SettingsManagerAPI mManager;

  private final List< OnControllerApplicationSettingsChangeListener > mOnApplicationSettingsChangeListener = new ArrayList<>();
  private final List< OnControllerErrorListener >                     mOnControllerErrorListeners          = new ArrayList<>();
  @Nullable
  private final ScreenOpenerAPI mScreenOpener;

  private ApplicationSettings mControllerApplicationSettingsState;

  BaseSettingsController(@NonNull SettingsManagerAPI manager, @Nullable ScreenOpenerAPI screenOpener)
  {
    mManager = manager;
    mScreenOpener = screenOpener;
    mControllerApplicationSettingsState = mManager.loadApplicationSettings();
    mManager.registerNotifier(this);
  }

  @Override
  public void registerNotifier( @NonNull Notifier notifier )
  {
    if (notifier instanceof OnControllerApplicationSettingsChangeListener && !mOnApplicationSettingsChangeListener.contains(notifier))
    {
      mOnApplicationSettingsChangeListener.add((OnControllerApplicationSettingsChangeListener) notifier);
    }
    if (notifier instanceof OnControllerErrorListener && !mOnControllerErrorListeners.contains(notifier))
    {
      mOnControllerErrorListeners.add((OnControllerErrorListener) notifier);
    }
  }

  @Override
  public void unregisterNotifier( @NonNull Notifier notifier )
  {
    if (notifier instanceof OnControllerApplicationSettingsChangeListener)
    {
      mOnApplicationSettingsChangeListener.remove(notifier);
    }
    if (notifier instanceof OnControllerErrorListener)
    {
      mOnControllerErrorListeners.remove(notifier);
    }
  }

  @Override
  void saveNewApplicationSettings( ApplicationSettings newApplicationSettings )
  {
    if ( !mControllerApplicationSettingsState.equals(newApplicationSettings) )
    {
      saveApplicationSettings(newApplicationSettings);
    }
  }

  @Override
  ApplicationSettings getApplicationSettings()
  {
    return new ApplicationSettings(mControllerApplicationSettingsState);
  }

  @Override
  void openScreen(ScreenType screenType) {
    if (mScreenOpener != null) {
      mScreenOpener.openScreen(screenType);
    }
  }

  private void saveApplicationSettings( @NonNull ApplicationSettings settingsToSave )
  {
    try
    {
      mManager.saveApplicationSettings(settingsToSave);
    }
    catch ( ManagerInitException | LocalResourceUnavailableException exception )
    {
      notifyError(exception);
    }
  }

  private void notifySettingsChange()
  {
    for (OnControllerApplicationSettingsChangeListener listener : mOnApplicationSettingsChangeListener)
    {
      listener.onSettingsChanged();
    }
  }

  private void notifyError( Exception exception )
  {
    for (OnControllerErrorListener listener : mOnControllerErrorListeners)
    {
      listener.onControllerError(exception);
    }
  }

  @Override
  public void onApplicationSettingsSaved( @NonNull ApplicationSettings applicationSettings )
  {
    if ( !mControllerApplicationSettingsState.equals(applicationSettings) )
    {
      mControllerApplicationSettingsState = new ApplicationSettings(applicationSettings);
      notifySettingsChange();
    }
  }
}
