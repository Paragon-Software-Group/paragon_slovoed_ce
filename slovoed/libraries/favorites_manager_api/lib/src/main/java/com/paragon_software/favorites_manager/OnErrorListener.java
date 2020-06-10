package com.paragon_software.favorites_manager;

public interface OnErrorListener extends FavoritesManagerAPI.Notifier
{
  void onFavoritesManagerError( Exception exception );
}
