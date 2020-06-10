package com.paragon_software.article_manager;

public interface OnControllerErrorListener extends ArticleControllerAPI.Notifier
{
  void onControllerError( Exception exception );
}
