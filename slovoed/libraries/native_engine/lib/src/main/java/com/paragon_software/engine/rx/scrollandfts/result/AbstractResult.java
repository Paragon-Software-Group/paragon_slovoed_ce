package com.paragon_software.engine.rx.scrollandfts.result;

import androidx.annotation.Nullable;

import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.engine.nativewrapper.NativeDictionary;

public abstract class AbstractResult
{
  @Nullable
  public final Dictionary.DictionaryId dictionaryId;

  @Nullable
  public final NativeDictionary dictionary;

  AbstractResult( @Nullable Dictionary.DictionaryId _dictionaryId, @Nullable NativeDictionary _dictionary )
  {
    dictionaryId = _dictionaryId;
    dictionary = _dictionary;
  }
}
