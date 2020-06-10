package com.paragon_software.navigation_manager;

import android.content.Context;
import android.os.Bundle;
import android.util.Pair;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.article_manager.ShowArticleOptions;
import com.paragon_software.screen_opener_api.ScreenOpenerAPI;
import com.paragon_software.screen_opener_api.ScreenType;

import java.util.List;

import io.reactivex.Observable;
import io.reactivex.subjects.PublishSubject;
import io.reactivex.subjects.Subject;

public class StandardScreenOpener implements ScreenOpenerAPI
{

  protected final NavigationManager mNavigationManager;

  protected String mArticleControllerId;

  private static Subject< Pair< ScreenType, Bundle > > mScreenOpenerSubject = PublishSubject.create();

  StandardScreenOpener(NavigationManager navigationManager, String articleControllerId) {

    mNavigationManager = navigationManager;
    mArticleControllerId = articleControllerId;
  }

  @Override
  public boolean showArticleActivity(@Nullable ArticleItem article, Context context )
  {
    if ( mNavigationManager.getArticleManager() != null )
    {
      return mNavigationManager.getArticleManager().showArticleActivity(article, mArticleControllerId, context);
    }
    return false;
  }

  @Override
  public boolean showArticle(@Nullable ArticleItem article, @Nullable ShowArticleOptions showArticleOptions,
                                     Context context )
  {
    if ( mNavigationManager.getArticleManager() != null )
    {
        return mNavigationManager
                .getArticleManager().showArticleActivity(article, showArticleOptions, mArticleControllerId, context);
    }
    return false;
  }

  @Override
  public boolean showArticleFromSeparateList(@NonNull List<ArticleItem> separateArticles, int currentArticleIndex, Context context) {
    if (mNavigationManager.getArticleManager() != null) {
        return mNavigationManager.getArticleManager()
                .showArticleActivityFromSeparateList(separateArticles, currentArticleIndex, mArticleControllerId, context);
    }
    return false;
  }

  @Override
  public void openScreen( @NonNull ScreenType screenType) {
    openScreen(screenType, null);
  }

  @Override
  public void openScreen( @NonNull ScreenType screenType, @Nullable Bundle bundle) {
    Pair<ScreenType, Bundle> screenTypePair = new Pair<>(screenType, bundle);
    mScreenOpenerSubject.onNext(screenTypePair);
  }

  @NonNull
  @Override
  public Observable< Pair< ScreenType, Bundle > > getScreenOpenerObservable() {
    return mScreenOpenerSubject;
  }

  @Override
  public String getArticleControllerId() {
    return mArticleControllerId;
  }

  @NonNull
  @Override
  public Observable<Boolean> getTopScreenOverlayStateObservable() {
    return mNavigationManager.getTopScreenOverlayStateObservable();
  }

  @Override
  public void registerFragment(@NonNull ScreenType screenType, @Nullable Fragment fragment) {
    mNavigationManager.registerFragment(screenType, fragment);
  }
}
