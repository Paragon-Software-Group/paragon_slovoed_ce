package com.paragon_software.engine.rx.scrollandfts.params;

import androidx.annotation.Nullable;

import com.paragon_software.dictionary_manager.Dictionary;

import java.util.Collection;

public class ScrollParams extends AbstractParams
{
	public final boolean exactly;
  public ScrollParams( @Nullable Dictionary.DictionaryId _dictionaryId, @Nullable Dictionary.Direction _direction,
					   @Nullable String _word, @Nullable Collection<Dictionary.Direction> availableDirections, boolean exactly)
  {
    super(_dictionaryId, _direction, _word, availableDirections);
	  this.exactly = exactly;
  }
}
