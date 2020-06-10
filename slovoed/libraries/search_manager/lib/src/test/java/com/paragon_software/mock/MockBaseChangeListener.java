package com.paragon_software.mock;

public abstract class MockBaseChangeListener
{

  protected final int mMaxNumberOfCalls;

  protected int mNumberOfCalls;

  public int getNumberOfCalls()
  {
    return mNumberOfCalls;
  }

  public MockBaseChangeListener(int maxNumberOfCalls)
  {
    mMaxNumberOfCalls = maxNumberOfCalls;
  }

}