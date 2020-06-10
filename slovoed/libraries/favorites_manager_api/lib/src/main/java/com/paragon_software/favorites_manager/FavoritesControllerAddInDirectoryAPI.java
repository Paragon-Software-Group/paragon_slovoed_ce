package com.paragon_software.favorites_manager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.FragmentManager;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.hint_manager.HintParams;
import com.paragon_software.hint_manager.HintType;
import com.paragon_software.utils_slovoed.directory.Directory;
import io.reactivex.Observable;

abstract class FavoritesControllerAddInDirectoryAPI
{
  public static final String                 UI_TYPE = "ADD_IN_DIRECTORY";

  public abstract void registerNotifier( @NonNull FavoritesControllerAPI.Notifier notifier );
  public abstract void unregisterNotifier( @NonNull FavoritesControllerAPI.Notifier notifier );

  public abstract float getEntryListFontSize();
  public abstract boolean isInTransition();

  public abstract boolean hasWord();
  @NonNull
  public abstract Observable< Directory< ArticleItem > > getCurrentDirectoryObservable();
  @Nullable
  public abstract Directory< ArticleItem > getCurrentDirectory();
  public abstract void goToDirectory( @NonNull Directory< ArticleItem > destination );
  public abstract void goToRootDirectory();
  public abstract boolean hasChildDirectory( @Nullable String name );
  public abstract boolean addDirectory( @NonNull String name );

  abstract void setArticleItemToSave( @Nullable ArticleItem articleItemToSave );

  public abstract boolean save();

  public abstract void activate();
  public abstract void deactivate();

  abstract boolean showHintManagerDialog( @NonNull HintType hintType, @Nullable FragmentManager fragmentManager, @Nullable HintParams hintParams );
}
