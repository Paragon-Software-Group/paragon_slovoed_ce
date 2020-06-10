package com.paragon_software.engine.rx.getwordreferenceinlist;

import androidx.annotation.NonNull;

import com.paragon_software.dictionary_manager.Dictionary;

public final class Params
{
  @NonNull
  final Dictionary.DictionaryId dictionaryId;

  @NonNull
  final Dictionary.Direction direction;

  @NonNull
  final String query;

  public Params( @NonNull Dictionary.DictionaryId dictionaryId, @NonNull Dictionary.Direction direction,
                 @NonNull String query )
  {
    this.dictionaryId = dictionaryId;
    this.direction = direction;
    this.query = query;
  }
}
