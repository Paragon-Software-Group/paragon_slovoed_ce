package com.paragon_software.history_manager.mock;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.history_manager.OnHistoryListChangedListener;

import java.util.List;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;

public abstract class MockForManager_HistoryListChangesListener extends BaseChangeListener
        implements OnHistoryListChangedListener
{

  private List<ArticleItem> expectedWords;

  public MockForManager_HistoryListChangesListener(int maxNumberOfCalls)
  {
    super(maxNumberOfCalls);
  }

  public void setExpectedWords(List< ArticleItem > expectedWords)
  {
    this.expectedWords = expectedWords;
  }

  @Override
  public void onHistoryListChanged( List< ArticleItem > historyWords )
  {
    assertTrue(mNumberOfCalls++ < mMaxNumberOfCalls);
    if (expectedWords != null) {
      assertNotNull(historyWords);
      assertEquals(expectedWords.size(), historyWords.size());
      for (int i = 0; i < expectedWords.size(); i++) {
        assertEquals(expectedWords.get(i), historyWords.get(i));
      }
    }
  }
}