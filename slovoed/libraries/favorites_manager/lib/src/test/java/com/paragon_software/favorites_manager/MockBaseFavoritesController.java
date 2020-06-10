package com.paragon_software.favorites_manager;

import android.content.Intent;

import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.hint_manager.HintManagerAPI;
import com.paragon_software.screen_opener_api.ScreenOpenerAPI;
import com.paragon_software.settings_manager.SettingsManagerAPI;
import com.paragon_software.sound_manager.SoundManagerAPI;
import com.paragon_software.toolbar_manager.ToolbarManager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

public class MockBaseFavoritesController extends BaseFavoritesController {
    private Intent mIntent;

    MockBaseFavoritesController(@NonNull FavoritesManagerAPI manager, @NonNull ScreenOpenerAPI screenOpener, @Nullable SettingsManagerAPI settingsManager, @Nullable DictionaryManagerAPI dictionaryManager, @Nullable ToolbarManager toolbarManager, @Nullable SoundManagerAPI soundManager, @Nullable HintManagerAPI hintManager, @Nullable FavoritesSorterAPI favoritesSorter, @Nullable Class<IExportHtmlBuilder> exportHtmlBuilderClass, @NonNull String controllerId, @Nullable String fileProvider) {
        super(manager, screenOpener, settingsManager, dictionaryManager, toolbarManager, soundManager, hintManager, favoritesSorter, exportHtmlBuilderClass, controllerId, fileProvider);
    }

    void setIntentForExport(Intent intent) {
        mIntent = intent;
    }

    @Override
    protected Intent getIntentForExport(String mimeTypes) {
        mIntent.setType(mimeTypes);
        return mIntent;
    }
}
