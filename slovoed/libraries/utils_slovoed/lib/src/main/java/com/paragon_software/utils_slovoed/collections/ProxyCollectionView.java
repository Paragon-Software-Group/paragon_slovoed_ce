package com.paragon_software.utils_slovoed.collections;

import androidx.annotation.Nullable;

import java.util.Objects;

public class ProxyCollectionView<ITEM, METADATA>
        extends AbstractWrapperCollectionView<ITEM, ITEM, METADATA, CollectionView<ITEM, METADATA>> {
  private final boolean saveSelectionOnInnerCollectionChanges;

  @Nullable
  private final METADATA defaultMetadata;

  private final boolean defaultInProgress;

  @Nullable
  private Integer countCap = null;

  public ProxyCollectionView() {
    this(null,
         true,
         null,
         false);
  }

  public ProxyCollectionView( @Nullable CollectionView<ITEM, METADATA> _inner,
                              boolean _saveSelectionOnInnerCollectionChanges,
                              @Nullable METADATA _defaultMetadata,
                              boolean _defaultInProgress) {
    super(nonNull(_inner, _defaultMetadata, _defaultInProgress, null));
    saveSelectionOnInnerCollectionChanges = _saveSelectionOnInnerCollectionChanges;
    defaultMetadata = _defaultMetadata;
    defaultInProgress = _defaultInProgress;
  }

  public final void set(@Nullable CollectionView<ITEM, METADATA> newCore) {
    CollectionView<ITEM, METADATA> oldCore = getCore();
    int oldSelection = getSelection();
    if((oldCore != newCore) && ((!(oldCore instanceof NullCollectionView)) || (newCore != null))) {
      newCore = nonNull(newCore,
                        defaultMetadata,
                        defaultInProgress,
                        (saveSelectionOnInnerCollectionChanges && (getCore().getSelection() >= 0)) ?
                                getCore().getSelection() : null);
      int oldCount = oldCore.getCount();
      int newCount = newCore.getCount();
      int minCount = Math.min(oldCount, newCount);
      oldCore.unregisterListener(this);
      countCap = minCount;
      if(oldCount > newCount)
        callOnItemRangeChanged(OPERATION_TYPE.ITEM_RANGE_REMOVED,
                               newCount,
                               oldCount - newCount);
      setCore(newCore);
      if(minCount > 0)
        callOnItemRangeChanged(OPERATION_TYPE.ITEM_RANGE_CHANGED,
                               0,
                               minCount);
      countCap = null;
      if(oldCount < newCount)
        callOnItemRangeChanged(OPERATION_TYPE.ITEM_RANGE_INSERTED,
                               oldCount,
                               newCount - oldCount);
      if(!Objects.equals(oldCore.getMetadata(), newCore.getMetadata()))
        callOnMetadataChanged();
      if(oldSelection != newCore.getSelection())
        callOnSelectionChanged();
      if(oldCore.isInProgress() != newCore.isInProgress())
        callOnProgressChanged();
      newCore.registerListener(this);
    }
  }

  @Override
  public int getCount() {
    int res = super.getCount();
    if((countCap != null) && (countCap < res))
      res = countCap;
    return res;
  }

  @Override
  public ITEM getItem(int position) {
    return getCore().getItem(position);
  }

  @Override
  public int getSelection() {
    int res = super.getSelection();
    if(res >= getCount())
      res= -1;
    return res;
  }

  private static <ITEM, METADATA>
    CollectionView<ITEM, METADATA> nonNull(@Nullable CollectionView<ITEM, METADATA> collectionView,
                                           @Nullable METADATA defaultMetadata,
                                           boolean defaultInProgress,
                                           @Nullable Integer defaultSelection) {
    if(collectionView == null) {
      BasicCollectionView<ITEM, METADATA> res = new NullCollectionView<>();
      res.updateMetadata(defaultMetadata);
      res.toggleProgress(defaultInProgress);
      collectionView = res;
    }
    if(defaultSelection != null)
      collectionView.updateSelection(defaultSelection);
    return collectionView;
  }

  private static class NullCollectionView<ITEM, METADATA>
          extends BasicCollectionView<ITEM, METADATA> {
    @Override
    public int getCount() {
      return 0;
    }

    @Override
    public ITEM getItem(int position) {
      return null;
    }

    @Override
    protected void callOnSelectionChanged() { }
  }
}
