package com.paragon_software.favorites_manager.mock;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.favorites_manager.OnControllerSelectionModeChangedListener;

import java.util.List;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

public abstract class MockForController_SelectionModeChangedListener
        implements OnControllerSelectionModeChangedListener
{

  private int mNumberOfListChangeCalls;

  private final int mMaxNumberOfListChangeCalls;

  private int mExpectedListSize;

  private int mNumberOfModeCalls;

  private final int mMaxNumberOfModeCalls;

  public MockForController_SelectionModeChangedListener(int maxNumberOfModeCalls, int maxNumberOfListChangeCalls) {
    mMaxNumberOfModeCalls = maxNumberOfModeCalls;
    mMaxNumberOfListChangeCalls = maxNumberOfListChangeCalls;
  }

  public void setExpectedListSize( int expectedListSize ) {
    mExpectedListSize = expectedListSize;
  }

  @Override
  public void onSelectedListChanged( List<ArticleItem> selectedWords )
  {
    assertTrue(mNumberOfListChangeCalls++ < mMaxNumberOfListChangeCalls);
    assertEquals(mExpectedListSize, selectedWords.size());
  }

  @Override
  public void onSelectionModeChanged( boolean selectionMode )
  {
    assertTrue(mNumberOfModeCalls++ < mMaxNumberOfModeCalls);
  }
}