/*
 * splash_screen_manager_paragon
 *
 *  Created on: 21.02.18
 *      Author: Mikhail Belyshov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

package com.paragon_software.splash_screen_manager;

import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.util.Iterator;
import java.util.LinkedHashMap;

import com.paragon_software.settings_manager.SettingsManagerAPI;
import com.paragon_software.settings_manager.exceptions.LocalResourceUnavailableException;
import com.paragon_software.settings_manager.exceptions.ManagerInitException;
import com.paragon_software.settings_manager.exceptions.WrongTypeException;
import com.paragon_software.splash_screen_manager.exceptions.SplashScreenManagerInitException;

@SuppressWarnings( { "unused" } )
public class ParagonSplashScreenManager implements ISplashScreenManager
{

  private final static String BASE_SETTINGS_KEY        = "ParagonSplashScreenManager";
  private final static String HIDE_SPLASH_SETTINGS_KEY = BASE_SETTINGS_KEY + "HideSplash";

  @NonNull
  private Iterator< SplashScreen > mCurrentSplashScreenIterator;

  private       SplashScreen             mCurrentScreen;
  @NonNull
  private final SettingsManagerAPI       mSettingsManager;
  @NonNull
  private final MainActivitySplashScreen mMainActivitySplashScreen;
  @Nullable
  private       PendingIntent            mOnCompletePendingIntent;

  @NonNull
  private final LinkedHashMap< String, SplashScreen > mSplashScreens = new LinkedHashMap<>();


  public ParagonSplashScreenManager( @NonNull final SettingsManagerAPI settingsManager,
                                     @NonNull final ISplashScreen[] splashScreens,
                                     @NonNull final Class< ? > mainActivity ) throws SplashScreenManagerInitException
  {

    mSettingsManager = settingsManager;
    for ( ISplashScreen splashScreen : splashScreens )
    {
      final String name = splashScreen.getName();
      if ( mSplashScreens.containsKey(name) )
      {
        throw new SplashScreenManagerInitException(
            "Splash Screen Manager not supporting two storage with the same names");
      }
      mSplashScreens.put(name, new SplashScreen(splashScreen, loadScreenHideStatus(name)));
    }
    reset();
    mMainActivitySplashScreen = new MainActivitySplashScreen(mainActivity);
  }

  private boolean loadScreenHideStatus( @NonNull final String screenName )
  {
    try
    {
      return mSettingsManager.load(settingsKeyForSplashByName(screenName), true);
    }
    catch ( WrongTypeException | ManagerInitException e )
    {
      return false;
    }
  }

  @NonNull
  private String settingsKeyForSplashByName( @NonNull final String splashName )
  {
    return HIDE_SPLASH_SETTINGS_KEY + splashName;
  }

  @Override
  public boolean showNext( @NonNull final Context context )
  {
    if ( mCurrentScreen != null )
    {
      hide(mCurrentScreen);
    }
    while ( mCurrentSplashScreenIterator.hasNext() )
    {
      mCurrentScreen = mCurrentSplashScreenIterator.next();
      if ( mCurrentScreen.needShow() )
      {
        mCurrentScreen.getSplashScreen().show(context);
        return true;
      }
    }
    mCurrentScreen = null;
    if ( null != mOnCompletePendingIntent )
    {
      sendPendingIntent(mOnCompletePendingIntent);
      mOnCompletePendingIntent = null;
    }
    else
    {
      mMainActivitySplashScreen.show(context);
    }
    return false;
  }

  private void sendPendingIntent( @NonNull PendingIntent pendingIntent )
  {
    try
    {
      pendingIntent.send();
    }
    catch ( PendingIntent.CanceledException e )
    {
      e.printStackTrace();
    }
  }

  @Override
  public void hide( @NonNull final String name )
  {
    SplashScreen screen = mSplashScreens.get(name);
    if ( null == screen )
    {
      return;
    }
    hide(screen);
  }

  private void hide( @NonNull SplashScreen screen )
  {
    screen.hide();
    saveScreenStatus(screen.getSplashScreen().getName(), false);
  }

  @Override
  public void reset()
  {
    mCurrentScreen = null;
    mOnCompletePendingIntent = null;
    mCurrentSplashScreenIterator = mSplashScreens.values().iterator();
  }

  @Override
  public boolean haveNotShownScreens() {
    boolean res = false;
    for(SplashScreen splashScreen : mSplashScreens.values())
      if(splashScreen.needShow()) {
        res = true;
        break;
      }
    return res;
  }

  @Override
  public void setOnCompletePendingIntent( @Nullable PendingIntent onCompletePendingIntent )
  {
    mOnCompletePendingIntent = onCompletePendingIntent;
  }

  private void saveScreenStatus( @NonNull final String screenName,
                                 @SuppressWarnings( "SameParameterValue" ) final boolean needShow )
  {
    try
    {
      mSettingsManager.save(settingsKeyForSplashByName(screenName), needShow, true);
    }
    catch ( ManagerInitException | LocalResourceUnavailableException ignore )
    {

    }
  }

  private class SplashScreen
  {
    @NonNull
    final private ISplashScreen mSplashScreen;
    boolean mNeedShow = true;

    SplashScreen( @NonNull ISplashScreen splashScreen )
    {
      this(splashScreen, true);
    }

    SplashScreen( @NonNull ISplashScreen splashScreen, boolean needShow )
    {
      mSplashScreen = splashScreen;
      mNeedShow = needShow;
    }

    final synchronized boolean needShow()
    {
      return mNeedShow;
    }

    final synchronized void hide()
    {
      mNeedShow = false;
    }

    @NonNull
    final ISplashScreen getSplashScreen()
    {
      return mSplashScreen;
    }

  }

  private class MainActivitySplashScreen implements ISplashScreen
  {
    private final Class< ? > mMainActivity;
    @SuppressWarnings( "CanBeFinal" )
    private boolean mIsShown = false;

    MainActivitySplashScreen( @NonNull final Class< ? > mainActivity )
    {
      mMainActivity = mainActivity;
    }

    @NonNull
    @Override
    public String getName()
    {
      return "MainActivity";
    }

    @Override
    public void show( @NonNull Context context )
    {
      if ( !mIsShown )
      {
        Intent i = new Intent(context, mMainActivity);
        context.startActivity(i);
      }
    }
  }
}
