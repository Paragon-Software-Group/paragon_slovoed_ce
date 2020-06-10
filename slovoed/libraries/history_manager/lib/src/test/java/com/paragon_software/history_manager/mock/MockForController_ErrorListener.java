package com.paragon_software.history_manager.mock;

import com.paragon_software.history_manager.OnControllerErrorListener;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

public abstract class MockForController_ErrorListener extends BaseChangeListener
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