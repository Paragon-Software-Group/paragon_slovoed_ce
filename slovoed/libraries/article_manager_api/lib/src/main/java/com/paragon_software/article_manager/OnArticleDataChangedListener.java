package com.paragon_software.article_manager;

interface OnArticleDataChangedListener extends ArticleControllerAPI.Notifier
{

  void onArticleDataChanged();
}
