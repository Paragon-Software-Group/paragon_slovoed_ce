package com.paragon_software.dictionary_manager;

import java.util.ArrayList;

public abstract class MockForManager_DictionaryListChangedNotifier
{

  private final ArrayList< DictionaryManagerAPI.IDictionaryListObserver > mOnDictionaryListListeners = new ArrayList<>();

  public void registerNotifier( DictionaryManagerAPI.IDictionaryListObserver notifier )
  {
    if (!mOnDictionaryListListeners.contains(notifier))
    {
      mOnDictionaryListListeners.add(notifier);
    }
  }

  public void onDictionaryListChanged()
  {
    for (final DictionaryManagerAPI.IDictionaryListObserver listener : mOnDictionaryListListeners)
    {
      listener.onDictionaryListChanged();
    }
  }
}