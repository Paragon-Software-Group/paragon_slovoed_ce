package com.paragon_software.analytics_manager.events;

public enum ScreenName
{
  ARTICLE_TRIAL("ARTICLE_TRIAL"),
  ARTICLE_DEMO("ARTICLE_DEMO"),
  MY_DICTIONARIES("MY_DICTIONARIES"),
  RECENTLY_CATALOG("RECENTLY_CATALOG"),
  FEATURED_CATALOG("FEATURED_CATALOG"),
  BESTSELLERS_CATALOG("BESTSELLERS_CATALOG"),
  NEW_CATALOG("NEW_CATALOG"),
  DEALS_CATALOG("DEALS_CATALOG"),
  ALL_CATALOG("ALL_CATALOG"),
  PRODUCT_DESCRIPTION_AND_PRICES_SCREEN("PRODUCT_DESCRIPTION_AND_PRICES_SCREEN");

  private final String analyticsName;

  ScreenName( String analyticsName )
  {
    this.analyticsName = analyticsName;
  }

  public String getAnalyticsName()
  {
    return analyticsName;
  }
}