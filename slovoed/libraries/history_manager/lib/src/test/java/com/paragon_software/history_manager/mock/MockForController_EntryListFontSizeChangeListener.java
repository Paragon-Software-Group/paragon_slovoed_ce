package com.paragon_software.history_manager.mock;

import com.paragon_software.history_manager.BaseHistoryController;
import com.paragon_software.history_manager.OnControllerEntryListFontSizeChangeListener;

import static com.paragon_software.history_manager.mock.StaticConsts.DELTA;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

public abstract class MockForController_EntryListFontSizeChangeListener extends BaseChangeListener
      implements OnControllerEntryListFontSizeChangeListener
{
  private float mExpectedEntryListFontSize;
  private BaseHistoryController historyController;

  public MockForController_EntryListFontSizeChangeListener(int maxNumberOfCalls, BaseHistoryController historyController)
  {
    super(maxNumberOfCalls);
    this.historyController = historyController;
  }

  public void setExpectedEntryListFontSize(float expectedEntryListFontSize)
  {
    mExpectedEntryListFontSize = expectedEntryListFontSize;
  }

  @Override
  public void onControllerEntryListFontSizeChanged()
  {
    assertTrue(mNumberOfCalls++ < mMaxNumberOfCalls);
    assertEquals(mExpectedEntryListFontSize, historyController.getEntryListFontSize(), DELTA);
  }
}