package com.paragon_software.analytics_manager;

import android.os.Bundle;
import androidx.annotation.NonNull;
import androidx.annotation.VisibleForTesting;

import static androidx.annotation.VisibleForTesting.PACKAGE_PRIVATE;

public abstract class Event
{
  @NonNull
  private final String name;

  @NonNull
  protected final Bundle bundle = new Bundle();

  public Event( @NonNull String name )
  {
    this.name = name;
  }

  @NonNull
  String getName()
  {
    return name;
  }

  @NonNull @VisibleForTesting(otherwise = PACKAGE_PRIVATE)
  public Bundle getBundle()
  {
    return bundle;
  }
}
