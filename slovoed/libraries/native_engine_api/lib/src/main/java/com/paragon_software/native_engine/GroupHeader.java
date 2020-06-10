package com.paragon_software.native_engine;

import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.utils_slovoed.text.LocalizedString;

public class GroupHeader
{
  public enum LIST_TYPE
  {
    BASE,
    HEADWORD,
    CONTENT,
    TRANSLATION,
    EXAMPLE,
    DEFINITION,
    PHRASE,
    IDIOM,
    LAST
  }

  final public Dictionary.DictionaryId id;

  final public LIST_TYPE type;

  final public int listTypeOffset;

  final public LocalizedString listName;

  final public int languageFrom;

  final public int languageTo;

  final public boolean isHeadwordVisible;

  public GroupHeader(Dictionary.DictionaryId id, LIST_TYPE type, int offset, LocalizedString name, int languageFrom,
                     int languageTo, boolean isHeadwordVisible )
  {
    this.id = id;
    this.type = type;
    this.listTypeOffset = offset;
    this.listName = name;
    this.languageFrom = languageFrom;
    this.languageTo = languageTo;
    this.isHeadwordVisible = isHeadwordVisible;
  }

}
