package com.paragon_software.favorites_manager;

public interface OnControllerErrorListener extends FavoritesControllerAPI.Notifier
{
  void onControllerError( Exception exception );
}
