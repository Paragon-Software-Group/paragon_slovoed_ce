package com.paragon_software.engine.rx.getwordreferenceinlist;

import androidx.annotation.NonNull;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.utils_slovoed.collections.CachedCollectionView;
import com.paragon_software.utils_slovoed.collections.CollectionView;
import com.paragon_software.utils_slovoed.text.SubstringInfo;

public class ResultContainer
{
  private CachedCollectionView<CollectionView<ArticleItem, SubstringInfo>, Void> mCollection =
      new CachedCollectionView<>();

  void beforeSearch()
  {
    mCollection.resetCache();
    mCollection.updateSelection(-1);
    mCollection.toggleProgress(true);
  }

  @NonNull
  public CollectionView< CollectionView< ArticleItem, SubstringInfo >, Void > get()
  {
    return mCollection;
  }

  void onResult(Result result)
  {
    mCollection.cacheItems(result.get());
    mCollection.toggleProgress(false);
  }
}
