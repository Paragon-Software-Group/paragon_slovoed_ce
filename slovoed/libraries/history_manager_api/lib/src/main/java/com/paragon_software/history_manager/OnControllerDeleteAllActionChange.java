package com.paragon_software.history_manager;

import com.paragon_software.toolbar_manager.ToolbarController;

public interface OnControllerDeleteAllActionChange extends ToolbarController.Notifier
{
  void onDeleteAllActionVisibilityStatusChange();

  void onDeleteAllActionEnableStatusChange();
}
