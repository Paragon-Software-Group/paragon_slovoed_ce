package com.paragon_software.analytics_manager.events;

import com.paragon_software.analytics_manager.Event;

public class TrialScreen extends Event
{
  private static final String FROM         = "FROM";
  private static final String TRAIL_SCREEN = "TRAIL_SCREEN";

  public TrialScreen( ScreenName from )
  {
    super(TRAIL_SCREEN);
    bundle.putString(FROM, from.getAnalyticsName());
  }

}