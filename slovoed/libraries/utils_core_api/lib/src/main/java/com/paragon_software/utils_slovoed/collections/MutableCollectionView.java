package com.paragon_software.utils_slovoed.collections;

import androidx.annotation.Nullable;

public interface MutableCollectionView< ITEM, METADATA_TYPE > extends CollectionView< ITEM, METADATA_TYPE >
{
  void update( int position, ITEM item );

  void remove( int position );

  void updateMetadata( @Nullable METADATA_TYPE metadata );
}
