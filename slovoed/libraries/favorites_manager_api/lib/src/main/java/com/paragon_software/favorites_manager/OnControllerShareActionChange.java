package com.paragon_software.favorites_manager;

public interface OnControllerShareActionChange  extends FavoritesControllerAPI.Notifier
{
    void onShareActionVisibilityStatusChange();

    void onShareActionEnableStatusChange();
}
