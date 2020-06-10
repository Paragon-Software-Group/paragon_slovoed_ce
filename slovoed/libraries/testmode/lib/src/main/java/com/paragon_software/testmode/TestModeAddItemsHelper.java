package com.paragon_software.testmode;

import androidx.annotation.NonNull;
import androidx.collection.ArraySet;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.favorites_manager.FavoritesManagerAPI;
import com.paragon_software.history_manager.HistoryManagerAPI;
import com.paragon_software.native_engine.SearchEngineAPI;
import com.paragon_software.utils_slovoed.collections.CollectionView;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

/**
 * Class to simplify process of adding new items in managers (favorites_manager, history_manager).
 */
class TestModeAddItemsHelper implements CollectionView.OnProgressChanged
{
  private SearchEngineAPI     searchEngineAPI;
  private FavoritesManagerAPI favoritesManagerAPI;
  private HistoryManagerAPI   historyManagerAPI;

  private CollectionView<ArticleItem, Void> searchResult;
  private String                            forManager;
  private TestModeAPI.OnAddItemsInManager   addItemsCallback;

  void registerSearchEngine( @NonNull SearchEngineAPI searchEngineAPI )
  {
    this.searchEngineAPI = searchEngineAPI;
  }

  void registerFavoritesManager( @NonNull FavoritesManagerAPI favoritesManagerAPI )
  {
    this.favoritesManagerAPI = favoritesManagerAPI;
  }

  void registerHistoryManager( @NonNull HistoryManagerAPI historyManagerAPI )
  {
    this.historyManagerAPI = historyManagerAPI;
  }

  void addItemsInFavoritesBySearch( String query, TestModeAPI.OnAddItemsInManager callback )
  {
    addItemsCallback = callback;
    forManager = FavoritesManagerAPI.class.getSimpleName();
    searchItemsByFTS(query);
  }

  void addItemsInHistoryBySearch( String query, TestModeAPI.OnAddItemsInManager callback )
  {
    addItemsCallback = callback;
    forManager = HistoryManagerAPI.class.getSimpleName();
    searchItemsByFTS(query);
  }

  private void searchItemsByFTS(String query)
  {
    searchResult = searchEngineAPI.searchAll(query, 200).getArticleItemList();
    searchResult.registerListener(this);
  }

  @Override
  public void onProgressChanged()
  {
    if ( !searchResult.isInProgress() )
    {
      List<ArticleItem> foundWords = new ArrayList<>(getWordsCollectionFromSearchResult());
      List<ArticleItem> managerWords = new ArrayList<>();
      List<ArticleItem> oldManagerWords = new ArrayList<>();
      if ( forManager.equals(FavoritesManagerAPI.class.getSimpleName()) )
      {
        oldManagerWords = new ArrayList<>(favoritesManagerAPI.getWords());
        favoritesManagerAPI.addWordsForTestmode(foundWords);
        managerWords = favoritesManagerAPI.getWords();
      }
      else if ( forManager.equals(HistoryManagerAPI.class.getSimpleName()) )
      {
        oldManagerWords = new ArrayList<>(historyManagerAPI.getWords());
        historyManagerAPI.addWords(foundWords);
        managerWords = historyManagerAPI.getWords();
      }
      Collection<ArticleItem> newManagerWords = new ArrayList<>(managerWords);
      newManagerWords.removeAll(oldManagerWords);
      addItemsCallback.onItemsAddedBySearch(foundWords.size(), newManagerWords.size(), managerWords.size());
      searchResult.unregisterListener(this);
    }
  }

  private Collection<ArticleItem> getWordsCollectionFromSearchResult()
  {
    Collection<ArticleItem> foundWords = new ArraySet<>();
    for ( int i = 0; i < searchResult.getCount(); i++)
    {
      ArticleItem articleItem = searchResult.getItem(i);
      foundWords.add(articleItem);
    }
    return foundWords;
  }
}
