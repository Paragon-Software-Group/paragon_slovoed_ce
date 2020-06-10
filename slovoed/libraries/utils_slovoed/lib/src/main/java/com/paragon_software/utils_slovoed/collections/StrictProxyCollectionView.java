package com.paragon_software.utils_slovoed.collections;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.util.Objects;

public class StrictProxyCollectionView<ITEM,
                                       METADATA,
                                       CORE_COLLECTION extends CollectionView<ITEM,
                                                                              METADATA>>
        extends AbstractWrapperCollectionView<ITEM, ITEM, METADATA, CORE_COLLECTION> {
  private final boolean saveSelectionOnInnerCollectionChanges;

  @Nullable
  private Integer countCap = null;

  public StrictProxyCollectionView(@NonNull CORE_COLLECTION _inner) {
    this(_inner, true);
  }

  public StrictProxyCollectionView(@NonNull CORE_COLLECTION _inner,
                                   boolean _saveSelectionOnInnerCollectionChanges) {
    super(_inner);
    saveSelectionOnInnerCollectionChanges = _saveSelectionOnInnerCollectionChanges;
  }

  public final void set(@NonNull CORE_COLLECTION newCore) {
    CORE_COLLECTION oldCore = getCore();
    int oldSelection = getSelection();
    if(oldCore != newCore) {
      if(saveSelectionOnInnerCollectionChanges && (getCore().getSelection() >= 0))
        newCore.updateSelection(getCore().getSelection());

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
}
