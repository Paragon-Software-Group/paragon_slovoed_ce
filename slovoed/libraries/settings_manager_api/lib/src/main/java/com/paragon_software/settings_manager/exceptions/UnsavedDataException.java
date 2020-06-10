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
 * UnsavedDataException throws when there is local changes changes, that were not backuped.
 */
@SuppressWarnings( "WeakerAccess, unused" )
public class UnsavedDataException extends SettingsManagerException
{
  public UnsavedDataException( @NonNull final String reason )
  {
    super(reason);
  }

  public UnsavedDataException()
  {
    super();
  }
}
