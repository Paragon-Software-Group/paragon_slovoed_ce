package com.paragon_software.history_manager;

import java.util.List;

import com.paragon_software.article_manager.ArticleItem;

public interface OnControllerSelectionModeChangedListener extends HistoryControllerAPI.Notifier
{
  void onSelectedListChanged( List< ArticleItem > selectedWords );

  void onSelectionModeChanged( boolean selectionMode );
}
