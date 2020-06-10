package com.paragon_software.dictionary_manager.filter;

import androidx.annotation.Nullable;

import com.paragon_software.dictionary_manager.DictionaryFilter;

public interface IFilterFactory
{
  @Nullable
  < T extends DictionaryFilter > T createByClass( Class< T > cls );

  DictionaryFilter createByType( FilterTypeSimple type );
}
