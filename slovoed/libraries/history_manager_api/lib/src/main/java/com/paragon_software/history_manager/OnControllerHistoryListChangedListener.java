package com.paragon_software.history_manager;

import java.util.List;

import com.paragon_software.article_manager.ArticleItem;

public interface OnControllerHistoryListChangedListener extends HistoryControllerAPI.Notifier
{
  void onControllerHistoryListChanged( List< ArticleItem > historyWords );
}
