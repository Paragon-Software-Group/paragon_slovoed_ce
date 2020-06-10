package com.paragon_software.article_manager;

import android.app.Activity;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.VisibleForTesting;

import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.favorites_manager.FavoritesManagerAPI;
import com.paragon_software.flash_cards_manager.FlashcardManagerAPI;
import com.paragon_software.hint_manager.HintManagerAPI;
import com.paragon_software.history_manager.HistoryManagerAPI;
import com.paragon_software.native_engine.EngineArticleAPI;
import com.paragon_software.search_manager.SearchManagerAPI;
import com.paragon_software.settings_manager.ApplicationSettings;
import com.paragon_software.settings_manager.OnApplicationSettingsSaveListener;
import com.paragon_software.settings_manager.SettingsManagerAPI;
import com.paragon_software.settings_manager.exceptions.LocalResourceUnavailableException;
import com.paragon_software.settings_manager.exceptions.ManagerInitException;
import com.paragon_software.sound_manager.SoundManagerAPI;
import com.paragon_software.splash_screen_manager.ISplashScreenManager;
import com.paragon_software.utils_slovoed.collections.CollectionView;
import com.paragon_software.utils_slovoed.collections.ProxyCollectionView;
import com.paragon_software.utils_slovoed.text.SubstringInfo;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;

public class StandardArticleManager extends ArticleManagerAPI implements
                                                              OnApplicationSettingsSaveListener
{
  @NonNull
  private List< Notifier > mNotifiers = new ArrayList<>();

  @NonNull
  private static final String ARTICLE_SCALE_SETTINGS_NAME         = "article_manager.article_scale";

  protected @Nullable @VisibleForTesting
  DictionaryManagerAPI mDictionaryManager = null;

  protected @Nullable @VisibleForTesting
  SoundManagerAPI mSoundManager = null;

  protected @Nullable @VisibleForTesting
  FavoritesManagerAPI mFavoritesManager = null;

  protected @Nullable @VisibleForTesting
  HistoryManagerAPI mHistoryManager = null;

  protected @Nullable @VisibleForTesting
  SettingsManagerAPI mSettingsManager = null;

  protected @Nullable @VisibleForTesting
  SearchManagerAPI mSearchManager = null;

  protected @Nullable @VisibleForTesting
  ISplashScreenManager mSplashScreenManager = null;

  protected @Nullable @VisibleForTesting
  EngineArticleAPI mEngine = null;

  protected @Nullable @VisibleForTesting
  FlashcardManagerAPI mFlashcardManager = null;

  protected @Nullable @VisibleForTesting
  HintManagerAPI mHintManager = null;

  private @NonNull
  final Map< String, UIData > mUIDataMap = new TreeMap<>();

  @NonNull
  private ArticleStack separateArticlesStack = new ArticleStack();

  @NonNull
  private ProxyCollectionView< CollectionView< ArticleItem, SubstringInfo >, Void >
                             mWordsCollectionView = new ProxyCollectionView<>();

  @NonNull
  private GetWordsParamsState mGetWordsParamsState = new GetWordsParamsState();

  private float articleScale = ApplicationSettings.getDefaultArticleScale();

  @Override
  public void registerNotifier( @NonNull Notifier notifier )
  {
    if ( !mNotifiers.contains(notifier) )
    {
      mNotifiers.add(notifier);
    }
  }

  @Override
  public void unregisterNotifier( @NonNull Notifier notifier )
  {
    mNotifiers.remove(notifier);
  }

  @Override
  public void registerArticleUI( @NonNull String articleUI, @NonNull Class aClass )
  {
    UIData uiData = getOrCreateUiDataByKey(articleUI);
    if ( uiData.articleActivityClass == null )
    {
      uiData.articleActivityClass = aClass;
    }
  }

  @Override
  public void registerShareUI( @NonNull String articleUI, @NonNull Class aClass )
  {
    UIData uiData = getOrCreateUiDataByKey(articleUI);
    if ( uiData.shareActivityClass == null )
    {
      uiData.shareActivityClass = aClass;
    }
  }

  @Override
  public void registerPractisePronunciationUI(@NonNull String articleUI, @NonNull Class aClass )
  {
    UIData uiData = getOrCreateUiDataByKey(articleUI);
    if ( uiData.practisePronunciationClass == null )
    {
      uiData.practisePronunciationClass = aClass;
    }
  }

  @Override
  public void registerDictionaryManager( @NonNull DictionaryManagerAPI manager )
  {
    mDictionaryManager = manager;
  }

  @Override
  public void registerSoundManager( @NonNull SoundManagerAPI manager )
  {
    mSoundManager = manager;
  }

  @Override
  public void registerFavoritesManager( @NonNull FavoritesManagerAPI manager )
  {
    mFavoritesManager = manager;
  }

  @Override
  public void registerHistoryManager( @NonNull HistoryManagerAPI manager )
  {
    mHistoryManager = manager;
  }

  @Override
  public void registerSettingsManager( @NonNull SettingsManagerAPI manager )
  {
    if ( mSettingsManager != null )
    {
      mSettingsManager.unregisterNotifier(this);
    }
    mSettingsManager = manager;
    if ( mSettingsManager != null )
    {
      mSettingsManager.registerNotifier(this);
    }
    preloadSettings();
  }

  @Override
  public void registerSearchManager( @NonNull SearchManagerAPI manager )
  {
    mSearchManager = manager;
  }

  @Override
  public void registerSplashScreenManager( @NonNull ISplashScreenManager manager )
  {
    mSplashScreenManager = manager;
  }

  @Override
  public void registerEngine( @NonNull EngineArticleAPI engine )
  {
    mEngine = engine;
  }

  @Override
  public void registerFlashcardManager( @NonNull FlashcardManagerAPI manager )
  {
    mFlashcardManager = manager;
  }

  @Override
  public void registerHintManager( @Nullable HintManagerAPI hintManager )
  {
    mHintManager = hintManager;
  }

  @Override
  public boolean showArticleActivity( @Nullable ArticleItem articleItem,
                                      @NonNull String articleUI, Context context )
  {
    return showArticleActivity(articleItem, null, articleUI, context);
  }

  @Override
  public boolean showArticleActivity( @Nullable ArticleItem articleItem,
                                      @Nullable ShowArticleOptions showArticleOptions,
                                      @NonNull String articleUI, Context context )
  {
    boolean res = false;
    UIData uiData = getUiDataByKey(articleUI);
    if ( uiData != null )
    {
      getArticleController(articleUI);
      if ( uiData.articleActivityClass != null )
      {
        if ( ( !uiData.activityShown ) && ( Activity.class.isAssignableFrom(uiData.articleActivityClass) ) )
        {
          Intent intent = new Intent(context, uiData.articleActivityClass);
          intent.putExtra(EXTRA_CONTROLLER_ID, articleUI);
          context.startActivity(intent);
          uiData.activityShown = true;
        }
      }
      else if ( uiData.practisePronunciationClass != null )
      {
        if( (!uiData.activityShown ) &&  Activity.class.isAssignableFrom(uiData.practisePronunciationClass) )
        {
          Intent intent = new Intent(context, uiData.practisePronunciationClass);
          intent.putExtra(EXTRA_CONTROLLER_ID, articleUI);
          context.startActivity(intent);
        }
      }
      tryShowArticleAction(uiData, articleItem, showArticleOptions);
      res = true;
    }
    return res;
  }

  @Override
  public boolean showArticle(@Nullable ArticleItem articleItem, @Nullable ShowArticleOptions showArticleOptions,
                             @NonNull String articleUI, Context context) {
    UIData uiData = getUiDataByKey(articleUI);
    if (uiData != null)
    {
      getArticleController(articleUI);
      return tryShowArticleAction(uiData, articleItem, showArticleOptions);
    }
    else
      return false;
  }

  @Override
  public boolean showArticleActivityFromSeparateList(@NonNull List<ArticleItem> separateArticles,
                                                     int currentArticleIndex,
                                                     @NonNull String articleUI,
                                                     Context context) {
    ShowArticleOptions optionsForSeparateList = new ShowArticleOptions(SwipeMode.IN_SEPARATE_LIST);
    this.separateArticlesStack.setNewArticleItems(separateArticles, optionsForSeparateList, currentArticleIndex);

    return showArticleActivity(separateArticlesStack.getCurrentItem(), optionsForSeparateList, articleUI, context);
  }

  @Override
  public boolean showArticleFromSeparateList(@NonNull List<ArticleItem> separateArticles,
                                             int currentArticleIndex,
                                             @NonNull String articleUI,
                                             Context context) {
    ShowArticleOptions articleOptionForSeparateList = new ShowArticleOptions(SwipeMode.IN_SEPARATE_LIST);
    this.separateArticlesStack.setNewArticleItems(separateArticles, articleOptionForSeparateList, currentArticleIndex);
    return showArticle(separateArticlesStack.getCurrentItem(), articleOptionForSeparateList, articleUI, context);
  }

  @Nullable
  @Override
  public Class<Activity> getShareActivity()
  {
    for(Map.Entry<String,UIData>data : mUIDataMap.entrySet())
    {
      if(data.getValue().shareActivityClass!=null)
        return data.getValue().shareActivityClass;
    }
    return null;
  }

  @Override
  public float getArticleScale()
  {
    return articleScale;
  }

  @Override
  void setArticleScale( float newScale )
  {
    float newArticleScale = Math.min(ApplicationSettings.getMaxArticleScale(),
                                     Math.max(ApplicationSettings.getMinArticleScale(), newScale));
    if ( articleScale != newArticleScale )
    {
      articleScale = newArticleScale;
      for ( Notifier notifier : mNotifiers )
      {
        if ( notifier instanceof OnManagerArticleScaleChangedListener )
        {
          ( (OnManagerArticleScaleChangedListener) notifier ).onManagerArticleScaleChanged();
        }
      }
    }
  }

  @Override
  void saveState()
  {
    if ( mSettingsManager != null )
    {
      ApplicationSettings applicationSettings = mSettingsManager.loadApplicationSettings();
      if ( articleScale != applicationSettings.getArticleScale() )
      {
        applicationSettings.setArticleScale(articleScale);
        try
        {
          mSettingsManager.saveApplicationSettings(applicationSettings);
        }
        catch ( ManagerInitException | LocalResourceUnavailableException ignore )
        {
        }
      }
    }
  }

  @VisibleForTesting
  protected void preloadSettings()
  {
    // try to load article scale value located by old key in settings
    articleScale = Utils.loadSetting(mSettingsManager, ARTICLE_SCALE_SETTINGS_NAME,
                                     ApplicationSettings.getDefaultArticleScale());
    if ( mSettingsManager != null && articleScale == ApplicationSettings.getDefaultArticleScale() )
    {
      articleScale = mSettingsManager.loadApplicationSettings().getArticleScale();
    }
  }

  @NonNull
  @Override
  public ArticleControllerAPI getArticleController( @NonNull String articleUI )
  {
    UIData uiData = getOrCreateUiDataByKey(articleUI);
    if ( uiData.articleController == null )
    {
      if(ArticleControllerType.OALD_CONTROLLER.equals(articleUI) ||
              ArticleControllerType.OALD_CONTROLLER_ID_ADDITIONAL_INFO.equals(articleUI) ||
              ArticleControllerType.OALD_CONTROLLER_ID_SHARE.equals(articleUI) ||
              ArticleControllerType.OALD_CONTROLLER_ID_PRACTICE_PRONUNCIATION.equals(articleUI) ||
              ArticleControllerType.OALD_CONTROLLER_ID_WORD_OF_THE_DAY.equals(articleUI) ||
              ArticleControllerType.OALD_CONTROLLER_ID_QUIZ.equals(articleUI))
      {
        uiData.articleController =
                ArticleControllerOald.create(this, articleUI, mDictionaryManager, mSoundManager, mFavoritesManager,
                        mHistoryManager, mSettingsManager, mEngine, mFlashcardManager, mHintManager);
      }
      else if(ArticleControllerType.CONTROLLER_ID_FLASHCARDS.equals(articleUI))
        uiData.articleController = FlashcardArticleController.create(this, articleUI, mDictionaryManager, mSoundManager,
                                                                     mFavoritesManager, mHistoryManager, mSettingsManager, mEngine, mFlashcardManager, mHintManager);
      else {
        uiData.articleController =
                ArticleController.create(this, articleUI, mDictionaryManager, mSoundManager, mFavoritesManager,
                                         mHistoryManager, mSettingsManager, mEngine, mFlashcardManager, mHintManager);
      }
    }
    return uiData.articleController;
  }

  @Override
  public void freeArticleController(@NonNull String articleUI )
  {
    UIData uiData = getUiDataByKey(articleUI);
    if ( uiData != null )
    {
      uiData.activityShown = false;
    }
  }

  //TODO: what is this
  @Override
  public void setActiveArticleUI( @Nullable String articleUI )
  {

  }

  @NonNull
  @Override
  ShareControllerAPI getShareController( @NonNull String articleUI )
  {
    UIData uiData = getOrCreateUiDataByKey(articleUI);
    if ( uiData.shareController == null )
    {
      uiData.shareController =
          ShareController.create(this, articleUI, mDictionaryManager);
    }
    return uiData.shareController;
  }


  @NonNull
  @Override
  PractisePronunciationControllerAPI getPractisePronunciationController(@NonNull Context context,
                                                                        @NonNull String articleUI)
  {
    UIData uiData = getOrCreateUiDataByKey(articleUI);
    if ( uiData.practisePronunciationControllerAPI == null )
    {
      uiData.practisePronunciationControllerAPI =
              PractisePronunciationController.create(context);
    }
    return uiData.practisePronunciationControllerAPI;
  }

  @NonNull
  @Override
  CollectionView< CollectionView< ArticleItem, SubstringInfo >, Void > getWords(
      @Nullable String query, @Nullable Dictionary dictionary, @Nullable Dictionary.Direction direction )
  {
    if ( null != mEngine && mGetWordsParamsState.isChanged(query, dictionary, direction) )
    {
      CollectionView<CollectionView<ArticleItem, SubstringInfo>, Void> words =
              mEngine.getWordReferenceInList(dictionary.getId(), direction, query);
      words.updateSelection(0);
      mWordsCollectionView.set(words);
    }
    return mWordsCollectionView;
  }

  @Override
  void showSplashScreen( @NonNull Activity activity, @Nullable PendingIntent onCompletePendingIntent )
  {
    if ( null != mSplashScreenManager )
    {
      mSplashScreenManager.reset();
      if ( null != onCompletePendingIntent )
      {
        mSplashScreenManager.setOnCompletePendingIntent(onCompletePendingIntent);
      }
      mSplashScreenManager.showNext(activity);
    }
  }

  @Override
  boolean haveNotShownSplashScreens()
  {
    return null != mSplashScreenManager && mSplashScreenManager.haveNotShownScreens();
  }

  @Nullable
  @Override
  public byte[] getExternalImage(Dictionary.DictionaryId dictId, String imageBaseId, int listId, String imageKey) {
    if (mEngine != null) {
      return mEngine.getExternalImage(dictId, imageBaseId, listId, imageKey);
    }
    return null;
  }

  @Nullable
  @Override
  public ArticleItem findForSwipeSeparateArticles(boolean isSwipeForward) {
    return isSwipeForward ? separateArticlesStack.getNextItem() : separateArticlesStack.getPreviousItem();
  }

  @Nullable
  @Override
  public ArticleItem makeStepForSwipeSeparateArticles(boolean isSwipeForward) {
    return isSwipeForward ? separateArticlesStack.goToNext() : separateArticlesStack.goToPrevious();
  }

  @Override
  public void onApplicationSettingsSaved( @NonNull ApplicationSettings applicationSettings )
  {
    if ( articleScale != applicationSettings.getArticleScale() )
    {
        setArticleScale(applicationSettings.getArticleScale());
    }
  }

  static class GetWordsParamsState
  {
    String state;

    boolean isChanged( @Nullable String query, @Nullable Dictionary dictionary,
                       @Nullable Dictionary.Direction direction )
    {
      if ( null != query && null != dictionary && null != direction )
      {
        String newState = query + dictionary + direction;
        boolean equals = newState.equals(state);
        if ( !equals )
        {
          state = newState;
        }

        return !equals;
      }
      else
      {
        return false;
      }
    }
  }

  @VisibleForTesting
  protected static class UIData
  {
    @Nullable
    Class                articleActivityClass = null;
    @Nullable
    Class<Activity>      shareActivityClass = null;
    @Nullable
    Class<Activity>      practisePronunciationClass = null;
    @Nullable
    ArticleControllerAPI articleController    = null;
    @Nullable
    ShareControllerAPI   shareController      = null;
    @Nullable
    PractisePronunciationControllerAPI practisePronunciationControllerAPI = null;

    boolean activityShown = false;
  }

  private UIData getUiDataByKey(@NonNull String articleUI) {
    return mUIDataMap.get(articleUI);
  }

  private UIData getOrCreateUiDataByKey(@NonNull String articleUI)
  {
    UIData uiData = mUIDataMap.get(articleUI);
    if (uiData == null)
    {
      mUIDataMap.put(articleUI, uiData = new UIData());
    }
    return uiData;
  }

  @VisibleForTesting
  protected boolean tryShowArticleAction( @NonNull UIData uiData,
                                        @Nullable ArticleItem articleItem,
                                        @Nullable ShowArticleOptions showArticleOptions )
  {
    if( uiData.articleController != null )
    {
      uiData.articleController.nextTranslation(articleItem, showArticleOptions);
      return true;
    }
    return false;
  }
}
