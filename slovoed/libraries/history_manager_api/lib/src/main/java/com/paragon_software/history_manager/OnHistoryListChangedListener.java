package com.paragon_software.history_manager;

import java.util.List;

import com.paragon_software.article_manager.ArticleItem;

public interface OnHistoryListChangedListener extends HistoryManagerAPI.Notifier
{
  void onHistoryListChanged( List< ArticleItem > favoritesWords );
}
