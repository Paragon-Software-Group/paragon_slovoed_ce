package com.paragon_software.settings_manager;

/*
 * local settings container. To store non-backuped local data
 *
 *  Created on: 08.02.18
 *      Author: Mikhail Belyshov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

//thread unsafe!

import android.content.Context;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.VisibleForTesting;

import java.util.HashMap;

import com.paragon_software.settings_manager.exceptions.LocalResourceUnavailableException;

class LocalSettingsContainer
{
  private final static String LOCAL_STORAGE_NAME = "LocalSettings";
  private final ILocalSystemStorage mLocalStorage;
  private HashMap< String, String > mData = new HashMap<>();

  @VisibleForTesting
  LocalSettingsContainer( @NonNull final ILocalSystemStorage storage ) throws LocalResourceUnavailableException
  {
    mLocalStorage = storage;
    mData = mLocalStorage.load();
  }

  LocalSettingsContainer( @NonNull final Context context ) throws LocalResourceUnavailableException
  {
    mLocalStorage = new SharedPreferencesLocalStorage(context, LOCAL_STORAGE_NAME);
    mData = mLocalStorage.load();
  }

  final void putString( @NonNull final String name, @NonNull String data ) throws LocalResourceUnavailableException
  {
    mData.put(name, data);
    mLocalStorage.save(name, data);
  }

  @Nullable
  final String getString( @NonNull final String name )
  {
    return mData.get(name);
  }
}
