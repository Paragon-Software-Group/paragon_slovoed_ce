package com.paragon_software.settings_manager;


import com.paragon_software.settings_manager.exceptions.LocalResourceUnavailableException;
import com.paragon_software.settings_manager.exceptions.ManagerInitException;
import com.paragon_software.settings_manager.exceptions.WrongTypeException;

import java.io.Serializable;
import java.util.Map;
import java.util.TreeMap;

import androidx.annotation.NonNull;

public abstract class MockSettingsManager extends SettingsManagerAPI {
  private Map<String, Object> settings = new TreeMap<>();

  @Override
  public void save(@NonNull String s, @NonNull Serializable serializable, boolean b)
      throws ManagerInitException, LocalResourceUnavailableException {
    settings.put(s, serializable);
  }

  @Override
  public <T> T load(@NonNull String s, @NonNull T t) throws WrongTypeException, ManagerInitException {
    T res = null;
    try {
      res = (T) settings.get(s);
    } catch (Exception ignore) {

    }
    if (res == null) {
      res = t;
    }
    return res;
  }
}

