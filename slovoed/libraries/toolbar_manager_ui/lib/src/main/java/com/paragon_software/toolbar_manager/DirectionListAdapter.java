package com.paragon_software.toolbar_manager;

import android.graphics.Bitmap;
import android.os.Bundle;
import androidx.annotation.NonNull;
import android.widget.ImageView;

import java.util.ArrayList;

public class DirectionListAdapter
{
  public static final  int    UNDEFINED_DIRECTION_INDEX     = -2;
  private static final Bitmap STUB_TRANSPARENT_BITMAP       = Bitmap.createBitmap(10, 10, Bitmap.Config.ARGB_8888);
  private static final String STATE_CURRENT_DIRECTION_INDEX =
      DirectionListAdapter.class.getName() + ".STATE_CURRENT_DIRECTION_INDEX";

  private ImageView                mDirectionImage;
  private ArrayList<DirectionView> mDirections;
  private int mCurrentDirection = UNDEFINED_DIRECTION_INDEX;

  public void setImageView( ImageView view )
  {
    mDirectionImage = view;
  }

  public void setDirections(ArrayList<DirectionView> directions)
  {
    mDirections = directions;
    if (null == mDirections || 2 > mDirections.size())
    {
      changeImageBitmap(STUB_TRANSPARENT_BITMAP);
      mCurrentDirection = UNDEFINED_DIRECTION_INDEX;
    }
    else
    {
      mCurrentDirection = mCurrentDirection == UNDEFINED_DIRECTION_INDEX ? 0 : mCurrentDirection;
      changeImageBitmap(mDirections.get(mCurrentDirection).getIcon().getBitmap());
    }
  }

  public void setDirection(int element)
  {
    if (null == mDirections || 2 > mDirections.size() || element < 0 || element >= mDirections.size())
    {
      changeImageBitmap(STUB_TRANSPARENT_BITMAP);
      mCurrentDirection = UNDEFINED_DIRECTION_INDEX;
    }
    else
    {
      changeImageBitmap(mDirections.get(element).getIcon().getBitmap());
      mCurrentDirection = element;
    }
  }

  public DirectionView changeDirection()
  {
    if ( null == mDirections || 2 > mDirections.size() )
    {
      changeImageBitmap(STUB_TRANSPARENT_BITMAP);
      return null;
    }
    else
    {
      if (mCurrentDirection >= (mDirections.size() - 1) || mCurrentDirection < 0)
      {
        mCurrentDirection = 0;
      }
      else
      {
        ++mCurrentDirection;
      }
      final DirectionView view = mDirections.get(mCurrentDirection);
      changeImageBitmap(view.getIcon().getBitmap());
      return view;
    }
  }

  private void changeImageBitmap( Bitmap bitmap )
  {
    mDirectionImage.setImageBitmap(bitmap);
    mDirectionImage.setEnabled(bitmap != STUB_TRANSPARENT_BITMAP);
  }

  public void onRestoreInstanceState( @NonNull Bundle bundle )
  {
    mCurrentDirection = bundle.getInt(STATE_CURRENT_DIRECTION_INDEX, mCurrentDirection);
  }

  public Bundle onSaveInstanceState()
  {
    Bundle bundle = new Bundle();
    bundle.putInt(STATE_CURRENT_DIRECTION_INDEX, mCurrentDirection);
    return bundle;
  }
}
