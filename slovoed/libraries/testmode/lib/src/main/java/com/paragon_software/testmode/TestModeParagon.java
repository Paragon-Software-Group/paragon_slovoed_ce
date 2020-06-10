package com.paragon_software.testmode;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.dictionary_manager.errors.ErrorType;
import com.paragon_software.favorites_manager.FavoritesManagerAPI;
import com.paragon_software.history_manager.HistoryManagerAPI;
import com.paragon_software.native_engine.SearchEngineAPI;
import com.paragon_software.news_manager.NewsManagerAPI;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;


public class TestModeParagon implements TestModeAPI
{
  private static final String TEST_MODE_KEYWORD = "*#testmode#";
  private DictionaryManagerAPI dictionaryManagerAPI;
  private Class                activityTestMode;

  private TestModeAddItemsHelper addItemsHelper = new TestModeAddItemsHelper();
  @Nullable
  private NewsManagerAPI mNewsManager;

  @Override
  public void registerTestModeUI( @NonNull Class activityClass )
  {
    this.activityTestMode = activityClass;
  }

  @Override
  public boolean isTestModeKeyword( @NonNull String text )
  {
    return text.trim().equals(TEST_MODE_KEYWORD);
  }

  @Override
  public void startTestModeUI( @NonNull Activity activity ) throws UnsupportedOperationException
  {
    if ( null == activityTestMode )
    {
      throw new UnsupportedOperationException();
    }
    TestModeHolderUI.set(this);
    Intent intent = new Intent(activity, activityTestMode);
    activity.startActivity(intent);
  }

  @Override
  public void registerDictionaryManager( @NonNull DictionaryManagerAPI dictionaryManagerAPI )
  {
    this.dictionaryManagerAPI = dictionaryManagerAPI;
  }

  @Override
  public void registerSearchEngine( @NonNull SearchEngineAPI searchEngineAPI )
  {
    addItemsHelper.registerSearchEngine(searchEngineAPI);
  }

  @Override
  public void registerFavoritesManager( @NonNull FavoritesManagerAPI favoritesManagerAPI )
  {
    addItemsHelper.registerFavoritesManager(favoritesManagerAPI);
  }

  @Override
  public void registerHistoryManager( @NonNull HistoryManagerAPI historyManagerAPI )
  {
    addItemsHelper.registerHistoryManager(historyManagerAPI);
  }

  @Override
  public void registerNewsManager(@NonNull NewsManagerAPI newsManagerAPI) {
    mNewsManager = newsManagerAPI;
  }

  @Override
  public void changeNewsTestModeState( boolean isTestMode ) {
    if (mNewsManager != null) {
      mNewsManager.changeTestModeState(isTestMode);
      mNewsManager.refreshNews();
    }
  }

  @Override
  public boolean getNewsTestModeState()
  {
    if (mNewsManager != null)
      return mNewsManager.getTestModeState();
    else
      return false;
  }

  @Nullable
  @Override
  public DictionaryManagerAPI getDictionaryManager()
  {
    return dictionaryManagerAPI;
  }

  @Override
  public List< Dictionary > getDictionariesForConsume() throws UnsupportedOperationException
  {
    if ( null == dictionaryManagerAPI )
    {
      throw new UnsupportedOperationException("Dictionary Manager not registered in Test Mode");
    }

    Collection< Dictionary > allDictionaries = dictionaryManagerAPI.getDictionaries();
    List< Dictionary > forConsume = new ArrayList<>();
    for ( Dictionary dictionary : allDictionaries )
    {
      Dictionary.DICTIONARY_STATUS status = dictionary.getStatus();
//      if (status.isPurchased() && (!status.isSubscription()))
//      {
//        forConsume.add(dictionary);
//      }
    }
    return forConsume;
  }

  /**
   * @return {@link ErrorType#OK} - Consume request successfully send. <br/>Possible errors: {@link ErrorType#PURCHASE_ITEM_UNAVAILABLE}, {@link ErrorType#PURCHASE_ITEM_NOT_OWNED}
   */
  @Override
  public ErrorType consumeDictionaryPurchase( Context appContext, Dictionary dictionary )
      throws UnsupportedOperationException
  {
    if ( null == dictionaryManagerAPI )
    {
      throw new UnsupportedOperationException("Dictionary Manager not registered in Test Mode");
    }

    return dictionaryManagerAPI.consume(appContext, dictionary);
  }

  @Override
  public void addItemsInFavoritesBySearch( String query, OnAddItemsInManager callback )
  {
    addItemsHelper.addItemsInFavoritesBySearch(query, callback);
  }

  @Override
  public void addItemsInHistoryBySearch( String query, OnAddItemsInManager callback )
  {
    addItemsHelper.addItemsInHistoryBySearch(query, callback);
  }
}
