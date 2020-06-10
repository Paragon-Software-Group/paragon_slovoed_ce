package com.paragon_software.analytics_manager.events;

import com.paragon_software.analytics_manager.Event;

public class OpenHistoryScreen extends Event
{
  private static final String OPEN_HISTORY_SCREEN = "OPEN_HISTORY_SCREEN";

  public OpenHistoryScreen()
  {
    super(OPEN_HISTORY_SCREEN);
  }
}
