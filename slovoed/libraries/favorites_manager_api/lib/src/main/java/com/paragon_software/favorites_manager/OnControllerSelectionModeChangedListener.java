package com.paragon_software.favorites_manager;

import java.util.List;

import com.paragon_software.article_manager.ArticleItem;

public interface OnControllerSelectionModeChangedListener extends FavoritesControllerAPI.Notifier
{
  void onSelectedListChanged( List< ArticleItem > selectedWords );

  void onSelectionModeChanged( boolean selectionMode );
}
