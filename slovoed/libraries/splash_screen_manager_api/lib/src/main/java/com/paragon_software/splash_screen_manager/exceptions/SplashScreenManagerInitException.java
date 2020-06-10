package com.paragon_software.splash_screen_manager.exceptions;

/*
 * settings_manager_api.exceptions
 *
 *  Created on: 21.02.18
 *      Author: Mikhail Belyshov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

import androidx.annotation.NonNull;

/**
 * SplashScreenManagerInitException throws when something went wrong when splash screen manager is initialization
 */
@SuppressWarnings( "WeakerAccess, unused" )
public class SplashScreenManagerInitException extends SplashScreenManagerException
{
  public SplashScreenManagerInitException( @NonNull final String reason )
  {
    super(reason);
  }
}
