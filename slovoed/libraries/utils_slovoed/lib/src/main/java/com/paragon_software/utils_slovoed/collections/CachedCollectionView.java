package com.paragon_software.utils_slovoed.collections;

import androidx.annotation.NonNull;

import java.util.ArrayList;
import java.util.List;

public class CachedCollectionView< ITEM_TYPE, LABEL_TYPE > extends BasicCollectionView< ITEM_TYPE, LABEL_TYPE >
{
  private static int MAX_RESULT_COUNT = 27;

  @NonNull
  private final List< List< ITEM_TYPE > > arrays = new ArrayList<>(MAX_RESULT_COUNT);

  @NonNull
  private final List< Integer > cumulativeSizes = new ArrayList<>(MAX_RESULT_COUNT);

  @Override
  public ITEM_TYPE getItem( int i )
  {
    int first = 0;
    int last = cumulativeSizes.size();
    while ( first < last )
    {
      int middle = ( first + last ) / 2;
      if ( cumulativeSizes.get(middle) <= i )
      {
        first = middle + 1;
      }
      else
      {
        last = middle;
      }
    }

    ITEM_TYPE res = null;
    if ( ( first < cumulativeSizes.size() ) && ( first < arrays.size() ) )
    {
      int delta = 0;
      if ( first > 0 )
      {
        delta = cumulativeSizes.get(first - 1);
      }
      res = arrays.get(first).get(i - delta);
    }
    return res;
  }

  @Override
  public int getCount()
  {
    int res = 0;
    int len = cumulativeSizes.size();
    if ( len > 0 )
    {
      res = cumulativeSizes.get(len - 1);
    }
    return res;
  }

  public void resetCache()
  {
    int previousTotalCount = getCount();
    arrays.clear();
    cumulativeSizes.clear();
    callOnItemRangeChanged(OPERATION_TYPE.ITEM_RANGE_REMOVED, 0, previousTotalCount);
  }

  public void cacheItems(@NonNull List< ITEM_TYPE > result )
  {
    int previousTotalCount = getCount();
    int resultSize = result.size();
    arrays.add(result);
    cumulativeSizes.add(previousTotalCount + resultSize);
    MAX_RESULT_COUNT = Math.max(MAX_RESULT_COUNT, arrays.size());
    callOnItemRangeChanged(OPERATION_TYPE.ITEM_RANGE_INSERTED, previousTotalCount, resultSize);
  }
}
