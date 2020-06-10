package com.paragon_software.splash_screen_manager;

/*
 * splash_screen_manager_api
 *
 *  Created on: 20.02.18
 *      Author: Mikhail Belyshov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

import android.content.Context;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

@SuppressWarnings( "unused" )
public interface ISplashScreen
{
  interface Factory
  {
    @Nullable ISplashScreen create( @Nullable Context context );
  }

  @NonNull
  String getName();

  void show( @NonNull final Context context );
}
