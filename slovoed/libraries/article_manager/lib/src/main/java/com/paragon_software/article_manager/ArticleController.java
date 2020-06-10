package com.paragon_software.article_manager;

import android.app.Activity;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Typeface;
import android.text.SpannableString;
import android.text.Spanned;
import android.text.style.RelativeSizeSpan;
import android.text.style.StyleSpan;
import android.text.style.SuperscriptSpan;
import android.util.Pair;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.VisibleForTesting;
import androidx.fragment.app.FragmentManager;

import com.paragon_software.analytics_manager.AnalyticsManagerAPI;
import com.paragon_software.analytics_manager.events.ProductDescriptionAndPricesScreenEvent;
import com.paragon_software.analytics_manager.events.ScreenName;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryControllerAPI;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.dictionary_manager.DictionaryPrice;
import com.paragon_software.dictionary_manager.IDictionaryIcon;
import com.paragon_software.dictionary_manager.components.DictionaryComponent;
import com.paragon_software.favorites_manager.FavoritesManagerAPI;
import com.paragon_software.favorites_manager.OnErrorListener;
import com.paragon_software.favorites_manager.OnStateChangedListener;
import com.paragon_software.flash_cards_manager.CardsChangedListener;
import com.paragon_software.flash_cards_manager.FlashcardManagerAPI;
import com.paragon_software.hint_manager.HintManagerAPI;
import com.paragon_software.hint_manager.HintParams;
import com.paragon_software.hint_manager.HintType;
import com.paragon_software.history_manager.HistoryManagerAPI;
import com.paragon_software.native_engine.ArticleSearcherAPI;
import com.paragon_software.native_engine.EngineArticleAPI;
import com.paragon_software.native_engine.HtmlBuilderParams;
import com.paragon_software.settings_manager.ApplicationSettings;
import com.paragon_software.settings_manager.OnApplicationSettingsSaveListener;
import com.paragon_software.settings_manager.SettingsManagerAPI;
import com.paragon_software.sound_manager.SoundManagerAPI;
import com.paragon_software.utils_slovoed.ui_states.ButtonState;
import com.paragon_software.utils_slovoed.ui_states.CheckState;
import com.paragon_software.utils_slovoed.ui_states.VisibilityState;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import io.reactivex.Observable;
import io.reactivex.subjects.PublishSubject;
import io.reactivex.subjects.Subject;

public class ArticleController implements ArticleControllerAPI, OnErrorListener, OnManagerArticleScaleChangedListener,
                                          OnApplicationSettingsSaveListener
{
  private static final int NOTIFIER_COUNT = 40;

  @NonNull
  private static final String HIDE_OR_SWITCH_BLOCKS_SETTINGS_NAME =
      "article_manager.hide_or_switch_blocks_initial_setting";
  @NonNull
  private static final String SEARCH_MODE_SETTINGS_PREFIX         = "article_manager.search_mode.";
  @NonNull
  private static final String SEARCH_TEXT_SETTINGS_PREFIX         = "article_manager.search_text.";

  @NonNull
  protected final ArticleManagerAPI mArticleManager;
  @NonNull
  protected final String            mControllerId;

  @Nullable
  protected final DictionaryManagerAPI mDictionaryManager;
  @Nullable
  final SoundManagerAPI                mSoundManager;
  @Nullable
  protected final FavoritesManagerAPI  mFavoritesManager;
  @Nullable
  protected final HistoryManagerAPI    mHistoryManager;
  @Nullable
  private final SettingsManagerAPI     mSettingsManager;
  @Nullable
  private final EngineArticleAPI       mEngine;
  @Nullable
  private final FlashcardManagerAPI    mFlashcardManager;
  @Nullable
  protected final HintManagerAPI       mHintManager;

  @NonNull
  private final ArticleTranslateCallback mArticleTranslateCallback = new ArticleTranslateCallback();
  @NonNull
  private final PopupTranslateCallback   mPopupTranslateCallback   = new PopupTranslateCallback();

  @NonNull
  protected final Article         mArticle         = new Article();
  @NonNull
  private final Popup             mPopup           = new Popup();
  @NonNull
  protected final DictionaryInfo  mDictionaryInfo  = new DictionaryInfo();
  @NonNull
  protected final TransitionState mTransitionState = new TransitionState();
  @VisibleForTesting
  protected boolean mHideOrSwitchBlocksSetting = true;

  @NonNull @VisibleForTesting
  protected final SearchState                    mSearchState          = new SearchState();
  @Nullable
  private       SearchTarget                     mSearchTarget         = null;
  @NonNull
  private final SearchTargetStateChangedListener mSearchTargetListener = new SearchTargetStateChangedListener();

  @NonNull
  private List< Notifier > mNotifiers               = new ArrayList<>(NOTIFIER_COUNT);
  private boolean          mInsideChangeStateMethod = false;

  @Nullable
  protected ArticleSearcherAPI               mArticleSearcher;
  @NonNull
  private VisibilityState mDictionaryIconVisibility  = VisibilityState.enabled;
  @NonNull
  private VisibilityState mDictionaryTitleVisibility = VisibilityState.enabled;

  private Subject< Pair<HintType, HintParams> > mNeedToShowHintSubject = PublishSubject.create();

  static ArticleControllerAPI create( @NonNull ArticleManagerAPI _articleManager, @NonNull String _controllerId,
                                      @Nullable DictionaryManagerAPI _dictionaryManager,
                                      @Nullable SoundManagerAPI _soundManager,
                                      @Nullable FavoritesManagerAPI _favoritesManager,
                                      @Nullable HistoryManagerAPI _historyManager,
                                      @Nullable SettingsManagerAPI _settingsManager, @Nullable EngineArticleAPI _engine,
                                      @Nullable FlashcardManagerAPI _flashcardManager, @Nullable HintManagerAPI _hintManager )
  {
    ArticleController articleController =
        new ArticleController(_articleManager, _controllerId, _dictionaryManager, _soundManager, _favoritesManager,
                              _historyManager, _settingsManager, _engine, _flashcardManager, _hintManager);
    initController(articleController);
    return articleController;
  }

  protected ArticleController( @NonNull ArticleManagerAPI _articleManager, @NonNull String _controllerId, @Nullable DictionaryManagerAPI _dictionaryManager,
                               @Nullable SoundManagerAPI _soundManager, @Nullable FavoritesManagerAPI _favoritesManager,
                               @Nullable HistoryManagerAPI _historyManager, @Nullable SettingsManagerAPI _settingsManager,
                               @Nullable EngineArticleAPI _engine, @Nullable FlashcardManagerAPI _flashcardManager, @Nullable HintManagerAPI _hintManager )
  {
    mArticleManager = _articleManager;
    mControllerId = _controllerId;
    mDictionaryManager = _dictionaryManager;
    mSoundManager = _soundManager;
    mFavoritesManager = _favoritesManager;
    mHistoryManager = _historyManager;
    mSettingsManager = _settingsManager;
    mEngine = _engine;
    mFlashcardManager = _flashcardManager;
    mHintManager = _hintManager;
  }

  protected static void initController(ArticleController articleController){
    articleController.mArticleManager.registerNotifier(articleController);
    articleController.mArticle.scale = articleController.mArticleManager.getArticleScale();
    if ( articleController.mSettingsManager != null )
    {
      ApplicationSettings applicationSettings = articleController.mSettingsManager.loadApplicationSettings();
      articleController.mArticle.usePinchToZoom = applicationSettings.isPinchToZoomEnabled();
      articleController.mArticle.showHighlighting = applicationSettings.isShowHighlightingEnabled();

      articleController.mArticle.myViewEnabled = applicationSettings.getMyView().isMyViewEnabled();
      articleController.mArticle.hidePronunciations = applicationSettings.getMyView().isHidePronunciations();
      articleController.mArticle.hideExamples = applicationSettings.getMyView().isHideExamples();
      articleController.mArticle.hidePictures = applicationSettings.getMyView().isHidePictures();
      articleController.mArticle.hideIdioms = applicationSettings.getMyView().isHideIdioms();
      articleController.mArticle.hidePhrasalVerbs = applicationSettings.getMyView().isHidePhrasalVerbs();
      articleController.mArticle.hideSoundIcons = applicationSettings.getHideSoundIcons().toArray(new String[0]);

      articleController.mSettingsManager.registerNotifier(articleController);
    }
    if ( articleController.mFavoritesManager != null )
    {
      articleController.mFavoritesManager.registerNotifier(articleController);
      articleController.mFavoritesManager.registerNotifier(articleController.new FavoritesStateChangedListener());
      articleController.mTransitionState.favoritesBusy = articleController.mFavoritesManager.isInTransition();
    }
    if ( articleController.mSoundManager != null )
    {
      articleController.mSoundManager.registerOnStateChangedListener(articleController.new SoundStateChangedListener());
      articleController.mTransitionState.soundBusy = articleController.mSoundManager.isPlaying();
    }
    if ( articleController.mDictionaryManager != null )
    {
      articleController.mDictionaryManager
              .registerDictionaryListObserver(articleController.new DictionaryListChangedListener());
    }
    if (articleController.mFlashcardManager != null )
    {
      articleController.mFlashcardManager.registerNotifier(articleController.new FlashcardStateChangedListener());
    }
    articleController.preloadSettings();
  }
  @Override
  public void nextTranslation( @Nullable final ArticleItem articleItem )
  {
    nextTranslation( articleItem, null );
  }

  @Override
  public void nextTranslation( @Nullable ArticleItem articleItem, @Nullable ShowArticleOptions showArticleOptions )
  {
    translationHelper(articleItem, ArticleTranslationKind.Next, showArticleOptions);
  }

  @Override
  public void setActive()
  {
    mArticleManager.setActiveArticleUI(mControllerId);
  }

  @Override
  public void free()
  {
    mArticleManager.freeArticleController(mControllerId);
  }

  @Nullable
  @Override
  public String getDictionaryTitle()
  {
    return mDictionaryInfo.title;
  }

  //TODO: get bitmap properly
  @Nullable
  @Override
  public Bitmap getDictionaryIcon()
  {
    Bitmap res = null;
    if ( mDictionaryInfo.icon != null )
    {
      res = mDictionaryInfo.icon.getBitmap();
    }
    return res;
  }

  @Override
  public String getDictionaryPrice()
  {
    String res = null;
    if ( mDictionaryInfo.price != null )
    {
      res = mDictionaryInfo.price.toString();
    }
    return res;
  }

  @Nullable
  @Override
  public String getArticleData()
  {
    return mArticle.data;
  }

  @Nullable
  @Override
  public String getArticleShowVariantText()
  {
    return mArticle.showVariantText;
  }

  @Nullable
  @Override
  public String getArticleFtsAnchor() {
    return mArticle.ftsAnchor;
  }

  @NonNull
  @Override
  public int[] getQueryHighlightData( @NonNull String articleText ) {
    ArticleItem currentItem = mArticle.stack.getCurrentItem();
    if ( mArticleSearcher != null && currentItem != null && currentItem.getFtsHeadword() != null
        && currentItem.getLabel() != null && currentItem.getSearchQuery() != null )
    {
      return mArticleSearcher.getQueryHighlightData(currentItem.getListId(), articleText, currentItem.getFtsHeadword(),
                                                    currentItem.getLabel(), currentItem.getSearchQuery());
    }
    return new int[0];
  }

  @Nullable
  @Override
  public String getPopupData()
  {
    return mPopup.data;
  }

  @VisibleForTesting
  protected void setPopupData( @Nullable String data)
  {
    mPopup.data = data;
  }

  @Nullable @VisibleForTesting
  protected ArticleItem getPopupItem()
  {
    return mPopup.item;
  }

  @VisibleForTesting
  protected void setPopupItem( @Nullable ArticleItem item)
  {
    mPopup.item = item;
  }

  @NonNull
  @Override
  public VisibilityState getOpenMyDictionariesVisibility()
  {
    boolean visible = ( null != mDictionaryInfo.edition
            && !mDictionaryInfo.isFullWordBaseAccessible
            && !isAdditional(mArticle.stack.getCurrentItem()));
    VisibilityState res;
    if ( !visible )
    {
      res = VisibilityState.gone;
    }
    else
    {
      res = mTransitionState.translation.mustDisableArticleButtons()
              ? VisibilityState.disabled : VisibilityState.enabled;
    }
    return res;
  }

  @NonNull
  @Override
  public VisibilityState getDemoBannerVisibility()
  {
    boolean visible = null != mDictionaryInfo.edition
            && mDictionaryInfo.edition.canBePurchased()
            && !isAdditional(mArticle.stack.getCurrentItem());
    VisibilityState res;
    if ( !visible )
    {
      res = VisibilityState.gone;
    }
    else
    {
      res =
          mTransitionState.translation.mustDisableArticleButtons()
                  ? VisibilityState.disabled : VisibilityState.enabled;
    }
    return res;
  }

  @NonNull
  @Override
  public VisibilityState getTrialStatusVisibility()
  {
    boolean visible = false;
    return visible ? VisibilityState.enabled : VisibilityState.gone;
  }

  @NonNull
  @Override
  public VisibilityState getArticleVisibility()
  {
    return ( mTransitionState.translation != TranslationKind.None ) ? VisibilityState.disabled
                                                                    : VisibilityState.enabled;
  }

  @NonNull
  @Override
  public ButtonState getFavoritesButtonState()
  {
    VisibilityState visibilityState = VisibilityState.gone;
    if ( !mSearchState.searchModeOn )
    {
      boolean isStateInvalid = mTransitionState.favoritesBusy
              || mTransitionState.translation.mustDisableArticleButtons()
              || mFavoritesManager == null
              || mArticle.stack.getCurrentItem() == null;
      visibilityState = isStateInvalid ? VisibilityState.disabled : VisibilityState.enabled;
    }
    CheckState checkState = mArticle.isArticleInFavorites ? CheckState.checked : CheckState.unchecked;
    return new ButtonState(visibilityState, checkState);
  }

  @NonNull
  @Override
  public ButtonState getFlashcardButtonState()
  {
    VisibilityState visibilityState = VisibilityState.gone;
    if ( !mSearchState.searchModeOn && mFlashcardManager != null )
    {
      if ( mArticle.canAddWordFlashcard || mArticle.canRemoveWordFlashcard )
      {
        boolean isStateInvalid = mTransitionState.translation.mustDisableArticleButtons()
                || mArticle.stack.getCurrentItem() == null;
        visibilityState = isStateInvalid ? VisibilityState.disabled : VisibilityState.enabled;
      }
    }

    CheckState checkState = !mArticle.canAddWordFlashcard && mArticle.canRemoveWordFlashcard
            ? CheckState.checked : CheckState.unchecked;
    return new ButtonState(visibilityState, checkState);
  }

  @NonNull
  @Override
  public ButtonState getBackButtonState()
  {
    VisibilityState visibilityState;
    if ( mArticle.stack.canGoBack() )
    {
      visibilityState = mTransitionState.translation.mustDisableNavigation()
                  ? VisibilityState.disabled : VisibilityState.enabled;
    }
    else
    {
      visibilityState = mArticle.stack.canGoForward()
              ? VisibilityState.disabled : VisibilityState.gone;
    }
    return new ButtonState(visibilityState, CheckState.uncheckable);
  }

  @NonNull
  @Override
  public ButtonState getForwardButtonState()
  {
    VisibilityState visibilityState;
    if ( mArticle.stack.canGoForward() )
    {
      visibilityState = mTransitionState.translation.mustDisableNavigation()
                  ? VisibilityState.disabled : VisibilityState.enabled;
    }
    else
    {
      visibilityState = mArticle.stack.canGoBack()
              ? VisibilityState.disabled : VisibilityState.gone;
    }
    return new ButtonState(visibilityState, CheckState.uncheckable);
  }

  @NonNull
  @Override
  public ButtonState getSoundButtonState()
  {
    VisibilityState visibilityState = VisibilityState.gone;
    if ( mArticle.hasSound && ( mSoundManager != null ) && ( !mSearchState.searchModeOn ) )
    {
      boolean isStateInvalid = mTransitionState.soundBusy
              || mTransitionState.translation.mustDisableArticleButtons();
      visibilityState = isStateInvalid ? VisibilityState.disabled : VisibilityState.enabled;
    }
    CheckState checkState = mTransitionState.soundBusy ? CheckState.checked : CheckState.unchecked;
    return new ButtonState(visibilityState, checkState);
  }

  @NonNull
  @Override
  public VisibilityState getSearchUIVisibility()
  {
    return mSearchState.searchModeOn ? VisibilityState.enabled : VisibilityState.gone;
  }

  @NonNull
  @Override
  public String getSearchUIInitialText()
  {
    return mSearchState.searchText;
  }

  @NonNull
  @Override
  public String getSearchUIResultText()
  {
    return mSearchState.searchResult;
  }

  @NonNull
  @Override
  public ButtonState getFindNextButtonState()
  {
    VisibilityState v = VisibilityState.gone;
    if ( mSearchState.searchModeOn )
    {
      v = mSearchState.canNavigateResults ? VisibilityState.enabled : VisibilityState.disabled;
    }
    return new ButtonState(v, CheckState.uncheckable);
  }

  @NonNull
  @Override
  public ButtonState getFindPreviousButtonState()
  {
    return getFindNextButtonState();
  }

  @NonNull
  @Override
  public ButtonState getSearchUIButtonState()
  {
    return new ButtonState(VisibilityState.enabled,
                           mSearchState.searchModeOn ? CheckState.checked : CheckState.unchecked);
  }

  @NonNull
  @Override
  public ButtonState getHideOrSwitchBlocksButtonState()
  {
    VisibilityState visibility = VisibilityState.gone;
    if ( mDictionaryInfo.hasHideOrSwitchBlocks && ( !mSearchState.searchModeOn ) )
    {
      visibility = ( mArticle.hasHideOrSwitchBlocks && ( mTransitionState.translation == TranslationKind.None ) )
                   ? VisibilityState.enabled : VisibilityState.disabled;
    }
    CheckState checkState = mHideOrSwitchBlocksSetting ? CheckState.checked : CheckState.unchecked;
    return new ButtonState(visibility, checkState);
  }

  @NonNull
  @Override
  public ButtonState getGoToHistoryButtonState()
  {
    return new ButtonState(mSearchState.searchModeOn ? VisibilityState.gone : VisibilityState.enabled
            , CheckState.uncheckable);
  }

  @Override
  public int getTrialLengthInMinutes()
  {
    return mDictionaryInfo.trialLengthInMinutes;
  }

  @Override
  public float getArticleScale()
  {
    return mArticle.scale;
  }

  @Override
  public boolean isPinchToZoomEnabled()
  {
    return mArticle.usePinchToZoom;
  }

  @Override
  public boolean isShowHighlightingEnabled()
  {
    return mArticle.showHighlighting;
  }

  @Override
  public boolean isMyViewEnabled()
  {
    return mArticle.myViewEnabled;
  }

  @Override
  public boolean isHidePronunciations()
  {
    return mArticle.hidePronunciations;
  }

  @Override
  public boolean isHideExamples()
  {
    return mArticle.hideExamples;
  }

  @Override
  public boolean isHidePictures()
  {
    return mArticle.hidePictures;
  }

  @Override
  public boolean isHideIdioms()
  {
    return mArticle.hideIdioms;
  }

  @Override
  public boolean isHidePhrasalVerbs()
  {
    return mArticle.hidePhrasalVerbs;
  }

  protected boolean isRemoveBodyMargin()
  {
      return false;
  }

    @Override
  public void buy( @NonNull Activity activity )
  {
    ArticleItem articleItem = mArticle.stack.getCurrentItem();
    if ( !mTransitionState.translation.mustDisableArticleButtons()
            && null != mDictionaryInfo.edition
            && mDictionaryInfo.edition.canBePurchased()
            && mDictionaryManager != null
            && articleItem != null )
    {
      ScreenName from = ScreenName.ARTICLE_DEMO;
      AnalyticsManagerAPI.get().logEvent(new ProductDescriptionAndPricesScreenEvent(from));
      mDictionaryManager.buy(activity, articleItem.getDictId());
    }
  }

  @Override
  public void openMyDictionariesUI( Activity activity )
  {
    ArticleItem articleItem = mArticle.stack.getCurrentItem();
    if ( !mTransitionState.translation.mustDisableArticleButtons()
            && null != mDictionaryInfo.edition
            && mDictionaryManager != null
            && articleItem != null )
    {
      mDictionaryManager.openMyDictionariesUI(activity, articleItem.getDictId());
    }
  }

  @Override
  public void openDownloadManagerUI( Context context )
  {
    ArticleItem articleItem = mArticle.stack.getCurrentItem();
    if ( ( mDictionaryManager != null ) && ( articleItem != null ) )
    {
      mDictionaryManager.openDownloadManagerUI(context, articleItem.getDictId());
    }
  }

  @Override
  public void nextTranslation( @NonNull String article, int language )
  {
    ArticleItem currentItem = mArticle.stack.getCurrentItem();
    if ( ( mArticleSearcher != null ) && ( currentItem != null ) )
    {
      ArticleItem next = mArticleSearcher.find(currentItem.getDictId(), article, language);
      if ( next != null )
      {
        ShowArticleOptions options = mArticle.stack.getCurrentShowArticleOptions();
//        If the translation was called from a SEPARATE LIST, then we need A_Z
        if (options != null && options.getSwipeMode() == SwipeMode.IN_SEPARATE_LIST) {
          options = new ShowArticleOptions(SwipeMode.A_Z_OF_DICTIONARY);
        }

        nextTranslation(next, options);
      }
    }
  }

  @Override
  public void nextTranslation(Context context,  int listId, int globalIdx, @Nullable String label )
  {
    ArticleItem currentItem = mArticle.stack.getCurrentItem();
    if ((mArticleSearcher != null) && (currentItem != null)) {
      ArticleItem next =
              mArticleSearcher.find(currentItem.getDictId(), listId, globalIdx, label);
      if (next != null) {
        if (isAdditional(next)) {
          mArticleManager.showArticleActivity(next, ArticleControllerType.CONTROLLER_ID_ADDITIONAL_INFO, context);
        } else {
          nextTranslation(next, mArticle.stack.getCurrentShowArticleOptions());
        }
      }
    }
  }

  @Override
  public void translatePopup( final int listId, final int globalIdx )
  {
    changeState(() ->
    {
      ArticleItem currentItem = mArticle.stack.getCurrentItem();
      if ( mEngine != null && currentItem != null
              && ( mTransitionState.translation == TranslationKind.Popup
                || mTransitionState.translation == TranslationKind.None ) )
      {
        ArticleItem popupItem = mEngine.find(currentItem.getDictId(), listId, globalIdx, null);
        if ( popupItem != null )
        {
          mTransitionState.translation = TranslationKind.Popup;
          if ( !mEngine.translateNext(currentItem, popupItem, getHtmlBuilderParams(), mPopupTranslateCallback) )
          {
            mTransitionState.translation = TranslationKind.None;
          }
        }
      }
    });
  }

  private HtmlBuilderParams getHtmlBuilderParams()
  {
    return createHtmlBuilderParamsBuilder().create();
  }

  protected HtmlBuilderParams.Builder createHtmlBuilderParamsBuilder()
  {
    boolean withScroll = mArticle.ftsAnchor != null && !mArticle.ftsAnchor.isEmpty();
    // if article with scroll then disable 'My View' settings
    boolean myViewEnabled = mArticle.myViewEnabled && !withScroll;
    return new HtmlBuilderParams.Builder().setScale(mArticle.scale)
                                          .setHidePhonetics(myViewEnabled && mArticle.hidePronunciations)
                                          .setHideExamples(myViewEnabled && mArticle.hideExamples)
                                          .setHideImages(myViewEnabled && mArticle.hidePictures)
                                          .setHideIdioms(myViewEnabled && mArticle.hideIdioms)
                                          .setHidePhrase(myViewEnabled && mArticle.hidePhrasalVerbs)
                                          .setHiddenSoundIcons(mArticle.hideSoundIcons)
                                          .setRemoveBodyMargin(isRemoveBodyMargin());
  }

  @Override
  public void removePopup()
  {
    changeState(this::discardPopup);
  }

  @Override
  public void addToFavorites()
  {
    ArticleItem currentArticle = mArticle.stack.getCurrentItem();
    if ( ( mFavoritesManager != null ) && ( currentArticle != null ) )
    {
      mFavoritesManager.addWord(currentArticle);
    }
  }

  @Override
  public void addToFavorites(FragmentManager fragment) {

  }

  @Override
  public void removeFromFavorites()
  {
    ArticleItem currentArticle = mArticle.stack.getCurrentItem();
    if ( ( mFavoritesManager != null ) && ( currentArticle != null ) )
    {
      mFavoritesManager.removeWord(currentArticle);
    }
  }

  @Override
  public void back()
  {
    if ( mArticle.stack.canGoBack() )
    {
      translationHelper(mArticle.stack.getPreviousItem(), ArticleTranslationKind.NavigationBack, mArticle.stack.getPreviousShowArticleOptions());
    }
  }

  @Override
  public void forward()
  {
    if ( mArticle.stack.canGoForward() )
    {
      translationHelper(mArticle.stack.getNextItem(), ArticleTranslationKind.NavigationForward, mArticle.stack.getNextShowArticleOptions());
    }
  }

  @Override
  public void playCurrentSound()
  {
    ArticleItem articleItem = mArticle.stack.getCurrentItem();
    if ( ( mSoundManager != null ) && ( articleItem != null ) )
    {
      mSoundManager.playSound(articleItem);
    }
  }

  @Override
  public void playSound( int soundIdx )
  {
    ArticleItem articleItem = mArticle.stack.getCurrentItem();
    if ( ( mSoundManager != null ) && ( articleItem != null ) )
    {
      mSoundManager.playSound(articleItem, soundIdx);
    }
  }

  @Override
  public void playSound(String soundBaseIdx, String soundKey)
  {
    ArticleItem articleItem = mArticle.stack.getCurrentItem();
    if ( mSoundManager != null
            && articleItem != null
            && isExternalBaseDownloaded( soundBaseIdx ) )
    {
      mSoundManager.playSound(articleItem, soundBaseIdx, soundKey);
    }
  }

  @Override
  public void playSound( String language, String dictId, int listId, String key )
  {
    ArticleItem articleItem = mArticle.stack.getCurrentItem();
    if ( ( mSoundManager != null ) && ( articleItem != null ) )
    {
      mSoundManager.playSound(articleItem, language, dictId, listId, key);
    }
  }

  @Override
  public void setPlaySoundChannel( @NonNull String playSoundChannel )
  {
    if( mSoundManager != null )
    {
      mSoundManager.setPlayChannel(playSoundChannel);
    }
  }

  @Override
  public void playSoundOnline( String soundBaseIdx, String soundKey )
  {

  }

  @Override
  public void toggleSearchUI( final boolean show )
  {
    changeState(() -> {
      if ( mSearchTarget != null )
      {
        mSearchTarget.clearMatches();
      }
      mSearchState.searchModeOn = show;
    });
  }

  @Override
  public void registerSearchTarget( @NonNull SearchTarget searchTarget )
  {
    if ( mSearchTarget != null )
    {
      unregisterSearchTarget(mSearchTarget);
    }
    mSearchTarget = searchTarget;
    mSearchTarget.registerListener(mSearchTargetListener);
    mSearchTargetListener.onSearchStateChanged();
  }

  @Override
  public void unregisterSearchTarget( @NonNull SearchTarget searchTarget )
  {
    if ( mSearchTarget == searchTarget )
    {
      mSearchTarget.unregisterListener(mSearchTargetListener);
      mSearchTarget = null;
      mSearchTargetListener.onSearchStateChanged();
    }
  }

  @Override
  public void search( @NonNull String text )
  {
    mSearchState.searchText = text;
    if ( mSearchState.searchModeOn && ( mSearchTarget != null ) )
    {
      mSearchTarget.search(text);
    }
  }

  @Override
  public void findNext()
  {
    if ( mSearchState.searchModeOn && ( mSearchTarget != null ) )
    {
      mSearchTarget.findNext();
    }
  }

  @Override
  public void findPrevious()
  {
    if ( mSearchState.searchModeOn && ( mSearchTarget != null ) )
    {
      mSearchTarget.findPrevious();
    }
  }

  @Override
  public void toggleHideOrSwitchBlocksButtonCheckState( boolean checked )
  {
    mHideOrSwitchBlocksSetting = checked;
  }

  @Override
  public void setArticleScale( final float newScale )
  {
    mArticleManager.setArticleScale(newScale);
  }

  @Override
  public synchronized void registerNotifier( @NonNull Notifier notifier )
  {
    if ( !mNotifiers.contains(notifier) )
    {
      mNotifiers.add(notifier);
    }
  }

  @Override
  public synchronized void unregisterNotifier( @NonNull Notifier notifier )
  {
    mNotifiers.remove(notifier);
  }

  @Override
  public void saveState()
  {
    Utils.saveSetting(mSettingsManager, HIDE_OR_SWITCH_BLOCKS_SETTINGS_NAME, mHideOrSwitchBlocksSetting);
    Utils.saveSetting(mSettingsManager, SEARCH_MODE_SETTINGS_PREFIX + mControllerId, mSearchState.searchModeOn);
    Utils.saveSetting(mSettingsManager, SEARCH_TEXT_SETTINGS_PREFIX + mControllerId, mSearchState.searchText);
    mArticleManager.saveState();
  }

  @Nullable
  @Override
  public DictionaryManagerAPI getDictionaryManager() {
    return mDictionaryManager;
  }

  @Override
  public void setDictionaryIconVisibility( @NonNull VisibilityState visibilityState )
  {
    mDictionaryIconVisibility = visibilityState;
  }

  @Override
  public void setDictionaryTitleVisibility( @NonNull VisibilityState visibilityState )
  {
    mDictionaryTitleVisibility = visibilityState;
  }

  @NonNull
  @Override
  public VisibilityState getDictionaryIconVisibility()
  {
    return mDictionaryIconVisibility;
  }

  @NonNull
  @Override
  public VisibilityState getDictionaryTitleVisibility()
  {
    return mDictionaryTitleVisibility;
  }

  @Override
  public boolean showHistoryScreen( @Nullable Context context )
  {
    if ( mHistoryManager != null && context != null )
    {
      if ( mHistoryManager.showHistoryScreen(context) )
      {
        free();
        return true;
      }
    }
    return false;
  }

  @Override
  public void openPractisePronunciationScreenActivity(@NonNull Context context)
  {

  }

  @Override
  public void openPractisePronunciationScreen(@NonNull Context context) {

  }

  @Override
  public SpannableString getRunningHeadsHeader(boolean leftToRight)
  {
    return leftToRight ? mArticle.forwardRunningHeadHeader : mArticle.backRunningHeadHeader;
  }

  @Nullable @VisibleForTesting
  protected SpannableString createRunningHeadsHeader( boolean leftToRight )
  {
    ArticleItem item = findRunningHeadsArticle(leftToRight);

    if( mArticle.stack.getCurrentItem() == null || mArticle.stack.getCurrentItem().equals(item) )
      return null;

    if ( item != null )
    {
      StringBuilder sb = new StringBuilder();
      String text = item.getShowVariantText();
      String num = item.getNumeration() != null ? item.getNumeration() : "";
      String partOfSpeech = item.getPartOfSpeech() != null ? item.getPartOfSpeech() : "";

      sb.append(text).append(num).append(partOfSpeech.isEmpty() ? "" : "\n" + partOfSpeech);
      SpannableString headWord = new SpannableString(sb);

      int position = text != null ? text.length() : 0;
      headWord.setSpan(new SuperscriptSpan(), position, position + num.length(), Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
      headWord.setSpan(new RelativeSizeSpan(0.7f), position, position += num.length(), Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);

      if ( !partOfSpeech.isEmpty() )
        headWord.setSpan(new StyleSpan(Typeface.ITALIC), position += "\n".length(), position + partOfSpeech.length(), Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);

      return headWord;
    }
    return null;
  }

  @VisibleForTesting
  protected ArticleItem findRunningHeadsArticle( boolean leftToRight )
  {
    ArticleItem currentItem = mArticle.stack.getCurrentItem();
    ShowArticleOptions options = mArticle.stack.getCurrentShowArticleOptions();
    if ( null != currentItem && null != options )
    {
      SwipeMode swipeMode = options.getSwipeMode();
      switch (swipeMode) {
        case A_Z_OF_DICTIONARY:
          if ( mArticleSearcher == null )
            break;
          return mArticleSearcher.findForSwipe(currentItem, leftToRight);

        case IN_SEPARATE_LIST:
          return mArticleManager.findForSwipeSeparateArticles(leftToRight);
      }
    }
    return null;
  }

  @NonNull
  @Override
  public ButtonState getRunningHeadsButtonState(boolean leftToRight)
  {
    VisibilityState vis = VisibilityState.gone;
    if (mArticle.stack.getCurrentShowArticleOptions() != null && !mArticle.isLocked
            && !ArticleControllerType.CONTROLLER_ID_SHARE.equals(mControllerId))
    {
      if ( getRunningHeadsHeader(leftToRight) != null )
      {
        vis = VisibilityState.enabled;
      }
    }
    return new ButtonState(vis, CheckState.uncheckable);
  }

  @Override
  public void addToFlashcards( @NonNull Context context )
  {
    ArticleItem currentArticle = mArticle.stack.getCurrentItem();
    if ( ( mFlashcardManager != null ) && ( currentArticle != null ) )
    {
      mFlashcardManager.addWord(context, currentArticle);
    }
  }

  @Override
  public void removeFromFlashcards( @NonNull Context context )
  {
    ArticleItem currentArticle = mArticle.stack.getCurrentItem();
    if ( ( mFlashcardManager != null ) && ( currentArticle != null ) )
    {
      mFlashcardManager.removeWord(context, currentArticle);
    }
  }

  @Override
  public void swipe( boolean leftToRight )
  {
    ArticleItem currentItem = mArticle.stack.getCurrentItem();
    ShowArticleOptions options = mArticle.stack.getCurrentShowArticleOptions();
    if ( null != currentItem && null != options )
    {
      SwipeMode swipeMode = options.getSwipeMode();
      ArticleItem next;
      switch (swipeMode) {
        case A_Z_OF_DICTIONARY:
          if ( mArticleSearcher == null )
            break;
          next = mArticleSearcher.findForSwipe(currentItem, !leftToRight);
          if ( next != null ) {
            nextTranslation(next, options);
          }
          break;

        case IN_SEPARATE_LIST:
          next = mArticleManager.makeStepForSwipeSeparateArticles(!leftToRight);
          if ( next != null ) {
            nextTranslation(next, options);
          }
          break;
      }
    }
  }

  @Override
  public boolean isCurrentArticle(int listId, int globalIndex) {
    ArticleItem currentItem = mArticle.stack.getCurrentItem();
    if ( currentItem == null )
      return false;
    else
      return currentItem.getListId() == listId
			  && currentItem.getGlobalIndex() == globalIndex;
  }

  private void preloadSettings()
  {
    mHideOrSwitchBlocksSetting = Utils.loadSetting(mSettingsManager, HIDE_OR_SWITCH_BLOCKS_SETTINGS_NAME, mHideOrSwitchBlocksSetting);
    mSearchState.searchModeOn = Utils.loadSetting(mSettingsManager, SEARCH_MODE_SETTINGS_PREFIX + mControllerId, mSearchState.searchModeOn);
    mSearchState.searchText = Utils.loadSetting(mSettingsManager, SEARCH_TEXT_SETTINGS_PREFIX + mControllerId, mSearchState.searchText);
  }

  private @Nullable
  Dictionary findDictionary( @NonNull ArticleItem articleItem )
  {
    Dictionary res = null;
    if ( mDictionaryManager != null )
    {
      for ( Dictionary dictionary : mDictionaryManager.getDictionaries() )
      {
        if ( dictionary != null )
        {
          if ( dictionary.getId().equals(articleItem.getDictId()) )
          {
            res = dictionary;
            break;
          }
        }
      }
    }
    return res;
  }

  protected synchronized void changeState(@NonNull Runnable op)
  {
    String dictionaryTitle = null, dictionaryPrice = null, articleData = null, popupData = null, searchUIResultText =
        null, ftsAnchor = null;
    SpannableString backRunningHeadHeader = null, forwardRunningHeadHeader = null;
    Bitmap dictionaryIcon = null;
    VisibilityState demoBannerVisibility = null, articleVisibility = null, searchUIVisibility = null,
        openMyDictionariesVisibility = null, trialStatusVisibility = null;
    ButtonState favoritesButtonState = null, backButtonState = null, forwardButtonState = null, soundButtonState = null,
        findNextButtonState = null, findPreviousButtonState = null, searchUIButtonState = null,
        hideOrSwitchBlocksButtonState = null, goToHistoryButtonState = null, backRunningHeadButtonState = null,
        forwardRunningHeadButtonState = null, flashcardButtonState = null;
    int trialLengthInMinutes = 0;

    boolean myViewEnabled = true, isHidePronunciations = false, isHideExamples = false, isHidePictures = false,
        isHideIdioms = false, isHidePhrasalVerbs = false;

    float articleScale = ApplicationSettings.getDefaultArticleScale();
    if ( !mInsideChangeStateMethod )
    {
      dictionaryTitle = getDictionaryTitle();
      dictionaryIcon = getDictionaryIcon();
      dictionaryPrice = getDictionaryPrice();
      articleData = getArticleData();
      ftsAnchor = getArticleFtsAnchor();
      articleScale = getArticleScale();
      popupData = getPopupData();
      demoBannerVisibility = getDemoBannerVisibility();
      backRunningHeadButtonState = getRunningHeadsButtonState(false);
      forwardRunningHeadButtonState = getRunningHeadsButtonState(true);
      openMyDictionariesVisibility = getOpenMyDictionariesVisibility();
      trialStatusVisibility = getTrialStatusVisibility();
      trialLengthInMinutes = getTrialLengthInMinutes();
      articleVisibility = getArticleVisibility();
      favoritesButtonState = getFavoritesButtonState();
      flashcardButtonState = getFlashcardButtonState();
      backButtonState = getBackButtonState();
      forwardButtonState = getForwardButtonState();
      soundButtonState = getSoundButtonState();
      searchUIVisibility = getSearchUIVisibility();
      searchUIResultText = getSearchUIResultText();
      findNextButtonState = getFindNextButtonState();
      findPreviousButtonState = getFindPreviousButtonState();
      searchUIButtonState = getSearchUIButtonState();
      hideOrSwitchBlocksButtonState = getHideOrSwitchBlocksButtonState();
      goToHistoryButtonState = getGoToHistoryButtonState();
      backRunningHeadHeader = getRunningHeadsHeader(false);
      forwardRunningHeadHeader = getRunningHeadsHeader(true);

      myViewEnabled = isMyViewEnabled();
      isHidePronunciations = isHidePronunciations();
      isHideExamples = isHideExamples();
      isHidePictures = isHidePictures();
      isHideIdioms = isHideIdioms();
      isHidePhrasalVerbs = isHidePhrasalVerbs();
    }

    boolean prevInsideValue = mInsideChangeStateMethod;
    mInsideChangeStateMethod = true;
    op.run();
    mInsideChangeStateMethod = prevInsideValue;

    if ( !mInsideChangeStateMethod )
    {
      if ( !areStringsEqual(dictionaryTitle, getDictionaryTitle()) )
      {
        for ( Notifier notifier : mNotifiers )
        {
          if ( notifier instanceof OnDictionaryTitleChangedListener )
          {
            ( (OnDictionaryTitleChangedListener) notifier ).onDictionaryTitleChanged();
          }
        }
      }
      if ( !areBitmapsEqual(dictionaryIcon, getDictionaryIcon()) )
      {
        for ( Notifier notifier : mNotifiers )
        {
          if ( notifier instanceof OnDictionaryIconChangedListener )
          {
            ( (OnDictionaryIconChangedListener) notifier ).onDictionaryIconChanged();
          }
        }
      }
      if ( demoBannerVisibility != getDemoBannerVisibility() )
      {
        for ( Notifier notifier : mNotifiers )
        {
          if ( notifier instanceof OnDemoBannerVisibilityChangedListener )
          {
            ( (OnDemoBannerVisibilityChangedListener) notifier ).onDemoBannerVisibilityChanged();
          }
        }
      }
      if ( openMyDictionariesVisibility != getOpenMyDictionariesVisibility() )
      {
        for ( Notifier notifier : mNotifiers )
        {
          if ( notifier instanceof OnOpenMyDictionariesVisibilityChangedListener )
          {
            ( (OnOpenMyDictionariesVisibilityChangedListener) notifier ).onOpenMyDictionariesVisibilityChanged();
          }
        }
      }
      if ( trialStatusVisibility != getTrialStatusVisibility() )
      {
        for ( Notifier notifier : mNotifiers )
        {
          if ( notifier instanceof OnTrialStatusVisibilityChangedListener )
          {
            ( (OnTrialStatusVisibilityChangedListener) notifier ).onTrialStatusVisibilityChanged();
          }
        }
      }
      if ( trialLengthInMinutes != getTrialLengthInMinutes() )
      {
        for ( Notifier notifier : mNotifiers )
        {
          if ( notifier instanceof OnTrialLengthChangedListener )
          {
            ( (OnTrialLengthChangedListener) notifier ).onTrialLengthChanged();
          }
        }
      }
      if ( !areStringsEqual(dictionaryPrice, getDictionaryPrice()) )
      {
        for ( Notifier notifier : mNotifiers )
        {
          if ( notifier instanceof OnDictionaryPriceChangedListener )
          {
            ( (OnDictionaryPriceChangedListener) notifier ).onDictionaryPriceChanged();
          }
        }
      }
      if ( !areStringsEqual(articleData, getArticleData()) )
      {
        for ( Notifier notifier : mNotifiers )
        {
          if ( notifier instanceof OnArticleDataChangedListener )
          {
            ( (OnArticleDataChangedListener) notifier ).onArticleDataChanged();
          }
        }
      }
      if ( !areStringsEqual(ftsAnchor, getArticleFtsAnchor()) )
      {
        for ( Notifier notifier : mNotifiers )
        {
          if ( notifier instanceof OnFtsAnchorChangedListener )
          {
            ( (OnFtsAnchorChangedListener) notifier ).onFtsAnchorDataChanged();
          }
        }
      }
      if ( articleScale != getArticleScale() )
      {
        for ( Notifier notifier : mNotifiers )
        {
          if ( notifier instanceof OnArticleScaleChangedListener )
          {
            ( (OnArticleScaleChangedListener) notifier ).onArticleScaleChanged();
          }
        }
      }
      if ( !areStringsEqual(popupData, getPopupData()) )
      {
        for ( Notifier notifier : mNotifiers )
        {
          if ( notifier instanceof OnPopupDataChangedListener )
          {
            ( (OnPopupDataChangedListener) notifier ).onPopupDataChanged();
          }
        }
      }
      if ( !articleVisibility.equals(getArticleVisibility()) )
      {
        for ( Notifier notifier : mNotifiers )
        {
          if ( notifier instanceof OnArticleVisibilityChangedListener )
          {
            ( (OnArticleVisibilityChangedListener) notifier ).onArticleVisibilityChanged();
          }
        }
      }
      if ( !favoritesButtonState.equals(getFavoritesButtonState()) )
      {
        for ( Notifier notifier : mNotifiers )
        {
          if ( notifier instanceof OnFavoritesButtonStateChangedListener )
          {
            ( (OnFavoritesButtonStateChangedListener) notifier ).onFavoritesButtonStateChanged();
          }
        }
      }
      if ( !backButtonState.equals(getBackButtonState()) )
      {
        for ( Notifier notifier : mNotifiers )
        {
          if ( notifier instanceof OnBackButtonStateChangedListener )
          {
            ( (OnBackButtonStateChangedListener) notifier ).onBackButtonStateChanged();
          }
        }
      }
      if ( !forwardButtonState.equals(getForwardButtonState()) )
      {
        for ( Notifier notifier : mNotifiers )
        {
          if ( notifier instanceof OnForwardButtonStateChangedListener )
          {
            ( (OnForwardButtonStateChangedListener) notifier ).onForwardButtonStateChanged();
          }
        }
      }
      if ( !soundButtonState.equals(getSoundButtonState()) )
      {
        for ( Notifier notifier : mNotifiers )
        {
          if ( notifier instanceof OnSoundButtonStateChangedListener )
          {
            ( (OnSoundButtonStateChangedListener) notifier ).onSoundButtonStateChanged();
          }
        }
      }
      if ( !searchUIVisibility.equals(getSearchUIVisibility()) )
      {
        for ( Notifier notifier : mNotifiers )
        {
          if ( notifier instanceof OnSearchUIVisibilityChangedListener )
          {
            ( (OnSearchUIVisibilityChangedListener) notifier ).onSearchUIVisibilityChanged();
          }
        }
      }
      if ( !areStringsEqual(searchUIResultText, getSearchUIResultText()) )
      {
        for ( Notifier notifier : mNotifiers )
        {
          if ( notifier instanceof OnSearchUIResultTextChangedListener )
          {
            ( (OnSearchUIResultTextChangedListener) notifier ).onSearchUIResultTextChanged();
          }
        }
      }
      if ( !findNextButtonState.equals(getFindNextButtonState()) )
      {
        for ( Notifier notifier : mNotifiers )
        {
          if ( notifier instanceof OnFindNextButtonStateChangedListener )
          {
            ( (OnFindNextButtonStateChangedListener) notifier ).onFindNextButtonStateChanged();
          }
        }
      }
      if ( !findPreviousButtonState.equals(getFindPreviousButtonState()) )
      {
        for ( Notifier notifier : mNotifiers )
        {
          if ( notifier instanceof OnFindPreviousButtonStateChangedListener )
          {
            ( (OnFindPreviousButtonStateChangedListener) notifier ).onFindPreviousButtonStateChanged();
          }
        }
      }
      if ( !searchUIButtonState.equals(getSearchUIButtonState()) )
      {
        for ( Notifier notifier : mNotifiers )
        {
          if ( notifier instanceof OnSearchUIButtonStateChangedListener )
          {
            ( (OnSearchUIButtonStateChangedListener) notifier ).onSearchUIButtonStateChanged();
          }
        }
      }
      if ( !hideOrSwitchBlocksButtonState.equals(getHideOrSwitchBlocksButtonState()) )
      {
        for ( Notifier notifier : mNotifiers )
        {
          if ( notifier instanceof OnHideOrSwitchBlocksButtonStateChangedListener )
          {
            ( (OnHideOrSwitchBlocksButtonStateChangedListener) notifier ).onHideOrSwitchBlocksButtonStateChanged();
          }
        }
      }
      if ( !goToHistoryButtonState.equals(getGoToHistoryButtonState()) )
      {
        for ( Notifier notifier : mNotifiers )
        {
          if ( notifier instanceof OnGoToHistoryButtonStateChangedListener )
          {
            ( (OnGoToHistoryButtonStateChangedListener) notifier ).onGoToHistoryButtonStateChanged();
          }
        }
      }
      if ( !backRunningHeadButtonState.equals(getRunningHeadsButtonState(false))
          || !forwardRunningHeadButtonState.equals(getRunningHeadsButtonState(true)) )
      {
        for ( Notifier notifier : mNotifiers )
        {
          if ( notifier instanceof OnRunningHeadsButtonStateChangedListener )
          {
            ( (OnRunningHeadsButtonStateChangedListener) notifier ).onRunningHeadsButtonStateChanged();
          }
        }
      }
      if ( !areSpannableStringsEqual(backRunningHeadHeader, getRunningHeadsHeader(false))
          || !areSpannableStringsEqual(forwardRunningHeadHeader, getRunningHeadsHeader(true)))
      {
        for ( Notifier notifier : mNotifiers )
        {
          if ( notifier instanceof OnRunningHeadsHeadersChangedListener )
          {
            ( (OnRunningHeadsHeadersChangedListener) notifier ).onRunningHeadsHeadersChanged();
          }
        }
      }
      if ( !flashcardButtonState.equals(getFlashcardButtonState()) )
      {
        for ( Notifier notifier : mNotifiers )
        {
          if ( notifier instanceof OnFlashcardButtonStateChangedListener )
          {
            ( (OnFlashcardButtonStateChangedListener) notifier ).onFlashcardButtonStateChanged();
          }
        }
      }
      if ( myViewEnabled != isMyViewEnabled() || isHidePronunciations != isHidePronunciations()
               || isHideExamples != isHideExamples() || isHidePictures != isHidePictures()
               || isHideIdioms != isHideIdioms() || isHidePhrasalVerbs != isHidePhrasalVerbs() )
      {
        for ( Notifier notifier : mNotifiers )
        {
          if ( notifier instanceof OnMyViewSettingsChangedListener )
          {
            ( (OnMyViewSettingsChangedListener) notifier ).onMyViewSettingsChanged();
          }
        }
      }
    }
  }

  @VisibleForTesting
  protected void translationHelper( @Nullable final ArticleItem articleItem,
                                  @NonNull final ArticleTranslationKind articleTranslationKind,
                                  @Nullable final ShowArticleOptions showArticleOptions )
  {
    changeState(() ->
    {
      discardPopup();
      boolean translationStarted = false;
      if ( mEngine != null )
      {
        //          mTransitionState.translation = translationKind;
        translationStarted =
            mEngine.translateNext(mArticle.stack.getCurrentItem(), articleItem, getHtmlBuilderParams(), mArticleTranslateCallback);
      }
      //        mTransitionState.translation = translationStarted ? translationKind : TranslationKind.None;
      if ( translationStarted )
      {
        mTransitionState.translation = TranslationKind.Article;
        if ( ArticleTranslationKind.NavigationBack.equals(articleTranslationKind) )
        {
          mArticle.stack.back(articleItem, showArticleOptions);
        }
        else if ( ArticleTranslationKind.NavigationForward.equals(articleTranslationKind) )
        {
          mArticle.stack.forward(articleItem, showArticleOptions);
        }
        else if(isAdditional(articleItem))
        {
          mArticle.stack.additional(articleItem, showArticleOptions);
        }
        else
        {
          mArticle.stack.nextTranslation(articleItem,  !ArticleTranslationKind.Init.equals(articleTranslationKind), showArticleOptions);
        }
      }
      else
      {
        mTransitionState.translation = TranslationKind.None;
      }
      if ( articleItem == null )
      {
        setTranslationResult(null, null, false);
      }
    });
  }

  /**
   * without notification, no callbacks
   */
  @VisibleForTesting
  protected void discardPopup()
  {
    if ( ( mTransitionState.translation == TranslationKind.Popup ) )
    {
      if ( ( mEngine != null ) )
      {
        mEngine.translateNext(mArticle.stack.getCurrentItem(), null, getHtmlBuilderParams(), mPopupTranslateCallback);
      }
      mTransitionState.translation = TranslationKind.None;
    }
    mPopup.item = null;
    mPopup.data = null;
  }

  /**
   * without notification, no callbacks
   */
  private void setTranslationResult(@Nullable final ArticleItem updatedArticleItem, @Nullable String translation,
                                    boolean hasHideOrSwitchBlocks )
  {
    if ( mSearchTarget != null )
    {
      mSearchTarget.clearMatches();
    }
    mArticle.data = translation;
    if (null != updatedArticleItem) {
      mArticle.showVariantText = updatedArticleItem.getShowVariantText();
      mArticle.isLocked = updatedArticleItem.isLocked();
      mArticle.ftsAnchor = updatedArticleItem.getFtsAnchor();

      mArticle.stack.updateCurrentArticleItem(updatedArticleItem);
    }
    addArticleInHistory(updatedArticleItem);
    boolean isInFavorites = mArticle.isArticleInFavorites;
    if ( ( mFavoritesManager != null ) && ( updatedArticleItem != null ) )
    {
      if ( ( !mFavoritesManager.isInTransition() ) && ( mTransitionState.translation != TranslationKind.Popup ) )
      {
        isInFavorites = mFavoritesManager.hasWord(updatedArticleItem);
      }
    }
    else
    {
      isInFavorites = false;
    }
    mArticle.isArticleInFavorites = isInFavorites;
    boolean canAddWordFlashcard = mArticle.canAddWordFlashcard;
    boolean canRemoveWordFlashcard = mArticle.canRemoveWordFlashcard;
    if ( mFlashcardManager != null && updatedArticleItem != null )
    {
      if ( mTransitionState.translation != TranslationKind.Popup )
      {
        canAddWordFlashcard = mFlashcardManager.canAddWord(updatedArticleItem);
        canRemoveWordFlashcard = mFlashcardManager.canRemoveWord(updatedArticleItem);
      }
    }
    else
    {
      canAddWordFlashcard = canRemoveWordFlashcard = false;
    }
    mArticle.canAddWordFlashcard = canAddWordFlashcard;
    mArticle.canRemoveWordFlashcard = canRemoveWordFlashcard;
    mArticle.hasHideOrSwitchBlocks = hasHideOrSwitchBlocks;
    boolean hasSound = false;
    if ( ( mSoundManager != null ) && ( updatedArticleItem != null ) )
    {
      hasSound = mSoundManager.itemHasSound(updatedArticleItem);
    }
    mArticle.hasSound = hasSound;
    mArticle.backRunningHeadHeader = createRunningHeadsHeader(false);
    mArticle.forwardRunningHeadHeader = createRunningHeadsHeader(true);
    String dictionaryTitle = null;
    IDictionaryIcon dictionaryIcon = null;
    Dictionary.DICTIONARY_STATUS dictionaryEdition = null;
    DictionaryPrice dictionaryPrice = null;
    boolean dictionaryHasHideOrSwitchBlocks = false;
    boolean isFullWordBaseAccessible = false;
    int trialLengthInMinutes = 0;
    if ( updatedArticleItem != null )
    {
      Dictionary dictionary = findDictionary(updatedArticleItem);
      if ( dictionary != null )
      {
        dictionaryTitle = dictionary.getTitle().get();
        dictionaryIcon = dictionary.getIcon();
        dictionaryEdition = dictionary.getStatus();
      }
    }
    if ( ( mDictionaryManager != null ) && ( updatedArticleItem != null ) )
    {
      trialLengthInMinutes = mDictionaryManager.getTrialLengthInMinutes(updatedArticleItem.getDictId());
      dictionaryPrice = mDictionaryManager.getDictionaryPrice(updatedArticleItem.getDictId());
      Dictionary dictionary = findDictionary(updatedArticleItem);
      if (dictionary != null)
      {
        isFullWordBaseAccessible = getFullBaseStatus(updatedArticleItem.getDictId());
      }
    }
    if ( ( mEngine != null ) && ( updatedArticleItem != null ) )
    {
      dictionaryHasHideOrSwitchBlocks = mEngine.hasHideOrSwitchBlocks(updatedArticleItem.getDictId());
    }
    mDictionaryInfo.title = dictionaryTitle;
    mDictionaryInfo.icon = dictionaryIcon;
    mDictionaryInfo.edition = dictionaryEdition;
    mDictionaryInfo.price = dictionaryPrice;
    mDictionaryInfo.hasHideOrSwitchBlocks = dictionaryHasHideOrSwitchBlocks;
    mDictionaryInfo.isFullWordBaseAccessible = isFullWordBaseAccessible;
    mDictionaryInfo.trialLengthInMinutes = trialLengthInMinutes;
    mTransitionState.translation = TranslationKind.None;
  }

  @Override
  public boolean getFullBaseStatus(Dictionary.DictionaryId dictionaryId) {
    return false;
  }

  @Override
  public boolean isPreviewMode() {
    return true;
  }

  private static DictionaryComponent getWordBaseComponent( Dictionary dictionary )
  {
    for ( DictionaryComponent component : dictionary.getDictionaryComponents() )
    {
      if ( !component.isDemo() && DictionaryComponent.Type.WORD_BASE.equals(component.getType()) )
      {
        return component;
      }
    }
    return null;
  }

  protected void addArticleInHistory( ArticleItem articleItem )
  {
    if ( mHistoryManager != null && articleItem != null)
    {
      mHistoryManager.addWord(articleItem);
    }
  }

  @SuppressWarnings( "BooleanMethodIsAlwaysInverted" )
  private static boolean areStringsEqual( @Nullable String str1, @Nullable String str2 )
  {
    boolean res = false;
    if ( ( str1 == null ) && ( str2 == null ) )
    {
      res = true;
    }
    else if ( ( str1 != null ) && ( str2 != null ) )
    {
      res = str1.equals(str2);
    }
    return res;
  }
  private static boolean areSpannableStringsEqual( @Nullable SpannableString str1, @Nullable SpannableString str2 )
  {
    boolean res = false;
    if ( ( str1 == null ) && ( str2 == null ) )
    {
      res = true;
    }
    else if ( ( str1 != null ) && ( str2 != null ) )
    {
      res = str1.equals(str2);
    }
    return res;
  }

  private static boolean areBitmapsEqual( @Nullable Bitmap bitmap1, @Nullable Bitmap bitmap2 )
  {
    boolean res = false;
    if ( ( bitmap1 == null ) && ( bitmap2 == null ) )
    {
      res = true;
    }
    else if ( ( bitmap1 != null ) && ( bitmap2 != null ) )
    {
      res = bitmap1.sameAs(bitmap2);
    }
    return res;
  }

  protected static boolean isAdditional(@Nullable ArticleItem articleItem) {
    boolean res = false;
    if(articleItem != null)
      res = articleItem.isAdditional();
    return res;
  }

  @Override
  public boolean isNeedCrossRef() {
    return mArticle.stack.getCurrentItem() != null && (!mArticle.stack.getCurrentItem().isAdditional()&& !mArticle.stack.getCurrentItem().isMorphologyArticle());
  }

  @Override
  public Dictionary.DictionaryId getCurrentDictionaryId() {
    if(mArticle.stack.getCurrentItem()!=null)
    return mArticle.stack.getCurrentItem().getDictId();
    else return null;
  }

  @Override
  public boolean isExternalBaseDownloaded(@Nullable String baseId)
  {
    if ( baseId != null && mDictionaryManager != null )
    {
      return mDictionaryManager.isExternalBaseDownloaded(getCurrentDictionaryId(), baseId);
    }
    return false;
  }

  @Override
  public boolean isNeedReturnPreview()
  {
      return false;
  }

  @Override
  public boolean showHintManagerDialog( @NonNull HintType hintType,
                                        @Nullable FragmentManager fragmentManager,
                                        @Nullable HintParams hintParams )
  {
    if ( mHintManager != null )
    {
      return mHintManager.showHintDialog(hintType, fragmentManager, hintParams);
    }
    return false;
  }

  @Override
  public Observable< Pair< HintType, HintParams > > getNeedToShowHintObservable()
  {
    return mNeedToShowHintSubject;
  }

  protected void sendRequestToShowHint( @NonNull HintType hintType, @Nullable HintParams hintParams )
  {
    mNeedToShowHintSubject.onNext(new Pair<>(hintType, hintParams));
  }

  @Override
  public boolean isNeedOpenPictures()
  {
    return true;
  }

  @Override
  public void openMorphologicalTable(Context context) {
    ArticleItem currentItem = mArticle.stack.getCurrentItem();
    if (mArticleSearcher != null && currentItem != null && currentItem.getLinkInfo() != null) {
      for (LinkInfo linkInfo : currentItem.getLinkInfo()) {
        ArticleItem next =
            mArticleSearcher.find(currentItem.getDictId(), linkInfo.getListId(), linkInfo.getWordIndex(), null);
        if (next != null && next.isMorphologyArticle()) {
          mArticleManager.showArticleActivity(next, ArticleControllerType.OALD_CONTROLLER_ID_ADDITIONAL_INFO, context);
          break;
        }
      }
    }
  }

  @Override
  public ButtonState getMorphoTableButtonState() {
    boolean show = mArticle.stack.getCurrentItem() != null &&
            mArticle.stack.getCurrentItem().getLinkInfo() != null &&
            mArticle.stack.getCurrentItem().getLinkInfo().length != 0;
    return new ButtonState(show ? VisibilityState.enabled : VisibilityState.gone, CheckState.uncheckable);
  }

  @Override
  public boolean isMorphoTable() {
    return mArticle.stack.getCurrentItem() != null && mArticle.stack.getCurrentItem().isMorphologyArticle();
  }

  @Override
  public void onFavoritesManagerError( Exception exception )
  {
    for ( Notifier notifier : mNotifiers )
    {
      if ( notifier instanceof OnControllerErrorListener )
      {
        ( (OnControllerErrorListener) notifier ).onControllerError(exception);
      }
    }
  }

  @Override
  public void onManagerArticleScaleChanged()
  {
    changeState(() -> mArticle.scale = mArticleManager.getArticleScale());
  }

  @Override
  public void onApplicationSettingsSaved( @NonNull final ApplicationSettings applicationSettings )
  {
    changeState(() ->
    {
      mArticle.usePinchToZoom = applicationSettings.isPinchToZoomEnabled();
      mArticle.showHighlighting = applicationSettings.isShowHighlightingEnabled();

      mArticle.myViewEnabled = applicationSettings.getMyView().isMyViewEnabled();
      mArticle.hidePronunciations = applicationSettings.getMyView().isHidePronunciations();
      mArticle.hideExamples = applicationSettings.getMyView().isHideExamples();
      mArticle.hidePictures = applicationSettings.getMyView().isHidePictures();
      mArticle.hideIdioms = applicationSettings.getMyView().isHideIdioms();
      mArticle.hidePhrasalVerbs = applicationSettings.getMyView().isHidePhrasalVerbs();
      if ( !Arrays.equals(mArticle.hideSoundIcons, applicationSettings.getHideSoundIcons().toArray(new String[0])))
      {
        mArticle.hideSoundIcons = applicationSettings.getHideSoundIcons().toArray(new String[0]);
        nextTranslation(mArticle.stack.getCurrentItem(), mArticle.stack.getCurrentShowArticleOptions());
      }
    });
  }

  private class ArticleTranslateCallback implements EngineArticleAPI.OnTranslationReadyCallback
  {

    @Override
    public void onTranslationReady(@NonNull final ArticleItem article, @NonNull final String translation,
                                   final boolean hasHideOrSwitchBlocks, @Nullable ArticleSearcherAPI articleSearcher)
    {

      mArticleSearcher = articleSearcher;

      changeState(() -> setTranslationResult(article, translation, hasHideOrSwitchBlocks));
    }
  }

  private class PopupTranslateCallback implements EngineArticleAPI.OnTranslationReadyCallback
  {

    @Override
    public void onTranslationReady( @NonNull final ArticleItem articleItem, @NonNull final String s, boolean b, @Nullable ArticleSearcherAPI articleSearcher )
    {
      changeState(() ->
      {
        mPopup.item = articleItem;
        mPopup.data = s;
        mTransitionState.translation = TranslationKind.None;
      });
    }
  }

  private class SoundStateChangedListener implements SoundManagerAPI.OnStateChangedListener
  {

    @Override
    public void onSoundStateChanged()
    {
      changeState(() ->
      {
        if ( mSoundManager != null )
        {
          mTransitionState.soundBusy = mSoundManager.isPlaying();
        }
      });
    }
  }

  private class FavoritesStateChangedListener implements OnStateChangedListener
  {

    @Override
    public void onFavoritesStateChanged()
    {
      changeState(() ->
      {
        if ( mFavoritesManager != null )
        {
          mTransitionState.favoritesBusy = mFavoritesManager.isInTransition();
          if ( ( !mTransitionState.favoritesBusy ) && ( !mTransitionState.translation.mustDisableArticleButtons() ) )
          {
            ArticleItem articleItem = mArticle.stack.getCurrentItem();
            if ( articleItem != null )
            {
              mArticle.isArticleInFavorites = mFavoritesManager.hasWord(articleItem);
            }
          }
        }
      });
    }
  }

  private class DictionaryListChangedListener implements DictionaryManagerAPI.IDictionaryListObserver
  {
    @Override
    public void onDictionaryListChanged() {
      ArticleItem articleItem = mArticle.stack.getCurrentItem();
      if ((!mTransitionState.translation.mustDisableArticleButtons()) && (articleItem != null)) {
        Dictionary dictionary = findDictionary(articleItem);
        if (dictionary != null) {
          if (mDictionaryInfo.edition != dictionary.getStatus() || mDictionaryInfo.isFullWordBaseAccessible != getFullBaseStatus(dictionary.getId())) {
            nextTranslation(articleItem, mArticle.stack.getCurrentShowArticleOptions());
          }
        }
      }
    }
  }

  private class SearchTargetStateChangedListener implements SearchTarget.OnStateChangedListener
  {
    @Override
    public void onSearchStateChanged()
    {
      changeState(() ->
      {
        mSearchState.searchResult = "";
        mSearchState.canNavigateResults = false;
        if ( mSearchTarget != null )
        {
          mSearchState.searchResult = mSearchTarget.getResult();
          mSearchState.canNavigateResults = mSearchTarget.canNavigate();
        }
      });
    }
  }

  private class FlashcardStateChangedListener implements CardsChangedListener
  {
    @Override
    public void onCardsChanged()
    {
      changeState(() ->
      {
        if ( mFlashcardManager != null )
        {
          if ( !mTransitionState.translation.mustDisableArticleButtons() )
          {
            ArticleItem articleItem = mArticle.stack.getCurrentItem();
            if ( articleItem != null )
            {
              mArticle.canAddWordFlashcard = mFlashcardManager.canAddWord(articleItem);
              mArticle.canRemoveWordFlashcard = mFlashcardManager.canRemoveWord(articleItem);
            }
          }
        }
      });
    }
  }

  @VisibleForTesting
  protected List< Notifier > getNotifiers() {
    return new ArrayList<>(mNotifiers);
  }

  @Nullable @VisibleForTesting
  protected EngineArticleAPI getEngine() {
    return mEngine;
  }
}
