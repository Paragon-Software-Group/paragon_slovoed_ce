package com.paragon_software.history_manager.mock;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.history_manager.OnControllerHistoryListChangedListener;

import java.util.List;

import static com.paragon_software.history_manager.mock.StaticConsts.ARTICLE1;
import static org.junit.Assert.assertTrue;

public abstract class MockForController_HistoryListChangedListener extends BaseChangeListener
        implements OnControllerHistoryListChangedListener
{
  public MockForController_HistoryListChangedListener(int maxNumberOfCalls) {
    super(maxNumberOfCalls);
  }

  @Override
  public void onControllerHistoryListChanged( List<ArticleItem> historyWords )
  {
    assertTrue(mNumberOfCalls++ < mMaxNumberOfCalls);
    assertTrue(historyWords.size() == 1);
    assertTrue(historyWords.get(0).equals(ARTICLE1));
  }
}