package com.paragon_software.flash_cards_manager;

import com.paragon_software.utils_slovoed.collections.BasicCollectionView;

public class EmptyQuizCollectionView extends BasicCollectionView< QuizItemView, BaseQuizAndFlashcardMetadata >
    implements AbstractQuizCollectionView
{
  @Override
  public void showMeaning()
  {

  }

  @Override
  public void answer( boolean right )
  {

  }

  @Override
  public void restart()
  {

  }

  @Override
  public void playSound()
  {

  }

  @Override
  public void remove()
  {

  }

  @Override
  public void setInQuizUI( boolean value )
  {

  }

  @Override
  public int getCount()
  {
    return 0;
  }

  @Override
  public QuizItemView getItem( int position )
  {
    return null;
  }

}
