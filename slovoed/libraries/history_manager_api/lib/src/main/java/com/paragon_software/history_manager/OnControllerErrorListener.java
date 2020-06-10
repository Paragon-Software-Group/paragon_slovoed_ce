package com.paragon_software.history_manager;

public interface OnControllerErrorListener extends HistoryControllerAPI.Notifier
{
  void onControllerError( Exception exception );
}
