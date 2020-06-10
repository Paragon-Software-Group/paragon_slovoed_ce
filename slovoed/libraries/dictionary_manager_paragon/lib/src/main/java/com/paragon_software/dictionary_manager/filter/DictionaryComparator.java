package com.paragon_software.dictionary_manager.filter;

import java.util.Comparator;
import java.util.HashMap;
import java.util.Map;

import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.Dictionary.DictionaryId;
import com.paragon_software.dictionary_manager.DictionaryFilter;

public class DictionaryComparator implements Comparator< Dictionary >
{

  private Map< DictionaryId, Integer > priority;

  public DictionaryComparator( Map< DictionaryId, Integer > priority )
  {

    this.priority = priority;
  }

  @Override
  public int compare( Dictionary left, Dictionary right )
  {
    Integer leftPriority = priority.get(left.getId());
    leftPriority = null == leftPriority ? 0 : leftPriority;
    Integer rightPriority = priority.get(right.getId());
    rightPriority = null == rightPriority ? 0 : rightPriority;
    return rightPriority - leftPriority;
  }

  public static Map< DictionaryId, Integer > getPriority( Dictionary[] dictionaries, DictionaryFilter filter )
  {
    Map< Dictionary.DictionaryId, Integer > result = new HashMap<>();
    RecentlyOpenedFilter recently = RecentlyOpenedFilter.cast(filter);
    if (null != recently) {
      int initialPriority = Integer.MAX_VALUE;
      for ( Dictionary.DictionaryId dictionaryId : recently.getRecentlyOpened() )
      {
        result.put(dictionaryId, initialPriority--);
      }
    }
    //TODO: Handle another types of filters
    return result;

  }
}
