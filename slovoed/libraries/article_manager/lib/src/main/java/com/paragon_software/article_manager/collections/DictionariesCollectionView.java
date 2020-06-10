package com.paragon_software.article_manager.collections;

import androidx.annotation.Nullable;

import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryAndDirection;
import com.paragon_software.utils_slovoed.collections.BasicCollectionView;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class DictionariesCollectionView extends BasicCollectionView< Dictionary, Void >
{
  private List< Dictionary > dictionaries = new ArrayList<>();

  public DictionariesCollectionView()
  {
  }

  @Override
  public int getCount()
  {
    return dictionaries.size();
  }

  @Override
  public Dictionary getItem( int position )
  {
    return dictionaries.get(position);
  }

  public void update( @Nullable Dictionary[] dictionariesArray, @Nullable DictionaryAndDirection dnd )
  {
    if ( null == dictionariesArray || dictionariesArray.length <= 0 )
    {
      return;
    }
    List< Dictionary > dictionaries = Arrays.asList(dictionariesArray);
    if ( !this.dictionaries.equals(dictionaries) )
    {
      callOnItemRangeChanged(OPERATION_TYPE.ITEM_RANGE_REMOVED, 0, this.dictionaries.size());
      this.dictionaries = dictionaries;
      callOnItemRangeChanged(OPERATION_TYPE.ITEM_RANGE_INSERTED, 0, this.dictionaries.size());
    }
    updateSelection(getSelectedDictionaryId(dnd));
  }

  public void updateSelection( Dictionary.DictionaryId id )
  {
    if ( null == id )
    {
      return;
    }

    int index = dictionaryIndexOf(id);
    if ( (index >= 0) && (index != getSelection()) )
    {
      updateSelection(index);
    }
  }

  @Nullable
  private Dictionary.DictionaryId getSelectedDictionaryId( @Nullable DictionaryAndDirection dnd )
  {
    Dictionary.DictionaryId dictionaryId = null;
    if ( null != dnd )
    {
      for ( Dictionary dictionary : dictionaries )
      {
        if ( dictionary.getId().equals(dnd.getDictionaryId()) )
        {
          dictionaryId = dnd.getDictionaryId();
        }
      }
    }
    if ( null == dictionaryId && !dictionaries.isEmpty() )
    {
      dictionaryId = dictionaries.get(0).getId();
    }
    return dictionaryId;
  }

  private int dictionaryIndexOf( Dictionary.DictionaryId id )
  {
    for ( int i = 0 ; i < dictionaries.size() ; i++ )
    {
      if ( id.equals(dictionaries.get(i).getId()) )
      {
        return i;
      }
    }
    return Integer.MIN_VALUE;
  }

  @Override
  public int getSelection()
  {
    int selection = super.getSelection();
    if((selection < 0) || (selection >= getCount()))
      selection = -1;
    return selection;
  }
}

