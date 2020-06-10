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

import android.app.PendingIntent;
import android.content.Context;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

@SuppressWarnings( "unused" )
public interface ISplashScreenManager
{
  boolean showNext( @NonNull final Context context );

  void hide( @NonNull final String name );

  void reset();

  boolean haveNotShownScreens();

  /**
   * If 'onCompletePendingIntent' not null, launch 'onCompletePendingIntent' instead 'mMainActivitySplashScreen'
   * while {@link ISplashScreenManager#showNext} <br\>
   * Call of {@link ISplashScreenManager#reset()} set 'onCompletePendingIntent' to null
   */
  void setOnCompletePendingIntent( @Nullable PendingIntent onCompletePendingIntent );
}
