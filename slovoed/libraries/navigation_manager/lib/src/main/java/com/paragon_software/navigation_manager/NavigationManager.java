package com.paragon_software.navigation_manager;

import android.app.Activity;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;

import com.paragon_software.article_manager.ArticleManagerAPI;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.favorites_manager.FavoritesManagerAPI;
import com.paragon_software.hint_manager.HintManagerAPI;
import com.paragon_software.history_manager.HistoryManagerAPI;
import com.paragon_software.native_engine.EngineInformationAPI;
import com.paragon_software.news_manager.NewsManagerAPI;
import com.paragon_software.screen_opener_api.ScreenOpenerAPI;
import com.paragon_software.screen_opener_api.ScreenType;
import com.paragon_software.search_manager.SearchManagerAPI;
import com.paragon_software.settings_manager.SettingsManagerAPI;
import com.paragon_software.toolbar_manager.ToolbarManager;
import com.paragon_software.word_of_day.WotDManagerAPI;

import java.util.EnumMap;
import java.util.Map;
import java.util.TreeMap;
import java.util.concurrent.atomic.AtomicReference;

import io.reactivex.Observable;

public class NavigationManager extends NavigationManagerApi {
    @NonNull
    private final Map<String, NavigationControllerApi> mControllers = new TreeMap<>();
    private final AtomicReference<NavigationControllerApi> mActiveController = new AtomicReference<>();

    @Nullable
    private DictionaryManagerAPI mDictionaryManager;

    @Nullable
    private SearchManagerAPI mSearchManager;

    @Nullable
    private NewsManagerAPI mNewsManager;

    @Nullable
    private ToolbarManager mToolbarManager;

    @Nullable
    private EngineInformationAPI mEngineInformation;

    @Nullable
    private FavoritesManagerAPI mFavoritesManager;

    @Nullable
    private HistoryManagerAPI mHistoryManager;

    @Nullable
    private WotDManagerAPI mWotDManager;

    @Nullable
    private SettingsManagerAPI mSettingsManager;

    @Nullable
    private HintManagerAPI mHintManager;

    @Nullable
    private ArticleManagerAPI mArticleManager;

    @NonNull
    private Map<ScreenType, Fragment> mFragments = new EnumMap<>(ScreenType.class);
    @NonNull
    private Map< ScreenType, Class> mActivities = new EnumMap<>(ScreenType.class);

    private ScreenOpenerAPI mScreenOpener;


    public NavigationManager() {
    }

    @Override
    public void registerDictionaryManager(@NonNull DictionaryManagerAPI dictionaryManager) {
        mDictionaryManager = dictionaryManager;
    }

    @Override
    public void registerSearchManager(@NonNull SearchManagerAPI searchManager) {
        mSearchManager = searchManager;
    }

    @Override
    public void registerFavoritesManager(@NonNull FavoritesManagerAPI favoritesManager) {
        mFavoritesManager = favoritesManager;
    }

    @Override
    public void registerHistoryManager(@NonNull HistoryManagerAPI historyManager) {
        mHistoryManager = historyManager;
    }

    @Override
    public void registerNewsManager(@NonNull NewsManagerAPI newsManager) {
        mNewsManager = newsManager;
    }

    @Override
    public void registerToolbarManager(@NonNull ToolbarManager toolbarManager) {
        mToolbarManager = toolbarManager;
    }

    @Override
    public void registerEngineInformation(@NonNull EngineInformationAPI engineInformation) {
        mEngineInformation = engineInformation;
    }

    @Override
    public void registerWotDManager(@NonNull WotDManagerAPI wotDManager) {
        mWotDManager = wotDManager;
    }

    @Override
    public void registerSettingsManager( @Nullable SettingsManagerAPI settingsManager )
    {
        mSettingsManager = settingsManager;
    }

    @Override
    public void registerHintManager( @Nullable HintManagerAPI hintManager )
    {
        mHintManager = hintManager;
    }

    @Override
    public void registerArticleManager( @Nullable ArticleManagerAPI articleManager )
    {
        mArticleManager = articleManager;
    }

    @Override
    public void registerFragment( @NonNull ScreenType screenType, @Nullable Fragment fragment ) {
        if ( fragment != null )
        {
            mFragments.put(screenType, fragment);
        }
    }

    @Override
    public void registerActivity( @NonNull ScreenType screenType, @Nullable Class activityClass )
    {
        if (activityClass != null && Activity.class.isAssignableFrom(activityClass))
        {
            mActivities.put(screenType, activityClass);
        }
    }

    @Nullable
    @Override
    Fragment getFragment( @NonNull ScreenType screenType )
    {
        return mFragments.get(screenType);
    }

    @Nullable
    @Override
    Class getActivity( @NonNull ScreenType screenType )
    {
        return mActivities.get(screenType);
    }

    @Override
    NavigationControllerApi getController(@NonNull String tag) {
        NavigationControllerApi controller = mControllers.get(tag);
        if(controller == null)
            mControllers.put(tag, controller = NavigationController.create(mDictionaryManager,
                                                                    mSearchManager,
                                                                    mNewsManager,
                                                                    mToolbarManager,
                                                                    mHistoryManager,
                                                                    mFavoritesManager,
                                                                    mEngineInformation,
                                                                    mWotDManager,
                                                                    mSettingsManager,
                                                                    mHintManager,
                                                                    mScreenOpener));
        mActiveController.set(controller);
        return controller;
    }

    @Override
    SettingsManagerAPI getSettingsManager() {
        return mSettingsManager;
    }

    @Override
    public void registerScreenOpener(ScreenOpenerAPI screenOpener) {
        mScreenOpener = screenOpener;
    }

    @Override
    public ScreenOpenerAPI createScreenOpener(String articleControllerId, boolean useTabletOpener ) {
        if (useTabletOpener ) {
            return new StandardTabletScreenOpener(this, articleControllerId);
        } else {
            return new StandardScreenOpener(this, articleControllerId);
        }
    }

    @Nullable
    @Override
    ArticleManagerAPI getArticleManager()
    {
        return mArticleManager;
    }

    @NonNull
    @Override
    Observable<Boolean> getTopScreenOverlayStateObservable() {
        NavigationControllerApi controller = mActiveController.get();
        if (controller != null) {
            return controller.getTopScreenOverlayStateObservable();
        } else {
            throw new NullPointerException("Navigation controller must be initialized at this stage.");
        }
    }
}
