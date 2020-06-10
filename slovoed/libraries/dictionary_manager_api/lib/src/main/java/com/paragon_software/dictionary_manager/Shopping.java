package com.paragon_software.dictionary_manager;

import android.os.Parcel;
import android.os.Parcelable;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.io.Serializable;
import java.util.Currency;
import java.util.HashMap;
import java.util.Map;

// NOTE This class should be immutable otherwise don't forget to implement cloning method and use it in Dictionary objects cloning
public class Shopping implements Parcelable, Serializable
{
  public static final Creator< Shopping > CREATOR = new Creator< Shopping >()
  {
    @Override
    public Shopping createFromParcel( Parcel in )
    {
      return new Shopping(in);
    }

    @Override
    public Shopping[] newArray( int size )
    {
      return new Shopping[size];
    }
  };
  private String                sku;
  private Map< Currency, Long > price;
  @Nullable
  private Integer               subscriptionPeriod;

  public Shopping( @NonNull String sku, @NonNull Map< Currency, Long > price, @Nullable Integer subscriptionPeriod )
  {
    this.sku = sku.trim();
    this.price = new HashMap<>(price);
    this.subscriptionPeriod = subscriptionPeriod;

    if ( this.sku.length() <= 0 )
    {
      throw new IllegalArgumentException("Sku (product id in shop) cannot be empty");
    }

    if ( this.price.isEmpty() )
    {
      throw new IllegalArgumentException("Price with currency cannot be empty");
    }

    for ( Map.Entry< Currency, Long > entry : this.price.entrySet() )
    {
      if ( null == entry.getValue() || null == entry.getKey() )
      {
        throw new IllegalArgumentException("Price with currency items cannot be null");
      }
    }

    if ( null != this.subscriptionPeriod && this.subscriptionPeriod <= 0 )
    {
      throw new IllegalArgumentException("Subscription period cannot be zero or less");
    }
  }

  @SuppressWarnings( "WeakerAccess" )
  protected Shopping( Parcel in )
  {
    sku = in.readString();
    int mapSize = in.readInt();
    this.price = new HashMap<>();
    for ( int i = 0 ; i < mapSize ; i++ )
    {
      this.price.put(Currency.getInstance(in.readString()), in.readLong());
    }
    if ( in.readByte() == 0 )
    {
      subscriptionPeriod = null;
    }
    else
    {
      subscriptionPeriod = in.readInt();
    }
  }

  @Override
  public int hashCode()
  {
    int result = getSku() != null ? getSku().hashCode() : 0;
    result = 31 * result + ( getPrice() != null ? getPrice().hashCode() : 0 );
    result = 31 * result + ( subscriptionPeriod != null ? subscriptionPeriod.hashCode() : 0 );
    return result;
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

    Shopping shopping = (Shopping) o;

    if ( getSku() != null ? !getSku().equals(shopping.getSku()) : shopping.getSku() != null )
    {
      return false;
    }
    //noinspection SimplifiableIfStatement
    if ( getPrice() != null ? !getPrice().equals(shopping.getPrice()) : shopping.getPrice() != null )
    {
      return false;
    }
    return subscriptionPeriod != null ? subscriptionPeriod.equals(shopping.subscriptionPeriod)
                                      : shopping.subscriptionPeriod == null;
  }

  public String getSku()
  {
    return sku;
  }

  public Map< Currency, Long > getPrice()
  {
    return price;
  }

  @Override
  public int describeContents()
  {
    return 0;
  }

  @Override
  public void writeToParcel( Parcel parcel, int i )
  {
    parcel.writeString(sku);
    parcel.writeInt(price.size());
    for ( Map.Entry< Currency, Long > entry : price.entrySet() )
    {
      parcel.writeString(entry.getKey().getCurrencyCode());
      parcel.writeLong(entry.getValue());
    }
    if ( subscriptionPeriod == null )
    {
      parcel.writeByte((byte) 0);
    }
    else
    {
      parcel.writeByte((byte) 1);
      parcel.writeInt(subscriptionPeriod);
    }
  }

  public boolean isSubscription()
  {
    return null != subscriptionPeriod;
  }

  @Override
  public String toString()
  {
    // NOTE When modify this method consider what it is used to create ParagonDictionaryManager object state trace
    return "Shopping{" + "sku='" + sku + '\'' + ", price=" + price + ", subscriptionPeriod=" + subscriptionPeriod + '}';
  }
}
