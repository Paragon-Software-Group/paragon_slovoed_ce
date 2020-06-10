package com.paragon_software.favorites_manager;

import android.content.Context;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.DialogFragment;
import androidx.fragment.app.FragmentManager;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.screen_opener_api.ScreenOpenerAPI;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.hint_manager.HintManagerAPI;
import com.paragon_software.native_engine.EngineSerializerAPI;
import com.paragon_software.native_engine.SearchEngineAPI;
import com.paragon_software.settings_manager.SettingsManagerAPI;
import com.paragon_software.sound_manager.SoundManagerAPI;
import com.paragon_software.toolbar_manager.ToolbarManager;
import com.paragon_software.utils_slovoed.directory.Directory;

import java.util.List;

import io.reactivex.Observable;

public abstract class FavoritesManagerAPI
{
  public interface Factory
  {
    FavoritesManagerAPI create(ScreenOpenerAPI screenOpener, EngineSerializerAPI engineSerializer,
                               SettingsManagerAPI settingsManager, DictionaryManagerAPI dictionaryManager,
                               ToolbarManager toolbarManager, SoundManagerAPI soundManager,
                               FavoritesSorterAPI favoritesSorter,
                               Class<IExportHtmlBuilder> exportHtmlBuilderClass, int maxNumberOfWords );

    Factory registerSearchEngine( @Nullable SearchEngineAPI engineSearcher );
    Factory registerHintManager( @Nullable HintManagerAPI hintManager );

    Factory registerFileProviderAuthorities(@Nullable String fileProviderAuthorities);
    Factory registerUI( @Nullable Class favoritesActivity );
    Factory registerAddInDirectoryUI( @Nullable DialogFragment addInDirectoryDialogFragment );
  }

  public abstract void registerNotifier( @NonNull Notifier notifier );
  public abstract void unregisterNotifier( @NonNull Notifier notifier );

  // set of deprecated methods to work with items with no hierarchy (items from root directory)
  @NonNull
  public abstract List< ArticleItem > getWords();
  public abstract boolean hasWord( @Nullable ArticleItem word );
  public abstract boolean addWord( @NonNull ArticleItem word );
  public abstract boolean removeWord( @NonNull ArticleItem word );
  public abstract boolean removeWords( @NonNull List< ArticleItem > words );
  public abstract boolean removeAllWords();

  // Only for testmode
  public abstract boolean addWordsForTestmode( @NonNull List< ArticleItem > words );

  @NonNull
  abstract Observable< Directory< ArticleItem > > getRootDirectoryObservable();
  abstract boolean hasChildDirectory( @Nullable Directory< ArticleItem > parent, @Nullable String childDirName );
  abstract boolean addDirectory( @NonNull Directory< ArticleItem > parent, @NonNull String name );
  abstract boolean removeDirectory( @NonNull Directory< ArticleItem > toDelete );
  abstract boolean hasWord( @Nullable ArticleItem word, @Nullable Directory< ArticleItem > directory );
  abstract boolean addWord( @NonNull ArticleItem word, @Nullable Directory< ArticleItem > directory );
  abstract boolean removeWord( @NonNull ArticleItem word, @Nullable Directory< ArticleItem > directory );
  abstract boolean removeWords( @NonNull List< ArticleItem > words, @Nullable Directory< ArticleItem > directory );
  abstract boolean removeAllWords( @Nullable Directory< ArticleItem > directory );

  public abstract boolean isInTransition();
  public abstract void selectionModeOff();

  public abstract boolean showFavoritesScreen( @Nullable Context context );
  public abstract boolean showAddArticleInDirectoryScreen( @Nullable FragmentManager fragmentManager, @Nullable ArticleItem itemToAdd );

  @NonNull
  abstract FavoritesControllerAPI getController( @NonNull String uiName );
  abstract void freeController( @NonNull String uiName );
  @NonNull
  abstract FavoritesControllerAddInDirectoryAPI getAddInDirectoryController( @NonNull String uiName );
  abstract void freeAddInDirectoryController( @NonNull String uiName );

  interface Notifier
  {
  }
}
