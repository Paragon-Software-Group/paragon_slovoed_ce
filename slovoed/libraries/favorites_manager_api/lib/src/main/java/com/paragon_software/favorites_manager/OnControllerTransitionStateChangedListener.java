package com.paragon_software.favorites_manager;

public interface OnControllerTransitionStateChangedListener extends FavoritesControllerAPI.Notifier
{
  void onControllerTransitionStateChanged( boolean inTransition );
}
