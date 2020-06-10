package com.paragon_software.dictionary_manager;

import androidx.annotation.NonNull;

public class DictionaryDiscount implements IDictionaryDiscount
{
  private static final int DEFAULT_MIN_PERCENTAGE_FOR_SHOW = 5;
  private final Long defaultPrice;
  private final Long actualPrice;

  public static IDictionaryDiscount create( Long defaultPrice, Long actualPrice )
  {
    return create(defaultPrice, actualPrice, DEFAULT_MIN_PERCENTAGE_FOR_SHOW);
  }

  public static IDictionaryDiscount create( Long defaultPrice, Long actualPrice, int minimalDiscountPercentageForShow )
  {
    if ( minimalDiscountPercentageForShow < 0 || minimalDiscountPercentageForShow > 99 )
    {
      throw new IllegalArgumentException(
          "minimalDiscountPercentageForShow invalid value : " + minimalDiscountPercentageForShow);
    }

    if ( null == defaultPrice || defaultPrice <= 0 || null == actualPrice || actualPrice <= 0
        || defaultPrice <= actualPrice )
    {
      return null;
    }

    IDictionaryDiscount IDictionaryDiscount = new DictionaryDiscount(defaultPrice, actualPrice);
    if ( IDictionaryDiscount.getPercentage() >= minimalDiscountPercentageForShow )
    {
      return IDictionaryDiscount;
    }
    else
    {
      return null;
    }
  }

  private DictionaryDiscount( @NonNull Long defaultPrice, @NonNull Long actualPrice )
  {
    this.defaultPrice = defaultPrice;
    this.actualPrice = actualPrice;
  }

  @Override
  public int getPercentage()
  {
    return (int) ( ( defaultPrice - actualPrice ) * 100 / defaultPrice );
  }

  @NonNull
  @Override
  public Long getDefaultPrice()
  {
    return defaultPrice;
  }

  @NonNull
  @Override
  public Long getActualPrice()
  {
    return actualPrice;
  }
}
