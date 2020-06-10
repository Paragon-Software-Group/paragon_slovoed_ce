package com.paragon_software.favorites_manager.mock;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.favorites_manager.OnControllerFavoritesListChangedListener;

import java.util.List;

import static com.paragon_software.favorites_manager.mock.StaticConsts.ARTICLE1;
import static org.junit.Assert.assertTrue;

public abstract class MockForController_FavoritesListChangedListener extends BaseChangeListener
        implements OnControllerFavoritesListChangedListener
{

  public MockForController_FavoritesListChangedListener(int maxNumberOfCalls) {
    super(maxNumberOfCalls);
  }

  @Override
  public void onControllerFavoritesListChanged( List<ArticleItem> favoriteWords )
  {
    assertTrue(mNumberOfCalls++ < mMaxNumberOfCalls);
    assertTrue(favoriteWords.size() == 1);
    assertTrue(favoriteWords.get(0).equals(ARTICLE1));
  }
}