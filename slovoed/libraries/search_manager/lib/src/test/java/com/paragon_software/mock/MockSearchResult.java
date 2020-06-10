
package com.paragon_software.mock;

import androidx.annotation.NonNull;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.native_engine.ScrollResult;
import com.paragon_software.utils_slovoed.collections.CollectionView;

import org.mockito.Mockito;

abstract class MockSearchResult implements ScrollResult
{
  MockCollectionView <ArticleItem, Dictionary.Direction> mArticleItemList;
  MockCollectionView <ArticleItem,Void> mMorphoList;

  public MockSearchResult()
  {
    mArticleItemList = Mockito.spy(MockCollectionView.class);
    mMorphoList = Mockito.spy(MockCollectionView.class);
  }

  @Override
  public CollectionView<ArticleItem, Dictionary.Direction> getArticleItemList() {
    return mArticleItemList;
  }

  @Override
  public CollectionView<ArticleItem, Void> getMorphoArticleItemList() {
    return mMorphoList;
  }

  @Override
  public boolean startsWith( @NonNull String text )
  {
    return false;
  }
}