package com.paragon_software.article_manager;

import android.app.Activity;
import androidx.annotation.NonNull;

import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.utils_slovoed.ui_states.VisibilityState;
import com.paragon_software.utils_slovoed.collections.CollectionView;
import com.paragon_software.utils_slovoed.text.SubstringInfo;

public interface ShareControllerAPI
{
  void showDictionariesFetchUI();

  void setInitiallyQuery( String query );

  @NonNull
  CollectionView< Dictionary, Void > getDictionaries();

  @NonNull
  CollectionView< CollectionView< ArticleItem, SubstringInfo >, Void > getWords();

  void selectDictionary( @NonNull Dictionary.DictionaryId dictionaryId );

  void selectWord( int wordIndex ) throws IndexOutOfBoundsException;

  void selectTab( int tabIndex ) throws IndexOutOfBoundsException;

  @NonNull
  DirectionButtonViewInfo getSwitchDirectionButtonViewInfo();

  @NonNull
  VisibilityState getDictionariesSpinnerVisibility();

  @NonNull
  VisibilityState getDictionariesFetchUIVisibility();

  @NonNull
  VisibilityState getDictionariesMissingPurchasesUIVisibility();

  @NonNull
  VisibilityState getDictionariesMissingFullBaseUIVisibility();

  void switchDirection();

  void registerNotifier( @NonNull Notifier notifier );

  void unregisterNotifier( @NonNull Notifier notifier );

  void openDictionarySearch( Activity activity );

  void hideDictionariesFetchUI();

  void hideMissingFullBaseUI();

  void hideMissingPurchasesUI();

  interface Notifier
  {
  }

  interface OnSwitchDirectionButtonViewStateChangedListener extends Notifier
  {
    void onSwitchDirectionButtonViewStateChanged();
  }

  interface OnDictionariesSpinnerVisibilityChangedListener extends Notifier
  {
    void onDictionariesSpinnerVisibilityChanged();
  }

  interface OnDictionariesFetchUIVisibilityChangedListener extends Notifier
  {
    void onDictionariesFetchUIVisibilityChanged();
  }

  interface OnDictionariesMissingPurchasesUIVisibilityListener extends Notifier
  {
    void onDictionariesMissingPurchasesUIVisibility();
  }

  interface OnDictionariesMissingFullBaseUIVisibilityListener extends Notifier
  {
    void onDictionariesMissingFullBaseUIVisibility();
  }
}
