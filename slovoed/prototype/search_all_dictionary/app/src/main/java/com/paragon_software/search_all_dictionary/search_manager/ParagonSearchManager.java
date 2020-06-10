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

import android.databinding.Observable;
import android.databinding.ObservableArrayList;
import android.databinding.ObservableField;
import android.support.annotation.NonNull;

import java.util.Date;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.atomic.AtomicReference;

import com.paragon_software.engine.MainThreadRunner;
import com.paragon_software.engine.NativeEngine;

public class ParagonSearchManager extends SearchManager
{
  @NonNull
  private final Map< String, ParagonSearchController > searchControllers = new HashMap<>();

  @NonNull
  private final AtomicReference< ParagonSearchController > activeController = new AtomicReference<>(null);

  @NonNull
  private final NativeEngine engine;

  public ParagonSearchManager( @NonNull NativeEngine engine )
  {
    this.engine = engine;
  }

  private void search( @NonNull final String word )
  {
    engine.search(word, new OnSearchStatus()
    {
      @Override
      public void onSearchStatus( final boolean newList, @NonNull final SearchResult result )
      {
        final ParagonSearchController controller = activeController.get();
        if ( null != controller )
        {
          if ( newList || controller.searchResults.size() < 1000 )
          {
            new MainThreadRunner< Void >()
            {
              @Override
              protected void onRun( Void param )
              {
                if ( newList )
                {
                  controller.searchResults.clear();
                }
                controller.searchResults.add(result);
              }
            }.run();
          }
        }
      }
    });
  }

  @NonNull
  @Override
  SearchController getController( @NonNull final String name )
  {
    ParagonSearchController controller = searchControllers.get(name);
    if ( null == controller )
    {
      controller = new ParagonSearchController();
      searchControllers.put(name, controller);
    }
    activeController.lazySet(controller);
    return controller;
  }

  public interface OnSearchStatus
  {
    void onSearchStatus( final boolean newList, @NonNull final SearchResult result );
  }

  class ParagonSearchController implements SearchController
  {
    private static final long                                ONLINE_SEARCH_PAUSE = 100;
    @NonNull
    private final        ObservableField< String >           searchText          = new ObservableField<>();
    @NonNull
    private final        ObservableArrayList< SearchResult > searchResults       = new ObservableArrayList<>();
    @NonNull
    private final        ObservableField< Boolean >          onlineSearch        = new ObservableField<>();
    Date[] lastSearch = { new Date(0) };

    ParagonSearchController()
    {
      onlineSearch.set(false);
      searchText.addOnPropertyChangedCallback(new Observable.OnPropertyChangedCallback()
      {
        @Override
        public void onPropertyChanged( Observable sender, int propertyId )
        {
          final Boolean isOnline = onlineSearch.get();
          if ( null != isOnline && isOnline && ( ( lastSearch[0].getTime() + ONLINE_SEARCH_PAUSE ) < new Date()
              .getTime() ) )
          {
            final String text = searchText.get();
            if ( null != text && text.length() > 1 )
            {
              lastSearch[0] = new Date();
              search(text);
            }
          }
        }
      });
    }

    @NonNull
    @Override
    public ObservableField< String > getSearchText()
    {
      return this.searchText;
    }

    @NonNull
    @Override
    public ObservableArrayList< SearchResult > getSearchResults()
    {
      return this.searchResults;
    }

    @NonNull
    @Override
    public ObservableField< Boolean > getOnlineSearch()
    {
      return onlineSearch;
    }

    @Override
    public void okInSoftKeyboard()
    {
      final String text = searchText.get();
      if ( null != text )
      {
        search(text);
      }
    }
  }
}
