package com.paragon_software.stubs;

import com.paragon_software.dictionary_manager.DictionaryManagerAPI;

public abstract class MockObserver implements DictionaryManagerAPI.IDictionaryListObserver
{
  @Override
  public void onDictionaryListChanged()
  {
  }
}