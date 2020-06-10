package com.paragon_software.toolbar_manager.Notifiers;

import com.paragon_software.toolbar_manager.ToolbarController;

public interface OnDeleteSelectedChange extends ToolbarController.Notifier
{
  void onDeleteSelectedActionVisibilityStatusChange( int status );
}
