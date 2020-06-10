package com.paragon_software.favorites_manager.mock;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.favorites_manager.OnFavoritesListChangesListener;

import java.util.Collection;
import java.util.List;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;

public abstract class MockForManager_FavoritesListChangesListener extends BaseChangeListener implements OnFavoritesListChangesListener
{

  private Collection<ArticleItem> expectedWords;

  public MockForManager_FavoritesListChangesListener(int maxNumberOfCalls)
  {
    super(maxNumberOfCalls);
  }

  public void setExpectedWords(List< ArticleItem > expectedWords)
  {
    this.expectedWords = expectedWords;
  }

  @Override
  public void onFavoritesListChanged( List< ArticleItem > favoritesWords )
  {
    assertTrue(mNumberOfCalls++ < mMaxNumberOfCalls);
    if (expectedWords != null) {
      assertNotNull(favoritesWords);
      assertEquals(expectedWords.size(), favoritesWords.size());
      for (int i = 0; i < expectedWords.size(); i++) {
        assertEquals(expectedWords.toArray()[i], favoritesWords.toArray()[i]);
      }
    }
  }
}