package com.paragon_software.engine.rx.translatearticle;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.native_engine.EngineArticleAPI;
import com.paragon_software.native_engine.HtmlBuilderParams;

class Params
{
  @NonNull
  final EngineArticleAPI.OnTranslationReadyCallback callback;

  @Nullable
  final ArticleItem articleItem;

  @Nullable
  final Dictionary dictionary;

  @NonNull
  final HtmlBuilderParams htmlParams;

  Params(@NonNull EngineArticleAPI.OnTranslationReadyCallback _callback, @Nullable ArticleItem _articleItem,
         @Nullable  Dictionary _dictionary, @NonNull HtmlBuilderParams _htmlParams)
  {
    callback = _callback;
    articleItem = _articleItem;
    dictionary = _dictionary;
    htmlParams = _htmlParams;
  }
}
