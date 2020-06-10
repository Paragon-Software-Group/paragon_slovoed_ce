package com.paragon_software.engine.views;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.engine.nativewrapper.NativeDictionary;
import com.paragon_software.utils_slovoed.collections.BasicCollectionView;

abstract public class NativeCollectionView< LABEL_TYPE > extends BasicCollectionView< ArticleItem, LABEL_TYPE >
{
  protected NativeDictionary dictionary = null;
  Dictionary.DictionaryId dictionaryId;
  int                     listIndex;

  private int listCurrentSize;

  public boolean isOpen()
  {
    return dictionary != null;
  }

  public void open( Dictionary.DictionaryId _dictionaryId, NativeDictionary _dictionary, int _listIndex )
  {
    if ( isOpen() )
    {
      close();
    }
    dictionaryId = _dictionaryId;
    dictionary = _dictionary;
    listIndex = _listIndex;
    listCurrentSize = dictionary.getListCurrentSize(listIndex);
    openHelper();
    callOnItemRangeChanged(OPERATION_TYPE.ITEM_RANGE_INSERTED, 0, listCurrentSize);
  }

  public int getListIndex()
  {
    return listIndex;
  }

  public void close()
  {
    if ( isOpen() )
    {
      dictionary = null;
      callOnItemRangeChanged(OPERATION_TYPE.ITEM_RANGE_REMOVED, 0, listCurrentSize);
    }
  }

  @Override
  public ArticleItem getItem( int i )
  {
    ArticleItem res = null;
    if ( isOpen() && ( i >= 0 ) && ( i < listCurrentSize ) )
    {
      res = getItemHelper(i);
    }
    return res;
  }

  @Override
  public int getCount()
  {
    int res = 0;
    if ( isOpen() )
    {
      res = listCurrentSize;
    }
    return res;
  }

  abstract void openHelper();

  abstract ArticleItem getItemHelper( int i );
}
