/*
 * SearchAllDictionary
 *
 *  Created on: 28.05.18
 *      Author: Ivan Zhdanov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

package com.paragon_software.search_all_dictionary.search_manager;

import android.support.annotation.NonNull;
import android.support.annotation.Nullable;

import com.paragon_software.search_all_dictionary.dictionary_manager.Dictionary;

public final class SearchResult
{
  @Nullable
  final private String word;

  @Nullable
  private Dictionary.DictionaryOptions dictionary = null;

  public SearchResult( @Nullable final String word )
  {
    this.word = word;
  }

  @NonNull
  public String getWord()
  {
    if ( null != dictionary )
    {
      return "[" + dictionary.getDictionaryLanguagePairShort() + "] " + word;
    }
    return "[-----] " + word;
  }

  @Nullable
  public Dictionary.DictionaryOptions getDictionary()
  {
    return dictionary;
  }

  public void setDictionary( @NonNull final Dictionary.DictionaryOptions dictionary )
  {
    this.dictionary = dictionary;
  }
}
