package com.paragon_software.settings_manager;

import android.content.Context;
import android.os.AsyncTask;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.VisibleForTesting;

import java.io.Serializable;
import java.lang.ref.WeakReference;
import java.util.*;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicReference;

import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import com.google.gson.JsonSerializer;
import com.google.gson.JsonDeserializer;
import com.google.gson.JsonObject;
import com.google.gson.JsonElement;
import com.google.gson.JsonSyntaxException;
import com.google.gson.JsonParseException;
import com.paragon_software.screen_opener_api.ScreenOpenerAPI;
import com.paragon_software.settings_manager.SettingsContainer.BackupContainerView;
import com.paragon_software.settings_manager.exceptions.*;

/*
 * settings_manager
 *
 *  Created on: 07.02.18
 *      Author: Mikhail Belyshov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

@SuppressWarnings( "unused,WeakerAccess" )
public final class BaseSettingsManager extends SettingsManagerAPI
{

  private static final String APPLICATION_SETTINGS_KEY = "settings_manager.application_settings";

  private final    Map< String, ISettingsStorage >   mStorages                   = new HashMap<>();
  private final    Object                            mDataSetLock                = new Object();
  private final    AtomicBoolean                     mStorageOperationInProgress = new AtomicBoolean(false);
  private          LocalResourceUnavailableException mException                  = null;
  private volatile SettingsContainer                 mDataset                    = null;

  private final List< OnSettingsSaveListener >            mOnSettingsSaveListeners            = new ArrayList<>();
  private final List< OnApplicationSettingsSaveListener > mOnApplicationSettingsSaveListeners = new ArrayList<>();

  private final Map< String, SettingsControllerAPI >     mSettingsControllers = new HashMap<>();
  private final AtomicReference< SettingsControllerAPI > mActiveController    = new AtomicReference<>();

  private Map< Class, TypeAdapter > mTypeAdapters = new HashMap<>();

  @Nullable
  private ScreenOpenerAPI mScreenOpener;

  public BaseSettingsManager( @NonNull final Context context, @NonNull final ISettingsStorage[] storageList, @Nullable MigrationHelper migrationHelper)
      throws ManagerInitException
  {
    fillStorageList(storageList);
    synchronized ( mDataSetLock )
    {
      initInBackground(context, migrationHelper);
      try
      {
        mDataSetLock.wait();
      }
      catch ( InterruptedException ignore )
      {
      }
    }
  }

  @Override
  public void registerScreenOpener(ScreenOpenerAPI screenOpener)
  {
    mScreenOpener = screenOpener;
  }

  private void fillStorageList( @NonNull final ISettingsStorage[] storageList ) throws ManagerInitException
  {
    for ( ISettingsStorage storage : storageList )
    {
      if ( mStorages.containsKey(storage.getName()) )
      {
        throw new ManagerInitException("Two storages have the same names ");
      }
      mStorages.put(storage.getName(), storage);
    }
  }

  private void initInBackground(@NonNull final Context context, MigrationHelper migrationHelper)
  {
    new AsyncInitSettingsContainer(this, context, migrationHelper).executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
  }

  @VisibleForTesting
  BaseSettingsManager( @NonNull final ISettingsStorage[] storageList, SettingsContainer dataSet )
      throws ManagerInitException
  {
    fillStorageList(storageList);
    mDataset = dataSet;
  }

  @VisibleForTesting
  void setFakeStorageOperationInProgress( boolean value )
  {
    mStorageOperationInProgress.set(value);
  }

  @VisibleForTesting
  void initDataSet( @NonNull final Context context )
  {
    synchronized ( mDataSetLock )
    {
      mDataSetLock.notify();
      try
      {
        mDataset = new SettingsContainer(context);
      }
      catch ( LocalResourceUnavailableException e )
      {
        mDataset = null;
        mException = e;
      }
    }
  }

  private Gson createGsonInstance() {
    GsonBuilder gsonBuilder = new GsonBuilder();

    // Register type adapter for enums to fix this: https://jira-shdd.paragon-software.com/browse/NSA-443
    // See also: https://medium.com/@hanru.yeh/gson-will-crash-at-enum-fields-in-custom-class-with-proguard-bbcf5ad1b623
    gsonBuilder.registerTypeAdapterFactory(new LowercaseEnumTypeAdapterFactory());

    for ( Map.Entry<Class, TypeAdapter> entry : mTypeAdapters.entrySet() )
    {
      gsonBuilder.registerTypeAdapter(entry.getKey(), entry.getValue());
    }
    return gsonBuilder.create();
  }

  @VisibleForTesting
  BackupContainerView loadSynchronous( @NonNull final Collection< String > storages,
                                       @Nullable final IReceiveSettingsResult receiveSettingsCallback )
  {
    return loadSynchronousWithTimeout(storages, receiveSettingsCallback, 0);
  }

  @VisibleForTesting
  BackupContainerView loadSynchronousWithTimeout( @NonNull final Collection< String > storages,
                                                  @Nullable final IReceiveSettingsResult receiveSettingsCallback,
                                                  long timeout )
  {
    final Object currentContainerLock = new Object();
    final AtomicReference< BackupContainerView > currentBackup = new AtomicReference<>(new BackupContainerView());
    final CountDownLatch signal = new CountDownLatch(storages.size());
    final int total = storages.size();
    final AtomicInteger current = new AtomicInteger(0);
    for ( String storageName : storages )
    {
      try
      {
        ISettingsStorage storage = mStorages.get(storageName);
        if ( null == storage )
        {
          if ( null != receiveSettingsCallback )
          {
            receiveSettingsCallback.onProgress(current.getAndIncrement(), total);
          }
          signal.countDown();
          continue;
        }
        storage.load(new ISettingsStorage.ILoadSettingsResult()
        {
          @Override
          public void onProgress( int current, int total )
          {
            //do nothing. Maybe later need to retranslate this values
          }

          @Override
          public void onFinished( @NonNull byte[] bytes )
          {
            synchronized ( currentContainerLock )
            {
              BackupContainerView set = SettingsContainer.deserializeBackupContainer(bytes);
              if ( currentBackup.get().version() < set.version() )
              {
                currentBackup.set(set);
              }
            }
            if ( null != receiveSettingsCallback )
            {
              receiveSettingsCallback.onProgress(current.getAndIncrement(), total);
            }
            signal.countDown();
          }
        });

      }
      catch ( StorageTransferException ignore )
      {
        if ( null != receiveSettingsCallback )
        {
          receiveSettingsCallback.onProgress(current.getAndIncrement(), total);
        }
        signal.countDown();
      }

    }

    try
    {
      if ( 0 != timeout )
      {
        signal.await(timeout, TimeUnit.MILLISECONDS);
      }
      else
      {
        signal.await();
      }
    }
    catch ( InterruptedException ignore )
    {
      ignore.printStackTrace();
    }
    if ( currentBackup.get().version() > 0 )
    {
      return currentBackup.get();
    }
    return new BackupContainerView();

  }

  @Override
  public boolean sendToStorages() throws AlreadyInProgressException, ManagerInitException, StorageTransferException
  {
    return doBroadcastBackup(mStorages.values());
  }

  private boolean doBroadcastBackup( @NonNull final Collection< ISettingsStorage > storages )
      throws AlreadyInProgressException, ManagerInitException, StorageTransferException
  {
    boolean status = false;
    if ( mStorageOperationInProgress.compareAndSet(false, true) )
    {
      byte[] arrayToSend;
      try
      {
        arrayToSend = getArraySnapshot();
      }
      finally
      {
        mStorageOperationInProgress.set(false);
      }

      for ( ISettingsStorage storage : storages )
      {
        try
        {
          storage.save(arrayToSend);
          confirmBackupFinished();
          status = true;
        }
        catch ( StorageTransferException | ManagerInitException | LocalResourceUnavailableException ignore )
        {
        }
      }
      mStorageOperationInProgress.set(false);
    }
    else
    {
      throw new AlreadyInProgressException();
    }
    return status;
  }

  private byte[] getArraySnapshot() throws StorageTransferException, ManagerInitException
  {
    synchronized ( mDataSetLock )
    {
      checkInit();
      mDataset.upVersion();
      return mDataset.serializePersistentData();
    }
  }

  private void confirmBackupFinished() throws ManagerInitException, LocalResourceUnavailableException
  {
    synchronized ( mDataSetLock )
    {
      checkInit();
      mDataset.confirmBackupFinished();
    }
  }

  @Override
  public void sendToStorage( @NonNull String storageName )
      throws NoSuchStorageException, StorageTransferException, AlreadyInProgressException, ManagerInitException,
             LocalResourceUnavailableException
  {
    ISettingsStorage storage = mStorages.get(storageName);
    if ( null == storage )
    {
      throw new NoSuchStorageException();
    }
    doBackup(storage);
  }

  private void doBackup( @NonNull final ISettingsStorage storage )
      throws StorageTransferException, AlreadyInProgressException, ManagerInitException,
             LocalResourceUnavailableException
  {
    if ( mStorageOperationInProgress.compareAndSet(false, true) )
    {
      try
      {
        byte[] arrayToSend = getArraySnapshot();
        storage.save(arrayToSend);
        confirmBackupFinished();
      }
      finally
      {
        mStorageOperationInProgress.set(false);
      }
    }
    else
    {
      throw new AlreadyInProgressException();
    }
  }

  @Override
  public void receiveFromStorages( @Nullable final IReceiveSettingsResult receiveSettingsResult, final boolean force )
      throws AlreadyInProgressException, UnsavedDataException, ManagerInitException
  {
    doBackgroundReceiveFromStorage(mStorages.keySet(), receiveSettingsResult, force);
  }

  private void doBackgroundReceiveFromStorage( @NonNull final Collection< String > storages,
                                               @Nullable final IReceiveSettingsResult receiveSettingsResult,
                                               final boolean force )
      throws AlreadyInProgressException, UnsavedDataException, ManagerInitException
  {
    checkInitAndDirty(force);
    if ( mStorageOperationInProgress.compareAndSet(false, true) )
    {
      runAsyncStoragesLoading(storages, receiveSettingsResult, force);
    }
    else
    {
      throw new AlreadyInProgressException();
    }
  }

  @VisibleForTesting
  void checkInitAndDirty( final boolean force ) throws UnsavedDataException, ManagerInitException
  {
    synchronized ( mDataSetLock )
    {
      checkInit();
      if ( mDataset.isDirty() && !force )
      {
        throw new UnsavedDataException();
      }
    }
  }

  private void runAsyncStoragesLoading( @NonNull final Collection< String > storages,
                                        @Nullable final IReceiveSettingsResult receiveSettingsResult,
                                        final boolean force )
  {
    new AsyncStorageSettingsLoader(storages, this, receiveSettingsResult, new AsyncStorageSettingsLoader.PostExecute()
    {
      @Override
      public void onFinished( BackupContainerView container )
      {
        boolean updated;
        try
        {
          updated = updateDataSet(container, force);
        }
        catch ( SettingsManagerException ignore )
        {
          updated = false;
        }

        if ( null != receiveSettingsResult )
        {
          receiveSettingsResult.onFinished(updated);
        }
        mStorageOperationInProgress.set(false);
      }
    }).executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
  }

  @VisibleForTesting
  boolean updateDataSet( @NonNull final BackupContainerView container, boolean force )
      throws ManagerInitException, LocalResourceUnavailableException
  {
    boolean updated = false;
    if ( 0 != container.version() )
    {
      synchronized ( mDataSetLock )
      {
        checkInit();
        if ( force || mDataset.version() < container.version() )
        {
          updated = true;
          mDataset.restorePersistentData(container);
        }
      }
    }
    return updated;
  }

  @Override
  public void receiveFromStorage( @NonNull final String storageName,
                                  @Nullable final IReceiveSettingsResult receiveSettingsResult, final boolean force )
      throws NoSuchStorageException, StorageTransferException, AlreadyInProgressException, UnsavedDataException,
             ManagerInitException
  {
    ISettingsStorage storage = mStorages.get(storageName);
    if ( null == storage )
    {
      throw new NoSuchStorageException();
    }
    doBackgroundReceiveFromStorage(Collections.singletonList(storageName), receiveSettingsResult, force);
  }

  @Override
  public void save( @NonNull String name, @NonNull Serializable data, boolean needBackup )
      throws ManagerInitException, LocalResourceUnavailableException
  {
    synchronized ( mDataSetLock )
    {
      checkInit();
      mDataset.putStringData(name, createGsonInstance().toJson(data), needBackup ? SettingsContainer.DATA_TYPE.PERSISTENT_DATA
                                                                       : SettingsContainer.DATA_TYPE.LOCAL_DATA);
      notifySettingsSave(name, data);
    }
  }

  @SuppressWarnings( "unchecked" )
  @Override
  public < T > T load( @NonNull final String name, @NonNull final T defValue )
      throws WrongTypeException, ManagerInitException
  {
    checkInit();
    Object ret;
    String json;
    synchronized ( mDataSetLock )
    {
      checkInit();
      json = mDataset.loadStringData(name);
    }
    try
    {
      ret = createGsonInstance().fromJson(json, defValue.getClass());
    }
    catch ( JsonSyntaxException e )
    {
      throw new WrongTypeException("Stored value is unassignable to required type");
    }
    if ( null == ret )
    {
      ret = defValue;
    }

    return (T) ( ret );
  }

  public boolean isRestored()
  {
    synchronized ( mDataSetLock )
    {
      try
      {
        checkInit();
      }
      catch ( SettingsManagerException e )
      {
        return false;
      }
      return mDataset.version() != 0;
    }
  }

  private void checkInit() throws ManagerInitException
  {
    if ( null == mDataset )
    {
      ManagerInitException exception = new ManagerInitException("Manager not initialized");
      if ( null != mException )
      {
        exception.initCause(mException);
      }
      throw exception;
    }
  }

  @Override
  public Collection< String > storageList()
  {
    return mStorages.keySet();
  }

  @Override
  public void registerNotifier( @NonNull Notifier notifier )
  {
    if (notifier instanceof OnSettingsSaveListener && !mOnSettingsSaveListeners.contains(notifier))
    {
      mOnSettingsSaveListeners.add((OnSettingsSaveListener) notifier);
    }
    if (notifier instanceof OnApplicationSettingsSaveListener && !mOnApplicationSettingsSaveListeners.contains(notifier))
    {
      mOnApplicationSettingsSaveListeners.add((OnApplicationSettingsSaveListener) notifier);
    }
  }

  @Override
  public void unregisterNotifier( @NonNull Notifier notifier )
  {
    if (notifier instanceof OnSettingsSaveListener)
    {
      mOnSettingsSaveListeners.remove(notifier);
    }
    if (notifier instanceof OnApplicationSettingsSaveListener )
    {
      mOnApplicationSettingsSaveListeners.remove(notifier);
    }
  }

  @NonNull
  @Override
  SettingsControllerAPI getController( @NonNull String uiName ) throws ManagerInitException, WrongTypeException
  {
    SettingsControllerAPI controller = mSettingsControllers.get(uiName);
    if (controller == null)
    {
      if (SettingsControllerType.DEFAULT_CONTROLLER.equals(uiName))
      {
        controller = new BaseSettingsController(this, mScreenOpener);
        mSettingsControllers.put(uiName, controller);
      }
    }
    mActiveController.set(controller);
    return controller;
  }

  @Override
  void freeController( @NonNull String uiName )
  {
    if (mActiveController.get() == mSettingsControllers.get(uiName))
    {
      mActiveController.set(null);
    }
  }

  @Override
  public @NonNull ApplicationSettings loadApplicationSettings()
  {
    ApplicationSettings resApplicationSettings = new ApplicationSettings();
    try
    {
      resApplicationSettings = load(APPLICATION_SETTINGS_KEY, resApplicationSettings);
    }
    catch ( WrongTypeException | ManagerInitException ignore )
    {
    }
    return resApplicationSettings;
  }

  @Override
  public void saveApplicationSettings( @NonNull ApplicationSettings newSettings )
      throws ManagerInitException, LocalResourceUnavailableException
  {
    save(APPLICATION_SETTINGS_KEY, newSettings, true);
    notifyApplicationSettingsSaved(newSettings);
  }

  private void notifySettingsSave( String name, Serializable data )
  {
    for (OnSettingsSaveListener listener : mOnSettingsSaveListeners)
    {
      listener.onSettingsSaved(name, data);
    }
  }

  private void notifyApplicationSettingsSaved( @NonNull ApplicationSettings newSettings )
  {
    for ( OnApplicationSettingsSaveListener listener : mOnApplicationSettingsSaveListeners )
    {
      listener.onApplicationSettingsSaved(newSettings);
    }
  }

  public void addTypeAdapter(Class clazz, TypeAdapter adapter) {
    mTypeAdapters.put(clazz, adapter);
  }

  private static class AsyncInitSettingsContainer extends AsyncTask< Void, Void, Void > {
    private final WeakReference<BaseSettingsManager> thisInstance;
    private final WeakReference<Context> mContext;
    @Nullable
    private final MigrationHelper mMigrationHelper;

    AsyncInitSettingsContainer(@NonNull final BaseSettingsManager baseSettingsManager, @NonNull Context context, @Nullable MigrationHelper migrationHelper) {
      thisInstance = new WeakReference<>(baseSettingsManager);
      mContext = new WeakReference<>(context);
      mMigrationHelper = migrationHelper;
    }

    @Override
    protected Void doInBackground(Void... voids) {
      thisInstance.get().initDataSet(mContext.get());
      if (mMigrationHelper != null)
        mMigrationHelper.migrateSettings(thisInstance.get(),mContext.get());
      return null;
    }
  }

  private static class AsyncStorageSettingsLoader extends AsyncTask< Void, Void, Void >
  {

    private final IReceiveSettingsResult               mReceiveSettingsResult;
    private final Collection< String >                 mStorages;
    private final PostExecute                          mPostExecuteCallback;
    private       WeakReference< BaseSettingsManager > mThisInstance;

    public AsyncStorageSettingsLoader( @NonNull final Collection< String > storages,
                                       @NonNull final BaseSettingsManager settingsManager,
                                       @Nullable final IReceiveSettingsResult receiveSettingsResult,
                                       @NonNull final PostExecute postExecuteCallback )
    {
      mThisInstance = new WeakReference<>(settingsManager);
      mReceiveSettingsResult = receiveSettingsResult;
      mStorages = storages;
      mPostExecuteCallback = postExecuteCallback;
    }

    interface PostExecute
    {
      void onFinished( final BackupContainerView container );
    }

    @Override
    protected Void doInBackground( Void... voids )
    {

      BackupContainerView container = mThisInstance.get().loadSynchronous(mStorages, mReceiveSettingsResult);
      mPostExecuteCallback.onFinished(container);
      return null;
    }
  }

  public static abstract class TypeAdapter<T> implements JsonSerializer<T>, JsonDeserializer<T> {

    protected JsonElement getJsonElement(final JsonObject wrapper, String memberName) {
      final JsonElement elem = wrapper.get(memberName);
      if (elem == null)
      {
        throw new JsonParseException("no '" + memberName + "' member found in what was expected to be an element wrapper");
      }
      return elem;
    }

  }
}
