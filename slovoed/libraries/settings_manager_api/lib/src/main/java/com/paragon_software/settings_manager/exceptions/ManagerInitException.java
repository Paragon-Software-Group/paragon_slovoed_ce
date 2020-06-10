package com.paragon_software.settings_manager.exceptions;

/*
 * settings_manager_api.exceptions
 *
 *  Created on: 15.02.18
 *      Author: Mikhail Belyshov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

import androidx.annotation.NonNull;

/**
 * ManagerInitException throws when something went wrong in time of storage manager initialization
 */
@SuppressWarnings( "WeakerAccess, unused" )
public class ManagerInitException extends SettingsManagerException
{
  public ManagerInitException( @NonNull final String reason )
  {
    super(reason);
  }
}

