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

import com.paragon_software.dictionary_manager.FeatureName;
import com.paragon_software.license_manager_api.exceptions.*;

/**
 * Manager for control product features.
 * Activate, check activation and check status.
 */
@SuppressWarnings( { "unused", "RedundantThrows" } )
public interface LicenseManager
{

  void update( @NonNull Context context );

  void consume( @NonNull Context appContext, @NonNull FeatureName featureName )
    throws ItemUnavailableException, ItemNotOwnedException;

  /**
   * Check state of feature by name.
   *
   * @param name feature name.
   *
   * @return state of feature, if feature not exist return LicenseFeature.FEATURE_STATE.DISABLED
   */
  @NonNull
  LicenseFeature.FEATURE_STATE checkFeature( @NonNull final FeatureName name );

  /**
   * Get feature by name
   *
   * @param name feature name.
   *
   * @return feature if exists or null.
   */
  @Nullable
  LicenseFeature getFeature( @NonNull final FeatureName name );

  /**
   * Get registered features
   *
   * @return collection of features
   */
  @NonNull
  Collection< LicenseFeature > getFeatures();

  /**
   * Activate feature by Name with any source
   *
   * @param activity    Activity for return after activation.
   * @param featureName name of feature for activating
   *
   * @throws ItemUnavailableException       if can't activate this feature ( unknown name, not have sources ).
   * @throws ItemAlreadyOwnedException      if feature already activated.
   * @throws UserAccountManagementException if user not register before begin activating.
   */
  void activationBegin( @NonNull final Activity activity, @NonNull final FeatureName featureName )
      throws ItemUnavailableException, ItemAlreadyOwnedException, UserAccountManagementException;

  /**
   * Activate feature by Name & SKU with any source
   *
   * @param activity    Activity for return after activation.
   * @param featureName name of feature for activating
   * @param skuToActivate Sku to activate. Most common usage - activate subscription
   *
   * @throws ItemUnavailableException       if can't activate this feature ( unknown name, not have sources ).
   * @throws ItemAlreadyOwnedException      if feature already activated.
   * @throws UserAccountManagementException if user not register before begin activating.
   */
  void activationBegin( @NonNull final Activity activity, @NonNull final FeatureName featureName, SKU skuToActivate )
      throws ItemUnavailableException, ItemAlreadyOwnedException, UserAccountManagementException;

  /**
   * Activate feature by Name with specific source
   *
   * @param activity    Activity for return after activation.
   * @param featureName name of feature for activating
   * @param sourceName  name of source for activating
   *
   * @throws ItemUnavailableException         if can't activate this feature ( unknown name, not have sources ).
   * @throws ItemAlreadyOwnedException        if feature already activated.
   * @throws UserAccountManagementException   if user not register before begin activating.
   * @throws InvalidActivationSourceException if source can not activate this feature
   */
  void activationBegin( @NonNull final Activity activity, @NonNull final FeatureName featureName,
                        @NonNull final String sourceName )
      throws ItemUnavailableException, ItemAlreadyOwnedException, UserAccountManagementException,
             InvalidActivationSourceException;

  /**
   * End activation process
   *
   * @param requestCode from onActivityResult
   * @param resultCode
   * @param intent      from onActivityResult
   *
   * @return {@code true} if activation valid or {@code false} if requestCode not valid and intent not for activation.
   *
   * @throws PurchaseExceptions if activate not valid.
   */
  boolean activationEnd( final int requestCode, int resultCode, @NonNull final Intent intent ) throws ActivateException;

  /**
   * Register notifier for get information about features changes.
   *
   * @param notifier Callback for information get
   */
  void registerNotifier( @NonNull final Notifier notifier );

  /**
   * Unregister notifier for get information about features changes.
   *
   * @param notifier Callback for information get
   */
  void unregisterNotifier( @NonNull final Notifier notifier );

  enum OPERATION_TYPE
  {
    UPDATE_FEATURES,
    ACTIVATE_FEATURE,
    CONSUME_FEATURE
  }

  /**
   * Interface for get notify on Feature list changes.
   */
  interface Notifier
  {
    /**
     * Call on changes Feature list.
     */
    void onChange();
  }

  /**
   * Interface for get notify about exceptions in async operations
   */
  interface ErrorNotifier extends Notifier
  {
    /**
     * Get error information in asynchronous operation
     *
     * @param type   Type of operation processed error.
     * @param errors Errors value.
     */
    void onErrors( @NonNull OPERATION_TYPE type, @NonNull final ActivateException[] errors );
  }
}
