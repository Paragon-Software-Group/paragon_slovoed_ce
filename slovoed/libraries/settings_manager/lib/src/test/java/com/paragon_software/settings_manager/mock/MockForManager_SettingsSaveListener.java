package com.paragon_software.settings_manager.mock;

import com.paragon_software.settings_manager.OnSettingsSaveListener;

import java.io.Serializable;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

public abstract class MockForManager_SettingsSaveListener extends MockBaseChangeListener
        implements OnSettingsSaveListener
{

  private String expectedName;

  private Serializable expectedData;

  public MockForManager_SettingsSaveListener(int maxNumberOfCalls )
  {
    super(maxNumberOfCalls);
  }

  public void setExpectedSetting( String name, Serializable data )
  {
    this.expectedName = name;
    this.expectedData = data;
  }

  @Override
  public void onSettingsSaved( String name, Serializable data )
  {
    assertTrue(mNumberOfCalls++ < mMaxNumberOfCalls);
    assertEquals(expectedName, name);
    assertEquals(expectedData, data);
  }
}