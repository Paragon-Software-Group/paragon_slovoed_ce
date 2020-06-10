package com.paragon_software.search_manager;

import android.app.Activity;

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
import com.paragon_software.testmode.TestModeAPI;
import com.paragon_software.utils_slovoed.collections.CollectionView;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public abstract class MockSearchManager extends ParagonSearchManager
{
  String mSearchText;
  String mSearchAllText;
  String mSearch;
  int mSearchDirecton = -1;
  int mSelectedDirection = -1;
  Dictionary.DictionaryId mSelectedDictionary;
  SearchEngineAPI mTestSearchEngine;
  Dictionary.Direction mSelectedDictionaryDirection;
  Dictionary.Direction mAutoSelectedDirection;
  List<Dictionary> mDictionaries;
  ArticleItem mPlaySoundArticle;
  boolean mItemHasSound;

  public MockSearchManager()
  {
    mDictionaries = new ArrayList<>();
    mItemHasSound = false;
  }

  @Override
  boolean hasSound(ArticleItem articleItem) {
    return mItemHasSound;
  }

  @Override
  void playSound(ArticleItem articleItem) {
    mPlaySoundArticle = articleItem;
  }

  @Override
  void setSelectedDirection(Dictionary.Direction selectedDirection) {
    mAutoSelectedDirection = selectedDirection;
  }

  @Override
  public void registerSearchEngine(SearchEngineAPI searchEngineAPI) {
    mTestSearchEngine = searchEngineAPI;
  }

  @Override
  List< Dictionary > getDictionaries() {
    return mDictionaries;
  }

  @Override
  ScrollResult scroll(int i, String text, boolean autoChangeDirection, boolean exactly)
  {
    mSearchText = text;
    mSearchDirecton = i;
    if (null != mTestSearchEngine)
    {
      return mTestSearchEngine.scroll(mSelectedDictionary,mSelectedDictionaryDirection,text,null, false);
    }
    else
    {
      return null;
    }
  }

  @Override
  SearchAllResult searchAll(String text) {
    mSearchAllText = text;
    if (null != mTestSearchEngine)
    {
      return mTestSearchEngine.searchAll(text,0);
    }
    else
    {
      return null;
    }
  }
    @Override
  CollectionView<CollectionView<ArticleItem, GroupHeader>,Dictionary.Direction> search(int direction, @NonNull String word, boolean autoChangeDirection, @NonNull SearchType searchType, @NonNull SortType sortType, @NonNull Boolean needRunSearch) {
    mSearchDirecton = direction;
    mSearch = word;
    if (null != mTestSearchEngine)
    {
      return mTestSearchEngine.search(mSelectedDictionary, mSelectedDictionaryDirection, word, Collections.singleton(mSelectedDictionaryDirection), searchType, sortType, false);
    }
    else
    {
      return null;
    }
  }

  @Override
  Dictionary.DictionaryId getSelectedDictionary()
  {
    return mSelectedDictionary;
  }

  @Override
  int getSelectedDirection() {
    return mSelectedDirection;
  }

  @Override
  public void saveSearchRequest(@NonNull String text) {
  }

  @Override
  public void saveIsScrollSelected(boolean isScrollSelected) {
  }

  @Override
  public String restoreSearchRequest() {
    return "";
  }

  @Override
  public boolean restoreIsScrollSelected() {
    return true;
  }

  @Override
  public boolean launchTestMode( @NonNull Activity activity, @NonNull String text )
  {
    return false;
  }

  @Override
  public void registerTestMode( @NonNull TestModeAPI testMode )
  {

  }
}
