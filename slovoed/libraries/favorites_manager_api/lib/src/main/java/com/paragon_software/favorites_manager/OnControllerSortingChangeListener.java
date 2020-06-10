package com.paragon_software.favorites_manager;

/**
 * Interface for listeners that observes notifications about favorites sorting type changes.
 */
public interface OnControllerSortingChangeListener extends FavoritesControllerAPI.Notifier
{
  /**
   * Notify that favorites sorting type has been changed.
   */
  void onFavoritesSortingChanged();
}
