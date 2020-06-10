package com.paragon_software.dictionary_manager.filter.search;

import com.paragon_software.utils_slovoed.collections.CollectionView;

public class EngineListener implements CollectionView.OnItemRangeChanged, CollectionView.OnProgressChanged
{
  private SearchAll searchAll;

  EngineListener( SearchAll searchAll )
  {
    this.searchAll = searchAll;
  }

  @Override
  public void onItemRangeChanged( CollectionView.OPERATION_TYPE operation_type, int i, int i1 )
  {
    searchAll.handleChanges();
  }

  @Override
  public void onProgressChanged()
  {
    searchAll.handleChanges();
  }
}
