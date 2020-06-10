package com.paragon_software.toolbar_manager.Notifiers;

import androidx.annotation.Nullable;

import com.paragon_software.dictionary_manager.Dictionary.DictionaryId;
import com.paragon_software.toolbar_manager.ToolbarManager;

public interface OnDictionarySelect extends ToolbarManager.Notifier
{
  void onDictionarySelect( @Nullable DictionaryId id );

  void onDirectionSelect( int directionId );
}
