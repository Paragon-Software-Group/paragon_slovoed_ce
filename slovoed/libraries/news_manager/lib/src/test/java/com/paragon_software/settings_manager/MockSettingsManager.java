package com.paragon_software.settings_manager;

import androidx.annotation.NonNull;

import com.paragon_software.settings_manager.exceptions.LocalResourceUnavailableException;
import com.paragon_software.settings_manager.exceptions.ManagerInitException;
import com.paragon_software.settings_manager.exceptions.WrongTypeException;

import java.io.Serializable;
import java.util.Map;
import java.util.TreeMap;

public abstract class MockSettingsManager extends SettingsManagerAPI
{
  private Map< String, Object > settings = new TreeMap<>();
  private ApplicationSettings appSettings;

  protected MockSettingsManager(ApplicationSettings appSettings) {
    this.appSettings = appSettings;
  }


  @Override
  public void save( @NonNull String s, @NonNull Serializable serializable, boolean b )
          throws ManagerInitException, LocalResourceUnavailableException
  {
    settings.put(s, serializable);
  }

  @Override
  public < T > T load( @NonNull String s, @NonNull T t ) throws WrongTypeException, ManagerInitException
  {
    T res = null;
    try
    {
      res = (T) settings.get(s);
    }
    catch ( Exception ignore )
    {

    }
    if ( res == null )
    {
      res = t;
    }
    return res;
  }

  @NonNull
  @Override
  public ApplicationSettings loadApplicationSettings()
  {
    return appSettings;
  }
}
