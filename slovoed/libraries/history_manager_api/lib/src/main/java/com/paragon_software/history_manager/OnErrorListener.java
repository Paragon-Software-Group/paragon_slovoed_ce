package com.paragon_software.history_manager;

public interface OnErrorListener extends HistoryManagerAPI.Notifier
{
  void onHistoryManagerError( Exception exception );
}
