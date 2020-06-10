/*
 * license_manager_api
 *
 *  Created on: 08.02.18
 *      Author: Ivan Zhdanov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

package com.paragon_software.license_manager_api;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.util.Collection;

import com.paragon_software.license_manager_api.exceptions.ActivateException;
import com.paragon_software.settings_manager.SettingsManagerAPI;

/**
 * Source for control feature activation.
 * Purchase in different markets, offline activation by serial, etc.
 */
@SuppressWarnings( "unused" )
public interface ActivateSource
{
  /**
   * Source name for identity
   *
   * @return name of source
   */
  @NonNull
  String getName();

  /**
   * Begin activate Feature by SKU ( async function )
   *
   * @param requestCode for onActivityResult
   * @param activity    activity with implemented method onActivityResult
   * @param item        SKU for activate
   * @param extra       Additional information
   *
   * @return Purchase information or null in unavailable immediately
   */
  Purchase activateBegin( final int requestCode, @NonNull final Activity activity, @NonNull final SKU item,
                      @NonNull final String extra );

  /**
   * Parse activation result
   *
   * @param data from onActivityResult
   *
   * @param resultCode
   * @return Purchase information
   */
  @Nullable
  Purchase parseActivateResult( @NonNull final Intent data, int resultCode ) throws ActivateException;

  /**
   * Update features information ( async function )
   *
   * @param context  Activity context
   * @param features Features list for update
   * @param result   Callback for return result
   */
  void update( @NonNull final Context context, @NonNull final Collection< LicenseFeature > features,
               @NonNull final UpdateResult result );

  void consume (@NonNull final Context context, @NonNull Purchase purchase, @NonNull ConsumeResult consumeResult);

  void setSettingsManager( SettingsManagerAPI settingsManagerAPI );
  /**
   * Update result returning callback.
   */
  interface UpdateResult
  {
    /**
     * Result of update features.
     *
     * @param status  Update status null if success, or exception with problem.
     * @param updated Null on error, or array with updated features.
     */
    void onUpdate( @Nullable final ActivateException status, @Nullable final LicenseFeature[] updated );
  }

  interface ConsumeResult
  {
    void onConsume(@Nullable final ActivateException status);
  }
}
