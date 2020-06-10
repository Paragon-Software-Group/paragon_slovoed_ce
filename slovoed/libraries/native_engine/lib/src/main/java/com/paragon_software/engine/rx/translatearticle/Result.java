package com.paragon_software.engine.rx.translatearticle;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.engine.nativewrapper.ArticleSearcher;
import com.paragon_software.native_engine.EngineArticleAPI;

class Result
{
  @NonNull
  final EngineArticleAPI.OnTranslationReadyCallback callback;

  @Nullable
  final ArticleItem articleItem;

  @NonNull
  final String translation;

  boolean hasHideOrSwitchBlocks;

  @Nullable
  ArticleSearcher articleSearcher;

  Result( @NonNull EngineArticleAPI.OnTranslationReadyCallback _callback, @Nullable ArticleItem _articleItem,
          @NonNull String _translation, boolean _hasHideOrSwitchBlocks, @Nullable ArticleSearcher _articleSearcher )
  {
    callback = _callback;
    articleItem = _articleItem;
    translation = _translation;
    hasHideOrSwitchBlocks = _hasHideOrSwitchBlocks;
    articleSearcher = _articleSearcher;
  }
}
