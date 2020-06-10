package com.paragon_software.settings_manager.mock;

import androidx.annotation.NonNull;

import com.paragon_software.settings_manager.ILocalSystemStorage;

import java.util.HashMap;

public abstract class MockStorageLocal implements ILocalSystemStorage
{
  private HashMap< String, String > mMap = new HashMap<>();

  public MockStorageLocal()
  {
  }

  public MockStorageLocal(final HashMap< String, String > map )
  {
    mMap = map;
  }

  @Override
  public void save(@NonNull String key, @NonNull String value )
  {
    mMap.put(key, value);
  }

  @NonNull
  @Override
  public HashMap< String, String > load()
  {
    HashMap< String, String > newMap = (HashMap< String, String >) mMap.clone();
    return newMap;
  }

  @Override
  public void clear()
  {
    mMap.clear();
  }

  public void setNewMap(HashMap< String, String > newMap){
    mMap = newMap;
  }
}