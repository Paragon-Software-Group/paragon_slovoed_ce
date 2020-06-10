/*
 * slovoed
 *
 *  Created on: 28.02.18
 *      Author: Ivan Zhdanov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

package com.paragon_software.slovoed;

import android.app.Application;

import com.paragon_software.slovoed.exceptions.InitializeException;

public final class SlovoedApplication extends Application
{
  @Override
  public void onCreate()
  {
    super.onCreate();
    try
    {
      ApplicationState.initialize(this);
    }
    catch ( InitializeException e )
    {
      e.printStackTrace();
    }
  }
}
