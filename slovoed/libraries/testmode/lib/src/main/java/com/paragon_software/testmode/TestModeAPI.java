package com.paragon_software.testmode;


import android.app.Activity;
import android.content.Context;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.util.List;

import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.dictionary_manager.errors.ErrorType;
import com.paragon_software.favorites_manager.FavoritesManagerAPI;
import com.paragon_software.history_manager.HistoryManagerAPI;
import com.paragon_software.native_engine.SearchEngineAPI;
import com.paragon_software.news_manager.NewsManagerAPI;

public interface TestModeAPI
{
  void registerTestModeUI( @NonNull Class activityClass );

  boolean isTestModeKeyword( @NonNull String text );

  void startTestModeUI( @NonNull Activity activity )
      throws UnsupportedOperationException;

  void registerDictionaryManager( @NonNull DictionaryManagerAPI dictionaryManagerAPI );

  void registerSearchEngine( @NonNull SearchEngineAPI searchEngineAPI );

  void registerFavoritesManager( @NonNull FavoritesManagerAPI favoritesManagerAPI );

  void registerHistoryManager( @NonNull HistoryManagerAPI historyManagerAPI );

  void registerNewsManager( @NonNull NewsManagerAPI newsManagerAPI );

  void changeNewsTestModeState( boolean isTestMode );
  boolean getNewsTestModeState();

  @Nullable
  DictionaryManagerAPI getDictionaryManager();

  List< Dictionary > getDictionariesForConsume()
      throws UnsupportedOperationException;

  ErrorType consumeDictionaryPurchase( Context appContext, Dictionary dictionary )
      throws UnsupportedOperationException;

  void addItemsInFavoritesBySearch( String query, OnAddItemsInManager callback );

  void addItemsInHistoryBySearch( String query, OnAddItemsInManager callback );

  interface OnAddItemsInManager
  {
    void onItemsAddedBySearch( int found, int added, int size );
  }
}
