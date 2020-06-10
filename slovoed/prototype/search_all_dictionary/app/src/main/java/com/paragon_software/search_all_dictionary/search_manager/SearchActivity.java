/*
 * SearchAllDictionary
 *
 *  Created on: 28.05.18
 *      Author: Ivan Zhdanov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

package com.paragon_software.search_all_dictionary.search_manager;

import android.app.Activity;
import android.content.Context;
import android.databinding.BindingAdapter;
import android.databinding.DataBindingUtil;
import android.databinding.ObservableArrayList;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.BaseAdapter;
import android.widget.ListAdapter;
import android.widget.ListView;
import android.widget.TextView;

import com.paragon_software.search_all_dictionary.ApplicationState;
import com.paragon_software.search_all_dictionary.R;
import com.paragon_software.search_all_dictionary.databinding.SearchActivityBinding;
import com.paragon_software.search_all_dictionary.databinding.SearchResultBinding;

public class SearchActivity extends Activity
{
  @BindingAdapter( "bind:items" )
  public static void bindList( final ListView view, @NonNull final ObservableArrayList< SearchResult > list )
  {
    ListAdapter adapter = new BaseAdapter()
    {
      @Nullable
      private LayoutInflater inflater = null;

      @Override
      public int getCount()
      {
        return list.size();
      }

      @NonNull
      @Override
      public Object getItem( int position )
      {
        return list.get(position);
      }

      @Override
      public long getItemId( int position )
      {
        return position;
      }

      @NonNull
      @Override
      public View getView( int position, View convertView, @NonNull ViewGroup parent )
      {
        if ( inflater == null )
        {
          inflater = (LayoutInflater) parent.getContext().getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        }
        if ( null != inflater )
        {

          SearchResultBinding binding = DataBindingUtil.inflate(inflater, R.layout.search_result, parent, false);
          binding.setResult(list.get(position));

          return binding.getRoot();
        }
        return convertView;
      }
    };
    view.setAdapter(adapter);
  }

  @BindingAdapter( "onOkInSoftKeyboard" ) // I like it to match the listener method name
  public static void setOnOkInSoftKeyboardListener( TextView view, final OnOkInSoftKeyboardListener listener )
  {
    if ( listener == null )
    {
      view.setOnEditorActionListener(null);
    }
    else
    {
      view.setOnEditorActionListener(new TextView.OnEditorActionListener()
      {
        @Override
        public boolean onEditorAction( TextView v, int actionId, KeyEvent event )
        {
          if ( actionId == EditorInfo.IME_ACTION_SEARCH )
          {
            final InputMethodManager imm =
                (InputMethodManager) v.getContext().getSystemService(Context.INPUT_METHOD_SERVICE);
            if ( null != imm )
            {
              imm.hideSoftInputFromWindow(v.getWindowToken(), 0);
            }
            listener.onOkInSoftKeyboard();
            return true;
          }
          return false;
        }
      });
    }
  }

  @Override
  protected void onCreate( Bundle savedInstanceState )
  {
    super.onCreate(savedInstanceState);
    ApplicationState.instance().init(this);
    SearchActivityBinding binding = DataBindingUtil.setContentView(this, R.layout.search_activity);
    binding.setSearch(ApplicationState.instance().getSearchManager().getController("main"));
  }

  public interface OnOkInSoftKeyboardListener
  {
    void onOkInSoftKeyboard();
  }
}
