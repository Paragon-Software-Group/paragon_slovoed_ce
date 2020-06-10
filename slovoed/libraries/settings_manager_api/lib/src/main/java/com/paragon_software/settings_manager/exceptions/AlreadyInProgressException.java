package com.paragon_software.settings_manager.exceptions;

/*
 * settings_manager_api.exceptions
 *
 *  Created on: 13.02.18
 *      Author: Mikhail Belyshov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

import androidx.annotation.NonNull;

/**
 * AlreadyInProgressException throws when you trying to perform backup or restore operation, while other backup or restore in progress.
 */
@SuppressWarnings( "WeakerAccess, unused" )
public class AlreadyInProgressException extends SettingsManagerException
{
  public AlreadyInProgressException( @NonNull final String reason )
  {
    super(reason);
  }

  public AlreadyInProgressException()
  {
    super();
  }
}
