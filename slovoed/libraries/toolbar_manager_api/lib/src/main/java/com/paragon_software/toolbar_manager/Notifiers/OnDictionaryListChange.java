package com.paragon_software.toolbar_manager.Notifiers;

import com.paragon_software.toolbar_manager.ToolbarController;

public interface OnDictionaryListChange extends ToolbarController.Notifier
{
  void onDictionaryListContentChange();

  void onDictionaryListStatusChange( int status );

  void onDictionaryListSelectionChange( int element );
}
