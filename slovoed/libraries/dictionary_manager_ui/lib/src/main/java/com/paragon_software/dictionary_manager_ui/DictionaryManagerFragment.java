/*
 * dictionary manager ui
 *
 *  Created on: 14.03.18
 *      Author: Mikhail Belyshov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

package com.paragon_software.dictionary_manager_ui;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.inputmethod.InputMethodManager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentManager;
import androidx.viewpager.widget.ViewPager;

import com.google.android.material.tabs.TabLayout;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryControllerAPI;
import com.paragon_software.dictionary_manager.DictionaryFragmentAPI;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.dictionary_manager.DictionaryManagerHolder;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class DictionaryManagerFragment extends DictionaryFragmentAPI
                                       implements DictionaryManagerAPI.IDictionaryListObserver
{

  private RecentlyViewedHandler mRecentlyViewedHandler;
  private SectionsPagerAdapter adapter;
  private ViewPager viewPager;

  @Override
  public void onCreate(@Nullable Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    DictionaryManagerHolder.getManager().loadOnlineDictionaryStatusInformation(this);
  }

  @Nullable
  @Override
  public View onCreateView( @NonNull LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState )
  {
    mRecentlyViewedHandler = new RecentlyViewedHandler(inflater.getContext().getApplicationContext());
    View view = inflater.inflate(R.layout.fragment_dictionary_manager, container, false);
    initViewPager(view, inflater);
    TabLayout tabLayout = view.findViewById(R.id.tabs);
    tabLayout.clearOnTabSelectedListeners();
    tabLayout.setupWithViewPager(viewPager);
    listenForTabsUpdate(viewPager, adapter);
    DictionaryManagerHolder.getManager().registerDictionaryListObserver(this);
    return view;
  }

  private void initViewPager( @NonNull View rootView, @NonNull LayoutInflater inflater )
  {
    viewPager = rootView.findViewById(R.id.tab_container);
    adapter = new SectionsPagerAdapter(inflater.getContext(), getChildFragmentManager());
    viewPager.setAdapter(adapter);
    viewPager.addOnPageChangeListener(new ViewPager.OnPageChangeListener() {
      public void onPageScrollStateChanged(int state) {}
      public void onPageScrolled(int position, float positionOffset, int positionOffsetPixels) {}
      public void onPageSelected(int position)
      {
        hideKeyboard();
      }
    });
  }

  private void hideKeyboard()
  {
    InputMethodManager imm = (InputMethodManager) getActivity().getSystemService(Activity.INPUT_METHOD_SERVICE);
    imm.hideSoftInputFromWindow(getView().getRootView().getWindowToken(), 0);
  }

  private void listenForTabsUpdate( final ViewPager viewPager, final SectionsPagerAdapter adapter )
  {
    mRecentlyViewedHandler.registerReceiver(new BroadcastReceiver()
    {
      @Override
      public void onReceive( Context context, Intent intent )
      {
        int currentItem = viewPager.getCurrentItem();
        adapter.update();
        Tabs tab = Tabs.fromSerializable(intent.getSerializableExtra(RecentlyViewedHandler.KEY_CURRENT_ACTIVE_TAB));
        if ( null != tab )
        {
          int activeTabPosition = adapter.getTabPosition(tab);
          viewPager.setCurrentItem(activeTabPosition >= 0 ? activeTabPosition : currentItem);
        }
      }
    });
  }

  @Override
  public void onDestroyView()
  {
    super.onDestroyView();
    mRecentlyViewedHandler.unregisterReceiversAll();
    DictionaryManagerHolder.getManager().unRegisterDictionaryListObserver(this);
  }

  @Override
  public void onDictionaryListChanged()
  {
    if (null != adapter)
    {
      adapter.update();
    }
  }

  @Override
  public void selectMyDictionariesTab( Dictionary.DictionaryId dictionaryId )
  {
    DictionaryControllerAPI myDictionariesController =
        DictionaryManagerHolder.getManager().createController(MyDictionariesFragment.MY_DICTIONARIES_CONTROLLER_TAG);
    if ( null != myDictionariesController )
    {
      myDictionariesController.setDictionaryIdSelectedInMyDictionaries(dictionaryId);
    }

    if ( null != adapter && null != viewPager )
    {
      int position = adapter.getTabPosition(Tabs.MY_DICTIONARIES);
      if ( position >= 0 )
      {
        viewPager.setCurrentItem(position);
      }
    }
  }

  private static class SectionsPagerAdapter extends ParagonPagerAdapter
  {
    private List< Tabs >        tabs      = new ArrayList<>();
    private Map< Tabs, String > tabTitles = new HashMap<>();

    SectionsPagerAdapter( Context context, FragmentManager fm )
    {
      super(fm);
      fillTabTitles(context);
      fillAvailableTabs();
    }

    private void fillTabTitles( Context context )
    {
      for ( Tabs tab : Tabs.values() )
      {
        tabTitles.put(tab, context.getString(tab.getTabLabelId()));
      }
    }

    private void fillAvailableTabs()
    {
      DictionaryManagerAPI dictionaryManager = DictionaryManagerHolder.getManager();
      for ( Tabs t : Tabs.values() )
      {
        if ( t.isTabAvailable(dictionaryManager) )
        {
          tabs.add(t);
        }
      }
    }

    @Override
    public Fragment getItem( int position )
    {
      Fragment res = null;
      if ( ( position >= 0 ) && ( position < tabs.size() ) )
      {
        res = tabs.get(position).createFragment();
      }
      return res;
    }

    @Override
    public int getCount()
    {
      return tabs.size();
    }

    @Override
    public int getItemPosition( @NonNull Object object )
    {
      return POSITION_NONE;
    }

    @Nullable
    @Override
    public CharSequence getPageTitle( int position )
    {
      String res = null;
      if ( ( position >= 0 ) && ( position < tabs.size() ) )
      {
        res = tabTitles.get(tabs.get(position));
      }
      return res;
    }

    public void update()
    {
      tabs = new ArrayList<>();
      fillAvailableTabs();
      notifyDataSetChanged();
    }

    public int getTabPosition( Tabs tab )
    {
      return tabs.indexOf(tab);
    }
  }
}
