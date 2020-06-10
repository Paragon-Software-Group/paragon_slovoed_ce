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
 * StorageTransferException throws when uploading to existing storage fails.
 * Use {@code getMessage()} to get additional text info.
 */
@SuppressWarnings( "WeakerAccess, unused" )
public class StorageTransferException extends SettingsManagerException
{

  public StorageTransferException( @NonNull final String reason )
  {
    super(reason);
  }
}
