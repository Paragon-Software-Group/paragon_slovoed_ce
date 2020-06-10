package com.paragon_software.article_manager;

interface OnFlashcardButtonStateChangedListener extends ArticleControllerAPI.Notifier
{
  void onFlashcardButtonStateChanged();
}
