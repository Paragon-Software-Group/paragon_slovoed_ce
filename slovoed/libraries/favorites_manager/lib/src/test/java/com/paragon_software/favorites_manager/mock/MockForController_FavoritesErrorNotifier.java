package com.paragon_software.favorites_manager.mock;

import com.paragon_software.favorites_manager.OnErrorListener;

import java.util.ArrayList;

public abstract class MockForController_FavoritesErrorNotifier
{

  private final ArrayList<OnErrorListener> mOnHistoryErrorListeners = new ArrayList<>();

  public void registerNotifier( OnErrorListener notifier )
  {
    if (!mOnHistoryErrorListeners.contains(notifier))
    {
      mOnHistoryErrorListeners.add(notifier);
    }
  }

  public void onFavoritesError( Exception exception )
  {
    for (final OnErrorListener listener : mOnHistoryErrorListeners )
    {
      listener.onFavoritesManagerError(exception);
    }
  }
}