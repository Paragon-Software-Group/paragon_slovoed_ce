package com.paragon_software.dictionary_manager.filter.external;

import androidx.annotation.NonNull;

import java.util.List;

import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryFilter;

public interface IRecentlyOpenedFilter extends DictionaryFilter
{
  void addRecentlyOpened( @NonNull Dictionary.DictionaryId dictionaryId );

  @NonNull
  List< Dictionary.DictionaryId > getRecentlyOpened();
}
