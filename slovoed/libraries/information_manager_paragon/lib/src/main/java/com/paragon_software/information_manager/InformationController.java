package com.paragon_software.information_manager;

import android.content.Context;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.native_engine.SearchEngineAPI;
import com.paragon_software.utils_slovoed.collections.CachedCollectionView;
import com.paragon_software.utils_slovoed.collections.CollectionView;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

class InformationController extends InformationControllerAPI implements DictionaryManagerAPI.IDictionaryListObserver
{
  @NonNull
  private final InformationManagerAPI                               informationManager;
  @NonNull
  private final DictionaryManagerAPI                                dictionaryManager;
  @NonNull
  private final SearchEngineAPI                                     searchEngine;
  @NonNull
  private       CachedCollectionView< InformationItem, Void >       collectionVew;
  @Nullable
  private       ListenerAdditionalItems                             listenerAdditionalItems;
  @Nullable
  private       Map< InformationItemType, String >                  itemTypes;

  InformationController( @NonNull InformationManagerAPI informationManager, @NonNull DictionaryManagerAPI dictionaryManager, @NonNull SearchEngineAPI searchEngine )
  {
    this.informationManager = informationManager;
    this.dictionaryManager = dictionaryManager;
    this.dictionaryManager.registerDictionaryListObserver(this);
    this.searchEngine = searchEngine;
    this.collectionVew = new CachedCollectionView<>();
  }

  @NonNull
  @Override
  public CollectionView< ? extends InformationItem, Void > getInformationItems(
      @NonNull Map< InformationItemType, String > types )
  {
    this.itemTypes = types;
    Dictionary.DictionaryId selectedDictionaryId = getSelectedDictionaryId();
    updateItemsInCollectionView(createInformationItems(this.itemTypes, selectedDictionaryId));
    if ( null != selectedDictionaryId )
    {
      listenAdditionalItems(this.itemTypes, selectedDictionaryId);
    }
    return collectionVew;
  }

  private void updateItemsInCollectionView( @NonNull List< InformationItem > informationItems )
  {
    List< InformationItem > orderedItems = orderItems(informationItems);

    if ( hasChanges(orderedItems) )
    {
      collectionVew.resetCache();
      collectionVew.cacheItems(orderedItems);
    }
  }

  private List< InformationItem > orderItems( List< InformationItem > informationItems )
  {
    List< InformationItem > items = new ArrayList<>(informationItems);
    List< InformationItem > ordered = new ArrayList<>();
    if ( null != itemTypes )
    {
      for ( InformationItemType type : itemTypes.keySet() )
      {
        Iterator< InformationItem > iterator = items.iterator();
        while ( iterator.hasNext() )
        {
          InformationItem informationItem = iterator.next();
          if ( informationItem.getType().equals(type) )
          {
            ordered.add(informationItem);
            iterator.remove();
          }
        }
      }
    }
    return ordered;
  }

  private boolean hasChanges( @NonNull List< InformationItem > informationItems )
  {
    if ( informationItems.size() != collectionVew.getCount() )
    {
      return true;
    }

    for ( int i = 0 ; i < collectionVew.getCount() ; i++ )
    {
      if ( !informationItems.get(i).equals(collectionVew.getItem(i)) )
      {
        return true;
      }
    }
    return false;
  }

  private void listenAdditionalItems( @NonNull Map< InformationItemType, String > types,
                                      @NonNull Dictionary.DictionaryId selectedDictionaryId )
  {
    CollectionView< ArticleItem, Void > additionalArticles =
        searchEngine.getAdditionalArticles(selectedDictionaryId);

    if ( null != listenerAdditionalItems )
    {
      listenerAdditionalItems.stopListenPrevious();
    }

    if ( null != additionalArticles && types.containsKey(InformationItemType.ADDITIONAL_ARTICLES) )
    {
      listenerAdditionalItems = new ListenerAdditionalItems(additionalArticles, selectedDictionaryId);
      listenerAdditionalItems.startListen();
    }
  }

  @NonNull
  private List< InformationItem > createInformationItems( @NonNull Map< InformationItemType, String > types,
                                                          @Nullable Dictionary.DictionaryId selectedDictionaryId )
  {
    List< InformationItem > result = new ArrayList<>();
    for ( InformationItemType type : types.keySet() )
    {
      InformationItem informationItem = InformationItem.create(type, types.get(type), selectedDictionaryId);
      if ( null != informationItem )
      {
        result.add(informationItem);
      }
    }
    return result;
  }

  @Override
  public void openInformationItem(@NonNull Context context, @NonNull InformationItem item )
  {
    informationManager.openInformationItem(context, item);
  }

  @Nullable
  private Dictionary.DictionaryId getSelectedDictionaryId()
  {
    Dictionary.DictionaryId dictionaryId = null;
    if ( !dictionaryManager.isSelectAllDictionaries()
        && dictionaryManager.getDictionaryAndDirectionSelectedByUser() != null )
    {
      dictionaryId = dictionaryManager.getDictionaryAndDirectionSelectedByUser().getDictionaryId();
    }
    return dictionaryId;
  }

  @Override
  public void onDictionaryListChanged() {
    if (this.itemTypes != null) {
      getInformationItems(itemTypes);
    }
  }

  private class ListenerAdditionalItems implements CollectionView.OnItemRangeChanged
  {
    @NonNull
    private final CollectionView< ArticleItem, Void > additionalArticles;
    @NonNull
    private final Dictionary.DictionaryId selectedDictionaryId;

    ListenerAdditionalItems( @NonNull CollectionView< ArticleItem, Void > additionalArticles,
                             @NonNull Dictionary.DictionaryId selectedDictionaryId )
    {
      this.additionalArticles = additionalArticles;
      this.selectedDictionaryId = selectedDictionaryId;
    }

    @Override
    public void onItemRangeChanged( CollectionView.OPERATION_TYPE type, int startPosition, int itemCount )
    {
      if ( itemCount > 0 )
      {
        List< InformationItem > informationItems = new ArrayList<>();
        if ( null != itemTypes )
        {
          informationItems.addAll(createInformationItems(itemTypes, selectedDictionaryId));
        }
        informationItems.addAll(createAdditionalInformationItems());
        updateItemsInCollectionView(informationItems);
      }
    }

    private List< InformationItem > createAdditionalInformationItems()
    {
      List< InformationItem > informationItems = new ArrayList<>();
      for ( int i = 0 ; i < additionalArticles.getCount() ; i++ )
      {
        ArticleItem articlesItem = additionalArticles.getItem(i);
        if ( null != articlesItem )
        {
          informationItems.add(InformationItem.createAdditionalArticleItem(articlesItem));
        }
      }
      return informationItems;
    }

    void stopListenPrevious()
    {
      this.additionalArticles.unregisterListener(this);
    }

    void startListen()
    {
      this.additionalArticles.registerListener(this);
    }
  }
}
