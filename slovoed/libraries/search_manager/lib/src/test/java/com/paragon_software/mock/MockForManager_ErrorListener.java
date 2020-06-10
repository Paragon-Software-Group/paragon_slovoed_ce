package com.paragon_software.mock;

import com.paragon_software.search_manager.OnErrorListener;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

public abstract class MockForManager_ErrorListener extends MockBaseChangeListener
        implements OnErrorListener
{
  private Exception expectedException;

  public MockForManager_ErrorListener(int maxNumberOfCalls)
  {
    super(maxNumberOfCalls);
  }

  public void setExpectedException(Exception expectedException)
  {
    this.expectedException = expectedException;
  }

  @Override
  public void onSearchManagerError( Exception exception )
  {
    assertTrue(mNumberOfCalls++ < mMaxNumberOfCalls);
    assertEquals(expectedException, exception);
  }
}