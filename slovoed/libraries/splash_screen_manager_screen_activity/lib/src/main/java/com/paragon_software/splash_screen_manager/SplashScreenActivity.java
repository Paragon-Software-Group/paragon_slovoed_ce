package com.paragon_software.splash_screen_manager;

import android.content.Context;
import android.content.Intent;
import androidx.annotation.NonNull;

/*
 * splash_screen_activity
 *
 *  Created on: 26.02.18
 *      Author: Mikhail Belyshov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

@SuppressWarnings( "unused" )
public class SplashScreenActivity implements ISplashScreen
{

  private final String     mName;
  private final Class< ? > mActivity;

  public SplashScreenActivity( @NonNull final String name, @NonNull final Class< ? > activity )
  {
    this.mName = name;
    this.mActivity = activity;
  }

  @NonNull
  @Override
  public String getName()
  {
    return mName;
  }

  @Override
  public void show( @NonNull Context context )
  {
    Intent intent = new Intent(context, mActivity);
    intent.setFlags(intent.getFlags() | Intent.FLAG_ACTIVITY_NO_HISTORY);
    context.startActivity(intent);
  }
}
