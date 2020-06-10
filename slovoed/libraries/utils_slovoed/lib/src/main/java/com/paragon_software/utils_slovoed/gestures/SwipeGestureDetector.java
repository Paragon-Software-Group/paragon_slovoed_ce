package com.paragon_software.utils_slovoed.gestures;

import android.content.Context;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import android.view.GestureDetector;
import android.view.MotionEvent;

public class SwipeGestureDetector extends GestureDetector
{
  public SwipeGestureDetector( @Nullable Context context, @NonNull SwipeListener listener )
  {
    super(context, new SwipeGestureListener(listener));
  }

  private static class SwipeGestureListener extends GestureDetector.SimpleOnGestureListener
  {
    private static final int SWIPE_DISTANCE_THRESHOLD = 200;
    private static final int SWIPE_VELOCITY_THRESHOLD = 200;
    @NonNull
    private final SwipeListener swipeListener;

    SwipeGestureListener( @NonNull SwipeListener swipeListener )
    {
      this.swipeListener = swipeListener;
    }

    @Override
    public boolean onFling( MotionEvent e1, MotionEvent e2, float velocityX, float velocityY )
    {
      boolean consumed = false;
      if (e1 != null && e2 != null) {
        float distanceX = e2.getX() - e1.getX();
        if (isHorizontalMotion(e1, e2) && Math.abs(distanceX) > SWIPE_DISTANCE_THRESHOLD
                && Math.abs(velocityX) > SWIPE_VELOCITY_THRESHOLD) {
          swipeListener.onSwipe(distanceX > 0);
          consumed = true;
        }
      }
      return consumed;
    }

    private boolean isHorizontalMotion( MotionEvent e1, MotionEvent e2 )
    {
      return Math.abs(e2.getX() - e1.getX()) > Math.abs(e2.getY() - e1.getY());
    }
  }
}
