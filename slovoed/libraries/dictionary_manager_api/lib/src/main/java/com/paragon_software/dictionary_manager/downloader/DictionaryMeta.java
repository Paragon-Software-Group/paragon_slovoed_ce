package com.paragon_software.dictionary_manager.downloader;

import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.utils_slovoed.text.LocalizedString;

public class DictionaryMeta
{
  private final Dictionary.DictionaryId id;
  private final LocalizedString         title;

  public DictionaryMeta( Dictionary dictionary )
  {
    id = dictionary.getId();
    title = dictionary.getTitle();
  }

  public Dictionary.DictionaryId getId()
  {
    return id;
  }

  public LocalizedString getTitle()
  {
    return title;
  }

  @Override
  public String toString()
  {
    return "DictionaryMeta{" + "id=" + id + ", title='" + title + '\'' + '}';
  }
}
