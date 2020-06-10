package com.paragon_software.settings_manager;

import android.content.Context;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.VisibleForTesting;

import java.io.*;
import java.util.HashMap;

import com.paragon_software.settings_manager.exceptions.LocalResourceUnavailableException;
import com.paragon_software.settings_manager.exceptions.StorageTransferException;

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

class SettingsContainer
{

  @NonNull
  private final PersistentSettingsContainer mPersistentSettingsContainer;
  @NonNull
  private final LocalSettingsContainer      mLocalSettingsContainer;
  @VisibleForTesting
  SettingsContainer( @NonNull final PersistentSettingsContainer persistentSettingsContainer,
                     @NonNull final LocalSettingsContainer localSettingsContainer )
  {
    mPersistentSettingsContainer = persistentSettingsContainer;
    mLocalSettingsContainer = localSettingsContainer;
  }

  SettingsContainer( @NonNull final Context context ) throws LocalResourceUnavailableException
  {
    mPersistentSettingsContainer = new PersistentSettingsContainer(context);
    mLocalSettingsContainer = new LocalSettingsContainer(context);
  }

  static BackupContainerView deserializeBackupContainer( @NonNull final byte[] rawData )
  {
    ByteArrayInputStream bis = new ByteArrayInputStream(rawData);
    ObjectInput in = null;
    try
    {
      in = new ObjectInputStream(bis);
      Object object = in.readObject();
      if ( object instanceof BackupContainerView )
      {
        return (BackupContainerView) object;
      }
    }
    catch ( IOException | ClassNotFoundException ignore )
    {

    }
    finally
    {
      try
      {
        if ( in != null )
        {
          in.close();
        }
      }
      catch ( IOException ex )
      {
        // ignore close exception
      }
    }
    return new BackupContainerView();
  }

  void putStringData( @NonNull final String name, @NonNull String data, @NonNull final DATA_TYPE dataType )
      throws LocalResourceUnavailableException
  {

    switch ( dataType )
    {
      case LOCAL_DATA:
        mLocalSettingsContainer.putString(name, data);
        break;
      case PERSISTENT_DATA:
        mPersistentSettingsContainer.putString(name, data);
        break;
      default:
        break;
    }
  }

  @Nullable
  String loadStringData( @NonNull final String name )
  {
    String ret = mPersistentSettingsContainer.getString(name);
    if ( null == ret )
    {
      ret = mLocalSettingsContainer.getString(name);
    }
    return ret;
  }

  boolean isDirty()
  {
    return mPersistentSettingsContainer.isDirty();
  }

  void upVersion()
  {
    mPersistentSettingsContainer.upVersion();
  }

  long version()
  {
    return mPersistentSettingsContainer.version();
  }

  void confirmBackupFinished() throws LocalResourceUnavailableException
  {
    mPersistentSettingsContainer.confirmBackupFinished();
  }

  byte[] serializePersistentData() throws StorageTransferException
  {
    return mPersistentSettingsContainer.getContainerView().toByteArray();
  }

  void restorePersistentData( @NonNull final BackupContainerView backupContainer )
      throws LocalResourceUnavailableException
  {
    mPersistentSettingsContainer.restoreFromBackup(backupContainer.mVersion, backupContainer.mData);
  }

  enum DATA_TYPE
  {
    LOCAL_DATA,
    PERSISTENT_DATA
  }

  static class BackupContainerView implements Serializable
  {
    private final HashMap< String, String > mData;
    private final long                      mVersion;

    BackupContainerView( @NonNull final HashMap< String, String > data, final long version )
    {
      mData = data;
      mVersion = version;
    }

    BackupContainerView()
    {
      mData = new HashMap<>();
      mVersion = 0;
    }

    long version()
    {
      return mVersion;
    }

    final byte[] toByteArray() throws StorageTransferException
    {
      ByteArrayOutputStream bos = new ByteArrayOutputStream();
      try
      {
        ObjectOutput out = new ObjectOutputStream(bos);
        out.writeObject(this);
        out.flush();
        return bos.toByteArray();
      }
      catch ( IOException e )
      {
        //i think that it's impossible case
        throw new StorageTransferException(e.getMessage());
      }
      finally
      {
        try
        {
          bos.close();
        }
        catch ( IOException ex )
        {
          // ignore close exception
        }
      }
    }
  }
}
