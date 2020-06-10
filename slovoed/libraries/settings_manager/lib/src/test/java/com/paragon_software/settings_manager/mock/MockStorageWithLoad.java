package com.paragon_software.settings_manager.mock;

import androidx.annotation.NonNull;

import com.paragon_software.settings_manager.ISettingsStorage;
import com.paragon_software.settings_manager.exceptions.StorageTransferException;

public abstract class MockStorageWithLoad extends MockStorage
{

  byte[] loadResult = new byte[0];

  MockStorageWithLoad(@NonNull final String name )
  {
    super(name);
  }

  public void setLoadResult( byte[] newResult )
  {
    loadResult = newResult;
  }

  @Override
  public void load( @NonNull ISettingsStorage.ILoadSettingsResult iLoadSettingsResult )
      throws StorageTransferException
  {
    iLoadSettingsResult.onProgress(1, 2);
    iLoadSettingsResult.onProgress(2, 2);
    iLoadSettingsResult.onFinished(loadResult);
  }
}