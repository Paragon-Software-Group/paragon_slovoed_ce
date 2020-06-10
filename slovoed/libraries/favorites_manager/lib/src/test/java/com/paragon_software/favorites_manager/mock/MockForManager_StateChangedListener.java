package com.paragon_software.favorites_manager.mock;

import com.paragon_software.favorites_manager.BaseFavoritesManager;
import com.paragon_software.favorites_manager.OnStateChangedListener;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

public abstract class MockForManager_StateChangedListener extends BaseChangeListener implements OnStateChangedListener
{

  private boolean inTransition = false;
  private BaseFavoritesManager favoritesManager;

  public MockForManager_StateChangedListener(int maxNumberOfCalls, BaseFavoritesManager favoritesManager)
  {
    super(maxNumberOfCalls);
    this.favoritesManager = favoritesManager;
  }

  @Override
  public void onFavoritesStateChanged()
  {
    assertTrue(mNumberOfCalls < mMaxNumberOfCalls);
    if (inTransition)
    {
      mNumberOfCalls++;
    }
    assertEquals(inTransition, !favoritesManager.isInTransition());
    inTransition = favoritesManager.isInTransition();
  }
}