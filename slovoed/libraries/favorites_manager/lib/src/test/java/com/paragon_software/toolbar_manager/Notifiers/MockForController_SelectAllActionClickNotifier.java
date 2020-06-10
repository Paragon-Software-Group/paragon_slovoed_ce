package com.paragon_software.toolbar_manager.Notifiers;

import java.util.ArrayList;

public abstract class MockForController_SelectAllActionClickNotifier
{

  private final ArrayList<OnSelectAllActionClick> mSelectAllActionClicks = new ArrayList<>();

  public void registerNotifier( OnSelectAllActionClick notifier )
  {
    if (!mSelectAllActionClicks.contains(notifier))
    {
      mSelectAllActionClicks.add(notifier);
    }
  }

  public void selectAllActionClick()
  {
    for (final OnSelectAllActionClick listener : mSelectAllActionClicks)
    {
      listener.onSelectAllActionClick();
    }
  }
}