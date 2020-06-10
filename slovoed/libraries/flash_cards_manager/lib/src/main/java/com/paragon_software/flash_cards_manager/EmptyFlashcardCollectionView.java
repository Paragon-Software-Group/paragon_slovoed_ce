package com.paragon_software.flash_cards_manager;

import androidx.annotation.NonNull;

import com.paragon_software.utils_slovoed.collections.BasicCollectionView;

public class EmptyFlashcardCollectionView extends BasicCollectionView< FlashcardView, BaseQuizAndFlashcardMetadata >
    implements AbstractFlashcardCollectionView
{
  @Override
  public void checkCard( int position, boolean checked )
  {
  }

  @Override
  public void checkAll( boolean checked )
  {
  }

  @Override
  public void remove( int position )
  {
  }

  @Override
  public void setSortMode( @NonNull FlashcardMetadata.SortMode sortMode )
  {
  }

  @Override
  public void activateRegularMode()
  {
  }

  @Override
  public void activateSelectionMode( int position )
  {
  }

  @Override
  public int getCheckedCount()
  {
    return 0;
  }

  @Override
  public int getCount()
  {
    return 0;
  }

  @Override
  public FlashcardView getItem( int position )
  {
    return null;
  }
}
