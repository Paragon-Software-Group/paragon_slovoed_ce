package com.paragon_software.mock;

import com.paragon_software.search_manager.OnControllerFontSizeChangeListener;
import com.paragon_software.search_manager.ParagonSearchController;

import static com.paragon_software.mock.StaticConsts.DELTA;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

public abstract class MockForController_FontSizeChangeListener extends MockBaseChangeListener
      implements OnControllerFontSizeChangeListener
{
  private float mExpectedEntryListFontSize;
  private ParagonSearchController mSearchController;

  public MockForController_FontSizeChangeListener(ParagonSearchController searchController, int maxNumberOfCalls )
  {
    super(maxNumberOfCalls);
    mSearchController = searchController;
  }

  public void setExpectedEntryListFontSize(float expectedEntryListFontSize)
  {
    mExpectedEntryListFontSize = expectedEntryListFontSize;
  }

  @Override
  public void onControllerEntryListFontSizeChanged()
  {
    assertTrue(mNumberOfCalls++ < mMaxNumberOfCalls);
    assertEquals(mExpectedEntryListFontSize, mSearchController.getEntryListFontSize(), DELTA);
  }
}