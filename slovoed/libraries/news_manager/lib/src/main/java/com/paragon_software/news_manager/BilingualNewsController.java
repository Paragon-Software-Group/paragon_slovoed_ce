package com.paragon_software.news_manager;

import androidx.annotation.Nullable;

import com.paragon_software.hint_manager.HintManagerAPI;
import com.paragon_software.settings_manager.SettingsManagerAPI;
import com.paragon_software.toolbar_manager.ToolbarManager;

public class BilingualNewsController extends BaseNewsController {

    BilingualNewsController(NewsManagerAPI newsManager, ToolbarManager toolbarManager,
                            SettingsManagerAPI settingsManager, @Nullable HintManagerAPI hintManager) {
        super(newsManager, toolbarManager, settingsManager, hintManager);
    }

    @Override
    protected void showNewsContentToolbarUpdate() {
    }
}
