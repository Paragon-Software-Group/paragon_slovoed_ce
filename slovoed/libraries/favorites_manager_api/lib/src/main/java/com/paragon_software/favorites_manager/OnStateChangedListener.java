package com.paragon_software.favorites_manager;

public interface OnStateChangedListener extends FavoritesManagerAPI.Notifier
{
  void onFavoritesStateChanged();
}
