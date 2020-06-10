package com.paragon_software.toolbar_manager.Notifiers;

import androidx.annotation.NonNull;

import com.paragon_software.toolbar_manager.ToolbarController;

public interface OnTitleChange extends ToolbarController.Notifier
{
  void onTitleTextChange( @NonNull String text );

  void onTitleStatusChange( int status );
}
