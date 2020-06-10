package com.paragon_software.dictionary_manager_factory;

import androidx.annotation.NonNull;

import com.google.gson.annotations.SerializedName;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Currency;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

@SuppressWarnings( "WeakerAccess" )
class Shopping
{
  @SerializedName( "shop" )
  private String shop;

  @SerializedName( "id_in_shop" )
  private String idInShop;

  @SerializedName( "subscription_period" )
  private String subscriptionPeriod;

  @SerializedName( "currency" )
  private String currency;

  @SerializedName( "price" )
  private Float price;

  @SerializedName( "modification" )
  private String modification;

  String getIdInShop()
  {
    return idInShop;
  }

  String getShop()
  {
    return shop;
  }

  String getSubscriptionPeriod()
  {
    return subscriptionPeriod;
  }

  String getCurrency()
  {
    return currency;
  }

  Float getPrice()
  {
    return price;
  }

  String getModification()
  {
    return modification;
  }

  @Override
  public String toString()
  {
    return "Shopping{" + "shop='" + shop + '\'' + ", idInShop='" + idInShop + '\'' + ", subscriptionPeriod='"
        + subscriptionPeriod + '\'' + ", currency='" + currency + '\'' + ", price=" + price + ", modification='"
        + modification + '\'' + '}';
  }

  static Map< String, List< Shopping > > groupByIdInShop( List< Shopping > shoppings )
  {
    if ( shoppings.isEmpty() )
    {
      return Collections.emptyMap();
    }

    Map< String, List< Shopping > > groupped = new HashMap<>();

    Set< String > idsInShop = new HashSet<>();
    for ( Shopping s : shoppings )
    {
      idsInShop.add(s.getIdInShop());
    }

    for ( String idInShop : idsInShop )
    {
      List< Shopping > sublist = getSublistWithIdInShop(idInShop, shoppings);
      validateForEqualsSubscriptionPeriod(sublist);
      validateForDifferentCurrency(sublist);
      groupped.put(idInShop, sublist);
    }
    return groupped;
  }

  private static void validateForDifferentCurrency( List< Shopping > list )
  {
    Set<String> currencies = new HashSet<>();
    for ( Shopping shopping : list )
    {
      if (!currencies.add(shopping.getCurrency().toUpperCase()))
      {
        throw new IllegalArgumentException("Shoppings must has different currency. Error with shoppings : " + list);
      }
    }
  }

  private static void validateForEqualsSubscriptionPeriod( List< Shopping > list )
  {
    String subscriptionPeriod = null;
    for ( Shopping shopping : list )
    {
      if (null == subscriptionPeriod)
      {
        subscriptionPeriod = shopping.getSubscriptionPeriod();
      }
      else
      {
        if (!subscriptionPeriod.equals(shopping.getSubscriptionPeriod()))
        {
          throw new IllegalArgumentException("Subscription period is not equals for shoppings : " + list);
        }
      }
    }
  }

  private static List< Shopping > getSublistWithIdInShop( @NonNull String idInShop, List< Shopping > shoppings )
  {
    List< Shopping > sublist = new ArrayList<>();
    for ( Shopping s : shoppings )
    {
      if ( idInShop.equals(s.getIdInShop()) )
      {
        sublist.add(s);
      }
    }
    if (sublist.isEmpty())
    {
      throw new IllegalArgumentException("For each id in shop (sku) must present Shopping object. Error with idInShop: " + idInShop);
    }
    return sublist;
  }

  static Map< Currency, Long > toPriceMap( List< Shopping > shoppings )
  {
    Map< Currency, Long > map = new HashMap<>();
    for ( Shopping s : shoppings )
    {
      Long priceMillis = Math.round(((double)s.getPrice()) * 1000000);
      map.put(Currency.getInstance(s.getCurrency()), priceMillis);
    }
    return map;
  }

  static Integer parseSubscriptionPeriod( @NonNull Shopping shopping )
  {
    try
    {
      return Integer.parseInt(shopping.getSubscriptionPeriod());
    }
    catch ( NumberFormatException e )
    {
      return null;
    }
  }
}
