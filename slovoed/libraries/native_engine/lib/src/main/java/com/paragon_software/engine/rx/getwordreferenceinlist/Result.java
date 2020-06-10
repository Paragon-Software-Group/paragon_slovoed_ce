package com.paragon_software.engine.rx.getwordreferenceinlist;

import androidx.annotation.NonNull;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.utils_slovoed.collections.CollectionView;
import com.paragon_software.utils_slovoed.text.SubstringInfo;

import java.util.List;

public class Result
{

  @NonNull
  private List< CollectionView< ArticleItem, SubstringInfo > > mCollection;

  public Result( @NonNull List< CollectionView< ArticleItem, SubstringInfo > > mCollection )
  {
    this.mCollection = mCollection;
  }

  public List< CollectionView< ArticleItem, SubstringInfo > > get()
  {
    return mCollection;
  }
}
