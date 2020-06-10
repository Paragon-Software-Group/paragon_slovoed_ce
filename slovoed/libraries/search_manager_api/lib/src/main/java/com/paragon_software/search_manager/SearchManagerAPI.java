package com.paragon_software.search_manager;

import android.app.Activity;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.favorites_manager.FavoritesManagerAPI;
import com.paragon_software.hint_manager.HintManagerAPI;
import com.paragon_software.history_manager.HistoryManagerAPI;
import com.paragon_software.native_engine.GroupHeader;
import com.paragon_software.native_engine.ScrollResult;
import com.paragon_software.native_engine.SearchAllResult;
import com.paragon_software.native_engine.SearchEngineAPI;
import com.paragon_software.native_engine.SearchType;
import com.paragon_software.native_engine.SortType;
import com.paragon_software.screen_opener_api.ScreenOpenerAPI;
import com.paragon_software.settings_manager.SettingsManagerAPI;
import com.paragon_software.sound_manager.SoundManagerAPI;
import com.paragon_software.testmode.TestModeAPI;
import com.paragon_software.toolbar_manager.ToolbarManager;
import com.paragon_software.utils_slovoed.collections.CollectionView;

public abstract class SearchManagerAPI
{
  abstract public void registerScreenOpener( ScreenOpenerAPI manager );

  abstract public void registerSearchEngine( SearchEngineAPI engine );

  abstract public void registerSettingsManager( SettingsManagerAPI settingsManager );

  abstract public void registerSoundManager( SoundManagerAPI soundManager);

  abstract public void registerToolbarManager( ToolbarManager toolbarManager );

  abstract public void registerDictionaryManager( DictionaryManagerAPI dictionaryManager);

  abstract public void registerHistoryManager( HistoryManagerAPI historyManagerAPI );

  abstract public void registerFavoritesManager( FavoritesManagerAPI favoritesManagerAPI );

  abstract public void registerHintManager( @Nullable HintManagerAPI hintManager );

  abstract ScrollResult scroll(int direction, String word, boolean autoChangeDirection, boolean exactly);
  abstract public void setControllerSearchText(String uiName, @NonNull String searchText);


  abstract SearchAllResult searchAll(String word);

  abstract CollectionView<CollectionView<ArticleItem, GroupHeader>, Dictionary.Direction>
  search(int direction, @NonNull String word, boolean autoChangeDirection, @NonNull SearchType searchType, @NonNull SortType sortType, @NonNull Boolean needRunSearch);

  abstract public void registerNotifier( @NonNull Notifier notifier );

  abstract public void unregisterNotifier( @NonNull Notifier notifier );

  abstract SearchController getController( String uiName );

  abstract void freeController(String uiName);

  abstract void saveSearchRequest(@NonNull String text);

  abstract void saveIsScrollSelected(boolean isSearchTypeSelected);

  abstract String restoreSearchRequest();

  abstract boolean restoreIsScrollSelected();

  abstract boolean launchTestMode( @NonNull Activity activity, @NonNull String text );

  public abstract void registerTestMode( @NonNull TestModeAPI testMode );

  interface Notifier { }
}
