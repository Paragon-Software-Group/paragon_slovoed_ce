package com.paragon_software.hint_manager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.FragmentManager;

import com.paragon_software.settings_manager.SettingsManagerAPI;

public abstract class HintManagerAPI
{
  public interface Factory
  {
    HintManagerAPI create();

    Factory registerSettingsManager( @Nullable SettingsManagerAPI settingsManager );

    Factory registerHintUIHelper( @Nullable HintUIHelperAPI hintUIHelper );
  }

  abstract void setHintParams( @NonNull HintType hintType, @Nullable HintParams hintParams );
  abstract @Nullable HintParams getHintParams( @NonNull HintType hintType );

  public abstract boolean isNeedToShowHint( @NonNull HintType hintType );
  public abstract void setNeedToShowHint( @NonNull HintType hintType, boolean show );

  public abstract boolean showHintDialog( @Nullable HintType hintType, @Nullable FragmentManager fragmentManager, @Nullable HintParams hintParams );
}
