package com.paragon_software.dictionary_manager.filter;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.dictionary_manager.filter.external.ILanguageFilter;
import com.paragon_software.settings_manager.SettingsManagerAPI;
import com.paragon_software.settings_manager.exceptions.LocalResourceUnavailableException;
import com.paragon_software.settings_manager.exceptions.ManagerInitException;
import com.paragon_software.settings_manager.exceptions.WrongTypeException;

import java.util.Collection;
import java.util.LinkedList;

class LanguageFilter implements ILanguageFilter
{
  private static final boolean needBackup = false;
  @NonNull
  private DictionaryManagerAPI dictionaryManager;
  @NonNull
  private String               settingsKeyFrom;
  @NonNull
  private String               settingsKeyTo;
  @Nullable
  private SettingsManagerAPI   settingsManager;
  @Nullable
  private Integer              from;
  @Nullable
  private Integer              to;
  private Collection< FilterStateChangedListener > listeners = new LinkedList<>();

  LanguageFilter( @NonNull DictionaryManagerAPI dictionaryManager, @NonNull String settingsKey,
                  @Nullable SettingsManagerAPI settingsManager )
  {
    this.dictionaryManager = dictionaryManager;
    this.settingsKeyFrom = settingsKey + "_from";
    this.settingsKeyTo = settingsKey + "_to";
    this.settingsManager = settingsManager;
    loadState();
  }

  @Nullable
  @Override
  public Integer getInitialLanguageFrom()
  {
    return from;
  }

  @Nullable
  @Override
  public Integer getInitialLanguageTo()
  {
    return to;
  }

  /**
   * @param from if null - means 'any'
   * @param to if null - means 'any'
   */
  @Override
  public void setLanguages( @Nullable Integer from, @Nullable Integer to )
  {
    this.from = from;
    this.to = to;
    saveState();
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
    for ( Dictionary d : dictionaryManager.getDictionaries() )
    {
      if ( dictionaryId.equals(d.getId()) )
      {
        return applyForLanguage(d);
      }
    }
    return false;
  }

  private boolean applyForLanguage( @NonNull Dictionary d )
  {
    boolean res = true;
    if((from != null) || (to != null))
      res = false;
      for(Dictionary.Direction direction : d.getDirections()) {
        boolean applyFrom = true;
        if(from != null)
          applyFrom = (from == direction.getLanguageFrom());
        boolean applyTo = true;
        if(to != null)
          applyTo = (to == direction.getLanguageTo());
        if(applyFrom && applyTo) {
          res = true;
          break;
        }
      }
    return res;
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
      settingsManager.save(settingsKeyFrom, null == from ? "" : from, needBackup);
      settingsManager.save(settingsKeyTo, null == to ? "" : to, needBackup);
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

  private void loadState()
  {
    if ( null == settingsManager )
    {
      return;
    }

    try
    {
      int f = settingsManager.load(settingsKeyFrom, 0);
      int t = settingsManager.load(settingsKeyTo, 0);
      from = f == 0 ? null : f;
      to = t == 0 ? null : t;
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
}
