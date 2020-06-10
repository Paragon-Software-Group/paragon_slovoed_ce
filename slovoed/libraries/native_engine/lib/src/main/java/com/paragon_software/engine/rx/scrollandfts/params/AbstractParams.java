package com.paragon_software.engine.rx.scrollandfts.params;

import androidx.annotation.Nullable;

import com.paragon_software.dictionary_manager.Dictionary;

import java.util.Collection;

public abstract class AbstractParams
{
  @Nullable
  public final Dictionary.DictionaryId dictionaryId;

  @Nullable
  public final Dictionary.Direction direction;

  @Nullable
  public final String word;

  @Nullable
  public final Collection<Dictionary.Direction> availableDirections;

  AbstractParams( @Nullable Dictionary.DictionaryId _dictionaryId, @Nullable Dictionary.Direction _direction,
                  @Nullable String _word, @Nullable Collection<Dictionary.Direction> _availableDirections )
  {
    dictionaryId = _dictionaryId;
    direction = _direction;
    word = _word;
    availableDirections = _availableDirections;
  }
}
