package com.paragon_software.navigation_manager;

import android.graphics.Bitmap;

import androidx.annotation.NonNull;

import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.favorites_manager.FavoritesManagerAPI;
import com.paragon_software.hint_manager.HintManagerAPI;
import com.paragon_software.hint_manager.HintType;
import com.paragon_software.history_manager.HistoryManagerAPI;
import com.paragon_software.native_engine.EngineInformationAPI;
import com.paragon_software.navigation_manager.mock.MockForController_NewsListChangedListener;
import com.paragon_software.navigation_manager.mock.MockForController_OnDictionariesChangedListener;
import com.paragon_software.navigation_manager.mock.MockNewsManager;
import com.paragon_software.navigation_manager.mock.MockWotdManager;
import com.paragon_software.navigation_manager.mock.dictionary_manager.MockDictionaryIcon;
import com.paragon_software.navigation_manager.mock.dictionary_manager.MockDictionaryManager;
import com.paragon_software.screen_opener_api.ScreenOpenerAPI;
import com.paragon_software.screen_opener_api.ScreenType;
import com.paragon_software.search_manager.SearchController;
import com.paragon_software.search_manager.SearchManagerAPI;
import com.paragon_software.settings_manager.ApplicationSettings;
import com.paragon_software.settings_manager.SettingsManagerAPI;
import com.paragon_software.settings_manager.exceptions.LocalResourceUnavailableException;
import com.paragon_software.settings_manager.exceptions.ManagerInitException;
import com.paragon_software.toolbar_manager.ToolbarManager;
import com.paragon_software.utils_slovoed.text.LocalizedString;
import com.paragon_software.word_of_day.WotDItem;
import com.paragon_software.word_of_day.WotDLinkType;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.junit.MockitoJUnitRunner;

import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.atomic.AtomicInteger;

import io.reactivex.disposables.CompositeDisposable;
import io.reactivex.observers.TestObserver;

import static com.paragon_software.dictionary_manager.Dictionary.DICTIONARY_STATUS.DEMO;
import static com.paragon_software.navigation_manager.BaseNavigationManagerTest.DELTA;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertSame;
import static org.junit.Assert.assertTrue;
import static org.mockito.BDDMockito.given;
import static org.mockito.Mockito.CALLS_REAL_METHODS;
import static org.mockito.Mockito.doAnswer;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.withSettings;

@RunWith(MockitoJUnitRunner.class)
public class BaseNavigationControllerTest {

    private static @NonNull
    final Dictionary.DictionaryId DICT1_ID = new Dictionary.DictionaryId("111");
    private static @NonNull
    final Dictionary.DictionaryId DICT2_ID = new Dictionary.DictionaryId("222");
    private static @NonNull
    final Dictionary.DictionaryId DICT3_ID = new Dictionary.DictionaryId("333");
    private static @NonNull
    final String DICT1_TITLE = "Title 1";
    private static @NonNull
    final String DICT2_TITLE = "Title 2";
    private static @NonNull
    final String DICT3_TITLE = "Title 3";

    private final String testText = "test";

    private final WotDItem wotdItem = new WotDItem(100L, "test", "verb", "111", "test", WotDLinkType.OXFORD_5000, "test", "test", false);
    @Mock
    private Bitmap BITMAP1;
    @Mock
    private Bitmap BITMAP2;
    @Mock
    private Bitmap BITMAP3;

    private MockDictionaryManager mDictionaryManager;

    private MockNewsManager mNewsManager;

    private MockWotdManager mWotDManager;

    @Mock
    private SearchManagerAPI mSearchManager;

    @Mock
    private ToolbarManager mToolbarManager;

    @Mock
    private EngineInformationAPI mEngineInformation;

    @Mock
    private HistoryManagerAPI mHistoryManager;

    @Mock
    private FavoritesManagerAPI mFavoritesManager;

    @Mock
    private NavigationManagerApi mNavigationManager;

    @Mock
    private SettingsManagerAPI mSettingsManager;

    @Mock
    private HintManagerAPI mHintManager;

    @Mock
    private ScreenOpenerAPI mScreenOpener;

    @Mock
    private ApplicationSettings mApplicationSettings;

    private NavigationController mNavigationController;

    private CompositeDisposable mCompositeDisposable;

    @Before
    public void setUp() {
        given(mApplicationSettings.getEntryListFontSize()).willReturn(ApplicationSettings.getDefaultFontSize());
        doAnswer(invocation -> mApplicationSettings).when(mSettingsManager).loadApplicationSettings();

        NavigationHolder.setNavigationManager(mNavigationManager);

        mDictionaryManager = Mockito.spy(MockDictionaryManager.class);
        mWotDManager = Mockito.spy(MockWotdManager.class);
        mNewsManager = Mockito.spy(MockNewsManager.class);

        mCompositeDisposable = new CompositeDisposable();

        mNavigationController = NavigationController.create(mDictionaryManager, mSearchManager,
                mNewsManager, mToolbarManager,
                mHistoryManager, mFavoritesManager,
                mEngineInformation, mWotDManager, mSettingsManager,
                mHintManager, mScreenOpener);
    }

    @After
    public void resetSingleton() throws SecurityException, NoSuchFieldException, IllegalArgumentException, IllegalAccessException {
        Field instance = NavigationHolder.class.getDeclaredField("mNavigationManager");
        instance.setAccessible(true);
        instance.set(null, null);
    }

    @Test
    public void testDictionaryManager() {

        MockForController_OnDictionariesChangedListener listener
                = Mockito.mock(MockForController_OnDictionariesChangedListener.class
                , withSettings().useConstructor(2, mNavigationController, mDictionaryManager).defaultAnswer(CALLS_REAL_METHODS));

        MockDictionaryIcon dictionaryIcon1 = Mockito.mock(MockDictionaryIcon.class,
                withSettings().useConstructor(1, BITMAP1).defaultAnswer(CALLS_REAL_METHODS));
        MockDictionaryIcon dictionaryIcon2 = Mockito.mock(MockDictionaryIcon.class,
                withSettings().useConstructor(1, BITMAP2).defaultAnswer(CALLS_REAL_METHODS));
        assertEquals(0, mNavigationController.getDictionaries().size());
        MockDictionaryIcon dictionaryIcon3 = Mockito.mock(MockDictionaryIcon.class,
                withSettings().useConstructor(1, BITMAP3).defaultAnswer(CALLS_REAL_METHODS));
        assertEquals(0, mNavigationController.getDictionaries().size());

        mNavigationController.registerNotifier(listener);
        mDictionaryManager.registerDictionary(DICT1_ID, LocalizedString.from(DICT1_TITLE)
                , LocalizedString.from(""), DEMO, dictionaryIcon1);

        assertEquals(DICT1_TITLE, mNavigationController.getDrawerTitle());
        assertEquals(BITMAP1, mNavigationController.getDrawerIcon());
        assertEquals(1, mNavigationController.getDictionaries().size());

        mNavigationController.isTrialExpired();
        verify(mDictionaryManager, times(1)).isTrialExpired(DICT1_ID);

        mDictionaryManager.registerDictionary(DICT2_ID, LocalizedString.from(DICT2_TITLE)
                , LocalizedString.from(""), DEMO, dictionaryIcon2);

        assertEquals(DICT2_TITLE, mNavigationController.getDrawerTitle());
        assertEquals(BITMAP2, mNavigationController.getDrawerIcon());
        assertEquals(2, mNavigationController.getDictionaries().size());
        assertEquals(2, listener.getNumberOfCalls());

        mNavigationController.unregisterNotifier(listener);

        mDictionaryManager.registerDictionary(DICT3_ID, LocalizedString.from(DICT3_TITLE)
                , LocalizedString.from(""), DEMO, dictionaryIcon3);

        assertEquals(DICT3_TITLE, mNavigationController.getDrawerTitle());
        assertEquals(BITMAP3, mNavigationController.getDrawerIcon());
        assertEquals(2, listener.getNumberOfCalls());
        assertEquals(3, mNavigationController.getDictionaries().size());

        mNavigationController.isTrialExpired();
        verify(mDictionaryManager, times(1)).isTrialExpired(DICT3_ID);
    }

    @Test
    public void testSetting() throws ManagerInitException, LocalResourceUnavailableException {
        assertEquals(mNavigationController.getEntryListFontSizeObservable().blockingFirst(), mApplicationSettings.getEntryListFontSize(), DELTA);
        assertEquals(mNavigationController.getApplicationSettings(), mApplicationSettings);

        mNavigationController.saveTabletColumnWidth(0.5f);
        verify(mSettingsManager, times(1)).saveApplicationSettings(mApplicationSettings);
        verify(mApplicationSettings, times(1)).setTabletColumnWidth(0.5f);
    }

    @Test
    public void testNewsUnreadCount() {
        MockForController_NewsListChangedListener listener
                = Mockito.mock(MockForController_NewsListChangedListener.class
                , withSettings().useConstructor(2).defaultAnswer(CALLS_REAL_METHODS));

        mNavigationController.registerNotifier(listener);
        mNewsManager.setUnreadCount(5);
        assertSame(5, mNavigationController.getUnreadNewsCount().blockingFirst());
        mNewsManager.setUnreadCount(2);
        assertSame(2, mNavigationController.getUnreadNewsCount().blockingFirst());

        mNavigationController.unregisterNotifier(listener);
        mNewsManager.setUnreadCount(3);
        assertSame(3, mNavigationController.getUnreadNewsCount().blockingFirst());
    }

    @Test
    public void testGetScreen() {
        mNavigationController.getActivity(ScreenType.Article);
        verify(mNavigationManager, times(1)).getActivity(ScreenType.Article);

        mNavigationController.getFragment(ScreenType.History);
        verify(mNavigationManager, times(1)).getFragment(ScreenType.History);
    }

    @Test
    public void testWotd() {
        List<WotDItem> list = new ArrayList<>();
        AtomicInteger result = new AtomicInteger();
        AtomicInteger countAnswer = new AtomicInteger();
        mCompositeDisposable.addAll(
                mNavigationController.getNewWotDCount()
                        .subscribe(count ->
                        {
                            result.set(count);
                            countAnswer.getAndIncrement();

                        }));
        assertSame(0, result.get());
        assertSame(1, countAnswer.get());

        list.add(wotdItem);
        mWotDManager.setWotDList(list);
        assertSame(1, mNavigationController.getNewWotDCount().blockingFirst());
        list.add(wotdItem);
        mWotDManager.setWotDList(list);
        assertSame(2, mNavigationController.getNewWotDCount().blockingFirst());

        assertSame(2, result.get());
        assertSame(3, countAnswer.get());
    }

    @Test
    public void testScreenOpener() {
        mNavigationController.openScreen(ScreenType.Article);
        verify(mScreenOpener, times(1)).openScreen(ScreenType.Article);

        mNavigationController.getScreenOpenerObservable();
        verify(mScreenOpener, times(1)).getScreenOpenerObservable();

        mNavigationController.getArticleControllerId();
        verify(mScreenOpener, times(1)).getArticleControllerId();

        mNavigationController.openWotDItemScreen(wotdItem);
        verify(mWotDManager, times(1)).openWotDItemScreen(wotdItem);

        mNavigationController.openNewsItemScreen(1);
        verify(mNewsManager, times(1)).openNewsItem(1);
    }

    @Test
    public void testFullScreenMode() {
        assertFalse(mNavigationController.getFullScreenViewStateObservable().blockingFirst());
        mNavigationController.setFullScreenViewState(true);
        assertTrue(mNavigationController.getFullScreenViewStateObservable().blockingFirst());
        mNavigationController.setFullScreenViewState(false);
        assertFalse(mNavigationController.getFullScreenViewStateObservable().blockingFirst());
        mNavigationController.clickFullScreenViewAction();
        assertTrue(mNavigationController.getFullScreenViewStateObservable().blockingFirst());
        mNavigationController.clickFullScreenViewAction();
        assertFalse(mNavigationController.getFullScreenViewStateObservable().blockingFirst());
    }

    @Test
    public void testTopScreenOverlayObservable() {
        TestObserver<Boolean> testObserver = new TestObserver<>();
        mNavigationController.getTopScreenOverlayStateObservable().subscribe(testObserver);
        testObserver.assertNoValues();
        mNavigationController.setTopScreenOverlayState(false);
        testObserver.assertValue(false);
        mNavigationController.setTopScreenOverlayState(true);
        testObserver.assertValues(false, true);
    }

    @Test
    public void testHint() {
        mNavigationController.showHintManagerDialog(HintType.ArticleFullScreenView, null, null);
        verify(mHintManager, times(1)).showHintDialog(HintType.ArticleFullScreenView, null, null);

        mNavigationController.clickFullScreenViewAction();
        verify(mHintManager, times(1)).isNeedToShowHint(HintType.ArticleFullScreenView);

        AtomicInteger result = new AtomicInteger();
        mCompositeDisposable.addAll(
                mNavigationController.getNeedToShowHintObservable()
                        .subscribe(pair -> result.incrementAndGet()));
        mNavigationController.sendRequestToShowHint(HintType.ArticleFullScreenView, null);
        mNavigationController.sendRequestToShowHint(HintType.News, null);
        assertSame(2, result.get());
    }

    @Test
    public void testSelectionMode() {
        mNavigationController.favoritesSelectionModeOff();
        verify(mFavoritesManager, times(1)).selectionModeOff();
        mNavigationController.historySelectionModeOff();
        verify(mHistoryManager, times(1)).selectionModeOff();
    }

    @Test
    public void testSetSearchText() {
        mNavigationController.setSearchText(SearchController.CONTROLLER_TYPE_DEFAULT, testText);
        verify(mSearchManager, times(1)).setControllerSearchText(SearchController.CONTROLLER_TYPE_DEFAULT, testText);
    }

    @Test
    public void testEngineVersion() {
        mNavigationController.getEngineVersion();
        verify(mEngineInformation, times(1)).getEngineVersion();
    }

    @Test
    public void testToolbarManager() {
        Dictionary.Direction direction = new Dictionary.Direction(100, 200, null);
        mNavigationController.showTitleInToolbar(testText);
        verify(mToolbarManager, times(1)).showTitle(testText);
        mNavigationController.showDictionaryListInToolbar(DICT1_ID, direction);
        verify(mToolbarManager, times(1)).showDictionaryList(DICT1_ID, direction);
        mNavigationController.showDictionaryListInToolbar();
        verify(mToolbarManager, times(1)).showDictionaryList();
    }


}
