package com.paragon_software.dictionary_manager.filter.external;

import androidx.annotation.Nullable;

import com.paragon_software.dictionary_manager.DictionaryFilter;

public interface ILanguageFilter extends DictionaryFilter
{
  @Nullable
  Integer getInitialLanguageFrom();

  @Nullable
  Integer getInitialLanguageTo();

  void setLanguages( @Nullable Integer from, @Nullable Integer to );
}
