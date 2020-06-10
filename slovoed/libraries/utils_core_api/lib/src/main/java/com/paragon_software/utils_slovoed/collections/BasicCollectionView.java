package com.paragon_software.utils_slovoed.collections;

import androidx.annotation.Nullable;

public abstract class BasicCollectionView< ITEM_TYPE, METADATA_TYPE >
        extends AbstractCollectionView< ITEM_TYPE, METADATA_TYPE > {
  private METADATA_TYPE metadata;
  private int selection = -1;
  private boolean inProgress = false;
  private int position = Integer.MIN_VALUE;

  @Override
  public METADATA_TYPE getMetadata() {
    return metadata;
  }

  @Override
  public int getSelection() {
    return selection;
  }

  @Override
  public int getPosition() {
    return position;
  }

  @Override
  public boolean isInProgress() {
    return inProgress;
  }

  public void updateMetadata( @Nullable METADATA_TYPE metadata ) {
    this.metadata = metadata;
    callOnMetadataChanged();
  }

  @Override
  public void updateSelection( int selection) {
    this.selection = selection;
    callOnSelectionChanged();
  }

  public void toggleProgress(boolean inProgress) {
    this.inProgress = inProgress;
    callOnProgressChanged();
  }

  public void updatePosition( int position ) {
    this.position = position;
    callOnPositionChanged();
  }
}
