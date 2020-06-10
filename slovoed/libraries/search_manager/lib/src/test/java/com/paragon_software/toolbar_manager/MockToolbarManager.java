package com.paragon_software.toolbar_manager;

import androidx.annotation.NonNull;

import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.toolbar_manager.Notifiers.OnDictionarySelect;

public abstract class MockToolbarManager extends ToolbarManager
{
  OnDictionarySelect mOnDictionarySelectListener;
  public boolean mShowAllDictionaries = true;
  public int mSetSelectedDirectionCounter = 0;
  public Dictionary.Direction mSelectedDirection;
  public Dictionary.DictionaryId mCurrentDictionary  = null;
  public int mCurrentDirection = -1;

  public void changeDictionary( Dictionary.DictionaryId dictionaryId )
  {
    if (null == mOnDictionarySelectListener) return;
    mOnDictionarySelectListener.onDictionarySelect(dictionaryId);
  }

  @NonNull
  @Override
  public ToolbarController getController( @NonNull String s )
  {
    return null;
  }

  @Override
  public void showDictionaryList(boolean showAllDictionaries) {
    mShowAllDictionaries = showAllDictionaries;
  }

  @Override
  public void setSelectedDirection(Dictionary.Direction selectedDirection)
  {
    ++mSetSelectedDirectionCounter;
    mSelectedDirection = selectedDirection;
  }

  @Override
  public void registerNotifier( @NonNull Notifier notifier )
  {
    mOnDictionarySelectListener = (OnDictionarySelect) notifier;
  }

  @Override
  public Dictionary.DictionaryId getCurrentDictionary()
  {
    return mCurrentDictionary;
  }

  @Override
  public int getCurrentDirection()
  {
    return mCurrentDirection;
  }
}