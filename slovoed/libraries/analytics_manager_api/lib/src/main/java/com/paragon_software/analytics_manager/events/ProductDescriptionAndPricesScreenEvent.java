package com.paragon_software.analytics_manager.events;

import androidx.annotation.VisibleForTesting;

import com.paragon_software.analytics_manager.Event;

public class ProductDescriptionAndPricesScreenEvent extends Event
{
  @VisibleForTesting
  public static final String FROM                                  = "FROM";
  private static final String PRODUCT_DESCRIPTION_AND_PRICES_SCREEN = "PRODUCT_DESCRIPTION_AND_PRICES_SCREEN";

  public ProductDescriptionAndPricesScreenEvent( ScreenName from )
  {
    super(PRODUCT_DESCRIPTION_AND_PRICES_SCREEN);
    bundle.putString(FROM, from.getAnalyticsName());
  }
}