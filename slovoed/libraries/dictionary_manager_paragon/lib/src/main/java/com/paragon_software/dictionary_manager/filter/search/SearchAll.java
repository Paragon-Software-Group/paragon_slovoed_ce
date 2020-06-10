package com.paragon_software.dictionary_manager.filter.search;

import androidx.annotation.NonNull;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.native_engine.SearchEngineAPI;
import com.paragon_software.utils_slovoed.collections.CollectionView;
import com.paragon_software.utils_slovoed.collections.CollectionView.Notifier;

import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Set;

public class SearchAll
{
  private final static int maxWords = 1;
  private final Notifier        engineListener;
  private final SearchEngineAPI searchEngine;
  private final OnResultChanged onResultChanged;
  private Map< Dictionary.DictionaryId, ArticleItem > dictionaryIds = new LinkedHashMap<>();
  private CollectionView<ArticleItem, Void> collectionView;

  public SearchAll( @NonNull SearchEngineAPI searchEngine, @NonNull OnResultChanged onResultChanged )
  {
    this.searchEngine = searchEngine;
    this.onResultChanged = onResultChanged;
    this.engineListener = new EngineListener(this);
  }

  public void search( @NonNull String text )
  {
    if ( null != collectionView )
    {
      collectionView.unregisterListener(engineListener);
    }
    if ( text.length() != 0 )
    {
      collectionView = searchEngine.searchAll(text, maxWords).getArticleItemList();
      if ( null != collectionView )
      {
        collectionView.registerListener(engineListener);
        handleChanges();
      }
    }
    else
    {
      dictionaryIds = new LinkedHashMap<>();
      onResultChanged.onChanged();
    }
  }

  void handleChanges()
  {
    if ( null == collectionView )
    {
      return;
    }

    Map< Dictionary.DictionaryId, ArticleItem > dictionaryIds = new LinkedHashMap<>();
    int count = collectionView.getCount();
    for ( int i = 0 ; i < count ; i++ )
    {
      ArticleItem item = collectionView.getItem(i);
      if ( null != item )
      {
        dictionaryIds.put(item.getDictId(), item);
      }
    }

    if ( isDictionaryIdsChanged(dictionaryIds.keySet()) )
    {
      SearchAll.this.dictionaryIds = dictionaryIds;
      onResultChanged.onChanged();
    }

    // TODO: Uncomment, when method 'isInProgress' works properly
    /*
    if ( !collectionView.isInProgress() )
    {
      collectionView.unRegisterListener(engineListener);
    }
    */
  }

  public Map< Dictionary.DictionaryId, ArticleItem> getDictionaryIds()
  {
    return new LinkedHashMap<>(dictionaryIds);
  }

  private boolean isDictionaryIdsChanged( Set< Dictionary.DictionaryId > newDictionaryIds )
  {
    if ( dictionaryIds.size() != newDictionaryIds.size() )
    {
      return true;
    }
    for ( Dictionary.DictionaryId dictionaryId : newDictionaryIds )
    {
      if ( !this.dictionaryIds.keySet().contains(dictionaryId) )
      {
        return true;
      }
    }
    return false;
  }

  public interface OnResultChanged
  {
    void onChanged();
  }
}
