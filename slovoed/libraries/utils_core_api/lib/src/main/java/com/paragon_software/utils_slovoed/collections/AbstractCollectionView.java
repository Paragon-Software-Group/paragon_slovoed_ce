package com.paragon_software.utils_slovoed.collections;

import androidx.annotation.NonNull;

import java.util.Collections;
import java.util.HashSet;
import java.util.Set;
import java.util.WeakHashMap;

abstract class AbstractCollectionView<ITEM, METADATA> implements CollectionView<ITEM, METADATA> {
  @NonNull
  private final Set<Notifier> notifiers =
          Collections.newSetFromMap(new WeakHashMap<Notifier, Boolean>());

  @Override
  public void registerListener( Notifier notifier ) {
    if ( notifier != null )
      notifiers.add(notifier);
  }

  @Override
  public void unregisterListener( Notifier notifier ) {
    notifiers.remove(notifier);
  }

  protected void callOnItemRangeChanged(@NonNull OPERATION_TYPE operationType,
                                        int startPosition,
                                        int itemCount ) {
    for(CollectionView.Notifier notifier : new HashSet<>(notifiers))
      if(notifier instanceof CollectionView.OnItemRangeChanged)
        ((CollectionView.OnItemRangeChanged) notifier).onItemRangeChanged(operationType,
                                                                          startPosition,
                                                                          itemCount);
  }

  protected void callOnMetadataChanged() {
    for(CollectionView.Notifier notifier : new HashSet<>(notifiers))
      if(notifier instanceof CollectionView.OnMetadataChanged)
        ((CollectionView.OnMetadataChanged) notifier).onMetadataChanged();
  }

  protected void callOnSelectionChanged() {
    for(CollectionView.Notifier notifier : new HashSet<>(notifiers))
      if(notifier instanceof CollectionView.OnSelectionChange)
        ((CollectionView.OnSelectionChange) notifier).onSelectionChanged();
  }

  protected void callOnProgressChanged() {
    for(CollectionView.Notifier notifier : new HashSet<>(notifiers))
      if(notifier instanceof CollectionView.OnProgressChanged)
        ((CollectionView.OnProgressChanged) notifier).onProgressChanged();
  }

  protected void callOnPositionChanged() {
    for(CollectionView.Notifier notifier : new HashSet<>(notifiers))
      if(notifier instanceof CollectionView.OnScrollToPosition)
        ((CollectionView.OnScrollToPosition) notifier).onScrollToPosition(this);
  }
}
