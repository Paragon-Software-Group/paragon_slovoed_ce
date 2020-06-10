package com.paragon_software.dictionary_manager.filter.external;

import androidx.annotation.NonNull;

import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryFilter;
import com.paragon_software.article_manager.ArticleItem;

public interface ISearchFilter extends DictionaryFilter
{
  @NonNull
  String getInitialSearchString();

  void setSearchString( @NonNull String search );

  ArticleItem getArticleItem( Dictionary.DictionaryId dictionaryId );
}
