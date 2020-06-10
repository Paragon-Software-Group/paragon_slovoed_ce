package com.paragon_software.favorites_manager;

public interface OnControllerDeleteAllActionChange extends FavoritesControllerAPI.Notifier
{
  void onDeleteAllActionVisibilityStatusChange();

  void onDeleteAllActionEnableStatusChange();
}
