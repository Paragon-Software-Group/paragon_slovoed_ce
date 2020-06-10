package com.paragon_software.android_native_backup_storage;
/*
 * settings_manager
 *
 *  Created on: 07.02.18
 *      Author: Mikhail Belyshov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

import android.app.backup.*;
import android.os.ParcelFileDescriptor;
import androidx.annotation.NonNull;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;

import com.paragon_software.settings_manager.ISettingsStorage;
import com.paragon_software.settings_manager.exceptions.StorageTransferException;

public class AndroidNativeBackupStorage extends BackupAgentHelper implements ISettingsStorage
{

  private static final String FILE_SETTINGS_KEY  = "appSettings";
  private static final String FILE_SETTINGS_NAME = "appSettings";
  private static final String mName              = "AndroidNativeBackupStorage";
  private BackupManager mBackupManager;
  private Object        mFileLock;

  //check BackupAgentHelper
  // adb shell bmgr enable true (на эмуляторе) или откройте Системные настройки, выберите Конфиденциальность, затем включите Резервное копирование моих данных и Автоматическое восстановление.
  // adb shell bmgr backup your.package.name
  // adb shell bmgr run

  @Override
  public void onCreate() {
    super.onCreate();
    mBackupManager = new BackupManager(this);
    FileBackupHelper fileBackupHelper = new FileBackupHelper(this, FILE_SETTINGS_NAME);
    addHelper(FILE_SETTINGS_KEY, fileBackupHelper);
    mFileLock = new Object();
  }

  @Override
  public String getName()
  {
    return mName;
  }

  @Override
  public void save( @NonNull byte[] bytes ) throws StorageTransferException
  {
    writeToFile(bytes);
    mBackupManager.dataChanged();
  }

  private void writeToFile( @NonNull final byte[] dataArray ) throws StorageTransferException
  {
    try
    {
      synchronized ( mFileLock )
      {
        FileOutputStream fos = new FileOutputStream(getSettingsFile(), false);
        fos.write(dataArray);
        fos.close();
      }
    }
    catch ( IOException e )
    {
      throw new StorageTransferException("Can't store settings: " + e.getMessage());
    }
  }

  @NonNull
  private File getSettingsFile()
  {
    return new File(getFilesDir(), FILE_SETTINGS_NAME);
  }

  @Override
  public void load( @NonNull final ILoadSettingsResult loadSettingsResult ) throws StorageTransferException
  {
    int restoreStatus = mBackupManager.requestRestore(new RestoreObserver()
    {
      int mTotalPackages = 0;

      @Override
      public void restoreStarting( int numPackages )
      {
        super.restoreStarting(numPackages);
        mTotalPackages = numPackages;
        loadSettingsResult.onProgress(0, mTotalPackages);
      }

      @Override
      public void onUpdate( int nowBeingRestored, String currentPackage )
      {
        super.onUpdate(nowBeingRestored, currentPackage);
        loadSettingsResult.onProgress(nowBeingRestored, mTotalPackages);
      }

      @Override
      public void restoreFinished( int error )
      {
        super.restoreFinished(error);
        try
        {
          loadSettingsResult.onFinished(readFromFile());
        }
        catch ( StorageTransferException ignore )
        {
          loadSettingsResult.onFinished(new byte[0]);
        }
      }
    });
    if ( 0 != restoreStatus )
    {
      throw new StorageTransferException("Restore fails with code " + restoreStatus);
    }
  }

  @NonNull
  private byte[] readFromFile() throws StorageTransferException
  {
    try
    {
      synchronized ( mFileLock )
      {
        File settingsFile = getSettingsFile();
        long fileSize = settingsFile.length();
        int arraySize = (int)(fileSize);
        if ( (long) arraySize != fileSize )
        {
          throw new ArithmeticException("integer overflow");
        }
        FileInputStream fis = new FileInputStream(getSettingsFile());
        byte[] retArray = new byte[arraySize];
        int read = 0;
        while ( read != arraySize )
        {
          read += fis.read(retArray, read, arraySize - read);
        }
        fis.close();
        return retArray;
      }
    }
    catch ( ArithmeticException e )
    {
      throw new StorageTransferException("Too long data size " + e.getMessage());
    }
    catch ( IOException e )
    {
      throw new StorageTransferException("Can't store settings: " + e.getMessage());
    }
  }

  @Override
  public void onBackup( ParcelFileDescriptor oldState, BackupDataOutput data, ParcelFileDescriptor newState )
      throws IOException
  {
    super.onBackup(oldState, data, newState);

  }

  @Override
  public void onRestore( BackupDataInput data, int appVersionCode, ParcelFileDescriptor newState ) throws IOException
  {
    super.onRestore(data, appVersionCode, newState);
  }
}
