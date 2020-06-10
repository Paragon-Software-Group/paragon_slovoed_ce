package com.paragon_software.dictionary_manager;

import androidx.annotation.NonNull;

import java.util.Collections;
import java.util.Currency;
import java.util.EnumSet;
import java.util.List;
import java.util.Set;
import java.util.ArrayList;

// NOTE This class should be immutable otherwise don't forget to implement cloning method and use it in Dictionary objects cloning
public class MarketingData
{
  private static final Long                UNDEFINED_DEFAULT_PRICE_VALUE = 0L;

  @NonNull
  private final Set<Category> categories;

  @NonNull
  private final List<Shopping> shoppings;

  private final boolean free;

  MarketingData()
  {
    this(EnumSet.noneOf(Category.class), Collections.<Shopping>emptyList(), false);
  }

  public MarketingData( @NonNull Set< Category > categories, @NonNull List< Shopping > shoppings, boolean free )
  {
    this.categories = EnumSet.copyOf(categories);
    this.shoppings = new ArrayList<>(shoppings);
    this.free = free;
  }

  @NonNull
  public Set< Category > getCategories()
  {
    return categories;
  }

  Long getDefaultPriceValue( Currency currency )
  {
    for ( Shopping s : getShoppings() )
    {
      if ( !s.isSubscription() )
      {
        Long priceValue = s.getPrice().get(currency);
        return null == priceValue ? UNDEFINED_DEFAULT_PRICE_VALUE : priceValue;
      }
    }
    return UNDEFINED_DEFAULT_PRICE_VALUE;
  }

  @NonNull
  public List< Shopping > getShoppings()
  {
    return shoppings;
  }

  public boolean isFree() {
    return free;
  }

  @Override
  public String toString()
  {
    // NOTE When modify this method consider what it is used to create ParagonDictionaryManager object state trace
    return "MarketingData{"
            + "categories=" + categories
            + ", shoppings=" + shoppings
            + ", free=" + free
            + '}';
  }

  public enum Category
  {
    FEATURED,
    BESTSELLER,
    NEW
  }
}
