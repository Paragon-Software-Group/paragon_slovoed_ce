package com.paragon_software.favorites_manager;


import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.hint_manager.HintManagerAPI;
import com.paragon_software.native_engine.EngineSerializerAPI;
import com.paragon_software.native_engine.SearchEngineAPI;
import com.paragon_software.screen_opener_api.ScreenOpenerAPI;
import com.paragon_software.settings_manager.SettingsManagerAPI;
import com.paragon_software.sound_manager.SoundManagerAPI;
import com.paragon_software.toolbar_manager.ToolbarManager;
import com.paragon_software.utils_slovoed.directory.Directory;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.DialogFragment;

import static com.paragon_software.favorites_manager.mock.StaticConsts.ARTICLE1;
import static org.junit.Assert.assertEquals;

class MockFavoritesManager extends BaseFavoritesManager {

  private FavoritesControllerAPI mController;

  MockFavoritesManager(ScreenOpenerAPI screenOpener, EngineSerializerAPI engineSerializer, SearchEngineAPI engineSearcher, SettingsManagerAPI settingsManager, DictionaryManagerAPI dictionaryManager, ToolbarManager toolbarManager, SoundManagerAPI soundManager, HintManagerAPI hintManager, FavoritesSorterAPI favoritesSorter, Class favoritesActivity, @Nullable DialogFragment addInDirectoryDialogFragment, @Nullable Class<IExportHtmlBuilder> exportHtmlBuilderClass, @Nullable String authoritiesFileProvider, int maxNumberOfDirWords) {
    super(screenOpener, engineSerializer, engineSearcher, settingsManager, dictionaryManager, toolbarManager, soundManager, hintManager, favoritesSorter, favoritesActivity, addInDirectoryDialogFragment, exportHtmlBuilderClass, authoritiesFileProvider, maxNumberOfDirWords);
  }

  void setController(FavoritesControllerAPI mockController) {
    mController = mockController;
  }

  @NonNull
  @Override
  FavoritesControllerAPI getController(@NonNull String uiName) {
    return mController;
  }

  @Override
  boolean hasWord(@Nullable ArticleItem word, @Nullable Directory<ArticleItem> directory) {
    assertEquals(ARTICLE1, word);
    return true;
  }

  @Override
  public boolean addWord(@NonNull ArticleItem articleItem, @Nullable Directory<ArticleItem> directory) {
    assertEquals(ARTICLE1, articleItem);
    return true;
  }
}

