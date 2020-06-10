package com.paragon_software.settings_manager;

import android.content.Context;
import android.content.SharedPreferences;
import androidx.annotation.NonNull;

import java.util.HashMap;
import java.util.Map;

import com.paragon_software.settings_manager.exceptions.LocalResourceUnavailableException;

/*
 * local system storage realisation. Using write to file serialized values
 *
 *  Created on: 14.02.18 with love
 *      Author: Mikhail Belyshov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

class SharedPreferencesLocalStorage implements ILocalSystemStorage
{
  @NonNull
  private final SharedPreferences mSharedPref;

  SharedPreferencesLocalStorage( @NonNull final Context context, @NonNull final String name )
      throws LocalResourceUnavailableException
  {
    mSharedPref = context.getSharedPreferences(name, Context.MODE_PRIVATE);
    if ( null == mSharedPref )
    {
      throw new LocalResourceUnavailableException("Cant get shared preferences");
    }
  }

  @Override
  public void save( @NonNull String key, @NonNull String value ) throws LocalResourceUnavailableException
  {
    SharedPreferences.Editor editor = mSharedPref.edit();
    if ( null == editor )
    {
      throw new LocalResourceUnavailableException("Cant edit shared preferences: editor instance is null");
    }
    editor.remove(key);
    editor.putString(key, value);
    editor.apply();
  }

  @Override
  @NonNull
  public HashMap< String, String > load() throws LocalResourceUnavailableException
  {

    try
    {
      HashMap< String, String > map = new HashMap<>();
      Map< String, ? > storedMap = mSharedPref.getAll();
      for ( Map.Entry< String, ? > entry : storedMap.entrySet() )
      {
        map.put(entry.getKey(), (String) entry.getValue());
      }
      return map;
    }
    catch ( NullPointerException e )
    {
      throw new LocalResourceUnavailableException(e.getMessage());
    }
  }

  @Override
  public void clear() throws LocalResourceUnavailableException
  {
    SharedPreferences.Editor editor = mSharedPref.edit();
    if ( null == editor )
    {
      throw new LocalResourceUnavailableException("Cant edit shared preferences: editor instance is null");
    }
    editor.clear();
    editor.apply();

  }
}
