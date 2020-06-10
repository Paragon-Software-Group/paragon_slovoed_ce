package com.paragon_software.settings_manager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.settings_manager.exceptions.LocalResourceUnavailableException;

import java.util.HashMap;

public abstract class MockContainer extends SettingsContainer
{
  private HashMap< String, String > mData = new HashMap<>();

  public MockContainer( @NonNull PersistentSettingsContainer persistentSettingsContainer,
                        @NonNull LocalSettingsContainer localSettingsContainer )
  {
    super(persistentSettingsContainer, localSettingsContainer);
  }

  @Override
  void putStringData( @NonNull String name, @NonNull String data, @NonNull DATA_TYPE dataType )
      throws LocalResourceUnavailableException
  {
    mData.put(name, data);
  }

  @Nullable
  @Override
  String loadStringData( @NonNull String name )
  {
    return mData.get(name);
  }
}