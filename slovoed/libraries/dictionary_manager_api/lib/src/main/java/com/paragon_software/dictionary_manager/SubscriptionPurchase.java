package com.paragon_software.dictionary_manager;

import android.os.Parcel;
import android.os.Parcelable;
import androidx.annotation.NonNull;

import java.util.Currency;

public class SubscriptionPurchase implements Parcelable
{
  private final long                               purchaseTime;
  private final boolean                            autoRenewing;
  private final long                               priceValue;
  private final Currency                           priceCurrency;
  @NonNull
  private final DictionaryPrice.PeriodSubscription period;
  private String skuId;

  SubscriptionPurchase( long purchaseTime, boolean autoRenewing, long priceValue, Currency priceCurrency,
                        @NonNull DictionaryPrice.PeriodSubscription period, @NonNull String skuId )
  {

    this.purchaseTime = purchaseTime;
    this.autoRenewing = autoRenewing;
    this.priceValue = priceValue;
    this.priceCurrency = priceCurrency;
    this.period = period;
    this.skuId = skuId;
  }

  protected SubscriptionPurchase( Parcel in )
  {
    purchaseTime = in.readLong();
    autoRenewing = in.readByte() != 0;
    priceValue = in.readLong();
    priceCurrency = Currency.getInstance(in.readString());
    skuId = in.readString();
    period = in.readParcelable(DictionaryPrice.PeriodSubscription.class.getClassLoader());
  }

  @Override
  public void writeToParcel( Parcel dest, int flags )
  {
    dest.writeLong(purchaseTime);
    dest.writeByte((byte) ( autoRenewing ? 1 : 0 ));
    dest.writeLong(priceValue);
    dest.writeString(priceCurrency.getCurrencyCode());
    dest.writeString(skuId);
    dest.writeParcelable(period, flags);
  }

  @Override
  public int describeContents()
  {
    return 0;
  }

  public static final Creator< SubscriptionPurchase > CREATOR = new Creator< SubscriptionPurchase >()
  {
    @Override
    public SubscriptionPurchase createFromParcel( Parcel in )
    {
      return new SubscriptionPurchase(in);
    }

    @Override
    public SubscriptionPurchase[] newArray( int size )
    {
      return new SubscriptionPurchase[size];
    }
  };

  public long getPurchaseTime()
  {
    return purchaseTime;
  }

  public boolean isAutoRenewing()
  {
    return autoRenewing;
  }

  public String getSkuId()
  {
    return skuId;
  }

  public boolean equalsPriceAndPeriod( @NonNull DictionaryPrice price )
  {
    return getPriceCurrency().equals(price.getCurrency()) && getPriceValue() == price.getPrice() && getPeriod()
        .equals(price.getPeriod());
  }

  @SuppressWarnings( "WeakerAccess" )
  public Currency getPriceCurrency()
  {
    return priceCurrency;
  }

  @SuppressWarnings( "WeakerAccess" )
  public long getPriceValue()
  {
    return priceValue;
  }

  @NonNull
  public DictionaryPrice.PeriodSubscription getPeriod()
  {
    return period;
  }

  @Override
  public String toString()
  {
    // NOTE When modify this method consider what it is used to create ParagonDictionaryManager object state trace
    return "SubscriptionPurchase{" + "purchaseTime=" + purchaseTime + ", autoRenewing=" + autoRenewing + ", priceValue="
        + priceValue + ", priceCurrency=" + priceCurrency + ", period=" + period + ", skuId='" + skuId + '\'' + '}';
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
    SubscriptionPurchase that = (SubscriptionPurchase) o;
    return that.toString().equals(this.toString());
  }

  @Override
  public int hashCode()
  {
    return toString().hashCode();
  }
}
