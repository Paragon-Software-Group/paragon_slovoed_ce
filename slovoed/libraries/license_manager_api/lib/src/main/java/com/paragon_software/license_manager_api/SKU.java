/*
 * license_manager_api
 *
 *  Created on: 08.02.18
 *      Author: Ivan Zhdanov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

package com.paragon_software.license_manager_api;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.util.Currency;
import java.util.Date;

/**
 * Stock Keeping Unit
 * Need for activate feature in market
 */
public final class SKU implements Cloneable
{
  /**
   * SKU Id unique identify in market
   */
  @NonNull
  private final String     id;
  @NonNull
  private final String     sourceName;
  @NonNull
  private final Currency   priceCurrency;
  private final long       priceValue;
  @Nullable
  private final Date       endDate;
  @Nullable
  private final PeriodShdd period;
  @Nullable
  private String displayCache = null;

  /**
   * Create new SKU
   *
   * @param id            Unique identity in market
   * @param sourceName    Name of market source
   * @param priceCurrency price currency example: USD
   * @param priceValue    Price in micro-units, where 1,000,000 micro-units equal one unit of the currency. Example: 7990000 = 7.99
   */
  public SKU( @NonNull final String id, @NonNull final String sourceName, @NonNull final Currency priceCurrency,
              final long priceValue )
  {
    this.id = id;
    this.sourceName = sourceName;
    this.priceCurrency = priceCurrency;
    this.priceValue = priceValue;
    this.endDate = null;
    this.period = null;
  }

  /**
   * Create new SKU for subscription
   *
   * @param id            Unique identity in market
   * @param sourceName    Name of market source
   * @param priceCurrency price currency example: $
   * @param priceValue    Price in micro-units, where 1,000,000 micro-units equal one unit of the currency. Example: 7990000 = 7.99
   * @param endDate       date of subscribe end
   * @param period        period of subscription
   */
  public SKU( @NonNull final String id, @NonNull final String sourceName, @NonNull final Currency priceCurrency,
              final long priceValue, @Nullable final Date endDate, @NonNull PeriodShdd period)
  {
    this.id = id;
    this.sourceName = sourceName;
    this.priceCurrency = priceCurrency;
    this.priceValue = priceValue;
    this.endDate = null != endDate? new Date(endDate.getTime()): null;
    this.period = period;
  }

  /**
   * SKU Id getter
   *
   * @return SKU Id
   */
  @NonNull
  public String getId()
  {
    return id;
  }

  /**
   * SKU Source getter
   *
   * @return SKU source
   */
  @NonNull
  public String getSourceName()
  {
    return sourceName;
  }

  /**
   * SKU price currency getter
   *
   * @return SKU price currency
   */
  @NonNull
  public Currency getPriceCurrency()
  {
    return priceCurrency;
  }

  /**
   * SKU price value getter
   *
   * @return price value
   */
  public long getPriceValue()
  {
    return priceValue;
  }

  /**
   * Hash code from this object
   *
   * @return hash code from id
   */
  @Override
  public int hashCode()
  {
    return id.hashCode();
  }

  /**
   * Compare current Storage Keep Unit with other object
   *
   * @param o object for compare
   *
   * @return true if SKU id is equal
   */
  @Override
  public boolean equals( final Object o )
  {
    return this == o || ( o instanceof SKU && id.equals(( (SKU) o ).id) );
  }

  /**
   * Clone object
   *
   * @return new Object with current state
   *
   * @throws CloneNotSupportedException on clone problem
   */
  @NonNull
  @Override
  public Object clone() throws CloneNotSupportedException
  {
    return super.clone();
  }

  /**
   * Show information about SKU
   *
   * @return string with information
   */
  @NonNull
  @Override
  public String toString()
  {
    if ( null == displayCache )
    {
      displayCache =
          "{id='" + id + "', source='" + sourceName + "', price=" + Float.toString((float) priceValue / 1000000f) + "'"
              + priceCurrency.getSymbol() + "'}";
    }
    return displayCache;
  }

  /**
   * Get end of subscription
   *
   * @return end date of subscription
   */
  @Nullable
  public Date getEndDate()
  {
    return null != endDate ? new Date(endDate.getTime()) : null;
  }

  /**
   * Current SKU is SKU for subscription
   *
   * @return true if exists end date
   */
  public boolean isSubscription()
  {
    return null != period;
  }

  /**
   * Check subscription for expire
   *
   * @return true if SKU is subscription and is expired
   */
  public boolean isExpire()
  {
    return null != endDate && endDate.compareTo(new Date()) <= 0;
  }

  @Nullable
  public PeriodShdd getPeriod()
  {
    return period;
  }
}
