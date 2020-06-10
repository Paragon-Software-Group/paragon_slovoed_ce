package com.paragon_software.analytics_manager.events;

import com.paragon_software.analytics_manager.Event;

public class AddArticleToFavourites extends Event
{
  private static final String ADD_ARTICLE_TO_FAVOURITES = "ADD_ARTICLE_TO_FAVOURITES";

  public AddArticleToFavourites()
  {
    super(ADD_ARTICLE_TO_FAVOURITES);
  }
}
