package com.paragon_software.toolbar_manager.Notifiers;

import com.paragon_software.toolbar_manager.ToolbarController;

public interface OnShowBackgroundChange extends ToolbarController.Notifier
{
  void onOnShowBackgroundStatusChange( boolean showBackground );
}
