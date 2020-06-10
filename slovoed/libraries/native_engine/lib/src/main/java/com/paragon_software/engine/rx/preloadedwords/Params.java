package com.paragon_software.engine.rx.preloadedwords;

import androidx.annotation.NonNull;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.utils_slovoed.directory.Directory;
import io.reactivex.subjects.SingleSubject;

class Params
{
  @NonNull
  final Dictionary.DictionaryId dictionaryId;
  @NonNull
  SingleSubject< Directory< ArticleItem > > callbackSubject;

  Params( @NonNull Dictionary.DictionaryId dictionaryId,
          @NonNull SingleSubject< Directory< ArticleItem > > callbackSubject )
  {
    this.dictionaryId = dictionaryId;
    this.callbackSubject = callbackSubject;
  }
}
