package com.paragon_software.toolbar_manager;

import com.paragon_software.dictionary_manager.IDictionaryIcon;

public class DirectionView
{

  public static final int INVALID_DIRECTION = -2;
  private final int            mDirectionFrom;
  private final int            mDirectionTo;
  private final IDictionaryIcon mIcon;

  public DirectionView( int directionFrom, int directionTo, IDictionaryIcon icon )
  {
    mDirectionFrom = directionFrom;
    mDirectionTo = directionTo;
    mIcon = icon;
  }

  public int getId()
  {
    return mDirectionFrom;
  }

  public int getDirectonTo()
  {
    return mDirectionTo;
  }

  public IDictionaryIcon getIcon()
  {
    return mIcon;
  }
}
