package com.paragon_software.engine.rx.scrollandfts.result;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.engine.nativewrapper.NativeDictionary;

import java.util.List;

public class ScrollSearchResult extends AbstractResult
{
  public final int listIndex;
  public final int wordIndex;
  public final List<Integer> baseFormIndexList;
  public final Dictionary.Direction resultDirection;

  public ScrollSearchResult(@Nullable Dictionary.DictionaryId _dictionaryId,
							@Nullable NativeDictionary _dictionary,
							int _listIndex,
							int _wordIndex,
							@NonNull List<Integer> _baseFormIndexList,
							@Nullable Dictionary.Direction _resultDirection)
  {
    super(_dictionaryId, _dictionary);
    listIndex = _listIndex;
    wordIndex = _wordIndex;
    baseFormIndexList =_baseFormIndexList;
    resultDirection = _resultDirection;
  }
}
