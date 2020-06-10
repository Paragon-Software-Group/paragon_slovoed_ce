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
 * OnlyOlderVersionFoundException throws when you trying to restore data, but local data has newest version.
 */
@SuppressWarnings( "WeakerAccess, unused" )
public class OnlyOlderVersionFoundException extends SettingsManagerException
{
  public OnlyOlderVersionFoundException( @NonNull final String reason )
  {
    super(reason);
  }

  public OnlyOlderVersionFoundException()
  {
    super();
  }
}
