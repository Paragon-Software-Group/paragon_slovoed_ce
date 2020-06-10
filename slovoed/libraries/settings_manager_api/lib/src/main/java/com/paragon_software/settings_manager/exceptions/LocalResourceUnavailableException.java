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
 * LocalResourceUnavailableException throws when something went wrong with storing settings on local application memory.
 */
@SuppressWarnings( "WeakerAccess, unused" )
public class LocalResourceUnavailableException extends SettingsManagerException
{
  public LocalResourceUnavailableException( @NonNull final String reason )
  {
    super(reason);
  }
}
