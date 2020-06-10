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

/**
 * Information about purchase
 */
public final class Purchase implements Cloneable
{
  @NonNull
  private final String orderId;
  @NonNull
  private final String skuId;
  @NonNull
  private final String purchaseToken;
  @NonNull
  private final String extra;
  private final long   purchaseTime;
  private final boolean subscription;
  private boolean autoRenewing;

  @Nullable
  private String displayCache = null;

  /**
   * Create purchase
   *  @param orderId       Unique identity of purchase
   * @param skuId         SKU identity for this purchase
   * @param purchaseTime  time of purchase
   * @param purchaseToken additional purchase token ( from market )
   * @param extra         additional purchase information ( from client )
   * @param autoRenewing Indicates whether the subscription renews automatically. If true, the subscription is active, and will automatically renew on the next billing date. If false, indicates that the user has canceled the subscription.
   */
  public Purchase( @NonNull final String orderId, @NonNull final String skuId, final long purchaseTime,
                   @NonNull final String purchaseToken, @NonNull final String extra, @NonNull String autoRenewing )
  {
    this.orderId = orderId;
    this.skuId = skuId;
    this.purchaseTime = purchaseTime;
    this.purchaseToken = purchaseToken;
    this.extra = extra;
    this.subscription = !TextUtils.isEmpty(autoRenewing);
    this.autoRenewing = Boolean.parseBoolean(autoRenewing);
  }

  /**
   * Unique purchase id getter
   *
   * @return Unique purchase id
   */
  @NonNull
  public String getOrderId()
  {
    return orderId;
  }

  /**
   * SKU Id getter
   *
   * @return SKU Id
   */
  @NonNull
  public String getSkuId()
  {
    return skuId;
  }

  /**
   * Time of purchase getter
   *
   * @return time of purchase
   */
  public long getPurchaseTime()
  {
    return purchaseTime;
  }

  /**
   * Additional purchase token ( from market ) getter
   *
   * @return additional purchase token ( from market )
   */
  @NonNull
  public String getPurchaseToken()
  {
    return purchaseToken;
  }

  /**
   * Additional purchase information ( from client ) getter
   *
   * @return additional purchase information ( from client )
   */
  @NonNull
  public String getExtra()
  {
    return extra;
  }

  /**
   * <b>Only for subscriptions</b><br/>
   * Indicates whether the subscription renews automatically.
   * @return  If true, the subscription is active, and will automatically renew on the next billing date. If false, indicates that the user has canceled the subscription.
   * @see
   * <a href="https://developer.android.com/google/play/billing/billing_reference#getBuyIntent">In-app Billing Reference - Table 7. Descriptions of the JSON fields for INAPP_PURCHASE_DATA.</a>
   */
  public boolean isAutoRenewing()
  {
    return autoRenewing;
  }

  public boolean isSubscription()
  {
    return subscription;
  }

  /**
   * Hash code from unique purchase id
   *
   * @return hash code
   */
  @Override
  public int hashCode()
  {
    return orderId.hashCode();
  }

  /**
   * Compare current purchase with other object
   *
   * @param o object for comparing
   *
   * @return true if Unique purchase id is equals
   */
  @Override
  public boolean equals( final Object o )
  {
    return this == o || o instanceof Purchase && orderId.equals(( (Purchase) o ).orderId);
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
   * Information about Purchase
   *
   * @return string with information
   */
  @NonNull
  @Override
  public String toString()
  {
    if ( null == displayCache )
    {
      displayCache = "{orderId='" + orderId + "', skuId='" + skuId + "', purchaseTime=" + purchaseTime + "}";
    }
    return displayCache;
  }
}
