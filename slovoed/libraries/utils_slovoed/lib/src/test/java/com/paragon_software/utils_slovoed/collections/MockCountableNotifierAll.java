package com.paragon_software.utils_slovoed.collections;

import androidx.annotation.NonNull;

import com.paragon_software.utils_slovoed.collections.util.AnyData;

public abstract class MockCountableNotifierAll implements FullNotifier
{
  private final CollectionView.OPERATION_TYPE type;
  private final Integer start;
  private final Integer count;
  private final CollectionView<AnyData, String > collectionView;

  public int counterOnSelectionChange = 0;
  public int counterOnItemRangeChanged = 0;
  public int counterOnScrollToPosition = 0;
  public int counterOnMetadataChanged = 0;
  public int counterOnProgressChanged = 0;

  public MockCountableNotifierAll()
  {
    this.type = null;
    this.start = this.count = null;
    this.collectionView = null;
  }

  public MockCountableNotifierAll(@NonNull CollectionView.OPERATION_TYPE type, int start, int count )
  {
    this.type = type;
    this.start = start;
    this.count = count;
    this.collectionView = null;
  }

  public MockCountableNotifierAll(CollectionView< AnyData, String > collectionView )
  {
    this.type = null;
    this.start = this.count = null;
    this.collectionView = collectionView;
  }

  @Override
  public void onSelectionChanged()
  {
    counterOnSelectionChange++;
  }

  @Override
  public void onItemRangeChanged( CollectionView.OPERATION_TYPE type, int startPosition, int itemCount )
  {
    if ( null != this.type && null != this.start && null != this.count )
    {
      if ( this.type.equals(type) && this.start.equals(startPosition) && this.count.equals(itemCount) )
      {
        counterOnItemRangeChanged++;
      }
    }
    else
    {
      counterOnItemRangeChanged++;
    }
  }

  @Override
  public void onProgressChanged()
  {
    counterOnProgressChanged++;
  }

  @Override
  public void onScrollToPosition( @NonNull CollectionView collectionView )
  {
    if ( null != this.collectionView )
    {
      if ( this.collectionView.equals(collectionView) )
      {
        counterOnScrollToPosition++;
      }
    }
    else
    {
      counterOnScrollToPosition++;
    }
  }

  @Override
  public void onMetadataChanged()
  {
    counterOnMetadataChanged++;
  }
}