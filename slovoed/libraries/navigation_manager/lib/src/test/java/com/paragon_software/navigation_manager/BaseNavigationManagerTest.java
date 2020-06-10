package com.paragon_software.navigation_manager;

import com.paragon_software.article_manager.ArticleControllerType;
import com.paragon_software.article_manager.ArticleManagerAPI;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.favorites_manager.FavoritesManagerAPI;
import com.paragon_software.hint_manager.HintManagerAPI;
import com.paragon_software.history_manager.HistoryManagerAPI;
import com.paragon_software.native_engine.EngineInformationAPI;
import com.paragon_software.navigation_manager.mock.MockActivity;
import com.paragon_software.navigation_manager.mock.MockFragment;
import com.paragon_software.news_manager.NewsManagerAPI;
import com.paragon_software.screen_opener_api.ScreenOpenerAPI;
import com.paragon_software.screen_opener_api.ScreenType;
import com.paragon_software.search_manager.SearchManagerAPI;
import com.paragon_software.settings_manager.ApplicationSettings;
import com.paragon_software.settings_manager.SettingsManagerAPI;
import com.paragon_software.toolbar_manager.ToolbarManager;
import com.paragon_software.word_of_day.WotDManagerAPI;

import org.junit.After;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.junit.MockitoJUnit;
import org.mockito.junit.MockitoJUnitRunner;
import org.mockito.junit.MockitoRule;

import java.lang.reflect.Field;
import java.util.Map;
import java.util.TreeMap;

import io.reactivex.Observable;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNotSame;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertSame;
import static org.mockito.BDDMockito.given;
import static org.mockito.Mockito.doAnswer;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;

@RunWith(MockitoJUnitRunner.class)
public class BaseNavigationManagerTest {

    public static final float DELTA = 0.01f;

    @Rule
    public MockitoRule rule = MockitoJUnit.rule();

    @Mock
    private final Map<String, NavigationControllerApi> mControllers = new TreeMap<>();

    @Mock
    private DictionaryManagerAPI mDictionaryManager;

    @Mock
    private SearchManagerAPI mSearchManager;

    @Mock
    private NewsManagerAPI mNewsManager;

    @Mock
    private ToolbarManager mToolbarManager;

    @Mock
    private EngineInformationAPI mEngineInformation;

    @Mock
    private FavoritesManagerAPI mFavoritesManager;

    @Mock
    private HistoryManagerAPI mHistoryManager;

    @Mock
    private WotDManagerAPI mWotDManager;

    @Mock
    private SettingsManagerAPI mSettingsManager;

    @Mock
    private HintManagerAPI mHintManager;

    @Mock
    private ArticleManagerAPI mArticleManager;

    @Mock
    private ScreenOpenerAPI mScreenOpener;

    @Mock
    private ApplicationSettings mApplicationSettings;

    private NavigationManager mNavigationManager;

    @After
    public void resetSingleton() throws SecurityException, NoSuchFieldException, IllegalArgumentException, IllegalAccessException {
        Field instance = NavigationHolder.class.getDeclaredField("mNavigationManager");
        instance.setAccessible(true);
        instance.set(null, null);
    }

    @Before
    public void setUp() {
        mNavigationManager = new NavigationManager();
        mNavigationManager.registerDictionaryManager(mDictionaryManager);
        mNavigationManager.registerSearchManager(mSearchManager);
        mNavigationManager.registerNewsManager(mNewsManager);
        mNavigationManager.registerFavoritesManager(mFavoritesManager);
        mNavigationManager.registerHistoryManager(mHistoryManager);
        mNavigationManager.registerToolbarManager(mToolbarManager);
        mNavigationManager.registerEngineInformation(mEngineInformation);
        mNavigationManager.registerWotDManager(mWotDManager);
        mNavigationManager.registerSettingsManager(mSettingsManager);
        mNavigationManager.registerHintManager(mHintManager);
        mNavigationManager.registerArticleManager(mArticleManager);
        mNavigationManager.registerScreenOpener(mScreenOpener);

        NavigationHolder.setNavigationManager(mNavigationManager);
    }

    @Test
    public void testSettingsManagerUsage() {
        doAnswer(invocation -> mApplicationSettings).when(mSettingsManager).loadApplicationSettings();
        given(mApplicationSettings.getEntryListFontSize()).willReturn(ApplicationSettings.getMaxFontSize());
        NavigationControllerApi controller = mNavigationManager.getController
                (NavigationControllerType.DEFAULT_CONTROLLER);
        verify(mSettingsManager, times(1)).loadApplicationSettings();
        assertEquals(controller.getEntryListFontSizeObservable().blockingFirst(), ApplicationSettings.getMaxFontSize(), DELTA);
    }

    @Test
    public void testCreateScreenOpener() {
        assertNotNull(mNavigationManager.createScreenOpener(ArticleControllerType.DEFAULT_CONTROLLER, true));
        assertNotNull(mNavigationManager.createScreenOpener(ArticleControllerType.DEFAULT_CONTROLLER, false));
        assertNotEquals(mNavigationManager.createScreenOpener(ArticleControllerType.DEFAULT_CONTROLLER, true).getClass(), mNavigationManager.createScreenOpener(ArticleControllerType.DEFAULT_CONTROLLER, false).getClass());
        assertNotEquals(mNavigationManager.createScreenOpener(ArticleControllerType.DEFAULT_CONTROLLER, true), mNavigationManager.createScreenOpener("test", true));
        assertEquals(mNavigationManager.createScreenOpener(ArticleControllerType.DEFAULT_CONTROLLER, true).getClass(), mNavigationManager.createScreenOpener(ArticleControllerType.DEFAULT_CONTROLLER, true).getClass());
        assertEquals(mNavigationManager.createScreenOpener(ArticleControllerType.DEFAULT_CONTROLLER, false).getClass(), mNavigationManager.createScreenOpener(ArticleControllerType.DEFAULT_CONTROLLER, false).getClass());
    }

    @Test
    public void testRegisterScreen() {
        assertNull(mNavigationManager.getActivity(ScreenType.Article));
        mNavigationManager.registerActivity(ScreenType.Article, MockActivity.class);
        mNavigationManager.registerActivity(ScreenType.History, MockFragment.class);
        assertNull(mNavigationManager.getActivity(ScreenType.History));
        assertNotNull(mNavigationManager.getActivity(ScreenType.Article));
        assertSame(mNavigationManager.getActivity(ScreenType.Article), MockActivity.class);
        assertNull(mNavigationManager.getFragment(ScreenType.Article));

        mNavigationManager.registerFragment(ScreenType.Article, new MockFragment());
        assertNotNull(mNavigationManager.getFragment(ScreenType.Article));
        assertSame(mNavigationManager.getFragment(ScreenType.Article).getClass(), MockFragment.class);
        assertNotNull(mNavigationManager.getActivity(ScreenType.Article));
    }

    @Test
    public void testGetters() {
        doAnswer(invocation -> mApplicationSettings).when(mSettingsManager).loadApplicationSettings();

        assertNotNull(mNavigationManager.getArticleManager());
        assertSame(mArticleManager, mNavigationManager.getArticleManager());

        assertNotNull(mNavigationManager.getSettingsManager());
        assertSame(mSettingsManager, mNavigationManager.getSettingsManager());

        NavigationControllerApi controllerDefault = mNavigationManager.getController(NavigationControllerType.DEFAULT_CONTROLLER);
        NavigationControllerApi controllerTest = mNavigationManager.getController("test");
        assertNotNull(controllerDefault);
        assertNotNull(controllerTest);
        assertSame(controllerDefault, mNavigationManager.getController(NavigationControllerType.DEFAULT_CONTROLLER));
        assertNotSame(controllerDefault, controllerTest);
    }

    @Test
    public void testTopScreenOverlayObservable() {
        doAnswer(invocation -> mApplicationSettings).when(mSettingsManager).loadApplicationSettings();

        NavigationControllerApi controller = mNavigationManager.getController
                (NavigationControllerType.DEFAULT_CONTROLLER);
        Observable<Boolean> observableFromManager = mNavigationManager.getTopScreenOverlayStateObservable();
        Observable<Boolean> observableFromController = controller.getTopScreenOverlayStateObservable();
        assertEquals(observableFromManager, observableFromController);
    }
}
