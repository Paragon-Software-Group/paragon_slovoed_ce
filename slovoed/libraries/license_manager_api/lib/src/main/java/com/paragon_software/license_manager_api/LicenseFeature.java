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

import java.util.HashMap;

import com.paragon_software.dictionary_manager.FeatureName;

/**
 * Information about application feature
 */
public final class LicenseFeature implements Cloneable
{
  /**
   * Feature name for identity
   */
  @NonNull
  private final FeatureName name;
  /**
   * SKU of feature for process purchase
   */
  @Nullable
  private final SKU[]       sku;
  /**
   * Information about purchase for this feature
   */
  @Nullable
  private final Purchase    purchase;

  public static LicenseFeature[] get( @NonNull FeatureName[] names )
  {
    final LicenseFeature[] result = new LicenseFeature[names.length];
    for ( int pos = 0, end = result.length ; pos < end ; ++pos )
    {
      result[pos] = new LicenseFeature(names[pos]);
    }
    return result;
  }

  private LicenseFeature( @NonNull final FeatureName name, @Nullable final SKU[] sku, @Nullable final Purchase purchase )
  {
    this.name = name;
    this.sku = sku;
    this.purchase = purchase;
  }

  /**
   * Create feature with name
   *
   * @param name Feature name for identity
   */
  public LicenseFeature( @NonNull final FeatureName name )
  {
    this.name = name;
    this.sku = null;
    this.purchase = null;
  }

  /**
   * Add SKU to feature.
   *
   * @param feature Feature for adding SKU
   * @param sku     SKU information
   */
  public LicenseFeature( @NonNull final LicenseFeature feature, @NonNull final SKU sku )
  {
    this.name = feature.name;
    this.purchase = feature.purchase;
    this.sku = mergeSKU(feature.sku, new SKU[]{ sku });
  }

  @Nullable
  private static SKU[] mergeSKU( @Nullable SKU[] lh, @Nullable SKU[] rh )
  {
    if ( null == lh )
    {
      return rh;
    }
    final HashMap< String, SKU > result = new HashMap<>();
    for ( final SKU sku : lh )
    {
      result.put(sku.getId(), sku);
    }
    if ( null != rh )
    {
      for ( final SKU sku : rh )
      {
        result.put(sku.getId(), sku);
      }
    }
    return result.values().toArray(new SKU[0]);
  }

  /**
   * Add purchase information to feature
   *
   * @param feature  Feature for adding Purchase information
   * @param purchase Purchase information
   */
  public LicenseFeature( @NonNull final LicenseFeature feature, @NonNull final Purchase purchase )
  {
    this.name = feature.name;
    this.purchase = purchase;
    this.sku = feature.sku;
  }

  /**
   * Merge this LicenseFeature with other.
   *
   * @param other other LicenseFeature for merging.
   *
   * @return new LicenseFeature with SKU from both and price from this or other if exists in other.
   */
  @NonNull
  public LicenseFeature merge( @NonNull LicenseFeature other )
  {
    return new LicenseFeature(this.name, mergeSKU(this.sku, other.sku),
                              null != other.purchase ? other.purchase : this.purchase);
  }

  /**
   * Get feature name
   *
   * @return current name
   */
  @NonNull
  public FeatureName getName()
  {
    return name;
  }

  /**
   * Get feature SKU information
   *
   * @return array of current SKU
   */
  @Nullable
  public SKU[] getSKU()
  {
    return null != sku ? sku.clone() : null;
  }

  /**
   * Get feature purchase information
   *
   * @return current purchase
   */
  @Nullable
  public Purchase getPurchase()
  {
    return purchase;
  }

  /**
   * Get hash code
   *
   * @return hash code of name
   */
  @Override
  public int hashCode()
  {
    return name.hashCode();
  }

  /**
   * Compare
   *
   * @param o other object
   *
   * @return true if other object is LicenseFeature with equal name
   */
  @Override
  public boolean equals( final Object o )
  {
    return this == o || ( o instanceof LicenseFeature && name.equals(( (LicenseFeature) o ).name) );
  }

  /**
   * Clone current feature
   *
   * @return new object with equal state
   *
   * @throws CloneNotSupportedException on clone error
   */
  @NonNull
  @Override
  public Object clone() throws CloneNotSupportedException
  {
    return super.clone();
    //    return new LicenseFeature(name, null == sku ? null : sku.clone(), purchase);
  }

  /**
   * Information about feature
   *
   * @return string with information
   */
  @NonNull
  @Override
  public String toString()
  {
    return "{name='" + name + "', state=" + getState() + "}";
  }

  /**
   * Get feature state
   *
   * @return current State
   */
  @NonNull
  public FEATURE_STATE getState()
  {
    if ( null != this.purchase )
    {
      return FEATURE_STATE.ENABLED;
    }
    else if ( null != this.sku )
    {

      return FEATURE_STATE.AVAILABLE;
    }
    return FEATURE_STATE.DISABLED;
  }

  public SKU getSku( Purchase purchase )
  {
    if ( null == purchase || null == sku )
    {
      return null;
    }

    for ( SKU skuItem : sku )
    {
      if ( null != skuItem && purchase.getSkuId().equals(skuItem.getId()) )
      {
        return skuItem;
      }
    }
    return null;
  }

  /**
   * Feature state
   */
  public enum FEATURE_STATE
  {
    /**
     * Feature disabled, user can't purchase and use this feature.
     */
    DISABLED,
    /**
     * Feature available, user can purchase but not can use this feature.
     */
    AVAILABLE,
    /**
     * Feature enabled, user already purchase and can use this feature.
     */
    ENABLED;

    /**
     * Show current state
     *
     * @return string with current state
     */
    @NonNull
    @Override
    public String toString()
    {
      switch ( this )
      {
        case AVAILABLE:
          return "AVAILABLE";
        case ENABLED:
          return "ENABLED";
        default:
          return "DISABLED";
      }
    }
  }
}
