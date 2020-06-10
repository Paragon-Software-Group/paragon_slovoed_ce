package com.paragon_software.history_manager;

import android.view.View;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.hint_manager.HintManagerAPI;
import com.paragon_software.screen_opener_api.ScreenOpenerAPI;
import com.paragon_software.settings_manager.SettingsManagerAPI;
import com.paragon_software.sound_manager.SoundManagerAPI;
import com.paragon_software.toolbar_manager.ToolbarManager;

public class BilingualHistoryController extends BaseHistoryController {
    public BilingualHistoryController(@NonNull HistoryManagerAPI manager, @NonNull ScreenOpenerAPI screenOpener,
                                      @Nullable SettingsManagerAPI settingsManager, @Nullable DictionaryManagerAPI dictionaryManager,
                                      @Nullable ToolbarManager toolbarManager, @Nullable SoundManagerAPI soundManager,
                                      @Nullable HintManagerAPI hintManager, @Nullable Class<IExportHTMLBuilder> exportHTMLBuilderClass,
                                      String authoritiesFileProvider) {
        super(manager, screenOpener, settingsManager, dictionaryManager, toolbarManager, soundManager,
                hintManager, exportHTMLBuilderClass, authoritiesFileProvider);
    }

    @Override
    protected void setDefaultToolbarState() {
        if (mToolbarManager != null) {
            mToolbarManager.showTitle(true);
        }
    }
}
