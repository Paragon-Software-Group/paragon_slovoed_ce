package com.paragon_software.favorites_manager;

import java.util.List;

import com.paragon_software.article_manager.ArticleItem;

public interface OnControllerFavoritesListChangedListener extends FavoritesControllerAPI.Notifier
{
  void onControllerFavoritesListChanged( List< ArticleItem > favoritesWords );
}
