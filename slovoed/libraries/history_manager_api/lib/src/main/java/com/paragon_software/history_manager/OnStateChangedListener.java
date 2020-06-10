package com.paragon_software.history_manager;

public interface OnStateChangedListener extends HistoryManagerAPI.Notifier
{
  void onHistoryStateChanged();
}
