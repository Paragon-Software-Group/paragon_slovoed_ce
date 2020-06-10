package com.paragon_software.history_manager.mock;

import com.paragon_software.history_manager.BaseHistoryManager;
import com.paragon_software.history_manager.OnStateChangedListener;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

public abstract class MockForManager_StateChangedListener extends BaseChangeListener
        implements OnStateChangedListener
{

  private BaseHistoryManager historyManager;
  private boolean inTransition = false;

  public MockForManager_StateChangedListener(int maxNumberOfCalls, BaseHistoryManager historyManager)
  {
    super(maxNumberOfCalls);
    this.historyManager = historyManager;
  }

  @Override
  public void onHistoryStateChanged()
  {
    assertTrue(mNumberOfCalls < mMaxNumberOfCalls);
    if (inTransition)
    {
      mNumberOfCalls++;
    }
    assertEquals(inTransition, !historyManager.isInTransition());
    inTransition = historyManager.isInTransition();
  }
}