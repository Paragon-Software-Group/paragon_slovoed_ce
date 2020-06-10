package com.paragon_software.history_manager;

import com.paragon_software.toolbar_manager.ToolbarController;

public interface OnControllerExportActionChangeListener extends ToolbarController.Notifier
{
    void onExportActionEnableStatusChange();

    void onExportVisibilityStatusChange();
}
