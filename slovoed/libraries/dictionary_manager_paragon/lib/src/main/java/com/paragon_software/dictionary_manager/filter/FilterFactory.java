package com.paragon_software.dictionary_manager.filter;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.dictionary_manager.DictionaryFilter;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.dictionary_manager.filter.external.ILanguageFilter;
import com.paragon_software.dictionary_manager.filter.external.ISearchFilter;
import com.paragon_software.dictionary_manager.filter.external.IRecentlyOpenedFilter;
import com.paragon_software.native_engine.SearchEngineAPI;
import com.paragon_software.settings_manager.SettingsManagerAPI;

public class FilterFactory implements IFilterFactory
{
  private static final String languageFilterSettingsKey       = "lfsk2";
  private static final String RecentlyOpenedFilterSettingsKey = "rofsk";
  private static final String SearchFilterSettingsKey         = "sfsk";
  private        SearchEngineAPI      searchEngine;
  private        DictionaryManagerAPI dictionaryManager;
  private        SettingsManagerAPI   settingsManager;
  private static RecentlyOpenedFilter mRecentlyOpenedFilter;

  public FilterFactory( @NonNull DictionaryManagerAPI dictionaryManager, SettingsManagerAPI settingsManager, SearchEngineAPI searchEngine )
  {
    this.dictionaryManager = dictionaryManager;
    this.settingsManager = settingsManager;
    this.searchEngine = searchEngine;
  }

  @Nullable
  @Override
  public < T extends DictionaryFilter > T createByClass( Class< T > aClass )
  {
    if ( ILanguageFilter.class.equals(aClass) )
    {
      return aClass.cast(new LanguageFilter(dictionaryManager, languageFilterSettingsKey, settingsManager));
    }
    else if ( IRecentlyOpenedFilter.class.equals(aClass) )
    {
      if ( null == mRecentlyOpenedFilter )
      {
        mRecentlyOpenedFilter = new RecentlyOpenedFilter(dictionaryManager, RecentlyOpenedFilterSettingsKey, settingsManager);
      }
      return aClass.cast(mRecentlyOpenedFilter);
    }
    else if ( ISearchFilter.class.equals(aClass) )
    {
      return aClass.cast(new SearchAllFilter(dictionaryManager, SearchFilterSettingsKey, settingsManager, searchEngine));
    }

    return null;
  }

  @Override
  public DictionaryFilter createByType( FilterTypeSimple filterTypeSimple )
  {
    switch ( filterTypeSimple )
    {
      case BESTSELLERS:
        return SimpleFilters.createBestsellersFilter(dictionaryManager);
      case DEALS:
        return SimpleFilters.createDealsFilter(dictionaryManager);
      case FEATURED:
        return SimpleFilters.createFeaturedFilter(dictionaryManager);
      case NEW_DICTIONARIES:
        return SimpleFilters.createNewDictionariesFilter(dictionaryManager);
      case CAN_BE_PURCHASED_DICTIONARIES:
        return SimpleFilters.createCanBePurchasedDictionariesFilter(dictionaryManager);
      case MY_DICTIONARIES:
        return SimpleFilters.createMyDictionariesFilter(dictionaryManager);
      case SHARE_AND_API:
        return SimpleFilters.createShareAndApiFilter(dictionaryManager);
    }
    return null;
  }
}
