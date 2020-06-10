package com.paragon_software.settings_manager.exceptions;

/*
 * settings_manager_api.exceptions
 *
 *  Created on: 08.02.18
 *      Author: Mikhail Belyshov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

import androidx.annotation.NonNull;

/**
 * WrongTypeException throws when storage manager can't cast stored data to requested data type
 */
@SuppressWarnings( "WeakerAccess,unused" )
public class WrongTypeException extends SettingsManagerException
{

  public WrongTypeException( @NonNull final String reason )
  {
    super(reason);
  }

  public WrongTypeException()
  {
    super();
  }
}
