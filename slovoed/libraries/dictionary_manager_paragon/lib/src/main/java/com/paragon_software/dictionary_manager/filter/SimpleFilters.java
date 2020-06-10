package com.paragon_software.dictionary_manager.filter;

import androidx.annotation.NonNull;

import com.paragon_software.dictionary_manager.*;
import com.paragon_software.dictionary_manager.components.DictionaryComponent;

final class SimpleFilters
{

  static DictionaryFilter createCatalogFilter( @NonNull DictionaryManagerAPI dictionaryManager )
  {
    return new SimpleFilter(dictionaryManager)
    {
      @Override
      public boolean apply( @NonNull Dictionary.DictionaryId dictionaryId )
      {
        return true;
      }
    };
  }

  static DictionaryFilter createFeaturedFilter( @NonNull DictionaryManagerAPI dictionaryManager )
  {
    return new SimpleFilter(dictionaryManager)
    {
      @Override
      public boolean apply( @NonNull Dictionary.DictionaryId dictionaryId )
      {
        return applyForMarketingCategory(MarketingData.Category.FEATURED, dictionaryId);
      }
    };
  }

  static DictionaryFilter createBestsellersFilter( @NonNull DictionaryManagerAPI dictionaryManager )
  {
    return new SimpleFilter(dictionaryManager)
    {
      @Override
      public boolean apply( @NonNull Dictionary.DictionaryId dictionaryId )
      {
        return applyForMarketingCategory(MarketingData.Category.BESTSELLER, dictionaryId);
      }
    };
  }

  static DictionaryFilter createNewDictionariesFilter( @NonNull DictionaryManagerAPI dictionaryManager )
  {
    return new SimpleFilter(dictionaryManager)
    {
      @Override
      public boolean apply( @NonNull Dictionary.DictionaryId dictionaryId )
      {
        return applyForMarketingCategory(MarketingData.Category.NEW, dictionaryId);
      }
    };
  }

  static DictionaryFilter createDealsFilter( @NonNull DictionaryManagerAPI dictionaryManager )
  {
    return new SimpleFilter(dictionaryManager)
    {
      @Override
      public boolean apply( @NonNull Dictionary.DictionaryId dictionaryId )
      {
        return null != dictionaryManager.getDictionaryDiscount(dictionaryId);
      }
    };
  }

  public static DictionaryFilter createCanBePurchasedDictionariesFilter( DictionaryManagerAPI dictionaryManager )
  {
    return new SimpleFilter(dictionaryManager)
    {
      @Override
      public boolean apply( @NonNull Dictionary.DictionaryId dictionaryId )
      {
        for ( Dictionary dictionary : dictionaryManager.getDictionaries() )
        {
          if (dictionaryId.equals(dictionary.getId()))
          {
            return dictionary.getStatus().canBePurchased();
          }
        }
        return false;
      }
    };
  }

  public static DictionaryFilter createMyDictionariesFilter( DictionaryManagerAPI dictionaryManager )
  {
    return new SimpleFilter(dictionaryManager)
    {
      @Override
      public boolean apply( @NonNull Dictionary.DictionaryId dictionaryId )
      {
        return false;
      }
    };
  }

  public static DictionaryFilter createShareAndApiFilter( DictionaryManagerAPI dictionaryManager )
  {
    final DictionaryControllerAPI controller = dictionaryManager.createController("FILTER");
    return new SimpleFilter(dictionaryManager)
    {
      @Override
      public boolean apply( @NonNull Dictionary.DictionaryId dictionaryId )
      {
        return false;
      }
    };
  }
}
