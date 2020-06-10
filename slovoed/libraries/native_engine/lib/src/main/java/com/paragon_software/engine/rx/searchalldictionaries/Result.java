package com.paragon_software.engine.rx.searchalldictionaries;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.util.Collections;
import java.util.List;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.Dictionary;

final class Result
{
  @NonNull
  static final Result TERMINATOR = new Result();

  @Nullable
  final Dictionary.DictionaryId dictionaryId;

  @NonNull
  final List< ArticleItem > items;

  Result( @NonNull Dictionary.DictionaryId _dictionaryId, @NonNull List< ArticleItem > _items )
  {
    dictionaryId = _dictionaryId;
    items = _items;
  }

  private Result()
  {
    dictionaryId = null;
    items = Collections.emptyList();
  }

  boolean isTerminator()
  {
    return dictionaryId == null;
  }
}
