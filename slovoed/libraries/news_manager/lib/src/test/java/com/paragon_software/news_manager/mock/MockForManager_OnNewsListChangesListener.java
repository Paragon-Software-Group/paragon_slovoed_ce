package com.paragon_software.news_manager.mock;

import com.paragon_software.news_manager.OnNewsListChangesListener;

import static org.junit.Assert.assertTrue;

public abstract class MockForManager_OnNewsListChangesListener extends MockBaseChangeListener
        implements OnNewsListChangesListener
{
  public MockForManager_OnNewsListChangesListener(int maxNumberOfCalls)
  {
    super(maxNumberOfCalls);
  }

  @Override
  public void onNewsListChanged()
  {
    assertTrue(mNumberOfCalls++ < mMaxNumberOfCalls);
  }
}