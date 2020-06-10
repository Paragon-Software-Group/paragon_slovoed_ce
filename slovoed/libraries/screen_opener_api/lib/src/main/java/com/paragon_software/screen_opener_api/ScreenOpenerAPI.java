package com.paragon_software.screen_opener_api;

import android.content.Context;
import android.os.Bundle;
import android.util.Pair;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.article_manager.ShowArticleOptions;

import java.util.List;

import io.reactivex.Observable;

public interface ScreenOpenerAPI
{
  boolean showArticleActivity(@Nullable ArticleItem article, Context context );

  boolean showArticle(@Nullable ArticleItem article, @Nullable ShowArticleOptions showArticleOptions, Context context );

  boolean showArticleFromSeparateList( @NonNull List<ArticleItem> separateArticles,
                                       int currentArticleIndex,
                                       Context context );

  void openScreen( @NonNull ScreenType screenType );
  void openScreen( @NonNull ScreenType screenType, @Nullable Bundle bundle );

  @NonNull
  Observable< Pair< ScreenType, Bundle > > getScreenOpenerObservable();

  String getArticleControllerId();

  @NonNull
  Observable< Boolean > getTopScreenOverlayStateObservable();

  void registerFragment( @NonNull ScreenType screenType, @Nullable Fragment fragment );
}
