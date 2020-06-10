package com.paragon_software.toolbar_manager.Notifiers;

import com.paragon_software.dictionary_manager.Dictionary;

import java.util.ArrayList;

public abstract class MockForController_DictionarySelectNotifier
{

  private final ArrayList<OnDictionarySelect> mDictionarySelects = new ArrayList<>();

  public void registerNotifier( OnDictionarySelect notifier )
  {
    if (!mDictionarySelects.contains(notifier))
    {
      mDictionarySelects.add(notifier);
    }
  }

  public void changeSelectedDictionary( Dictionary.DictionaryId dictionaryId )
  {
    for (final OnDictionarySelect listener : mDictionarySelects)
    {
      listener.onDictionarySelect(dictionaryId);
    }
  }
}