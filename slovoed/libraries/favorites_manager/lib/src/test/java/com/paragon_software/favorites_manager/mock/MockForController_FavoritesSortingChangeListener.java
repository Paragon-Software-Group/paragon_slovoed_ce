package com.paragon_software.favorites_manager.mock;

import com.paragon_software.favorites_manager.BaseFavoritesController;
import com.paragon_software.favorites_manager.FavoritesSorting;
import com.paragon_software.favorites_manager.OnControllerSortingChangeListener;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

public abstract class MockForController_FavoritesSortingChangeListener extends BaseChangeListener
      implements OnControllerSortingChangeListener
{

  private FavoritesSorting mExpectedFavoritesSorting;
  private BaseFavoritesController favoritesController;

  public MockForController_FavoritesSortingChangeListener(int maxNumberOfCalls, BaseFavoritesController favoritesController)
  {
    super(maxNumberOfCalls);
    this.favoritesController = favoritesController;
  }

  public void setExpectedFavoritesSorting(FavoritesSorting expectedFavoritesSorting)
  {
    mExpectedFavoritesSorting = expectedFavoritesSorting;
  }

  @Override
  public void onFavoritesSortingChanged()
  {
    assertTrue(mNumberOfCalls++ < mMaxNumberOfCalls);
    assertEquals(mExpectedFavoritesSorting, favoritesController.getFavoritesSorting());
  }
}