package com.paragon_software.mock;

import com.paragon_software.search_manager.OnControllerErrorListener;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

public abstract class MockForController_ErrorListener extends MockBaseChangeListener
          implements OnControllerErrorListener
{
  private Exception expectedException;

  public MockForController_ErrorListener(int maxNumberOfCalls)
  {
    super(maxNumberOfCalls);
  }

  public void setExpectedException(Exception expectedException)
  {
    this.expectedException = expectedException;
  }

  @Override
  public void onControllerError( Exception exception )
  {
    assertTrue(mNumberOfCalls++ < mMaxNumberOfCalls);
    assertEquals(expectedException, exception);
  }
}