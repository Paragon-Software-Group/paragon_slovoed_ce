/*
 * slovoed
 *
 *  Created on: 26.02.18
 *      Author: Ivan Zhdanov
 * Last Modify: $Id: ApplicationState.java 329367 2019-01-11 09:54:16Z mamedov $
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

package com.paragon_software.slovoed;

import android.annotation.TargetApi;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Build;
import android.os.Bundle;
import android.os.Parcelable;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.about_manager.AboutManagerAPI;
import com.paragon_software.about_manager.AboutMangerHolder;
import com.paragon_software.analytics_manager.AnalyticsManagerAPI;
import com.paragon_software.article_manager.ArticleControllerType;
import com.paragon_software.article_manager.ArticleManagerAPI;
import com.paragon_software.article_manager.ArticleManagerHolder;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.dictionary_manager.DictionaryManagerHolder;
import com.paragon_software.dictionary_manager.IDictionaryManagerFactory;
import com.paragon_software.dictionary_manager.ParagonDictionaryManager;
import com.paragon_software.dictionary_manager.downloader.DictionaryMeta;
import com.paragon_software.dictionary_manager.downloader.DownloadInfoWrapper;
import com.paragon_software.dictionary_manager.downloader.DownloadLibraryBuilder;
import com.paragon_software.dictionary_manager.downloader.IDownloadNotification;
import com.paragon_software.dictionary_manager.downloader.IDownloadStrings;
import com.paragon_software.dictionary_manager.exceptions.InitDictionaryManagerException;
import com.paragon_software.dictionary_manager_factory.BaseCatalogDictionariesCreator;
import com.paragon_software.dictionary_manager_factory.ParagonDictionaryManagerFactory;
import com.paragon_software.engine.SlovoedEngine;
import com.paragon_software.engine.SlovoedEngineSound;
import com.paragon_software.engine.SyncSlovoedEngine;
import com.paragon_software.favorites_manager.BaseFavoritesSorter;
import com.paragon_software.favorites_manager.FavoritesManagerAPI;
import com.paragon_software.favorites_manager.FavoritesManagerHolder;
import com.paragon_software.flash_cards_manager.FlashcardManagerAPI;
import com.paragon_software.hint_manager.HintManagerAPI;
import com.paragon_software.hint_manager.HintManagerHolder;
import com.paragon_software.history_manager.HistoryManagerAPI;
import com.paragon_software.history_manager.HistoryManagerHolder;
import com.paragon_software.information_manager.InformationManagerAPI;
import com.paragon_software.information_manager.InformationMangerHolder;
import com.paragon_software.license_manager_api.ActivateSource;
import com.paragon_software.license_manager_api.LicenseFeature;
import com.paragon_software.license_manager_api.LicenseManager;
import com.paragon_software.license_manager_api.LicenseManagerHolder;
import com.paragon_software.license_manager_api.exceptions.ActivateException;
import com.paragon_software.license_manager_api.exceptions.InvalidActivationSourceException;
import com.paragon_software.license_manager_api.exceptions.PurchaseExceptions;
import com.paragon_software.license_manager_api.exceptions.UserAccountManagementException;
import com.paragon_software.native_engine.EngineArticleAPI;
import com.paragon_software.native_engine.EngineInformationAPI;
import com.paragon_software.native_engine.EngineSerializerAPI;
import com.paragon_software.native_engine.EngineSoundAPI;
import com.paragon_software.native_engine.SearchEngineAPI;
import com.paragon_software.navigation_manager.NavigationHolder;
import com.paragon_software.navigation_manager.NavigationManagerApi;
import com.paragon_software.news_manager.NewsManagerAPI;
import com.paragon_software.news_manager.NewsManagerHolder;
import com.paragon_software.news_manager.NewsNotificationOptions;
import com.paragon_software.news_manager.NewsRes;
import com.paragon_software.odapi_ui.OpenDictionaryAPIService;
import com.paragon_software.quiz.QuizManagerAPI;
import com.paragon_software.quiz.QuizManagerHolder;
import com.paragon_software.screen_opener_api.ScreenOpenerAPI;
import com.paragon_software.screen_opener_api.ScreenType;
import com.paragon_software.search_manager.SearchManagerAPI;
import com.paragon_software.search_manager.SearchManagerHolder;
import com.paragon_software.settings_manager.BaseSettingsManager;
import com.paragon_software.settings_manager.ISettingsStorage;
import com.paragon_software.settings_manager.SettingsManagerAPI;
import com.paragon_software.settings_manager.SettingsManagerHolder;
import com.paragon_software.settings_manager.exceptions.ManagerInitException;
import com.paragon_software.slovoed.exceptions.InitializeArticleManagerException;
import com.paragon_software.slovoed.exceptions.InitializeDictionaryManagerException;
import com.paragon_software.slovoed.exceptions.InitializeSettingsManagerException;
import com.paragon_software.sound_manager.BaseSoundManager;
import com.paragon_software.sound_manager.OnlineSoundApi;
import com.paragon_software.sound_manager.SoundManagerAPI;
import com.paragon_software.splash_screen_manager.ISplashScreen;
import com.paragon_software.splash_screen_manager.ISplashScreenManager;
import com.paragon_software.splash_screen_manager.ParagonSplashScreenManager;
import com.paragon_software.splash_screen_manager.SplashScreenManagerHolder;
import com.paragon_software.splash_screen_manager.exceptions.SplashScreenManagerInitException;
import com.paragon_software.testmode.TestModeAPI;
import com.paragon_software.testmode.TestModeParagon;
import com.paragon_software.testmode_ui.TestModeActivity;
import com.paragon_software.theme_manager.ApplicationThemeManager;
import com.paragon_software.theme_manager.ApplicationThemeManagerAPI;
import com.paragon_software.theme_manager.ApplicationThemeManagerHolder;
import com.paragon_software.toolbar_manager.ParagonToolbarManager;
import com.paragon_software.toolbar_manager.ToolbarControllerType;
import com.paragon_software.toolbar_manager.ToolbarFactory;
import com.paragon_software.toolbar_manager.ToolbarManagerHolder;
import com.paragon_software.trial_manager.TrialManagerAPI;
import com.paragon_software.user_core_manager.UserCoreManagerAPI;
import com.paragon_software.user_core_manager.UserCoreManagerHolder;
import com.paragon_software.utils_slovoed.device.DeviceUtils;
import com.paragon_software.utils_slovoed.font.FontsUtils;
import com.paragon_software.utils_slovoed.pdahpc.PDAHPCDataParser;
import com.paragon_software.utils_slovoed.text.LocalizedString;
import com.paragon_software.word_of_day.WotDManagerAPI;
import com.paragon_software.word_of_day.WotDManagerHolder;
import com.paragon_software.word_of_day.WotDRes;
import com.paragon_software.word_of_day.WotDResProviderAPI;

import java.util.ArrayList;
import java.util.List;

public final class ApplicationState
{

  @NonNull
  private final static LicenseManagerHolder   mLicenseManager       = new LicenseManagerHolder();
  @Nullable
  private static SettingsManagerAPI           mSettingsManager      = null;

  private static SearchEngineAPI              mSearchEngine         = null;

  private static EngineInformationAPI         mEngineInformation    = null;

  private static EngineSoundAPI               mEngineSound          = null;
  @Nullable
  private static ISettingsStorage[]           storage;

  private static ScreenOpenerAPI              mScreenOpener;

  private static ParagonToolbarManager        mToolbarManager       = null;
  @Nullable
  private static ToolbarFactory               mToolbarFactory       = null;

  private static SearchManagerAPI             mSearchManager        = null;

  private static ArticleManagerAPI            mArticleManager       = null;

  private static FavoritesManagerAPI          mFavoritesManager     = null;

  private static HistoryManagerAPI            mHistoryManager       = null;

  private static SoundManagerAPI              mSoundManager         = null;

  private static AboutManagerAPI              mAboutManager         = null;

  private static InformationManagerAPI        mInformationManager   = null;

  private static HintManagerAPI               mHintManager          = null;

  private static DictionaryManagerAPI         mDictionaryManager;

  private static TestModeAPI                  mTestModeAPI;

  private static ISplashScreenManager         mSplashScreenManager;

  private static AnalyticsManagerAPI          mAnalyticsManager;
  private static NewsManagerAPI               mNewsManager;
  private static WotDManagerAPI               mWotDManager;
  private static QuizManagerAPI               mQuizManager;

  private static FlashcardManagerAPI          mFlashcardManager;
  private static UserCoreManagerAPI           mUserCoreManager;
  private static NavigationManagerApi         mNavigationManager;

  private static ApplicationThemeManagerAPI   mThemeManager;

  private static final String HTTP_DOWNLOADER_NOTIFICATION_CHANNEL_ID = "SLOVOED_HTTP_DOWNLOADER_NOTIFICATION";

  private static final int HTTP_DOWNLOADER_NOTIFICATION_BASE_ID = 5454;

  private static final String SLOVOED_NOTIFICATION_CHANNEL_ID = "SLOVOED_NOTIFICATION_CHANNEL";

  private static final int TRIAL_NOTIFICATION_BASE_ID = 7654;

  private static final int TRIAL_LOADER_BASE_ID = 100;

  @NonNull
  public static ArticleManagerAPI getArticleManager() throws InitializeArticleManagerException
  {
    if ( mArticleManager == null )
      throw new InitializeArticleManagerException();
    return mArticleManager;
  }

  @NonNull
  public static LicenseManager getLicenseManager()
  {
    return mLicenseManager;
  }

  @Nullable
  public static ToolbarFactory getToolbarFactory()
  {
    return mToolbarFactory;
  }

  public static EngineInformationAPI getEngineInformation()
  {
    return mEngineInformation;
  }

  public static SearchManagerAPI getSearchManager()
  {
    return mSearchManager;
  }

  public static ApplicationThemeManagerAPI getApplicationThemeManager()
  {
    return mThemeManager;
  }

  @NonNull
  static SearchEngineAPI getSearchEngine()
  {
    if(mSearchEngine == null)
      throw new IllegalStateException("Search engine not created");
    return mSearchEngine;
  }

  static void initialize( @NonNull final Context context )
      throws InitializeSettingsManagerException,
             InitializeDictionaryManagerException
  {
    FontsUtils.init(context);
    DeviceUtils.init(context);

    BaseCatalogDictionariesCreator creator = new BaseCatalogDictionariesCreator(context);
    ParagonDictionaryManagerFactory.initDictionaries(context, creator);
    // !!!
    // !!! DO NOT use BuildConfig fields before this line
    // !!!
    initEngine(context);

    configureTrial(context.getResources(), creator);

    if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
      createNotificationChannel(context);
    }

    initScreenOpener();

    initSettingsManager(context, mScreenOpener);

    ActivateSource[] activationSources = BuildConfig.activationSources;
    if (activationSources != null)
    {
      for (ActivateSource activateSource : activationSources) {
        activateSource.setSettingsManager(mSettingsManager);
      }
      initLicenseManager(context, activationSources);
    }

    initHintManager(mSettingsManager);

    initUserCoreManager(context, mSettingsManager);

    initDictionaryManager(context, mEngineSound, mLicenseManager, mScreenOpener
            , mSearchEngine, mSettingsManager, mUserCoreManager);

    initAboutManger(mDictionaryManager);

    initToolbarManager(context, mDictionaryManager);

    initSplashScreenManager(context, mSettingsManager);

    initAnalyticsManager(context, mSettingsManager);

    initSoundManager(context, mEngineSound, mDictionaryManager);

    initNewsManager(context, mHintManager, mScreenOpener, mSettingsManager, mToolbarManager);

    initFavoritesManager(mDictionaryManager, mHintManager, mScreenOpener, mSearchEngine
            , mSettingsManager, mSoundManager, mToolbarManager);

    initHistoryManager(mDictionaryManager, mHintManager, mScreenOpener, mSearchEngine
            , mSettingsManager, mSoundManager, mToolbarManager);

    initTestMode(mDictionaryManager, mFavoritesManager, mHistoryManager, mNewsManager, mSearchEngine);

    initSearchManager(mDictionaryManager, mFavoritesManager, mHintManager, mHistoryManager
            , mScreenOpener, mSearchEngine, mSettingsManager, mSoundManager, mTestModeAPI, mToolbarManager);

    initArticleManager(mDictionaryManager, mFavoritesManager, mHintManager, mHistoryManager
            , mSearchEngine, mSettingsManager, mSearchManager, mSoundManager);

    initInformationManager(context, mAboutManager, mArticleManager, mDictionaryManager, mSearchEngine);

    initWotDManager(context, mArticleManager, mSettingsManager, mSearchEngine
            , BuildConfig.navigationManager, mSoundManager, mDictionaryManager, mHintManager);

    initNavigationManager(mArticleManager, mDictionaryManager, mEngineInformation
            , mFavoritesManager, mHintManager, mHistoryManager, mNewsManager, mScreenOpener
            , mSearchManager, mSettingsManager, mToolbarManager, mWotDManager);

    initQuizManager(context, mArticleManager, mHintManager, mNavigationManager, mSearchEngine
            , mSettingsManager, mSoundManager);

    initFlashcardManager(context, mDictionaryManager, mNavigationManager, mSearchEngine
            , mSettingsManager, mSoundManager);

    initThemeManager(context);

    mArticleManager.registerSplashScreenManager(mSplashScreenManager);
    mArticleManager.registerFlashcardManager(mFlashcardManager);

    OpenDictionaryAPIService.registerDictionaryManager(mDictionaryManager);
    OpenDictionaryAPIService.registerEngine(new SyncSlovoedEngine(context, mDictionaryManager));
    OpenDictionaryAPIService.registerArticleManager(mArticleManager);
    OpenDictionaryAPIService.registerArticleActivity(BuildConfig.articleActivity.getClass());
    OpenDictionaryAPIService.registerSplashscreenManager(mSplashScreenManager);
  }

  private static void initScreenOpener()
  {
    boolean useTabletOpener = BuildConfig.useTabletOpener && DeviceUtils.isTablet();

    mScreenOpener = BuildConfig.navigationManager.createScreenOpener(
                                          BuildConfig.articleControllerId, useTabletOpener);
  }

  private static void initSettingsManager( @NonNull final Context context,
                                           ScreenOpenerAPI screenOpener)
          throws InitializeSettingsManagerException
  {
    try
    {
      mSettingsManager = new BaseSettingsManager(context, initStorage(context), BuildConfig.settingsMigrationHelper);
      mSettingsManager.registerScreenOpener(screenOpener);

      SettingsManagerHolder.setManager(mSettingsManager);
    }
    catch ( final ManagerInitException e )
    {
      e.printStackTrace();
      throw new InitializeSettingsManagerException();
    }
  }

  private static void configureTrial( @NonNull final Resources resources,
									  @NonNull BaseCatalogDictionariesCreator creator )
  {
    TrialManagerAPI.Builder trialBuilder = getTrialManagerBuilder();
    if ( trialBuilder != null) {
      ParagonDictionaryManagerFactory.initTrials(creator, trialBuilder);
      trialBuilder.setLoaderBaseId(TRIAL_LOADER_BASE_ID);
      trialBuilder.setMaxTrialProlongationCountExceededErrorMsg(
              R.string.utils_slovoed_ui_common_trial_prolongation_exceeded);
      trialBuilder.setNotificationOptions(BuildConfig.buyActivity,
              SLOVOED_NOTIFICATION_CHANNEL_ID,
              android.R.drawable.stat_sys_warning,
              BitmapFactory.decodeResource(resources, R.mipmap.ic_launcher),
              R.string.trial_notification_title_is_over,
              R.string.utils_slovoed_ui_common_trial_notification_title_expires,
              R.string.trial_notification_title_is_over,
              R.string.utils_slovoed_ui_common_trial_notification_time_remain_message,
              TRIAL_NOTIFICATION_BASE_ID);
    }
  }

  private static void initLicenseManager( @NonNull final Context context ,
                                          @NonNull ActivateSource[] activationSources )
  {
    try
    {
      mLicenseManager.initialize( null );
    }
    catch ( PurchaseExceptions ignore )
    {
    }
    catch ( InvalidActivationSourceException ignore )
    {
    }
    catch ( UserAccountManagementException ignore )
    {
    }
    catch (ActivateException e)
    {
        e.printStackTrace();
    }
  }

  private static void initHintManager( SettingsManagerAPI settingsManager )
  {
    if ( BuildConfig.hintManagerFactory != null )
    {
      mHintManager = BuildConfig.hintManagerFactory
          .registerHintUIHelper(BuildConfig.hintManagerUIHelper)
          .registerSettingsManager(settingsManager)
          .create();
    }
    HintManagerHolder.setManager(mHintManager);
  }

  private static void initUserCoreManager( @NonNull final Context context,
                                           SettingsManagerAPI settingsManager )
  {
    if ( BuildConfig.userCoreManagerFactory != null )
    {
      UserCoreManagerAPI.Factory factory = BuildConfig.userCoreManagerFactory;
      factory.registerSettingsManager(settingsManager)
              .registerUi(BuildConfig.userCoreManagerActivityClass)
              .registerLoginDialog(BuildConfig.userCoreManagerLoginDialog)
              .registerResetCodeDialog(BuildConfig.userCoreManagerResetPasswordDialog)
              .registerApplicationContext(context.getApplicationContext());

      mUserCoreManager = factory.create();
      UserCoreManagerHolder.setManager(mUserCoreManager);
    }
  }

  private static void initInformationManager( @NonNull final Context context,
                                              AboutManagerAPI aboutManager,
                                              ArticleManagerAPI articleManager,
                                              DictionaryManagerAPI dictionaryManager,
                                              SearchEngineAPI searchEngine )
  {
    Intent ppIntent = BuildConfig.ppInformationScreenFactory == null ? null :
                      BuildConfig.ppInformationScreenFactory.createIntent(context);

    Intent eulaIntent = BuildConfig.eulaInformationScreenFactory == null ? null :
                        BuildConfig.eulaInformationScreenFactory.createIntent(context);

    Intent riteAppIntent = BuildConfig.rateAppIntentFactory == null ? null :
        BuildConfig.rateAppIntentFactory.createIntent(context);

    mInformationManager = BuildConfig.informationManagerFactory
            .create(dictionaryManager, searchEngine, aboutManager,
                    ArticleControllerType.CONTROLLER_ID_ABOUT, articleManager,
                    BuildConfig.additionalArticleControllerId, ppIntent, eulaIntent, riteAppIntent);
    InformationMangerHolder.setManager(mInformationManager);
  }

  private static void initAboutManger( DictionaryManagerAPI dictionaryManager )
  {
    mAboutManager = BuildConfig.aboutManagerFactory.create(dictionaryManager, getEngineInformation());
    mAboutManager.registerUI(ArticleControllerType.CONTROLLER_ID_ABOUT, BuildConfig.aboutActivityClass);
    AboutMangerHolder.setManager(mAboutManager);
  }

  private static void initAnalyticsManager( @NonNull final Context context,
                                            SettingsManagerAPI settingsManager )
  {
    mAnalyticsManager = BuildConfig.analyticsManager;
    mAnalyticsManager.initialize(context, settingsManager);
  }

  private static void initWotDManager( @NonNull final Context context,
                                       ArticleManagerAPI articleManager,
                                       SettingsManagerAPI settingsManager,
                                       SearchEngineAPI searchEngine,
                                       NavigationManagerApi navigationManager,
                                       SoundManagerAPI soundManager,
                                       DictionaryManagerAPI dictionaryManager,
                                       HintManagerAPI hintManager )
  {
      if( BuildConfig.wotDManagerFactory != null )
      {
          WotDManagerAPI.Factory factory = BuildConfig.wotDManagerFactory;

        WotDResProviderAPI wotDResProvider = null;

        if( BuildConfig.wotDManagerResProviderFactory != null )
        {
          wotDResProvider = BuildConfig.wotDManagerResProviderFactory.registerContext(context)
                  .create();
          factory.registerResProvider(wotDResProvider);
        }

          factory.registerContext(context)
                  .registerSettingsManager(settingsManager)
                  .registerEngine((EngineArticleAPI) searchEngine)
                  .registerScreenOpener(navigationManager.createScreenOpener(
                          ArticleControllerType.OALD_CONTROLLER_ID_WORD_OF_THE_DAY,
                          BuildConfig.useTabletOpener && DeviceUtils.isTablet()))
                  .registerSoundManager(soundManager)
                  .registerDictionaryManager(dictionaryManager)
                  .registerFileProviderAuthorities(BuildConfig.APPLICATION_ID + ".word_of_day.fileprovider")
                  .registerHintManager(hintManager)
                  .registerNotificationOptions(
                          MainActivity.class,
                          wotDResProvider !=null ? wotDResProvider.getResId(WotDRes.NOTIFICATION_SMALL_ICON): R.mipmap.ic_launcher,
                          BitmapFactory.decodeResource(context.getResources(), R.drawable.ic_launcher));

          mWotDManager = factory.create();
          WotDManagerHolder.setManager(mWotDManager);

          mArticleManager.registerArticleUI(ArticleControllerType.OALD_CONTROLLER_ID_WORD_OF_THE_DAY, DeviceUtils.isTablet() ?
                  BuildConfig.articleActivityTablet.getClass()
                  : BuildConfig.articleActivity.getClass());
      }
  }

  private static void initQuizManager( @NonNull final Context context,
                                       ArticleManagerAPI articleManager,
                                       HintManagerAPI hintManager,
                                       NavigationManagerApi navigationManager,
                                       SearchEngineAPI searchEngine,
                                       SettingsManagerAPI settingsManager,
                                       SoundManagerAPI soundManager )
  {
      if( BuildConfig.quizManagerFactory != null )
      {
        mQuizManager = BuildConfig.quizManagerFactory
                .registerDictionaryManager(mDictionaryManager)
                .registerSettingsManager(mSettingsManager)
                .registerArticleEngine((EngineArticleAPI) mSearchEngine)
                .registerScreenOpener(navigationManager.createScreenOpener(
                        ArticleControllerType.OALD_CONTROLLER_ID_QUIZ,
                        BuildConfig.useTabletOpener && DeviceUtils.isTablet()))
                .registerSoundManager(soundManager)
                .registerHintManager(hintManager)
                .registerQuizManagerUI(MainActivity.class)
                .registerQuizAchievementsGenerator(
                        BuildConfig.quizManagerAchievementsGeneratorFactory
                                .registerContext(context)
                                .create())
                .registerQuizProgressListFactory(BuildConfig.quizManagerProgressListFactory)
                .create();

        QuizManagerHolder.setManager(mQuizManager);

        mArticleManager.registerArticleUI(ArticleControllerType.OALD_CONTROLLER_ID_QUIZ, DeviceUtils.isTablet() ?
                BuildConfig.articleActivityTablet.getClass()
                : BuildConfig.articleActivity.getClass());
      }
  }

  private static void initNavigationManager( ArticleManagerAPI articleManager,
                                             DictionaryManagerAPI dictionaryManager,
                                             EngineInformationAPI engineInformation,
                                             FavoritesManagerAPI favoritesManager,
                                             HintManagerAPI hintManager,
                                             HistoryManagerAPI historyManager,
                                             NewsManagerAPI newsManager,
                                             ScreenOpenerAPI screenOpener,
                                             SearchManagerAPI searchManager,
                                             SettingsManagerAPI settingsManager,
                                             ParagonToolbarManager toolbarManager,
                                             WotDManagerAPI wotDManager )
  {
    mNavigationManager = BuildConfig.navigationManager;
    mNavigationManager.registerDictionaryManager(dictionaryManager);
    mNavigationManager.registerSearchManager(searchManager);
    mNavigationManager.registerNewsManager(newsManager);
    mNavigationManager.registerFavoritesManager(favoritesManager);
    mNavigationManager.registerHistoryManager(historyManager);
    mNavigationManager.registerToolbarManager(toolbarManager);
    mNavigationManager.registerEngineInformation(engineInformation);
    mNavigationManager.registerWotDManager(wotDManager);
    mNavigationManager.registerSettingsManager(settingsManager);
    mNavigationManager.registerHintManager(hintManager);
    mNavigationManager.registerArticleManager(articleManager);

    mNavigationManager.registerFragment(ScreenType.Dictionaries, BuildConfig.dictionaryManagerFragment);
    mNavigationManager.registerFragment(ScreenType.DictionaryDescription, BuildConfig.dictionaryDescriptionFragmentOald);
    mNavigationManager.registerFragment(ScreenType.Search, DeviceUtils.isTablet() ? BuildConfig.searchManagerFragmentTablet : BuildConfig.searchManagerFragment);
    mNavigationManager.registerFragment(ScreenType.Favorites, BuildConfig.favoritesManagerFragment);
    mNavigationManager.registerFragment(ScreenType.History, BuildConfig.historyManagerFragment);
    mNavigationManager.registerFragment(ScreenType.BottomNavigation, BuildConfig.bottomNavigationFragment);
    mNavigationManager.registerFragment(ScreenType.Settings, BuildConfig.settingsManagerFragment);
    mNavigationManager.registerFragment(ScreenType.SettingsGeneral, BuildConfig.settingsManagerFragmentGeneral);
    mNavigationManager.registerFragment(ScreenType.SettingsMyView, BuildConfig.settingsManagerFragmentMyView);
    mNavigationManager.registerFragment(ScreenType.SettingsNotifications, BuildConfig.settingsManagerFragmentNotifications);
    mNavigationManager.registerFragment(ScreenType.Information, BuildConfig.informationFragment);
    mNavigationManager.registerFragment(ScreenType.NewsList, BuildConfig.newsManagerFragment);
    mNavigationManager.registerFragment(ScreenType.NewsItem, BuildConfig.newsFragmentItem);
    mNavigationManager.registerFragment(ScreenType.Flashcards, BuildConfig.flashCardsManagerFragment);
    mNavigationManager.registerFragment(ScreenType.WotDList, BuildConfig.wotDManagerListFragment);
    mNavigationManager.registerFragment(ScreenType.WotDItem, BuildConfig.wotDManagerItemFragment);
    mNavigationManager.registerFragment(ScreenType.Download, BuildConfig.downloadManagerFragment);
    mNavigationManager.registerFragment(ScreenType.QuizList, BuildConfig.quizManagerListFragment);
    mNavigationManager.registerFragment(ScreenType.Quiz, DeviceUtils.isTablet() ? BuildConfig.quizManagerFragmentTablet : BuildConfig.quizManagerFragment);
    mNavigationManager.registerFragment(ScreenType.QuizStatistic, BuildConfig.quizManagerStatisticFragment);
    mNavigationManager.registerFragment(ScreenType.QuizAchievement, BuildConfig.quizManagerAchievementFragment);
    mNavigationManager.registerFragment(ScreenType.Article, DeviceUtils.isTablet() ? BuildConfig.articleFragmentTablet : BuildConfig.articleFragment);

    mNavigationManager.registerActivity(ScreenType.DictionaryDescription, BuildConfig.descriptionActivityClass);
    mNavigationManager.registerActivity(ScreenType.History, BuildConfig.historyManagerActivityClass);
    mNavigationManager.registerActivity(ScreenType.Favorites, BuildConfig.favoritesManagerActivityClass);
    mNavigationManager.registerActivity(ScreenType.NewsItem, BuildConfig.newsItemActivityClass);
    mNavigationManager.registerActivity(ScreenType.WotDItem, BuildConfig.wotDManagerItemActivityClass);
    mNavigationManager.registerActivity(ScreenType.Quiz, BuildConfig.quizManagerActivityClass);
    mNavigationManager.registerActivity(ScreenType.QuizAchievement, BuildConfig.quizManagerAchievementActivityClass);
    mNavigationManager.registerActivity(ScreenType.QuizStatistic, BuildConfig.quizManageStatisticActivityClass);
    mNavigationManager.registerActivity(ScreenType.SettingsGeneral, BuildConfig.settingsGeneralActivityClass);
    mNavigationManager.registerActivity(ScreenType.SettingsMyView, BuildConfig.settingsMyViewActivityClass);
    mNavigationManager.registerActivity(ScreenType.SettingsNotifications, BuildConfig.settingsNotificationsActivityClass);

    mNavigationManager.registerScreenOpener(screenOpener);

    NavigationHolder.setNavigationManager(mNavigationManager);
  }

  private static void initNewsManager( @NonNull final Context context,
                                       HintManagerAPI hintManager,
                                       ScreenOpenerAPI screenOpener,
                                       SettingsManagerAPI settingsManager,
                                       ParagonToolbarManager toolbarManager )
  {
    if ( BuildConfig.newsManagerFactory != null )
    {
      mNewsManager = BuildConfig.newsManagerFactory
          .registerSettingsManager(mSettingsManager)
          .registerToolbarManager(mToolbarManager)
          .registerHintManager(mHintManager)
          .registerScreenOpener(mScreenOpener)
              .create(context,
                      new NewsNotificationOptions("chanel_news",
                              BuildConfig.newsResProvider != null ?
                                      BuildConfig.newsResProvider.getResId(NewsRes.NOTIFICATION_SMALL_ICON)
                                      : R.mipmap.ic_launcher,
                              R.drawable.ic_launcher,
                              MainActivity.class),
                      PDAHPCDataParser.parseCatalog(context).getId(),
                      NewsManagerAPI.Mode.CONTAINER_MODE,
                      BuildConfig.newsManagerPkey);
      NewsManagerHolder.setManager(mNewsManager);
    }
  }

  private static void initTestMode( DictionaryManagerAPI dictionaryManager,
                                    FavoritesManagerAPI favoritesManager,
                                    HistoryManagerAPI historyManager,
                                    NewsManagerAPI newsManager,
                                    SearchEngineAPI searchEngine )
  {
    mTestModeAPI = new TestModeParagon();
    mTestModeAPI.registerDictionaryManager(dictionaryManager);
    mTestModeAPI.registerSearchEngine(searchEngine);
    mTestModeAPI.registerFavoritesManager(favoritesManager);
    mTestModeAPI.registerHistoryManager(historyManager);
    mTestModeAPI.registerNewsManager(newsManager);
    mTestModeAPI.registerTestModeUI(TestModeActivity.class);
  }

  private static void initSplashScreenManager( @NonNull final Context context,
                                               SettingsManagerAPI settingsManager )
  {
    List<ISplashScreen> startScreens = new ArrayList<>();
    ISplashScreen.Factory[] startScreensFactories = BuildConfig.startScreensFactories;
    if ( startScreensFactories != null)
    {
      for ( int i = 0 ; i < startScreensFactories.length ; i++ )
      {
        ISplashScreen startScreen = startScreensFactories[i].create(context);
        if ( startScreen != null )
        {
          startScreens.add(startScreen);
        }
      }
    }
    try
    {
      mSplashScreenManager = new ParagonSplashScreenManager(
              settingsManager, startScreens.toArray(new ISplashScreen[0]), MainActivity.class);
    }
    catch ( SplashScreenManagerInitException exception )
    {
      throw new IllegalStateException(exception);
    }

    SplashScreenManagerHolder.setManager(mSplashScreenManager);
  }

  @NonNull
  private static ISettingsStorage[] initStorage( @NonNull final Context context )
  {
    if ( null != storage )
    {
      return storage;
    }

    ArrayList< ISettingsStorage > storageArray = new ArrayList<>();
    for ( Class< ? > storageClass : BuildConfig.settingsManagerStoragesClass )
    {
      ISettingsStorage newStorage = null;
      try
      {
        newStorage = (ISettingsStorage) storageClass.getConstructor().newInstance();
      }
      catch ( Exception ignore )
      {
      }
      if ( null == newStorage )
      {
        try
        {
          newStorage = (ISettingsStorage) storageClass.getConstructor(Context.class).newInstance(context);
        }
        catch ( Exception ignore )
        {
        }
      }
      if ( null == newStorage )
      {
        throw new ExceptionInInitializerError("Can't init storage: No such constructor. "
                                                  + "You storage should contains constructor without parameters, or constructor with one parameter: Context");
      }
      storageArray.add(newStorage);
    }
    return storageArray.toArray(new ISettingsStorage[0]);
  }

  private static void initEngine( @NonNull final Context context )
  {
    mSearchEngine = new SlovoedEngine(context);
    mEngineInformation = (EngineInformationAPI) mSearchEngine;
    mEngineSound = new SlovoedEngineSound(context);
  }

  private static void initDictionaryManager( @NonNull final Context context,
                                             EngineSoundAPI engineSound,
                                             LicenseManagerHolder licenseManager,
                                             ScreenOpenerAPI screenOpener,
                                             SearchEngineAPI searchEngine,
                                             SettingsManagerAPI settingsManager,
                                             UserCoreManagerAPI userCoreManager )
          throws InitializeDictionaryManagerException
  {
    Dictionary.DictionaryId[] idList = new Dictionary.DictionaryId[BuildConfig.enableDictionaryList.length];
    int i = 0;
    for ( String dictId : BuildConfig.enableDictionaryList )
    {
      idList[i++] = new Dictionary.DictionaryId(dictId);
    }

    IDictionaryManagerFactory dictionaryManagerFactory = new ParagonDictionaryManagerFactory();
    DownloadLibraryBuilder downloadLibraryBuilder = createDownloadLibraryBuilder(context);

    try
    {
      mDictionaryManager = new ParagonDictionaryManager( context, licenseManager
              , dictionaryManagerFactory, settingsManager, searchEngine, idList
              , getTrialManagerBuilder(), downloadLibraryBuilder, userCoreManager, screenOpener);
    }
    catch ( InitDictionaryManagerException e )
    {
      e.printStackTrace();
      throw new InitializeDictionaryManagerException();
    }

    mDictionaryManager.registerDictionaryManagerUI(BuildConfig.buyActivity, MainActivity.class);
    mDictionaryManager.setDefaultDictionaryAndDirection(
        BuildConfig.dictionaryManagerDefaultDictionaryId, BuildConfig.dictionaryManagerDefaultDirection);
    engineSound.registerDictionaryManager(mDictionaryManager);

    DictionaryManagerHolder.setManager(mDictionaryManager);
  }

  private static void initToolbarManager( @NonNull final Context context,
                                          DictionaryManagerAPI dictionaryManager )
  {
    //    mToolbarManager = (ParagonToolbarManager) BuildConfig.toolbarManager;

    //    Test initialization

    mToolbarManager = new ParagonToolbarManager(dictionaryManager);

    ToolbarManagerHolder.setManager(mToolbarManager);

    mToolbarManager.setDefaultDictionaryTitle(LocalizedString.from(context, R.string.utils_slovoed_ui_common_dictionary_list_default_title));
    Bitmap bitmap = BitmapFactory.decodeResource(context.getResources(), R.drawable.ic_launcher);
    mToolbarManager.setDefaultDictionaryIcon(bitmap);

    // NOTE
    // Create default controller in ParagonToolbarManager cause it is doing some initialization
    // to store state about current dictionary and direction ( in dictionary manager ).
    // Did't find out what exactly it is doing and how to refactor it, but without this controller
    // in the OALD10 app for example default dictionary and direction would not be used in search
    // screen on app start...
    // TODO Need to refactor toolbar, dictionary, search managers
    mToolbarManager.getController(ToolbarControllerType.DEFAULT_CONTROLLER);

    mToolbarFactory = BuildConfig.toolbarFactory;
  }

  private static void initSearchManager( DictionaryManagerAPI dictionaryManager,
                                         FavoritesManagerAPI favoritesManager,
                                         HintManagerAPI hintManager,
                                         HistoryManagerAPI historyManager,
                                         ScreenOpenerAPI screenOpener,
                                         SearchEngineAPI searchEngine,
                                         SettingsManagerAPI settingsManager,
                                         SoundManagerAPI soundManager,
                                         TestModeAPI testModeAPI,
                                         ParagonToolbarManager toolbarManager )
  {
    mSearchManager = BuildConfig.searchManager;
    mSearchManager.registerTestMode(testModeAPI);
    mSearchManager.registerSearchEngine(searchEngine);
    mSearchManager.registerScreenOpener(screenOpener);
    mSearchManager.registerSettingsManager(settingsManager);
    mSearchManager.registerToolbarManager(toolbarManager);
    mSearchManager.registerDictionaryManager(dictionaryManager);
    mSearchManager.registerSoundManager(soundManager);
    mSearchManager.registerHistoryManager(historyManager);
    mSearchManager.registerFavoritesManager(favoritesManager);
    mSearchManager.registerHintManager(hintManager);

    SearchManagerHolder.setManager(mSearchManager);
  }

  private static void initSoundManager( @NonNull final Context context,
                                        EngineSoundAPI engineSound,
                                        DictionaryManagerAPI dictionaryManager )
  {
    OnlineSoundApi onlineSound = null;
    if (BuildConfig.onlineSoundFactory != null) {
      onlineSound = BuildConfig.onlineSoundFactory.registerContext(context).registerEngineSound(engineSound).create();
    }
    mSoundManager = new BaseSoundManager(engineSound, dictionaryManager, onlineSound);
  }

  private static void initFavoritesManager( DictionaryManagerAPI dictionaryManager,
                                            HintManagerAPI hintManager,
                                            ScreenOpenerAPI screenOpener,
                                            SearchEngineAPI searchEngine,
                                            SettingsManagerAPI settingsManager,
                                            SoundManagerAPI soundManager,
                                            ParagonToolbarManager toolbarManager )
  {
    mFavoritesManager = BuildConfig.favoritesManagerFactory
            .registerSearchEngine(searchEngine)
            .registerHintManager(hintManager)
            .registerUI(BuildConfig.favoritesManagerActivityClass)
            .registerAddInDirectoryUI(BuildConfig.favoritesManagerAddInDirectoryUI)
            .registerFileProviderAuthorities(BuildConfig.APPLICATION_ID + ".favorites_manager.fileprovider")
            .create(screenOpener, (EngineSerializerAPI) searchEngine, settingsManager
                    , dictionaryManager, toolbarManager, soundManager, new BaseFavoritesSorter()
                    , BuildConfig.favoritesManagerExportHtmlBuilderClass, BuildConfig.favoritesMaxNumberOfWords);

    FavoritesManagerHolder.setManager(mFavoritesManager);
  }

  private static void initHistoryManager( DictionaryManagerAPI dictionaryManager,
                                          HintManagerAPI hintManager,
                                          ScreenOpenerAPI screenOpener,
                                          SearchEngineAPI searchEngine,
                                          SettingsManagerAPI settingsManager,
                                          SoundManagerAPI soundManager,
                                          ParagonToolbarManager toolbarManager )
  {
      mHistoryManager = BuildConfig.historyManagerFactory.registerScreenOpener(screenOpener)
              .registerEngineSerializer((EngineSerializerAPI) searchEngine)
              .registerSettingsManager(settingsManager)
              .registerDictionaryManager(dictionaryManager)
              .registerSoundManager(soundManager)
              .registerToolbarManager(toolbarManager)
              .registerHintManager(hintManager)
              .registerUI(BuildConfig.historyManagerActivityClass)
              .registerExportHTMLBuilderClass(BuildConfig.historyManagerExportHtmlBuilderClass)
              .setMaxNumberOfWords(BuildConfig.historyMaxNumberOfWords)
              .registerFileProviderAuthorities(BuildConfig.APPLICATION_ID + ".history_manager.fileprovider")
              .create();

      HistoryManagerHolder.setManager(mHistoryManager);
  }

  private static void initArticleManager( DictionaryManagerAPI dictionaryManager,
                                          FavoritesManagerAPI favoritesManager,
                                          HintManagerAPI hintManager,
                                          HistoryManagerAPI historyManager,
                                          SearchEngineAPI searchEngine,
                                          SettingsManagerAPI settingsManager,
                                          SearchManagerAPI searchManager,
                                          SoundManagerAPI soundManager )
  {
    mArticleManager = BuildConfig.articleManager;
    mArticleManager.registerDictionaryManager(dictionaryManager);
    mArticleManager.registerEngine((EngineArticleAPI) searchEngine);
    mArticleManager.registerFavoritesManager(favoritesManager);
    mArticleManager.registerHistoryManager(historyManager);
    mArticleManager.registerSettingsManager(settingsManager);
    mArticleManager.registerSoundManager(soundManager);
    mArticleManager.registerSearchManager(searchManager);
    mArticleManager.registerHintManager(hintManager);
    mArticleManager.registerArticleUI(BuildConfig.articleControllerId , BuildConfig.articleActivity.getClass());
    mArticleManager.registerShareUI(BuildConfig.shareArticleControllerId, BuildConfig.shareActivity.getClass());
    mArticleManager.registerArticleUI(BuildConfig.additionalArticleControllerId, BuildConfig.additionalActivity.getClass());

    ArticleManagerHolder.setManager(mArticleManager);

    if( BuildConfig.practisePronunciationControllerId != null )
    {
      mArticleManager.registerPractisePronunciationUI(BuildConfig.practisePronunciationControllerId
              , BuildConfig.practisePronunciationActivity);
    }
  }

  private static void initFlashcardManager( Context context,
                                            DictionaryManagerAPI dictionaryManager,
                                            NavigationManagerApi navigationManager,
                                            SearchEngineAPI searchEngine,
                                            SettingsManagerAPI settingsManager,
                                            SoundManagerAPI soundManager )
  {
    mFlashcardManager = BuildConfig.flashcardManager;
    if(mFlashcardManager != null) {
      mFlashcardManager.registerEngineSerializer((EngineSerializerAPI) searchEngine);
      mFlashcardManager.registerSettingsManager(settingsManager);
      mFlashcardManager.registerDictionaryManager(dictionaryManager);
      mFlashcardManager.registerSoundManager(soundManager);
      mFlashcardManager.registerArticleFragmentFactory(() -> {
        Bundle bundle = new Bundle();
        bundle.putString(ArticleManagerAPI.EXTRA_CONTROLLER_ID, ArticleControllerType.CONTROLLER_ID_FLASHCARDS);
        BuildConfig.articleFragment.setArguments(bundle);
        return BuildConfig.articleFragment;
      });
      mFlashcardManager.registerScreenOpener(navigationManager.createScreenOpener(ArticleControllerType.CONTROLLER_ID_FLASHCARDS,
              BuildConfig.useTabletOpener && DeviceUtils.isTablet()));
      if(BuildConfig.flashcardExchangerFactory != null) {
        mFlashcardManager.registerFlashcardExchanger(
                BuildConfig.flashcardExchangerFactory
                        .registerContext(context)
                        .registerEngine(new SyncSlovoedEngine(context, dictionaryManager))
                        .create());
      }
    }
  }

  @Nullable
  private static TrialManagerAPI.Builder getTrialManagerBuilder()
  {
    return BuildConfig.trialManagerBuilder;
  }

  @NonNull
  private static DownloadLibraryBuilder createDownloadLibraryBuilder( @NonNull final Context context )
  {
    DownloadLibraryBuilder downloadLibraryBuilder = new DownloadLibraryBuilder();
    downloadLibraryBuilder.setAppContext(context);
    final float scaleFactor = context.getResources().getDisplayMetrics().density;
    downloadLibraryBuilder.setLargeIcon(Bitmap.createScaledBitmap(
            BitmapFactory.decodeResource(context.getResources(), R.drawable.ic_launcher), (int) ( 48 * scaleFactor ),
            (int) ( 48 * scaleFactor ), true));
    downloadLibraryBuilder.setChannelId(HTTP_DOWNLOADER_NOTIFICATION_CHANNEL_ID);
    IDownloadStrings downloadStrings = BuildConfig.downloadStrings;
    downloadLibraryBuilder.setDownloadStrings(downloadStrings);
    if ( Build.VERSION.SDK_INT >= Build.VERSION_CODES.O )
    {
      downloadLibraryBuilder.setNotificationChannel(new NotificationChannel(HTTP_DOWNLOADER_NOTIFICATION_CHANNEL_ID, downloadStrings.getNotificationChannelName(context.getResources()), NotificationManager.IMPORTANCE_DEFAULT));
    }
    downloadLibraryBuilder.setDownloadNotification(new IDownloadNotification()
    {
      @Override
      public int getSmallIcon( DownloadLibraryBuilder.DownloadStatus status )
      {
        switch (status) {
          case CONNECTING:
          case DOWNLOADING:
            return android.R.drawable.stat_sys_download;
          case SUCCESSFULL:
            return android.R.drawable.stat_sys_download_done;
          default:
            return android.R.drawable.stat_sys_warning;
        }
      }

      @Override
      public PendingIntent createPendingIntent( DownloadInfoWrapper infoWrapper )
      {
        DictionaryMeta dictionaryMeta = DictionaryMeta.class
                .cast(infoWrapper.getMeta().get(DownloadLibraryBuilder.META_DICTIONARY));
        Bundle serializable = new Bundle();
        serializable.putSerializable(DictionaryManagerAPI.UI_PENDING_INTENT_KEY_BASE_INFO_STATUS,
                infoWrapper.getDownloadStatus());

        Intent intent = new Intent(context, MainActivity.class);
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
        intent.putExtra(DictionaryManagerAPI.UI_PENDING_INTENT_KEY_BASE_FROM_NOTIFICATION, true);
        intent.putExtra(DictionaryManagerAPI.UI_EXTRA_DICTIONARY_ID_KEY, (Parcelable) dictionaryMeta.getId());
        intent.putExtra(DictionaryManagerAPI.UI_EXTRA_SERIALIZABLE_BUNDLE, serializable);
        int requestCode = infoWrapper.getUrl().hashCode();
        return PendingIntent.getActivity(context, requestCode, intent, PendingIntent.FLAG_UPDATE_CURRENT);
      }
    });
    return downloadLibraryBuilder;
  }

  @TargetApi(Build.VERSION_CODES.O)
  private static void createNotificationChannel( @NonNull final Context context )
  {
    NotificationManager notificationManager = (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);
    if(notificationManager != null) {
      String name = context.getString(R.string.utils_slovoed_ui_common_notification_channel_name);
      int importance = NotificationManager.IMPORTANCE_HIGH;
      NotificationChannel channel = new NotificationChannel(SLOVOED_NOTIFICATION_CHANNEL_ID, name, importance);
      notificationManager.createNotificationChannel(channel);
    }
  }

  private static void initThemeManager(@NonNull Context context)
  {
    mThemeManager  = new ApplicationThemeManager(context, mSettingsManager);
    ApplicationThemeManagerHolder.setManager(mThemeManager);
  }
}
