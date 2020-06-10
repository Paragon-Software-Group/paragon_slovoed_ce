package com.paragon_software.news_manager;

import android.content.Context;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.screen_opener_api.ScreenOpenerAPI;
import com.paragon_software.hint_manager.HintManagerAPI;
import com.paragon_software.settings_manager.SettingsManagerAPI;
import com.paragon_software.toolbar_manager.ToolbarManager;

public class BaseNewsManagerFactory implements NewsManagerAPI.Factory
{
  @Nullable
  private SettingsManagerAPI mSettingsManager;
  @Nullable
  private ToolbarManager mToolbarManager;
  @Nullable
  private HintManagerAPI   mHintManager;
  private ScreenOpenerAPI mScreenOpener;

  @Override
  public NewsManagerAPI create( @NonNull Context context, @NonNull NewsNotificationOptions notificationOptions,
                                @NonNull String catalogOrProductId, @NonNull NewsManagerAPI.Mode mode, @Nullable String pkey )
  {
    return new BaseNewsManager(context, mSettingsManager, mToolbarManager, mHintManager, mScreenOpener, notificationOptions, catalogOrProductId, mode, pkey);
  }

  @Override
  public NewsManagerAPI.Factory registerSettingsManager( @Nullable SettingsManagerAPI settingManager )
  {
    mSettingsManager = settingManager;
    return this;
  }

  @Override
  public NewsManagerAPI.Factory registerToolbarManager( @Nullable ToolbarManager toolbarManager )
  {
    mToolbarManager = toolbarManager;
    return this;
  }

  @Override
  public NewsManagerAPI.Factory registerHintManager( @Nullable HintManagerAPI hintManager )
  {
    mHintManager = hintManager;
    return this;
  }

  @Override
  public NewsManagerAPI.Factory registerScreenOpener(@Nullable ScreenOpenerAPI screenOpener) {
    mScreenOpener = screenOpener;
    return this;
  }
}
