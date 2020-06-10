package com.paragon_software.settings_manager;

/*
 * settings_manager_api
 *
 *  Created on: 06.02.18
 *      Author: Ivan Zhdanov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

import androidx.annotation.NonNull;

import com.paragon_software.settings_manager.exceptions.StorageTransferException;

@SuppressWarnings( "unused" )
public interface ISettingsStorage
{
  /**
   * Get Settings storage name
   *
   * @return name of storage
   */
  String getName();

  /**
   * Save settings data into storage
   *
   * @param settings byte array with settings data
   *
   * @throws StorageTransferException if something in saving process was wrong. See Exception message.
   */
  void save( @NonNull final byte[] settings ) throws StorageTransferException;

  /**
   * Load settings data from storage. Asynchronous call. Status and progress will be updated in {@code result} callback.
   *
   * @param result callback for provide status and result of loading operation
   *
   * @throws StorageTransferException if something in loading process was wrong. See Exception message.
   */
  void load( @NonNull final ILoadSettingsResult result ) throws StorageTransferException;

  /**
   * Interface for provide status and result of loading operation
   */
  interface ILoadSettingsResult
  {
    /**
     * Called to inform user of current operation status
     *
     * @param current current progress
     * @param total   total progress
     */
    void onProgress( int current, int total );

    /**
     * Called when operation is finished
     *
     * @param result contains loaded data array
     */
    void onFinished( @NonNull byte[] result );
  }
}
