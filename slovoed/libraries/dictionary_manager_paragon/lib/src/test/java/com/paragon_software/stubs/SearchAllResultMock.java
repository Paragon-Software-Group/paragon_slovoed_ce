package com.paragon_software.stubs;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.native_engine.SearchAllResult;
import com.paragon_software.utils_slovoed.collections.CollectionView;

import org.mockito.Mockito;

import static org.mockito.Mockito.CALLS_REAL_METHODS;
import static org.mockito.Mockito.withSettings;

public abstract class SearchAllResultMock implements SearchAllResult
{
  private String text;

  public SearchAllResultMock( String text )
  {
    this.text = text;
  }

  @Override
  public CollectionView<ArticleItem, Void > getArticleItemList()
  {
    return Mockito.mock(ArticleItemsCollectionAsync.class, withSettings().useConstructor(text)
            .defaultAnswer(CALLS_REAL_METHODS));
  }

  @Override
  public CollectionView< Dictionary.DictionaryId, Void > getDictionaryIdList()
  {
    return null;
  }
}