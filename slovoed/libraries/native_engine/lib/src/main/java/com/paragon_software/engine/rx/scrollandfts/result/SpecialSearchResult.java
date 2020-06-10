package com.paragon_software.engine.rx.scrollandfts.result;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.engine.nativewrapper.NativeDictionary;

public class SpecialSearchResult extends AbstractResult
{
  @NonNull
  public final int[] listIndices;
  public final Dictionary.Direction resultDirection;

  @Nullable
  public final String word;

  public SpecialSearchResult( @Nullable Dictionary.DictionaryId _dictionaryId, @Nullable NativeDictionary _dictionary,
                    @NonNull int[] _listIndices,
                    @Nullable Dictionary.Direction _resultDirection, @Nullable String _word)
  {
    super(_dictionaryId, _dictionary);
    listIndices = _listIndices;
    resultDirection = _resultDirection;
    word = _word;
  }
}
