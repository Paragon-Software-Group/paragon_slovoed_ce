package com.paragon_software.settings_manager.mock;

public abstract class MockBaseChangeListener
{

  protected final int mMaxNumberOfCalls;

  protected int mNumberOfCalls;

  public int getNumberOfCalls()
  {
    return mNumberOfCalls;
  }

  public MockBaseChangeListener(int maxNumberOfCalls )
  {
    mMaxNumberOfCalls = maxNumberOfCalls;
  }

}