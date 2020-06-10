package com.paragon_software.search_manager;

import com.paragon_software.dictionary_manager.Dictionary;

public interface OnDictionarySelect extends SearchController.Notifier
{
  void onDictionarySelect( Dictionary.DictionaryId id );

  void onDirectionSelect( int directionId );
}
