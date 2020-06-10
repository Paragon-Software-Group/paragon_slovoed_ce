package com.paragon_software.settings_manager.mock;

import androidx.annotation.NonNull;

import com.paragon_software.settings_manager.ApplicationSettings;
import com.paragon_software.settings_manager.OnApplicationSettingsSaveListener;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

public abstract class MockForManager_AppSettingsSaveListener extends MockBaseChangeListener
          implements OnApplicationSettingsSaveListener
{
  private ApplicationSettings expectedApplicationSettings;

  public MockForManager_AppSettingsSaveListener(int maxNumberOfCalls )
  {
    super(maxNumberOfCalls);
  }

  public void setExpectedApplicationSettings( ApplicationSettings applicationSettings )
  {
    expectedApplicationSettings = applicationSettings;
  }

  @Override
  public void onApplicationSettingsSaved( @NonNull ApplicationSettings applicationSettings )
  {
    assertTrue(mNumberOfCalls++ < mMaxNumberOfCalls);
    assertEquals(expectedApplicationSettings, applicationSettings);
  }
}