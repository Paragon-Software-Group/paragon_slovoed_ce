package com.paragon_software.history_manager;

public interface OnControllerTransitionStateChangedListener extends HistoryControllerAPI.Notifier
{
  void onControllerTransitionStateChanged( boolean inTransition );
}
