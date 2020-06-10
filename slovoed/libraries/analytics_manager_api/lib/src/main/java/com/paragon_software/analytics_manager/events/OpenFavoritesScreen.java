package com.paragon_software.analytics_manager.events;

import com.paragon_software.analytics_manager.Event;

public class OpenFavoritesScreen extends Event
{
  private static final String OPEN_FAVORITES_SCREEN = "OPEN_FAVORITES_SCREEN";

  public OpenFavoritesScreen()
  {
    super(OPEN_FAVORITES_SCREEN);
  }
}
