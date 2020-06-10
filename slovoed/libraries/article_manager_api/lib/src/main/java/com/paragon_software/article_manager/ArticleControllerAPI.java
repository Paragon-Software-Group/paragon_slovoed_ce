package com.paragon_software.article_manager;

import android.app.Activity;
import android.content.Context;
import android.graphics.Bitmap;
import android.text.SpannableString;
import android.util.Pair;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.VisibleForTesting;
import androidx.fragment.app.FragmentManager;

import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.hint_manager.HintParams;
import com.paragon_software.hint_manager.HintType;
import com.paragon_software.utils_slovoed.ui_states.ButtonState;
import com.paragon_software.utils_slovoed.ui_states.VisibilityState;

import io.reactivex.Observable;

import static androidx.annotation.VisibleForTesting.PACKAGE_PRIVATE;

@VisibleForTesting(otherwise = PACKAGE_PRIVATE)
public interface ArticleControllerAPI
{
  void nextTranslation( @Nullable ArticleItem articleItem );

  void nextTranslation( @Nullable ArticleItem articleItem, @Nullable ShowArticleOptions showArticleOptions );

  void setActive();

  void free();

  @Nullable
  String getDictionaryTitle();

  @Nullable
  Bitmap getDictionaryIcon();

  String getDictionaryPrice();

  @Nullable
  String getArticleData();

  @Nullable
  String getArticleShowVariantText();

  @Nullable
  String getArticleFtsAnchor();

  @NonNull
  int[] getQueryHighlightData( @NonNull String articleText );

  @Nullable
  String getPopupData();

  @NonNull
  VisibilityState getOpenMyDictionariesVisibility();

  @NonNull
  VisibilityState getDemoBannerVisibility();

  @NonNull
  VisibilityState getTrialStatusVisibility();

  @NonNull
  VisibilityState getArticleVisibility();

  @NonNull
  ButtonState getFavoritesButtonState();

  @NonNull
  ButtonState getFlashcardButtonState();

  @NonNull
  ButtonState getBackButtonState();

  @NonNull
  ButtonState getForwardButtonState();

  @NonNull
  ButtonState getSoundButtonState();

  @NonNull
  VisibilityState getSearchUIVisibility();

  @NonNull
  String getSearchUIInitialText();

  @NonNull
  String getSearchUIResultText();

  @NonNull
  ButtonState getFindNextButtonState();

  @NonNull
  ButtonState getFindPreviousButtonState();

  @NonNull
  ButtonState getSearchUIButtonState();

  @NonNull
  ButtonState getHideOrSwitchBlocksButtonState();

  @NonNull
  ButtonState getGoToHistoryButtonState();

  int getTrialLengthInMinutes();

  float getArticleScale();
  boolean isPinchToZoomEnabled();
  boolean isShowHighlightingEnabled();

  boolean isMyViewEnabled();
  boolean isHidePronunciations();
  boolean isHideExamples();
  boolean isHidePictures();
  boolean isHideIdioms();
  boolean isHidePhrasalVerbs();

  void buy( @NonNull Activity activity );

  void openMyDictionariesUI( Activity activity );

  void openDownloadManagerUI( Context context );

  void nextTranslation( @NonNull String article, int language );

  void nextTranslation(Context context,  int listId, int globalIdx, @Nullable String label );

  void translatePopup( int listId, int globalIdx );

  void removePopup();

  void addToFavorites();

  void addToFavorites(FragmentManager fragmentManager);

  void removeFromFavorites();

  void back();

  void forward();

  void playCurrentSound();

  void playSound( int soundIdx );

  void playSound(String soundBaseIdx, String soundKey);

  void playSound( String language, String dictId, int listId, String key );

  void toggleSearchUI( boolean show );

  void setPlaySoundChannel( @NonNull String playSoundChannel );

  void playSoundOnline(String soundBaseIdx, String soundKey);

  void registerSearchTarget( @NonNull SearchTarget searchTarget );

  void unregisterSearchTarget( @NonNull SearchTarget searchTarget );

  void search( @NonNull String text );

  void findNext();

  void findPrevious();

  void toggleHideOrSwitchBlocksButtonCheckState(boolean checked);

  void setArticleScale( float newScale );

  void registerNotifier( @NonNull Notifier notifier );

  void unregisterNotifier( @NonNull Notifier notifier );

  void saveState();

  @Nullable
  DictionaryManagerAPI getDictionaryManager();

  void setDictionaryIconVisibility( @NonNull VisibilityState visibilityState );

  void setDictionaryTitleVisibility( @NonNull VisibilityState visibilityState );

  @NonNull
  VisibilityState getDictionaryIconVisibility();

  @NonNull
  VisibilityState getDictionaryTitleVisibility();

  boolean showHistoryScreen( @Nullable Context context );

  void openPractisePronunciationScreenActivity(@NonNull Context context );
  void openPractisePronunciationScreen(@NonNull Context context);

  void swipe( boolean leftToRight );

  boolean isCurrentArticle(int listId, int globalIndex);

  @Nullable
  SpannableString getRunningHeadsHeader(boolean leftToRight);

  @NonNull
  ButtonState getRunningHeadsButtonState(boolean leftToRight);

  boolean getFullBaseStatus(Dictionary.DictionaryId dictionaryId);

  boolean isPreviewMode();

  boolean isNeedCrossRef();
  void addToFlashcards( @NonNull Context context );

  void removeFromFlashcards( @NonNull Context context );

  @Nullable
  Dictionary.DictionaryId getCurrentDictionaryId();

  boolean isExternalBaseDownloaded(@Nullable String baseId);

  boolean isNeedReturnPreview();

  boolean showHintManagerDialog( @NonNull HintType hintType, @Nullable FragmentManager fragmentManager, @Nullable HintParams hintParams );
  Observable< Pair<HintType, HintParams> > getNeedToShowHintObservable();

  boolean isNeedOpenPictures();

  void openMorphologicalTable(Context context);

  ButtonState getMorphoTableButtonState();

  boolean isMorphoTable();

  interface Notifier
  {
  }
}
