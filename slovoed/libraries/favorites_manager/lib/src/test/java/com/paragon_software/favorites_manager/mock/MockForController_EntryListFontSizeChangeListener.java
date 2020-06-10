package com.paragon_software.favorites_manager.mock;

import com.paragon_software.favorites_manager.BaseFavoritesController;
import com.paragon_software.favorites_manager.OnControllerEntryListFontSizeChangeListener;

import static com.paragon_software.favorites_manager.mock.StaticConsts.DELTA;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

public abstract class MockForController_EntryListFontSizeChangeListener extends BaseChangeListener
      implements OnControllerEntryListFontSizeChangeListener
{

  private float mExpectedEntryListFontSize;
  private BaseFavoritesController favoritesController;

  public MockForController_EntryListFontSizeChangeListener(int maxNumberOfCalls,
                                                           BaseFavoritesController favoritesController)
  {
    super(maxNumberOfCalls);
    this.favoritesController = favoritesController;
  }

  public void setExpectedEntryListFontSize(float expectedEntryListFontSize)
  {
    mExpectedEntryListFontSize = expectedEntryListFontSize;
  }

  @Override
  public void onControllerEntryListFontSizeChanged()
  {
    assertTrue(mNumberOfCalls++ < mMaxNumberOfCalls);
    assertEquals(mExpectedEntryListFontSize, favoritesController.getEntryListFontSize(), DELTA);
  }
}