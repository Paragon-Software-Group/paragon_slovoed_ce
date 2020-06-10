package com.paragon_software.favorites_manager.mock;

import com.paragon_software.favorites_manager.OnStateChangedListener;

import java.util.ArrayList;

public abstract class MockForController_FavoritesStateChangedNotifier
{

  private final ArrayList<OnStateChangedListener> mOnFavoritesStateChangedListeners = new ArrayList<>();

  public void registerNotifier( OnStateChangedListener notifier )
  {
    if (!mOnFavoritesStateChangedListeners.contains(notifier))
    {
      mOnFavoritesStateChangedListeners.add(notifier);
    }
  }

  public void onStateChanged()
  {
    for (final OnStateChangedListener listener : mOnFavoritesStateChangedListeners )
    {
      listener.onFavoritesStateChanged();
    }
  }
}