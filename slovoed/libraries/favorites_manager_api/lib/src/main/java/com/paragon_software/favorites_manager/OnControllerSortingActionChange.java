package com.paragon_software.favorites_manager;

public interface OnControllerSortingActionChange extends FavoritesControllerAPI.Notifier
{
  void onSortingActionVisibilityStatusChange();

  void onSortingActionEnableStatusChange();
}
