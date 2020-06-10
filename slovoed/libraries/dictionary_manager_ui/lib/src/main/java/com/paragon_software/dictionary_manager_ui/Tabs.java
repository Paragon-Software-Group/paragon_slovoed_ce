package com.paragon_software.dictionary_manager_ui;

import androidx.annotation.LayoutRes;
import androidx.annotation.NonNull;
import androidx.annotation.StringRes;
import androidx.fragment.app.Fragment;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.Collections;
import java.util.EnumSet;
import java.util.List;

import com.paragon_software.dictionary_manager.DictionaryControllerAPI;
import com.paragon_software.dictionary_manager.DictionaryFilter;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.dictionary_manager.filter.FilterTypeSimple;
import com.paragon_software.dictionary_manager.filter.IFilterFactory;
import com.paragon_software.dictionary_manager.filter.external.IRecentlyOpenedFilter;

import com.paragon_software.analytics_manager.events.ScreenName;

enum Tabs
{
  MY_DICTIONARIES(R.layout.fragment_my_dictionaries, R.layout.my_dictionary_item_view, R.string.dictionary_manager_ui_tab_my_dictionaries_label, ScreenName.MY_DICTIONARIES),
  RECENTLY_VIEWED(R.layout.fragment_tab_dictionary_view_list, R.layout.dictionary_view,
                  R.string.dictionary_manager_ui_tab_recently_viewed_label, ScreenName.RECENTLY_CATALOG),
  FEATURED(R.layout.fragment_tab_dictionary_card_list, R.layout.dictionary_card, R.string.dictionary_manager_ui_tab_featured_label, ScreenName.FEATURED_CATALOG),
  BESTSELLERS(R.layout.fragment_tab_dictionary_view_list, R.layout.dictionary_view, R.string.dictionary_manager_ui_tab_bestsellers_label, ScreenName.BESTSELLERS_CATALOG),
  NEW(R.layout.fragment_tab_dictionary_view_list, R.layout.dictionary_view, R.string.dictionary_manager_ui_tab_new_label, ScreenName.NEW_CATALOG),
  DEALS(R.layout.fragment_tab_dictionary_view_list, R.layout.dictionary_view_deals, R.string.dictionary_manager_ui_tab_deals_label, ScreenName.DEALS_CATALOG),
  ALL(R.layout.fragment_tab_all_dictionaries, R.layout.dictionary_list_element, R.string.dictionary_manager_ui_tab_all_label, ScreenName.ALL_CATALOG);

  @LayoutRes
  private final int mContainerLayoutId;
  @LayoutRes
  private final int mItemLayoutId;
  @StringRes
  private final int mTabLabelId;
  @NonNull
  private final ScreenName mAnalyticsScreenName;

  Tabs( @LayoutRes int containerLayoutId, @LayoutRes int itemLayoutId, @StringRes int tabLabelId, @NonNull ScreenName analyticsEvent )
  {
    mContainerLayoutId = containerLayoutId;
    mItemLayoutId = itemLayoutId;
    mTabLabelId = tabLabelId;
    mAnalyticsScreenName = analyticsEvent;
  }

  Fragment createFragment()
  {
    if (equals(MY_DICTIONARIES))
    {
     return MyDictionariesFragment.newInstanceMyDictionaries();
    }
    else
    {
      return DictionaryCardsFragment.newInstance(this);
    }
  }

  @LayoutRes
  int getContainerLayoutId()
  {
    return mContainerLayoutId;
  }

  @LayoutRes
  int getItemLayoutId()
  {
    return mItemLayoutId;
  }

  @StringRes
  int getTabLabelId()
  {
    return mTabLabelId;
  }

  @NonNull
  public ScreenName getAnalyticsScreenName()
  {
    return mAnalyticsScreenName;
  }

  public boolean isTabAvailable( DictionaryManagerAPI dictionaryManager )
  {
    DictionaryControllerAPI controller = dictionaryManager.createController(null);
    if ( null != controller && controller.getDictionaries().length <= 0 )
    {
      return false;
    }
    else if ( equals(Tabs.ALL) )
    {
      return true;
    }
    else if ( equals(Tabs.RECENTLY_VIEWED) )
    {
      IRecentlyOpenedFilter recentlyOpened =
          dictionaryManager.getFilterFactory().createByClass(IRecentlyOpenedFilter.class);
      return null != recentlyOpened && !recentlyOpened.getRecentlyOpened().isEmpty();
    }
    else if ( EnumSet.of(Tabs.FEATURED, Tabs.BESTSELLERS, Tabs.NEW, Tabs.DEALS, Tabs.MY_DICTIONARIES).contains(this) )
    {
      List< DictionaryFilter > dictionaryFilters = getDictionaryFilter(dictionaryManager.getFilterFactory());
      if ( !dictionaryFilters.isEmpty() && null != controller )
      {
        for ( DictionaryFilter filter : dictionaryFilters )
        {
          controller.installFilter(filter);
        }

        boolean available = controller.getDictionaries().length != 0;
        for ( DictionaryFilter filter : dictionaryFilters )
        {
          controller.uninstallFilter(filter);
        }
        return available;
      }
      return false;
    }
    return false;
  }

  @NonNull
  public List<DictionaryFilter> getDictionaryFilter( IFilterFactory filterFactory )
  {
    EnumSet< FilterTypeSimple > types = getFilterTypeSimple();
    if (!types.isEmpty())
    {
      List<DictionaryFilter> filters = new ArrayList<>();
      for ( FilterTypeSimple typeSimple : types )
      {
        DictionaryFilter filter = filterFactory.createByType(typeSimple);
        if (null != filter)
        {
          filters.add(filter);
        }
      }
      return filters;
    }
    else
    {
      DictionaryFilter filterByClass;
      if ( equals(RECENTLY_VIEWED) && null != (filterByClass = filterFactory.createByClass(IRecentlyOpenedFilter.class)))
      {
        return Collections.singletonList(filterByClass);
      }
    }
    return Collections.emptyList();
  }

  @NonNull
  private EnumSet< FilterTypeSimple > getFilterTypeSimple()
  {
    switch ( this )
    {
      case MY_DICTIONARIES:
        return EnumSet.of(FilterTypeSimple.MY_DICTIONARIES);
      case FEATURED:
        return EnumSet.of(FilterTypeSimple.FEATURED);
      case BESTSELLERS:
        return EnumSet.of(FilterTypeSimple.BESTSELLERS);
      case NEW:
        return EnumSet.of(FilterTypeSimple.NEW_DICTIONARIES);
      case DEALS:
        return EnumSet.of(FilterTypeSimple.DEALS);
      case ALL:
      case RECENTLY_VIEWED:
      default:
        return EnumSet.noneOf(FilterTypeSimple.class);
    }
  }

  /**
   * @return null if cast impossible
   */
  public static Tabs fromSerializable( Serializable serializable )
  {
    try
    {
      return null == serializable ? null : (Tabs) serializable;
    }
    catch ( Exception e )
    {
      return null;
    }
  }
}
