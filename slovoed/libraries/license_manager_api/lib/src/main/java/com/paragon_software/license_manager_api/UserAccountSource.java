/*
 * license_manager_api
 *
 *  Created on: 09.02.18
 *      Author: Ivan Zhdanov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

package com.paragon_software.license_manager_api;

import androidx.annotation.NonNull;

import com.paragon_software.license_manager_api.exceptions.UserAccountManagementException;

/**
 * Source for manager user accounts
 */
@SuppressWarnings( { "unused", "RedundantThrows" } )
public interface UserAccountSource
{
  /**
   * Name of source
   *
   * @return current name
   */
  @NonNull
  String getName();

  /**
   * Create user account on source
   *
   * @param user user account for create.
   *
   * @throws UserAccountManagementException problem on account create process
   */
  void create( @NonNull final UserAccount user ) throws UserAccountManagementException;

  /**
   * Check user on source and get additional data.
   *
   * @param user User for registration
   *
   * @return full information about user account.
   *
   * @throws UserAccountManagementException problem on account register process
   */
  @NonNull
  UserAccount register( @NonNull final UserAccount user ) throws UserAccountManagementException;
}
