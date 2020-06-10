package com.paragon_software.favorites_manager;

import android.content.Context;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.FragmentManager;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.IDictionaryIcon;
import com.paragon_software.hint_manager.HintParams;
import com.paragon_software.hint_manager.HintType;
import com.paragon_software.utils_slovoed.directory.Directory;

import java.util.List;

import io.reactivex.Observable;

abstract class FavoritesControllerAPI
{

  public abstract void registerNotifier( @NonNull Notifier notifier );
  public abstract void unregisterNotifier( @NonNull Notifier notifier );

  // set of methods to work with items from current directory
  @NonNull
  public abstract List< ArticleItem > getWords();
  public abstract void deleteWord( int articleIndex );
  public abstract void openArticle( int articleIndex, Context context );

  public abstract void deleteAllWords();
  public abstract int getDeleteAllActionVisibilityStatus();
  public abstract boolean isDeleteAllActionEnable();

  public abstract boolean hasSound( int articleIndex );
  public abstract void playSound( int articleIndex );

  @NonNull
  public abstract List< ArticleItem > getSelectedWords();
  public abstract void selectItem( int articleIndex );
  public abstract void unselectItem( int articleIndex );
  public abstract boolean isInSelectionMode();
  public abstract void deleteSelectedWords();
  public abstract void selectionModeOff();

  public abstract float getEntryListFontSize();
  public abstract boolean isInTransition();

  @Nullable
  public abstract FavoritesSorting getFavoritesSorting();
  public abstract void selectSorting( int element );
  public abstract int getSortingActionVisibilityStatus();
  public abstract boolean isSortingActionEnable();

  @NonNull
  public abstract Observable< Directory< ArticleItem > > getCurrentDirectoryObservable();
  @Nullable
  public abstract Directory< ArticleItem > getCurrentDirectory();
  public abstract void goToDirectory( @NonNull Directory< ArticleItem > destination );
  public abstract void goToRootDirectory();
  public abstract boolean hasChildDirectory( @Nullable String name );
  public abstract boolean addDirectory( @NonNull String name );
  public abstract boolean deleteDirectory( @NonNull Directory< ArticleItem > toDelete );
  @NonNull
  public abstract Observable< Boolean > getEditModeStatusObservable();
  public abstract void setEditModeStatus( boolean editModeOn );
  public abstract boolean isInEditMode();

  public abstract void exportFavorites( @NonNull Context context, ExportParams params );
  public abstract void exportFavoritesAsString( @NonNull Context context, ExportParams params );
  public abstract int getShareActionVisibilityStatus();
  public abstract boolean isShareActionEnable();

  public abstract void activate();
  public abstract void deactivate();

  @Nullable
  public abstract IDictionaryIcon getDictionaryIcon(@NonNull Dictionary.DictionaryId dictionaryId);

  abstract boolean showHintManagerDialog( @NonNull HintType hintType, @Nullable FragmentManager fragmentManager, @Nullable HintParams hintParams );

  interface Notifier
  {
  }
}
