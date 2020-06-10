/*
 * SearchAllDictionary
 *
 *  Created on: 28.05.18
 *      Author: Ivan Zhdanov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

package com.paragon_software.engine;

import android.os.Handler;
import android.os.Looper;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;

/**
 * Helper for execute operation in Main ( UI ) thread.
 *
 * @param <T> execute option type.
 */
public abstract class MainThreadRunner< T >
{
  @NonNull
  private final Handler handler = new Handler(Looper.getMainLooper());

  /**
   * Execute operation.
   */
  public final void run()
  {
    run(null);
  }

  /**
   * Execute operation with parameters.
   *
   * @param param parameters for send to onRun function.
   */
  @SuppressWarnings( "WeakerAccess" )
  public final void run( @Nullable final T param )
  {
    handler.post(new Runnable()
    {
      @Override
      public void run()
      {
        onRun(param);
      }
    });
  }

  /**
   * Executed operation, this operation will be run in Main thread.
   *
   * @param param operation options or null.
   */
  protected abstract void onRun( @SuppressWarnings( "unused" ) @Nullable final T param );
}