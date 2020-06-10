package com.paragon_software.dictionary_manager.filter;

import androidx.annotation.NonNull;

import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryFilter;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.dictionary_manager.MarketingData;

abstract class SimpleFilter implements DictionaryFilter
{
  protected DictionaryManagerAPI dictionaryManager;

  SimpleFilter( @NonNull DictionaryManagerAPI dictionaryManager )
  {
    this.dictionaryManager = dictionaryManager;
  }

  boolean applyForMarketingCategory( @NonNull MarketingData.Category category, @NonNull Dictionary.DictionaryId dictionaryId ) {
    for ( Dictionary dictionary : dictionaryManager.getDictionaries() )
    {
      if (dictionaryId.equals(dictionary.getId()))
      {
        MarketingData data = dictionary.getMarketingData();
        return data.getCategories().contains(category);
      }
    }
    return false;
  }

  @Override
  public void saveState()
  {

  }

  @Override
  public void registerListener( @NonNull FilterStateChangedListener filterStateChangedListener )
  {

  }

  @Override
  public void unregisterListener( @NonNull FilterStateChangedListener filterStateChangedListener )
  {

  }
}
