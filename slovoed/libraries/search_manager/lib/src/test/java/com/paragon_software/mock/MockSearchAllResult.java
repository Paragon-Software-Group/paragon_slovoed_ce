package com.paragon_software.mock;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.native_engine.SearchAllResult;
import com.paragon_software.utils_slovoed.collections.CollectionView;

import org.mockito.Mockito;

abstract class MockSearchAllResult implements SearchAllResult
{
  MockCollectionView <ArticleItem,Void> mArticleItemList;
  MockCollectionView <Dictionary.DictionaryId,Void> mDictionaryList;

  public MockSearchAllResult()
  {
    mArticleItemList = Mockito.spy(MockCollectionView.class);
    mDictionaryList = Mockito.spy(MockCollectionView.class);
  }

  @Override
  public CollectionView<ArticleItem, Void> getArticleItemList() {
    return mArticleItemList;
  }

  @Override
  public CollectionView<Dictionary.DictionaryId, Void> getDictionaryIdList() {
    return mDictionaryList;
  }
}