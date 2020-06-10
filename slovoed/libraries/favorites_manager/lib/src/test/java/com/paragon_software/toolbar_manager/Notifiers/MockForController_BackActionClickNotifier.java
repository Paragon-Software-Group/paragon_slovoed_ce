package com.paragon_software.toolbar_manager.Notifiers;

import java.util.ArrayList;

public abstract class MockForController_BackActionClickNotifier
{

  private final ArrayList<OnBackActionClick> mBackActionClicks = new ArrayList<>();

  public void registerNotifier( OnBackActionClick notifier )
  {
    if (!mBackActionClicks.contains(notifier))
    {
      mBackActionClicks.add(notifier);
    }
  }

  public void backActionClick()
  {
    for (final OnBackActionClick listener : mBackActionClicks)
    {
      listener.onBackActionClick();
    }
  }
}