package com.paragon_software.toolbar_manager.Notifiers;

import com.paragon_software.toolbar_manager.ToolbarController;

public interface OnSelectionModeChange extends ToolbarController.Notifier
{
  void onSelectionModeStatusChange( int status );

  void onNumberOfSelectedItemsChange( int numberOfSelectedItems );
}
