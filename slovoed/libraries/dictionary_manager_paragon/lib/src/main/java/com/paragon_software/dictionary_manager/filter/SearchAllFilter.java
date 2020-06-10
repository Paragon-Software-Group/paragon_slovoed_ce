package com.paragon_software.dictionary_manager.filter;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.util.*;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.dictionary_manager.filter.external.ISearchFilter;
import com.paragon_software.dictionary_manager.filter.search.SearchAll;
import com.paragon_software.native_engine.SearchEngineAPI;
import com.paragon_software.settings_manager.SettingsManagerAPI;
import com.paragon_software.settings_manager.exceptions.LocalResourceUnavailableException;
import com.paragon_software.settings_manager.exceptions.ManagerInitException;
import com.paragon_software.settings_manager.exceptions.WrongTypeException;

class SearchAllFilter implements ISearchFilter, SearchAll.OnResultChanged
{
  private final static boolean needBackup = false;
  private final SearchAll            searchAll;
  private       DictionaryManagerAPI dictionaryManager;
  private       String               settingsKey;
  private       SearchEngineAPI      searchEngine;
  @Nullable
  private       SettingsManagerAPI   settingsManager;
  private String                                      searchText      = "";
  private Collection< FilterStateChangedListener >    listeners       = new LinkedList<>();
  private Map< Dictionary.DictionaryId, ArticleItem > textAvailableIn = new HashMap<>();

  SearchAllFilter( @NonNull DictionaryManagerAPI dictionaryManager, @NonNull String settingsKey,
                   @Nullable SettingsManagerAPI settingsManager, SearchEngineAPI searchEngine )
  {
    this.dictionaryManager = dictionaryManager;
    this.settingsKey = settingsKey;
    this.settingsManager = settingsManager;
    this.searchEngine = searchEngine;
    searchAll = new SearchAll(searchEngine, this);
    loadState();
    searchAll.search(searchText);
  }

  private void loadState()
  {
    if ( null == settingsManager )
    {
      return;
    }

    try
    {
      searchText = settingsManager.load(settingsKey, "");
    }
    catch ( WrongTypeException e )
    {
      e.printStackTrace();
    }
    catch ( ManagerInitException e )
    {
      e.printStackTrace();
    }
    catch ( Exception e )
    {
      e.printStackTrace();
    }
  }

  @NonNull
  @Override
  public String getInitialSearchString()
  {
    return searchText;
  }

  @Override
  public ArticleItem getArticleItem( Dictionary.DictionaryId dictionaryId )
  {
    return textAvailableIn.get(dictionaryId);
  }

  @Override
  public void setSearchString( @NonNull String searchText )
  {
    this.searchText = searchText;
    saveState();
    searchAll.search(searchText);
  }

  @Override
  public void onChanged()
  {
    textAvailableIn = searchAll.getDictionaryIds();
    notifyStateChangedListeners();
  }

  private void notifyStateChangedListeners()
  {
    for ( FilterStateChangedListener listener : new LinkedList<>(listeners) )
    {
      listener.onStateChanged();
    }
  }

  @Override
  public boolean apply( @NonNull Dictionary.DictionaryId dictionaryId )
  {
    return searchText.length() == 0 || textAvailableIn.keySet().contains(dictionaryId);
  }

  @Override
  public void saveState()
  {
    if ( null == settingsManager )
    {
      return;
    }

    try
    {
      settingsManager.save(settingsKey, null == searchText ? "" : searchText, needBackup);
    }
    catch ( ManagerInitException e )
    {
      e.printStackTrace();
    }
    catch ( LocalResourceUnavailableException e )
    {
      e.printStackTrace();
    }
    catch ( Exception e )
    {
      e.printStackTrace();
    }
  }

  @Override
  public void registerListener( @NonNull FilterStateChangedListener filterStateChangedListener )
  {
    listeners.add(filterStateChangedListener);
  }

  @Override
  public void unregisterListener( @NonNull FilterStateChangedListener filterStateChangedListener )
  {
    listeners.remove(filterStateChangedListener);
  }

}
