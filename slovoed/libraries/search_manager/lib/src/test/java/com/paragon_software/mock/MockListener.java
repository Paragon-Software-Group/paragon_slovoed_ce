package com.paragon_software.mock;

import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.search_manager.OnDictionaryListChanged;
import com.paragon_software.search_manager.OnDictionarySelect;

public abstract class MockListener implements OnDictionarySelect, OnDictionaryListChanged
{
  Dictionary.DictionaryId selectedDictionary          = null;
  int                     selectedDirection           = -1;
  int                     dictionaryListChangedCount  = 0;


  @Override
  public void onDictionarySelect(Dictionary.DictionaryId dictionaryId)
  {
    selectedDictionary = dictionaryId;
  }

  @Override
  public void onDirectionSelect(int direction)
  {
    selectedDirection = direction;
  }

  @Override
  public void onDictionaryListChanged() {
    ++dictionaryListChangedCount;
  }

  public Dictionary.DictionaryId getSelectedDictionary() {
    return selectedDictionary;
  }

  public int getSelectedDirection() {
    return selectedDirection;
  }

  public int getDictionaryListChangedCount() {
    return dictionaryListChangedCount;
  }
}