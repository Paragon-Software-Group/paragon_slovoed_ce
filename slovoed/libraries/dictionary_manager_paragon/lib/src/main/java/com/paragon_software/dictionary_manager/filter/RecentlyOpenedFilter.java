package com.paragon_software.dictionary_manager.filter;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.util.*;

import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryFilter;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.dictionary_manager.filter.external.IRecentlyOpenedFilter;
import com.paragon_software.settings_manager.SettingsManagerAPI;
import com.paragon_software.settings_manager.exceptions.LocalResourceUnavailableException;
import com.paragon_software.settings_manager.exceptions.ManagerInitException;
import com.paragon_software.settings_manager.exceptions.WrongTypeException;

public class RecentlyOpenedFilter implements IRecentlyOpenedFilter
{
  private static final boolean                                  needBackup = false;
  private static final int                                      THRESHOLD  = 10;
  private              Collection< FilterStateChangedListener > listeners  = new LinkedList<>();
  @NonNull
  private String               settingsKey;
  @Nullable
  private SettingsManagerAPI   settingsManager;
  private LinkedList< Dictionary.DictionaryId > recentlyOpened = new LinkedList<>();

  RecentlyOpenedFilter( @NonNull DictionaryManagerAPI dictionaryManager, @NonNull String settingsKey,
                        @Nullable SettingsManagerAPI settingsManager )
  {
    this.settingsKey = settingsKey;
    this.settingsManager = settingsManager;
    loadState();
  }

  private void loadState()
  {
    if ( null == settingsManager )
    {
      return;
    }

    try
    {
      Dictionary.DictionaryId[] restoredDictionaryIds =
          settingsManager.load(settingsKey, new Dictionary.DictionaryId[]{});
      recentlyOpened = new LinkedList<>(Arrays.asList(restoredDictionaryIds));
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

  @Nullable
  public static RecentlyOpenedFilter cast( DictionaryFilter filter )
  {
    try
    {
      return (RecentlyOpenedFilter) filter;
    }
    catch ( Exception e )
    {
      return null;
    }
  }

  @Override
  public void addRecentlyOpened( @NonNull Dictionary.DictionaryId dictionaryId )
  {
    while ( recentlyOpened.remove(dictionaryId) )
    {
    }
    recentlyOpened.add(0, dictionaryId);
    if ( recentlyOpened.size() > THRESHOLD )
    {
      recentlyOpened.removeLast();
    }
    saveState();
    notifyStateChangedListeners();
  }

  @NonNull
  @Override
  public List< Dictionary.DictionaryId > getRecentlyOpened()
  {
    return new LinkedList<>(recentlyOpened);
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
    return recentlyOpened.contains(dictionaryId);
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
      Dictionary.DictionaryId[] dictionaryIds = recentlyOpened.toArray(new Dictionary.DictionaryId[]{});
      settingsManager.save(settingsKey, dictionaryIds, needBackup);
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
