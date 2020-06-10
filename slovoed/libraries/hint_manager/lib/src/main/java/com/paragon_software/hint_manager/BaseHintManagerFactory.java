package com.paragon_software.hint_manager;

import androidx.annotation.Nullable;

import com.paragon_software.settings_manager.SettingsManagerAPI;

public class BaseHintManagerFactory implements HintManagerAPI.Factory
{

  private SettingsManagerAPI mSettingsManager;

  private HintUIHelperAPI mHintUIHelper;

  @Override
  public HintManagerAPI create()
  {
    BaseHintManager res = new BaseHintManager( mHintUIHelper, mSettingsManager );
    return res;
  }

  @Override
  public HintManagerAPI.Factory registerSettingsManager( @Nullable SettingsManagerAPI settingsManager )
  {
    mSettingsManager = settingsManager;
    return this;
  }

  @Override
  public HintManagerAPI.Factory registerHintUIHelper( @Nullable HintUIHelperAPI hintUIHelper )
  {
    mHintUIHelper = hintUIHelper;
    return this;
  }
}
