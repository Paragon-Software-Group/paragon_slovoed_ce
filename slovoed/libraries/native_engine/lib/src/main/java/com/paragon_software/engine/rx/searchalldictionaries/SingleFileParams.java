package com.paragon_software.engine.rx.searchalldictionaries;

import androidx.annotation.NonNull;

import com.paragon_software.dictionary_manager.Dictionary;

final class SingleFileParams
{
  @NonNull
  final Dictionary dictionary;

  @NonNull
  final String query;

  final int maxWords;

  SingleFileParams( @NonNull Dictionary _dictionary, @NonNull String _query, int _maxWords )
  {
    dictionary = _dictionary;
    query = _query;
    maxWords = _maxWords;
  }
}
