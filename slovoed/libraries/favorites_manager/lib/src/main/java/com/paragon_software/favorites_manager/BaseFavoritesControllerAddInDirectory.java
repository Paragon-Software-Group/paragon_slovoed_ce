package com.paragon_software.favorites_manager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.FragmentManager;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.hint_manager.HintParams;
import com.paragon_software.hint_manager.HintType;
import com.paragon_software.utils_slovoed.directory.Directory;
import io.reactivex.Observable;

public class BaseFavoritesControllerAddInDirectory extends FavoritesControllerAddInDirectoryAPI
{
  @NonNull
  private final        FavoritesManagerAPI    mManager;
  @NonNull
  private final        FavoritesControllerAPI mFavoritesController;
  @Nullable
  private              ArticleItem            mArticleItemToSave;

  public BaseFavoritesControllerAddInDirectory( @NonNull FavoritesManagerAPI manager,
                                                @NonNull String controllerId )
  {
    mManager = manager;
    mFavoritesController = mManager.getController(UI_TYPE + controllerId);
  }

  @Override
  public void registerNotifier( @NonNull FavoritesControllerAPI.Notifier notifier )
  {
    mFavoritesController.registerNotifier(notifier);
  }

  @Override
  public void unregisterNotifier( @NonNull FavoritesControllerAPI.Notifier notifier )
  {
    mFavoritesController.unregisterNotifier(notifier);
  }

  @Override
  public float getEntryListFontSize()
  {
    return mFavoritesController.getEntryListFontSize();
  }

  @Override
  public boolean isInTransition()
  {
    return mFavoritesController.isInTransition();
  }

  @Override
  public boolean hasWord()
  {
    return mManager.hasWord(mArticleItemToSave, mFavoritesController.getCurrentDirectory());
  }

  @NonNull
  @Override
  public Observable< Directory< ArticleItem > > getCurrentDirectoryObservable()
  {
    return mFavoritesController.getCurrentDirectoryObservable();
  }

  @Nullable
  @Override
  public Directory< ArticleItem > getCurrentDirectory()
  {
    return mFavoritesController.getCurrentDirectory();
  }

  @Override
  public void goToDirectory( @NonNull Directory< ArticleItem > destination )
  {
    mFavoritesController.goToDirectory(destination);
  }

  @Override
  public void goToRootDirectory() {
    mFavoritesController.goToRootDirectory();
  }

  @Override
  public boolean hasChildDirectory( @Nullable String name )
  {
    return mFavoritesController.hasChildDirectory(name);
  }

  @Override
  public boolean addDirectory( @NonNull String name )
  {
    return mFavoritesController.addDirectory(name);
  }

  @Override
  void setArticleItemToSave( @Nullable ArticleItem articleItemToSave )
  {
    mArticleItemToSave = articleItemToSave;
  }

  @Override
  public boolean save()
  {
    boolean res = false;
    if ( mArticleItemToSave != null )
    {
      res = mManager.addWord(mArticleItemToSave, mFavoritesController.getCurrentDirectory());
    }
    return res;
  }

  @Override
  public void activate()
  {
    mFavoritesController.activate();
  }

  @Override
  public void deactivate()
  {
    mFavoritesController.deactivate();
  }

  @Override
  boolean showHintManagerDialog( @NonNull HintType hintType,
                                 @Nullable FragmentManager fragmentManager,
                                 @Nullable HintParams hintParams )
  {
    return mFavoritesController.showHintManagerDialog(hintType, fragmentManager, hintParams);
  }
}
