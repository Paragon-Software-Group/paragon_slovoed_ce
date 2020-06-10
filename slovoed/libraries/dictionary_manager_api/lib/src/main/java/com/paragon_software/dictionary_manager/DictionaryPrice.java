/*
 * dictionary manager api
 *
 *  Created on: 19.03.18
 *      Author: Mikhail Belyshov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

package com.paragon_software.dictionary_manager;

import android.os.Parcel;
import android.os.Parcelable;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.util.Currency;

@SuppressWarnings( "unused" )
public final class DictionaryPrice
{
  private final long               mPrice;
  @NonNull
  private final Currency           mCurrency;
  @Nullable
  private final PeriodSubscription period;

  public DictionaryPrice( long price, @NonNull Currency currency )
  {
    this.mPrice = price;
    this.mCurrency = currency;
    this.period = null;
  }

  public DictionaryPrice( long price, @NonNull Currency currency, @NonNull PeriodSubscription period )
  {
    this.mPrice = price;
    this.mCurrency = currency;
    this.period = period;
  }

  @NonNull
  @Override
  public String toString()
  {
    // NOTE When modify this method consider what it is used to create ParagonDictionaryManager object state trace
    return "DictionaryPrice{" + "mPrice=" + mPrice + ", mCurrency=" + mCurrency
        + ", period=" + period + '}';
  }

  public long getPrice()
  {
    return mPrice;
  }

  @NonNull
  public Currency getCurrency()
  {
    return mCurrency;
  }

  @Nullable
  public PeriodSubscription getPeriod()
  {
    return period;
  }

  boolean isSubscription() {
    return null != period;
  }

  @Override
  public boolean equals( Object o )
  {
    if ( this == o )
    {
      return true;
    }
    if ( o == null || getClass() != o.getClass() )
    {
      return false;
    }

    DictionaryPrice that = (DictionaryPrice) o;

    if ( mPrice != that.mPrice )
    {
      return false;
    }
    return mCurrency.equals(that.mCurrency);
  }

  @Override
  public int hashCode()
  {
    int result = (int) ( mPrice ^ ( mPrice >>> 32 ) );
    result = 31 * result + mCurrency.hashCode();
    return result;
  }



  public enum TypePeriodSubscription
  {
    WEEK,
    MONTH,
    YEAR
  }

  public static class PeriodSubscription implements Parcelable
  {
    public final int quantity;
    public final TypePeriodSubscription type;

    public PeriodSubscription( int quantity, TypePeriodSubscription type )
    {
      this.quantity = quantity;
      this.type = type;
    }

    protected PeriodSubscription( Parcel in )
    {
      quantity = in.readInt();
      type = TypePeriodSubscription.values()[in.readInt()];
    }

    @Override
    public void writeToParcel( Parcel dest, int flags )
    {
      dest.writeInt(quantity);
      dest.writeInt(type.ordinal());
    }

    @Override
    public int describeContents()
    {
      return 0;
    }

    public static final Creator< PeriodSubscription > CREATOR = new Creator< PeriodSubscription >()
    {
      @Override
      public PeriodSubscription createFromParcel( Parcel in )
      {
        return new PeriodSubscription(in);
      }

      @Override
      public PeriodSubscription[] newArray( int size )
      {
        return new PeriodSubscription[size];
      }
    };

    @Override
    public boolean equals( Object o )
    {
      if ( this == o )
      {
        return true;
      }
      if ( o == null || getClass() != o.getClass() )
      {
        return false;
      }

      PeriodSubscription that = (PeriodSubscription) o;

      //noinspection SimplifiableIfStatement
      if ( quantity != that.quantity )
      {
        return false;
      }
      return type == that.type;
    }

    @Override
    public int hashCode()
    {
      int result = quantity;
      result = 31 * result + ( type != null ? type.hashCode() : 0 );
      return result;
    }

    @NonNull
    @Override
    public String toString()
    {
      // NOTE When modify this method consider what it is used to create ParagonDictionaryManager object state trace
      return "PeriodSubscription{" + "quantity=" + quantity + ", type=" + type + '}';
    }
  }
}
