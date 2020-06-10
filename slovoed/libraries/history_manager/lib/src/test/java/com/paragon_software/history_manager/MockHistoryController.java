package com.paragon_software.history_manager;

import android.content.Intent;

import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.hint_manager.HintManagerAPI;
import com.paragon_software.screen_opener_api.ScreenOpenerAPI;
import com.paragon_software.settings_manager.SettingsManagerAPI;
import com.paragon_software.sound_manager.SoundManagerAPI;
import com.paragon_software.toolbar_manager.ToolbarManager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

public class MockHistoryController extends BaseHistoryController {
  private Intent mIntent;

  MockHistoryController(@NonNull HistoryManagerAPI manager, @NonNull ScreenOpenerAPI screenOpener, @Nullable SettingsManagerAPI settingsManager, @Nullable DictionaryManagerAPI dictionaryManager, @Nullable ToolbarManager toolbarManager, @Nullable SoundManagerAPI soundManager, @Nullable HintManagerAPI hintManager, @Nullable Class<IExportHTMLBuilder> exportHTMLBuilderClass, String authoritiesFileProvider) {
    super(manager, screenOpener, settingsManager, dictionaryManager, toolbarManager, soundManager, hintManager, exportHTMLBuilderClass, authoritiesFileProvider);
  }

  void setIntent(Intent intent) {
    mIntent = intent;
  }

  @Override
  protected Intent getIntentForExport(String mimeTypes) {
    mIntent.setType(mimeTypes);
    return mIntent;
  }
}
