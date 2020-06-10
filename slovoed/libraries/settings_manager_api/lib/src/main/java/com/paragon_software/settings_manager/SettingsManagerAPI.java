package com.paragon_software.settings_manager;

/*
 * settings_manager_api
 *
 *  Created on: 07.02.18
 *      Author: Mikhail Belyshov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.VisibleForTesting;

import com.paragon_software.screen_opener_api.ScreenOpenerAPI;
import com.paragon_software.settings_manager.exceptions.AlreadyInProgressException;
import com.paragon_software.settings_manager.exceptions.LocalResourceUnavailableException;
import com.paragon_software.settings_manager.exceptions.ManagerInitException;
import com.paragon_software.settings_manager.exceptions.NoSuchStorageException;
import com.paragon_software.settings_manager.exceptions.StorageTransferException;
import com.paragon_software.settings_manager.exceptions.UnsavedDataException;
import com.paragon_software.settings_manager.exceptions.WrongTypeException;

import java.io.Serializable;
import java.util.Collection;

@SuppressWarnings( "unused" )
public abstract class SettingsManagerAPI
{

  /**
   * Send settings to all storages
   *
   * @return true, if at least one storage received backup, false otherwise.
   *
   * @throws AlreadyInProgressException        throws if other backup or restore operation in progress
   * @throws LocalResourceUnavailableException throws if settings can't be stored on internal app memory
   * @throws ManagerInitException              throws if setting manager cant load saved data, or other initialization error occured.
   * @throws StorageTransferException          throws from settings storage layer
   */
  public abstract boolean sendToStorages() throws AlreadyInProgressException, LocalResourceUnavailableException, ManagerInitException,
                                  StorageTransferException;

  /**
   * Send settings to concrete storage {@code name}
   *
   * @param name name of concrete storage
   *
   * @throws NoSuchStorageException            if there no storage with this name
   * @throws StorageTransferException          throws from settings storage layer
   * @throws AlreadyInProgressException        throws if other backup or restore operation in progress
   * @throws ManagerInitException              throws if setting manager cant load saved data, or other initialization error occured.
   * @throws LocalResourceUnavailableException throws if settings can't be stored on internal app memory
   */
  public abstract void sendToStorage( @NonNull final String name )
      throws NoSuchStorageException, StorageTransferException, AlreadyInProgressException, ManagerInitException,
             LocalResourceUnavailableException;

  /**
   * Receive settings from all storages.
   * What settings will be chosen depends on the realisation.
   * Asynchronous call. You can get progress in {@code IReceiveSettingsResult callback}
   *
   * @param result callback for provide status and result of receiving. Can be null. if null - it's
   *               mean that no progress update available, call staying asynchronous anyway.
   * @param force  if true - than local data will be replaced even if data on storage has oldest version, or
   *               there is local unsaved changes
   *
   * @throws AlreadyInProgressException throws if other backup or restore operation in progress
   * @throws UnsavedDataException       throws if there is local changes changes, that were not backuped
   * @throws ManagerInitException       throws if setting manager cant load saved data, or other initialization error occured.
   */
  public abstract void receiveFromStorages( @Nullable final IReceiveSettingsResult result, boolean force )
      throws AlreadyInProgressException, UnsavedDataException, ManagerInitException;

  /**
   * Receive settings from concrete storage {@code name}.
   * Asynchronous call. You can get progress in {@link IReceiveSettingsResult callback}
   *
   * @param name   name of concrete storage
   * @param result callback for provide status and result of receiving. Can be null. if null - it's
   *               mean that no progress update available, call staying asynchronous anyway.
   * @param force  if true - than local data will be replaced even if data on storage has oldest version, or
   *               there is local unsaved changes
   *
   * @throws NoSuchStorageException     if there no storage with this name
   * @throws StorageTransferException   throws from settings storage layer
   * @throws AlreadyInProgressException throws if other backup or restore operation in progress
   * @throws UnsavedDataException       throws if there is local changes changes, that were not backuped
   * @throws ManagerInitException       throws if setting manager cant load saved data, or other initialization error occured.
   */
  public abstract void receiveFromStorage( @NonNull final String name, @Nullable final IReceiveSettingsResult result, boolean force )
      throws NoSuchStorageException, StorageTransferException, AlreadyInProgressException, UnsavedDataException,
             ManagerInitException;

  /**
   * Save settings. Usually used for fast saving settings
   *
   * @param name       name of concrete storage
   * @param data       represents data for saving. Should be serializable
   * @param needBackup if true than this data will be saved on next backup. If false - this data will be stored only while app installed
   *
   * @throws ManagerInitException              throws if setting manager cant load saved data, or other initialization error occured.
   * @throws LocalResourceUnavailableException throws if settings can't be stored on internal app memory
   */
  public abstract void save( @NonNull final String name, @NonNull final Serializable data, boolean needBackup )
      throws ManagerInitException, LocalResourceUnavailableException;

  /**
   * Load settings. Usually used for loading settings, that was saved with {@code save}
   *
   * @param name     name of concrete storage
   * @param defValue will be returned if load fails
   * @param <T>      object type.
   *
   * @return loaded value, of {@code defValue} if there is no value with key {@code name}
   *
   * @throws WrongTypeException   if stored value has another type than {@link T}
   * @throws ManagerInitException throws if setting manager cant load saved data, or other initialization error occured.
   */
  public abstract < T > T load( @NonNull final String name, @NonNull final T defValue )
          throws WrongTypeException, ManagerInitException;

  /**
   * @return true if settings manager contains data, that has been backuped.
   * warning: if isRestored() returns true - it doesn't mean that there no local changes/
   */
  public abstract boolean isRestored();

  /**
   * @return collection of available storages
   */
  public abstract Collection< String > storageList();

  /**
   * Register new notifier to notify observers about settings manager state changes.
   * If notifier already registered: do nothing.
   * @param notifier new notifier
   */
  public abstract void registerNotifier( @NonNull Notifier notifier );

  /**
   * Unregister notifier. If notifier was not registered: do nothing.
   * @param notifier notifier to unregister
   */
  public abstract void unregisterNotifier( @NonNull Notifier notifier );

  /**
   * Get controller by specified ui name from {@link SettingsControllerType} tag strings. Create new controller
   * if it was not created for specified ui name. Set the returned controller as active. Before return controller
   * for specified ui name update its state.
   * @param uiName tag strings from {@link SettingsControllerType} to get controller for
   * @return controller {@link SettingsControllerAPI} for specified ui name
   * @throws ManagerInitException throws if setting manager can't load saved data during controller state update
   * @throws WrongTypeException   throws if setting manager can't load saved data during controller state update
   */
  @NonNull
  abstract SettingsControllerAPI getController( @NonNull String uiName )
      throws ManagerInitException, WrongTypeException;

  /**
   * Deactivate controller for specified ui name.
   * @param uiName tag strings from {@link SettingsControllerType} to deactivate controller for
   */
  abstract void freeController( @NonNull String uiName );

  /**
   * Load {@link ApplicationSettings} object.
   * @return loaded {@link ApplicationSettings} object or new {@link ApplicationSettings} with default values if there was an error while loading
   */
  public abstract @NonNull ApplicationSettings loadApplicationSettings();

  /**
   * Save {@link ApplicationSettings} object.
   * @param newSettings {@link ApplicationSettings} object to save.
   * @throws ManagerInitException              throws if setting manager cant load saved data, or other initialization error occured.
   * @throws LocalResourceUnavailableException throws if settings can't be stored on internal app memory
   */
  public abstract void saveApplicationSettings( @NonNull ApplicationSettings newSettings )
      throws ManagerInitException, LocalResourceUnavailableException;


  /**
   * register {@link ScreenOpenerAPI} object.
   * @param screenOpener {@link ScreenOpenerAPI} object to register.
   */
  public abstract void registerScreenOpener(ScreenOpenerAPI screenOpener);

  /**
   * Interface for provide status and result of receiving operation
   */
  public interface IReceiveSettingsResult
  {
    /**
     * Called to inform user of current operation status
     *
     * @param current current progress
     * @param total   total progress
     */
    void onProgress( int current, int total );

    /**
     * Calling when operation is finished
     *
     * @param updated true, if data was updated from any storage. False otherwise
     */
    void onFinished( boolean updated );
  }

  /**
   * Base interface of {@link SettingsManagerAPI} notifiers.
   */
  @VisibleForTesting
  public interface Notifier
  {
  }
}
