package com.paragon_software.history_manager.mock;

import com.paragon_software.history_manager.BaseHistoryController;
import com.paragon_software.history_manager.OnControllerTransitionStateChangedListener;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

public abstract class MockForController_TransitionStateChangedListener extends BaseChangeListener
      implements OnControllerTransitionStateChangedListener
{
  private boolean mExpectedState;
  private BaseHistoryController historyController;

  public MockForController_TransitionStateChangedListener(int maxNumberOfCalls, BaseHistoryController historyController)
  {
    super(maxNumberOfCalls);
    this.historyController = historyController;
  }

  public void setExpectedState(boolean expectedState)
  {
    mExpectedState = expectedState;
  }

  @Override
  public void onControllerTransitionStateChanged( boolean inTransition )
  {
    assertTrue(mNumberOfCalls++ < mMaxNumberOfCalls);
    assertEquals(mExpectedState, historyController.isInTransition());
  }
}