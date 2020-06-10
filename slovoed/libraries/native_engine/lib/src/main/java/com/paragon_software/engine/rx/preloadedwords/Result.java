package com.paragon_software.engine.rx.preloadedwords;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.utils_slovoed.directory.Directory;
import io.reactivex.subjects.SingleSubject;

class Result
{
  @Nullable
  Directory< ArticleItem >                  preloadedWords;
  @NonNull
  SingleSubject< Directory< ArticleItem > > callbackSubject;

  Result( @Nullable Directory< ArticleItem > preloadedWords,
          @NonNull SingleSubject< Directory< ArticleItem > > callbackSubject )
  {
    this.preloadedWords = preloadedWords;
    this.callbackSubject = callbackSubject;
  }
}
