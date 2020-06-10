package com.paragon_software.search_manager;

public interface OnControllerErrorListener extends SearchController.Notifier
{
  void onControllerError( Exception exception );
}
