package com.paragon_software.favorites_manager;

import com.paragon_software.analytics_manager.EmptyAnalyticsManger;

public class AnalyticsHelper
{
  public static void initialize()
  {
    try
    {
      new EmptyAnalyticsManger().initialize(null, null);
    }
    catch ( IllegalStateException e )
    {
      e.printStackTrace();
    }
  }
}