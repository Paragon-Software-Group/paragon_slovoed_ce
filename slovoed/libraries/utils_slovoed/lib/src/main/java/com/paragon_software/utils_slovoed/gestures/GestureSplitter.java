package com.paragon_software.utils_slovoed.gestures;

public class GestureSplitter
{
  private static final int THRESHOLD_GESTURE_HANDLING_MILLIS = 100;
  private final int threshold;
  private long lastConsumedGestureMillis = 0;

  public GestureSplitter( int threshold )
  {
    this.threshold = threshold;
  }

  public GestureSplitter()
  {
    threshold = THRESHOLD_GESTURE_HANDLING_MILLIS;
  }

  public boolean isNextGestureHandlingPossible( boolean currentEventConsumed )
  {
    if ( currentEventConsumed )
    {
      lastConsumedGestureMillis = System.currentTimeMillis();
    }
    return !currentEventConsumed
        && System.currentTimeMillis() - lastConsumedGestureMillis > threshold;
  }
}
