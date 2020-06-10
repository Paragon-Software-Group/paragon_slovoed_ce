package com.paragon_software.engine.rx.searchalldictionaries;

import androidx.annotation.NonNull;

import java.util.Collection;

import com.paragon_software.dictionary_manager.Dictionary;

final class Params
{
  @NonNull
  final Collection< Dictionary > dictionaries;

  @NonNull
  final String query;

  final int maxWords;

  Params( @NonNull Collection< Dictionary > _dictionaries, @NonNull String _query, int _maxWords )
  {
    dictionaries = _dictionaries;
    query = _query;
    maxWords = _maxWords;
  }
}
