package com.paragon_software.favorites_manager;

import java.util.List;

import com.paragon_software.article_manager.ArticleItem;

public interface OnFavoritesListChangesListener extends FavoritesManagerAPI.Notifier
{
  void onFavoritesListChanged( List< ArticleItem > favoritesWords );
}
