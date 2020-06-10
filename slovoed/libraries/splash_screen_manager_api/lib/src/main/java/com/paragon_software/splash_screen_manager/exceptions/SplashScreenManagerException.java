package com.paragon_software.splash_screen_manager.exceptions;

/*
 * splash_screen_manager_api.exceptions
 *
 *  Created on: 21.02.18
 *      Author: Mikhail Belyshov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

import androidx.annotation.NonNull;

@SuppressWarnings( "WeakerAccess, unused" )
public class SplashScreenManagerException extends Exception
{

  public SplashScreenManagerException( @NonNull final String reason )
  {
    super(reason);
  }

  public SplashScreenManagerException()
  {
    super();
  }
}
