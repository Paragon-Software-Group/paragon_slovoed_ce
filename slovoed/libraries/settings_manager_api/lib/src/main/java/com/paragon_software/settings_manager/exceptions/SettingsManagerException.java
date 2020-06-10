package com.paragon_software.settings_manager.exceptions;

/*
 * settings_manager_api.exceptions
 *
 *  Created on: 07.02.18
 *      Author: Mikhail Belyshov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

import androidx.annotation.NonNull;

/**
 * SettingsManagerException base class for all exceptions, that can be thrown of this API
 * See:
 * {@link AlreadyInProgressException}
 * {@link LocalResourceUnavailableException}
 * {@link ManagerInitException}
 * {@link NoSuchStorageException}
 * {@link OnlyOlderVersionFoundException}
 * {@link StorageTransferException}
 * {@link UnsavedDataException}
 * {@link WrongTypeException}
 */
@SuppressWarnings( "WeakerAccess, unused" )
public class SettingsManagerException extends Exception
{

  public SettingsManagerException( @NonNull final String reason )
  {
    super(reason);
  }

  public SettingsManagerException()
  {
    super();
  }
}
