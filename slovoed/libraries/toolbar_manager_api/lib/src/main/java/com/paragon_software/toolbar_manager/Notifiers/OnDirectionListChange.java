package com.paragon_software.toolbar_manager.Notifiers;

import androidx.annotation.NonNull;

import java.util.Collection;

import com.paragon_software.toolbar_manager.DirectionView;
import com.paragon_software.toolbar_manager.ToolbarController;

public interface OnDirectionListChange extends ToolbarController.Notifier
{
  void onDirectionListStatusChange( int status );

  void onDirectionListContentChange( Collection< DirectionView > content );

  void onDirectionChange( int element );
}
