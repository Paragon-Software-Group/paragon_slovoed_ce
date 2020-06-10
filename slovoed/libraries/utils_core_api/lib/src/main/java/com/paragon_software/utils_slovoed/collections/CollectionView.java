package com.paragon_software.utils_slovoed.collections;

import androidx.annotation.NonNull;

public interface CollectionView< ITEM_TYPE, METADATA_TYPE > {
  enum OPERATION_TYPE {
    ITEM_RANGE_INSERTED,
    ITEM_RANGE_REMOVED,
    ITEM_RANGE_CHANGED
  }

  int getCount();

  ITEM_TYPE getItem( int position );

  METADATA_TYPE getMetadata();

  int getSelection();

  boolean isInProgress();

  int getPosition();

  void updateSelection( int selection );

  void registerListener( Notifier notifier );

  void unregisterListener( Notifier notifier );

  /**
   * <b>IMPORTANT!</b> Update {@link FullNotifier} on create new Notifier type
   */
  interface Notifier { }

  interface OnItemRangeChanged extends Notifier {
    void onItemRangeChanged( OPERATION_TYPE type, int startPosition, int itemCount );
  }

  interface OnProgressChanged extends Notifier {
      void onProgressChanged();
  }

  interface OnScrollToPosition extends Notifier {
    void onScrollToPosition( @NonNull CollectionView collectionView );
  }

  interface OnMetadataChanged extends Notifier {
    void onMetadataChanged();
  }

  interface OnSelectionChange extends Notifier {
    void onSelectionChanged();
  }
}
