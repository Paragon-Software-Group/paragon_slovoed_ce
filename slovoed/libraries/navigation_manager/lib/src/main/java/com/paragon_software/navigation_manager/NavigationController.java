package com.paragon_software.navigation_manager;

import android.content.Context;
import android.graphics.Bitmap;
import android.os.Bundle;
import android.util.Pair;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentManager;

import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryAndDirection;
import com.paragon_software.dictionary_manager.DictionaryControllerAPI;
import com.paragon_software.dictionary_manager.DictionaryFilter;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.dictionary_manager.components.DictionaryComponent;
import com.paragon_software.dictionary_manager.filter.FilterTypeSimple;
import com.paragon_software.dictionary_manager.filter.IFilterFactory;
import com.paragon_software.favorites_manager.FavoritesManagerAPI;
import com.paragon_software.hint_manager.HintManagerAPI;
import com.paragon_software.hint_manager.HintParams;
import com.paragon_software.hint_manager.HintType;
import com.paragon_software.history_manager.HistoryManagerAPI;
import com.paragon_software.native_engine.EngineInformationAPI;
import com.paragon_software.native_engine.EngineVersion;
import com.paragon_software.news_manager.NewsManagerAPI;
import com.paragon_software.news_manager.OnNewsListChangesListener;
import com.paragon_software.screen_opener_api.ScreenOpenerAPI;
import com.paragon_software.screen_opener_api.ScreenType;
import com.paragon_software.search_manager.SearchManagerAPI;
import com.paragon_software.settings_manager.ApplicationSettings;
import com.paragon_software.settings_manager.OnApplicationSettingsSaveListener;
import com.paragon_software.settings_manager.SettingsManagerAPI;
import com.paragon_software.settings_manager.exceptions.LocalResourceUnavailableException;
import com.paragon_software.settings_manager.exceptions.ManagerInitException;
import com.paragon_software.toolbar_manager.ToolbarManager;
import com.paragon_software.word_of_day.WotDItem;
import com.paragon_software.word_of_day.WotDManagerAPI;

import java.util.Arrays;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;
import java.util.Set;
import java.util.WeakHashMap;

import io.reactivex.Observable;
import io.reactivex.subjects.BehaviorSubject;
import io.reactivex.subjects.PublishSubject;
import io.reactivex.subjects.Subject;

class NavigationController implements NavigationControllerApi,
                                      OnNewsListChangesListener,
                                      DictionaryManagerAPI.IDictionaryListObserver,
                                      OnApplicationSettingsSaveListener
{
    private static final String NAVIGATION_CONTROLLER_TAG = "NAVIGATION_CONTROLLER_TAG";

    @Nullable
    private final DictionaryManagerAPI mDictionaryManager;

    @Nullable
    private DictionaryControllerAPI mDictionaryController;

    @Nullable
    private final SearchManagerAPI mSearchManager;

    @Nullable
    private final NewsManagerAPI mNewsManager;

    @Nullable
    private final ToolbarManager mToolbarManager;

    @Nullable
    private final EngineInformationAPI mEngineInformation;

    @Nullable
    private final HistoryManagerAPI mHistoryManager;

    @Nullable
    private final FavoritesManagerAPI mFavoritesManager;

    @NonNull
    private final NavigationManagerApi mNavigationManager;

    @Nullable
    private final WotDManagerAPI mWotDManager;

    @Nullable
    private final SettingsManagerAPI mSettingsManager;

    @Nullable
    private final HintManagerAPI mHintManager;
    @Nullable
    private final ScreenOpenerAPI mScreenOpener;
    @Nullable
    private ScreenType mCurrentOpenedScreen = null;

    private BehaviorSubject< Float > entryListFontSizeSubject = BehaviorSubject.createDefault(
        ApplicationSettings.getDefaultFontSize());

    private BehaviorSubject< Boolean > mFullScreenViewStateSubject = BehaviorSubject.createDefault(false);

    private Subject< Pair<HintType, HintParams> > mNeedToShowHintSubject = PublishSubject.create();

    private Subject< Boolean > mTopScreenOverlayStateSubject = PublishSubject.create();
    private boolean mFreePreview = false;

    static NavigationController create(@Nullable DictionaryManagerAPI dictionaryManager,
                                       @Nullable SearchManagerAPI searchManager,
                                       @Nullable NewsManagerAPI newsManager,
                                       @Nullable ToolbarManager toolbarManager,
                                       @Nullable HistoryManagerAPI historyManager,
                                       @Nullable FavoritesManagerAPI favoritesManager,
                                       @Nullable EngineInformationAPI engineInformation,
                                       @Nullable WotDManagerAPI wotDManager,
                                       @Nullable SettingsManagerAPI settingsManager,
                                       @Nullable HintManagerAPI hintManager,
                                       @Nullable ScreenOpenerAPI screenOpener) {
        NavigationController res = new NavigationController(dictionaryManager, searchManager,
                                                            newsManager, toolbarManager,
                                                            historyManager, favoritesManager,
                                                            engineInformation, wotDManager, settingsManager,
                                                            hintManager, screenOpener);
        if(dictionaryManager != null)
            dictionaryManager.registerDictionaryListObserver(res);
        if(newsManager != null)
            newsManager.registerNotifier(res);
        if(settingsManager != null)
            settingsManager.registerNotifier(res);
        return res;
    }

    private NavigationController(@Nullable DictionaryManagerAPI dictionaryManager,
                                 @Nullable SearchManagerAPI searchManager,
                                 @Nullable NewsManagerAPI newsManager,
                                 @Nullable ToolbarManager toolbarManager,
                                 @Nullable HistoryManagerAPI historyManager,
                                 @Nullable FavoritesManagerAPI favoritesManager,
                                 @Nullable EngineInformationAPI engineInformation,
                                 @Nullable WotDManagerAPI wotDManager,
                                 @Nullable SettingsManagerAPI settingsManager,
                                 @Nullable HintManagerAPI hintManager,
                                 @Nullable ScreenOpenerAPI screenOpener) {
        mDictionaryManager = dictionaryManager;
        mSearchManager = searchManager;
        mNewsManager = newsManager;
        mToolbarManager = toolbarManager;
        mHistoryManager = historyManager;
        mFavoritesManager = favoritesManager;
        mEngineInformation = engineInformation;
        mNavigationManager = NavigationHolder.getNavigationManager();
        mWotDManager = wotDManager;
        mSettingsManager = settingsManager;
        mHintManager = hintManager;
        mScreenOpener = screenOpener;

        if ( mSettingsManager != null )
            entryListFontSizeSubject.onNext(mSettingsManager.loadApplicationSettings().getEntryListFontSize());

        if (mDictionaryManager != null)
            mDictionaryController = mDictionaryManager.createController(NAVIGATION_CONTROLLER_TAG);
    }

    @NonNull
    private final Set<Notifier> mNotifiers = Collections.newSetFromMap(new WeakHashMap<Notifier, Boolean>(2));

    @Override
    public void registerNotifier(@NonNull Notifier notifier) {
        mNotifiers.add(notifier);
    }

    @Override
    public void unregisterNotifier(@NonNull Notifier notifier) {
        mNotifiers.remove(notifier);
    }

    @Nullable
    @Override
    public Fragment getFragment( @NonNull ScreenType screenType )
    {
        return mNavigationManager.getFragment(screenType);
    }

    @Nullable
    @Override
    public Class getActivity( @NonNull ScreenType screenType )
    {
        return mNavigationManager.getActivity(screenType);
    }

    @NonNull
    @Override
    public List<Dictionary> getDictionaries() {
        List<Dictionary> res = Collections.emptyList();
        if(mDictionaryManager != null)
            res = mDictionaryManager.getDictionaries();
        return res;
    }

    @NonNull
    @Override
    public List<Dictionary> getMyDictionaries() {
        List<Dictionary> res = Collections.emptyList();
        if (mDictionaryManager != null && mDictionaryController != null)
        {
            IFilterFactory filterFactory = mDictionaryManager.getFilterFactory();
            DictionaryFilter dictionaryFilter = filterFactory.createByType(FilterTypeSimple.MY_DICTIONARIES);
            mDictionaryController.installFilter(dictionaryFilter);
            return Arrays.asList(mDictionaryController.getDictionaries());
        }
        return res;
    }

    @Override
    public boolean isHaveMyDictionaries() {
        return getMyDictionaries().size() != 0;
    }

    @Override
    public boolean isUserCoreLoggedIn() {
        return false;
    }

    @Override
    public void openUserCoreLoginDialog(FragmentManager fragmentManager) {
        if(mDictionaryManager!=null)
        mDictionaryManager.openSignIn(fragmentManager);
    }

    @Override
    public void userCoreLogout(Context context) {
        if(mDictionaryManager!=null)
            mDictionaryManager.signOut(context);
    }

    @Override
    public boolean isSelectedUserDictionaryDownloaded()
    {
        if (mDictionaryController != null && mDictionaryManager != null)
        {
            DictionaryAndDirection dictAndDir = mDictionaryManager.getDictionaryAndDirectionSelectedByUser();

            if (dictAndDir == null)
                return false;

            Dictionary.DictionaryId dictionaryId = dictAndDir.getDictionaryId();
            Dictionary selectedDictionary = mDictionaryController.getDictionaryById(dictionaryId);

            if (selectedDictionary != null) {
                return getIsDictionaryDownloaded(mDictionaryController, selectedDictionary);
            }
        }
        return false;
    }

    private boolean getIsDictionaryDownloaded( @NonNull DictionaryControllerAPI dictionaryController,
                                               @NonNull Dictionary dictionary )
    {
        for (DictionaryComponent component: dictionary.getDictionaryComponents())
        {
            if (DictionaryComponent.Type.WORD_BASE.equals(component.getType()) && component.isDemo())
                if (dictionaryController.isDownloaded(component)) {
                    return true;
                }
        }
        return false;
    }

    @Override
    public void openDictionaryForSearch(@NonNull Context context,
                                        @NonNull Dictionary.DictionaryId dictionaryId,
                                        @Nullable Dictionary.Direction direction,
                                        @Nullable String text)
    {
        if (mDictionaryManager != null)
            mDictionaryManager.openDictionaryForSearch(context, dictionaryId, direction, text);
    }

    @Override
    public boolean isFreePreview() {
        return mFreePreview;
    }

    @Override
    public void setIsFreePreview(boolean freePreview) {
        mFreePreview = freePreview;
    }

    @Override
    public void setDictionaryAndDirectionSelectedByUser(@NonNull Dictionary.DictionaryId dictionaryId,
                                                        @Nullable Dictionary.Direction direction)
    {
        if (mDictionaryManager != null)
        {
            if (direction == null)
                direction = findDirectionById(dictionaryId);
            if (direction == null)
                return;
            DictionaryAndDirection dictAndDirection = new DictionaryAndDirection(dictionaryId, direction);
            mDictionaryManager.setDictionaryAndDirectionSelectedByUser(dictAndDirection);
        }
    }

    @Override
    public String getDrawerTitle() {
        String res = null;
        Dictionary dictionary = findDictionary();
        if(dictionary != null)
            res = dictionary.getTitle().get();
        return res;
    }

    @Override
    public Bitmap getDrawerIcon() {
        Bitmap res = null;
        Dictionary dictionary = findDictionary();
        if(dictionary != null)
            res = dictionary.getIcon().getBitmap();
        return res;
    }

    @Override
    public Observable<Integer> getUnreadNewsCount() {
        if(mNewsManager != null)
            return mNewsManager.getUnreadNewsCount();
        return Observable.empty();
    }

    @Override
    public Observable< Integer > getNewWotDCount() {
        if( mWotDManager != null )
            return mWotDManager.getNewWotDCount();

        return Observable.empty();
    }

    @Override
    public void openWotDItemScreen( @NonNull WotDItem wotDItem ) {
        if( mWotDManager != null ) {
            mWotDManager.openWotDItemScreen(wotDItem);
        }
    }

    @Override
    public void openNewsItemScreen(int newsIdToShow) {
        if( mNewsManager != null ) {
            mNewsManager.openNewsItem(newsIdToShow);
        }
    }

    @Override
    public void showDictionaryListInToolbar() {
        if(mToolbarManager != null)
            mToolbarManager.showDictionaryList();
    }

    @Override
    public void showDictionaryListInToolbar(@Nullable Dictionary.DictionaryId selected, @Nullable Dictionary.Direction direction) {
        if(mToolbarManager != null)
            mToolbarManager.showDictionaryList(selected, direction);
    }

    @Override
    public void updateDictionaryListInToolbar() {
        if(mToolbarManager != null && mDictionaryManager != null) {
            DictionaryAndDirection dictionaryAndDirection = mDictionaryManager.getDictionaryAndDirectionSelectedByUser();
            if(dictionaryAndDirection == null)
                return;
            mToolbarManager.updateDictionaryList(dictionaryAndDirection.getDictionaryId(), dictionaryAndDirection.getDirection());
        }
    }

    @Override
    public void showTitleAndDirectionInToolbar(@NonNull String title) {
        if (mToolbarManager != null) {
            mToolbarManager.showTitleAndDirectionInToolbar(title);
        }
    }

    @Override
    public void showTitleInToolbar(@NonNull String title) {
        if(mToolbarManager != null)
            mToolbarManager.showTitle(title);
    }

    @Nullable
    @Override
    public EngineVersion getEngineVersion(){
        EngineVersion res = null;
        if(mEngineInformation != null)
            res = mEngineInformation.getEngineVersion();
        return res;
    }

    @Override
    public void setSearchText(@NonNull String searchUI, @NonNull String searchText) {
        if(mSearchManager != null)
            mSearchManager.setControllerSearchText(searchUI, searchText);
    }

    @Override
    public void historySelectionModeOff() {
        if(mHistoryManager!=null)
            mHistoryManager.selectionModeOff();
    }

    @Override
    public void favoritesSelectionModeOff() {
        if(mFavoritesManager!=null)
        {
            mFavoritesManager.selectionModeOff();
        }
    }

    @Override
    public boolean isTrialExpired()
    {
        if( mDictionaryManager != null )
        {
            Dictionary dictionary = findDictionary();
            if( dictionary != null )
            {
                return  mDictionaryManager.isTrialExpired(dictionary.getId());
            }
        }

        return false;
    }

    @NonNull
    @Override
    public Observable< Boolean > getFullScreenViewStateObservable()
    {
        return mFullScreenViewStateSubject;
    }

    @Override
    public void setFullScreenViewState( boolean state )
    {
        mFullScreenViewStateSubject.onNext(state);
    }

    @Override
    public void clickFullScreenViewAction()
    {
        Runnable hintOkRunnable = () -> mFullScreenViewStateSubject.onNext(!mFullScreenViewStateSubject.getValue());
        if ( mHintManager != null && mHintManager.isNeedToShowHint(HintType.ArticleFullScreenView) )
        {
            sendRequestToShowHint(HintType.ArticleFullScreenView, new HintParams.Builder().setOnFirstActionRunnable(hintOkRunnable).build());
        }
        else
        {
            hintOkRunnable.run();
        }
    }

    @NonNull
    @Override
    public Observable<Boolean> getTopScreenOverlayStateObservable() {
        return mTopScreenOverlayStateSubject;
    }

    @Override
    public void setTopScreenOverlayState(boolean state) {
        mTopScreenOverlayStateSubject.onNext(state);
    }

    protected void sendRequestToShowHint(@NonNull HintType hintType, @Nullable HintParams hintParams )
    {
        mNeedToShowHintSubject.onNext(new Pair<>(hintType, hintParams));
    }

    @Override
    public Observable< Pair< HintType, HintParams > > getNeedToShowHintObservable()
    {
        return mNeedToShowHintSubject;
    }

    @Override
    public boolean showHintManagerDialog( @NonNull HintType hintType, @Nullable FragmentManager fragmentManager, @Nullable HintParams hintParams )
    {
        if ( mHintManager != null )
        {
            return mHintManager.showHintDialog(hintType, fragmentManager, hintParams);
        }
        return false;
    }

    @NonNull
    @Override
    public Observable< Float > getEntryListFontSizeObservable()
    {
        return entryListFontSizeSubject;
    }

    @Override
    public Observable< Pair< ScreenType, Bundle > > getScreenOpenerObservable()
    {
        if (mScreenOpener != null) {
            return mScreenOpener.getScreenOpenerObservable();
        }
        return null;
    }

    @Override
    public void openScreen( @NonNull ScreenType screenType )
    {
        if (mScreenOpener != null) {
            mScreenOpener.openScreen(screenType);
        }
    }

    @Override
    public void openScreen( @NonNull ScreenType screenType, @Nullable Bundle bundle )
    {
        if (mScreenOpener != null) {
            mScreenOpener.openScreen(screenType, bundle);
        }
    }

    @Override
    public String getArticleControllerId()
    {
        if (mScreenOpener != null) {
            return mScreenOpener.getArticleControllerId();
        }
        return "";
    }

    @Override
    public void onDictionaryListChanged() {
        for(Notifier notifier : mNotifiers) {
            if(notifier instanceof OnDictionariesChangedListener)
                ((OnDictionariesChangedListener) notifier).onDictionariesChanged();
        }
    }

    @Override
    public void onNewsListChanged() {
        for(Notifier notifier : mNotifiers)
            if(notifier instanceof OnNewsListChangedListener)
                ((OnNewsListChangedListener) notifier).onNewsListChanged();
    }

    @Override
    public void setCurrentScreen(@NonNull ScreenType currentScreen) {
        mCurrentOpenedScreen = currentScreen;
    }

    @Nullable
    @Override
    public ScreenType getCurrentScreen() {
        return mCurrentOpenedScreen;
    }

    @Nullable
    private Dictionary findDictionary() {
        Dictionary res = null;
        if (mDictionaryManager != null && mDictionaryManager.getDictionaryAndDirectionSelectedByUser() != null) {
            res = mDictionaryManager.getDictionaryById(mDictionaryManager.getDictionaryAndDirectionSelectedByUser().getDictionaryId());
        }
        return res;
    }

    @Override
    public void onApplicationSettingsSaved( @NonNull ApplicationSettings applicationSettings )
    {
        if ( !entryListFontSizeSubject.getValue().equals(applicationSettings.getEntryListFontSize()) )
            entryListFontSizeSubject.onNext(applicationSettings.getEntryListFontSize());
    }

    @Override
    public @Nullable ApplicationSettings getApplicationSettings() {
        if (mSettingsManager == null) return null;

        return mSettingsManager.loadApplicationSettings();
    }

    @Override
    public void saveTabletColumnWidth(float widthPercent) {
        ApplicationSettings appSettings = getApplicationSettings();
        if (appSettings != null) {
            appSettings.setTabletColumnWidth(widthPercent);
            saveApplicationSettings(appSettings);
        }
    }

    @Override
    public void restorePurchases( @NonNull Context context )
    {
        if (mDictionaryManager != null)
            mDictionaryManager.openCatalogueAndRestorePurchase(context);
    }

    @NonNull
    @Override
    public Observable<Boolean> getChangeDictionaryDirectionObservable()
    {
        if (mDictionaryManager != null)
            return mDictionaryManager.getDictionaryAndDirectionChangeObservable();
        return Observable.empty();
    }

    private void saveApplicationSettings(@NonNull ApplicationSettings settingsToSave )
    {
        try
        {
            if ( mSettingsManager != null )
            {
                mSettingsManager.saveApplicationSettings(settingsToSave);
            }
        }
        catch ( ManagerInitException | LocalResourceUnavailableException ignore )
        {
        }
    }

    @Nullable
    private Dictionary.Direction findDirectionById( @NonNull Dictionary.DictionaryId dictionaryId)
    {
        Dictionary.Direction direction = null;
        if (mDictionaryManager == null)
            return null;
        Dictionary dictionary = mDictionaryManager.getDictionaryById(dictionaryId);
        if (dictionary == null)
            return null;
        Iterator<Dictionary.Direction> directionIterator = dictionary.getDirections().iterator();
        if (directionIterator.hasNext())
            direction = directionIterator.next();

        return direction;
    }
}
