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

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.license_manager_api.exceptions.UserAccountManagementException;

/**
 * Management user accounts.
 */
@SuppressWarnings( { "unused", "RedundantThrows" } )
public interface UserAccountManager
{
  /**
   * Register user in application.
   * Check user account on external service via UserAccountSource
   * Use all connected UserAccountSource for check account
   *
   * @param user User for registration.
   *
   * @throws UserAccountManagementException problem on account register process
   */
  void registerUserAccount( @NonNull final UserAccount user ) throws UserAccountManagementException;

  /**
   * Register user in application.
   * Check user account on external service via UserAccountSource
   * Use selected connected UserAccountSource for check account
   *
   * @param user   User for registration.
   * @param source Selected UserAccountSource.
   *
   * @throws UserAccountManagementException problem on account register process
   */
  void registerUserAccount( @NonNull final UserAccount user, @NonNull final String source )
      throws UserAccountManagementException;

  /**
   * Register user on external source.
   * Try create on all sources, stop on first created.
   *
   * @param user User for registration.
   *
   * @throws UserAccountManagementException problem on account create process
   */
  void createUserAccount( @NonNull final UserAccount user ) throws UserAccountManagementException;

  /**
   * Register user on external source.
   * Create on selected source.
   *
   * @param user   User for registration.
   * @param source Selected UserAccountSource.
   *
   * @throws UserAccountManagementException problem on account create process
   */
  void createUserAccount( @NonNull final UserAccount user, @NonNull final String source )
      throws UserAccountManagementException;

  /**
   * Get registered user in system.
   *
   * @return registered user account
   */
  @Nullable
  UserAccount getUserAccount();
}
