package com.paragon_software.article_manager;

import androidx.annotation.NonNull;

public class ShowArticleOptions
{
  @NonNull
  private final SwipeMode swipeMode;

  public ShowArticleOptions( @NonNull SwipeMode swipeMode )
  {
    this.swipeMode = swipeMode;
  }

  @NonNull
  public SwipeMode getSwipeMode()
  {
    return swipeMode;
  }
}
