package com.paragon_software.settings_manager;

import com.paragon_software.settings_manager.mock.MockBaseChangeListener;

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