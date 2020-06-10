package com.paragon_software.mock;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.native_engine.GroupHeader;
import com.paragon_software.native_engine.ScrollResult;
import com.paragon_software.native_engine.SearchAllResult;
import com.paragon_software.native_engine.SearchEngineAPI;
import com.paragon_software.native_engine.SearchType;
import com.paragon_software.native_engine.SortType;
import com.paragon_software.utils_slovoed.collections.CollectionView;

import org.mockito.Mockito;

import java.util.Collection;

public abstract class MockSearchEngine implements SearchEngineAPI
{
  public String                  mSearchString;
  public SearchType              mSearchType;
  public Dictionary.DictionaryId mSearchDictionaryId;
  public Dictionary.Direction    mSearchDirection;


  public MockSearchResult        mockSearchResult;
  public MockSearchAllResult     mockSearchAllResult;
  public CollectionView<CollectionView<ArticleItem, GroupHeader>, Dictionary.Direction>      mockSearch;

  public MockSearchEngine()
  {
    mockSearchResult = Mockito.spy(MockSearchResult.class);
    mockSearchAllResult = Mockito.spy(MockSearchAllResult.class);
    mockSearch = Mockito.spy(MockCollectionView.class);
  }

  @Override
  public SearchAllResult searchAll(String s, int i) {
    mSearchString = s;
    return mockSearchAllResult;
  }

  @Override
  public ScrollResult scroll(Dictionary.DictionaryId id, Dictionary.Direction direction, String word, @Nullable Collection<Dictionary.Direction> availableDirections, boolean exactly) {
    mSearchDictionaryId = id;
    mSearchDirection = direction;
    mSearchString = word;
    return mockSearchResult;
  }

  @Override
  public CollectionView<CollectionView<ArticleItem, GroupHeader>, Dictionary.Direction> search(Dictionary.DictionaryId id, Dictionary.Direction direction, String word, @Nullable Collection<Dictionary.Direction> availableDirections, @NonNull SearchType searchType, @NonNull SortType sortType, @NonNull Boolean needRunSearch) {
    mSearchDictionaryId = id;
    mSearchDirection = direction;
    mSearchString = word;
    mSearchType = searchType;
    return mockSearch;
  }
}
