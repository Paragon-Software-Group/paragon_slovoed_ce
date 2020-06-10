package com.paragon_software.mock;

import android.content.Context;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.article_manager.ShowArticleOptions;
import com.paragon_software.screen_opener_api.ScreenOpenerAPI;

import java.util.List;

public abstract class MockScreenOpener implements ScreenOpenerAPI
{
  public ArticleItem mArticleItem = null;

  @Override
  public boolean showArticleActivity(@Nullable ArticleItem articleItem, Context context )
  {
    mArticleItem = articleItem;
    return false;
  }

  @Override
  public boolean showArticle(@Nullable ArticleItem article, @Nullable ShowArticleOptions showArticleOptions, Context context)
  {
    mArticleItem = article;
    return false;
  }

  @Override
  public boolean showArticleFromSeparateList(@NonNull List<ArticleItem> separateArticles, int currentArticleIndex, Context context)
  {
    mArticleItem = separateArticles.get(currentArticleIndex);
    return false;
  }
}
