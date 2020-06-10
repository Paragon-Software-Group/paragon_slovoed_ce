package com.paragon_software.settings_manager;

import android.content.Context;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.VisibleForTesting;

import java.util.HashMap;

import com.paragon_software.settings_manager.exceptions.LocalResourceUnavailableException;

/*
 * settings_manager
 *
 *  Created on: 08.02.18
 *      Author: Mikhail Belyshov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

//thread unsafe!

final class PersistentSettingsContainer
{

  private final static String                    LOCAL_STORAGE_NAME = "PersistentSettings";
  private final static String                    VERSION_KEY        = "__PersistentSettings_Version";
  private final static String                    DIRTY_KEY          = "__PersistentSettings_Dirty";
  private final ILocalSystemStorage mLocalStorage;
  private              boolean                   mDirty             = false;
  private              long                      mVersion           = 0;
  private              HashMap< String, String > mData              = new HashMap<>();

  PersistentSettingsContainer( @NonNull final Context context ) throws LocalResourceUnavailableException
  {
    mLocalStorage = new SharedPreferencesLocalStorage(context, LOCAL_STORAGE_NAME);
    loadFromLocalStorage();
  }

  private void loadFromLocalStorage() throws LocalResourceUnavailableException
  {
    mData = mLocalStorage.load();
    String version = mData.get(VERSION_KEY);
    String dirty = mData.get(DIRTY_KEY);
    try
    {
      mVersion = Long.valueOf(version);
    }
    catch ( NumberFormatException e )
    {
      mVersion = 0;
    }
    try
    {
      mDirty = Boolean.valueOf(dirty);
    }
    catch ( NumberFormatException e )
    {
      mDirty = false;
    }

    mData.remove(VERSION_KEY);
    mData.remove(DIRTY_KEY);
  }

  @VisibleForTesting
  PersistentSettingsContainer( ILocalSystemStorage storage ) throws LocalResourceUnavailableException
  {
    mLocalStorage = storage;
    loadFromLocalStorage();
  }

  @VisibleForTesting
  static String VersionKey()
  {
    return VERSION_KEY;
  }

  @VisibleForTesting
  static String dirtyKey()
  {
    return DIRTY_KEY;
  }

  final void putString( @NonNull final String name, @NonNull String data ) throws LocalResourceUnavailableException
  {
    if ( !mDirty )
    {
      setDirtyAndCommit(true);
    }
    mData.put(name, data);
    mLocalStorage.save(name, data);
  }

  private void setDirtyAndCommit( boolean value ) throws LocalResourceUnavailableException
  {
    mDirty = value;
    mLocalStorage.save(DIRTY_KEY, String.valueOf(value));
    mData.remove(DIRTY_KEY);
  }

  @Nullable
  String getString( @NonNull final String name )
  {

    return mData.get(name);
  }

  boolean isDirty()
  {
    return mDirty;
  }

  long version()
  {
    return mVersion;
  }

  void upVersion()
  {
    ++mVersion;
  }

  @VisibleForTesting
  HashMap< String, String > getDataMap()
  {
    return mData;
  }

  void restoreFromBackup( long version, final HashMap< String, String > data ) throws LocalResourceUnavailableException
  {
    mDirty = false;
    mVersion = version;
    mData = data;
    rewriteLocalStorage();
  }

  private void rewriteLocalStorage() throws LocalResourceUnavailableException
  {
    mLocalStorage.clear();
    for ( HashMap.Entry< String, String > entry : mData.entrySet() )
    {
      mLocalStorage.save(entry.getKey(), entry.getValue());
    }
    String version = String.valueOf(mVersion);
    mLocalStorage.save(VERSION_KEY, version);
  }

  SettingsContainer.BackupContainerView getContainerView()
  {
    return new SettingsContainer.BackupContainerView(mData, mVersion);
  }

  void confirmBackupFinished() throws LocalResourceUnavailableException
  {
    setDirtyAndCommit(false);
  }

}