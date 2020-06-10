package com.paragon_software.utils_slovoed.collections;

import com.paragon_software.utils_slovoed.collections.util.AnyData;

public abstract class MockCollectionView extends BasicCollectionView<AnyData, String >
{
  static final int     POSITION    = 6;
  static final int     SELECTION   = 7;
  static final         int     COUNT       = 43;
  static final         AnyData ITEM        = new AnyData();
  static final String  METADATA    = "METADATA_TEST";
  static final boolean IN_PROGRESS = true;

  public MockCollectionView() 
  {
    updateMetadata(METADATA);
    updatePosition(POSITION);
    updateSelection(SELECTION);
  }

  @Override
  public int getCount()
  {
    return COUNT;
  }

  @Override
  public AnyData getItem( int position )
  {
    return ITEM;
  }

  @Override
  public boolean isInProgress()
  {
    return IN_PROGRESS;
  }
}