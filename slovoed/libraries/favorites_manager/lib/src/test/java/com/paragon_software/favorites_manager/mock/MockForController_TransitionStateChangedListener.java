package com.paragon_software.favorites_manager.mock;

import com.paragon_software.favorites_manager.BaseFavoritesController;
import com.paragon_software.favorites_manager.OnControllerTransitionStateChangedListener;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

public abstract class MockForController_TransitionStateChangedListener extends BaseChangeListener
      implements OnControllerTransitionStateChangedListener
{

  private boolean mExpectedState;
  private BaseFavoritesController favoritesController;

  public MockForController_TransitionStateChangedListener(int maxNumberOfCalls, BaseFavoritesController favoritesController)
  {
    super(maxNumberOfCalls);
    this.favoritesController = favoritesController;
  }

  public void setExpectedState(boolean expectedState)
  {
    mExpectedState = expectedState;
  }

  @Override
  public void onControllerTransitionStateChanged( boolean inTransition )
  {
    assertTrue(mNumberOfCalls++ < mMaxNumberOfCalls);
    assertEquals(mExpectedState, favoritesController.isInTransition());
  }
}