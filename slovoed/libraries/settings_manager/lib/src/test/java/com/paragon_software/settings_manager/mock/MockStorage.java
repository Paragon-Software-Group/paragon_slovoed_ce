package com.paragon_software.settings_manager.mock;

import androidx.annotation.NonNull;

import com.paragon_software.settings_manager.ISettingsStorage;

public abstract class MockStorage implements ISettingsStorage
{
  private final String name;

  public MockStorage(@NonNull final String name )
  {
    this.name = name;
  }

  @Override
  public String getName()
  {
    return name;
  }
}
