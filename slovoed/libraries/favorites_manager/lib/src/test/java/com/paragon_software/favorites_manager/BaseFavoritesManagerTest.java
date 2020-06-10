package com.paragon_software.favorites_manager;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.DictionaryAndDirection;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.dictionary_manager.MockDictionaryManager;
import com.paragon_software.dictionary_manager.MockForManager_DictionaryListChangedNotifier;
import com.paragon_software.favorites_manager.mock.MockForManager_ErrorListener;
import com.paragon_software.favorites_manager.mock.MockForManager_FavoritesListChangesListener;
import com.paragon_software.favorites_manager.mock.MockForManager_StateChangedListener;
import com.paragon_software.hint_manager.HintManagerAPI;
import com.paragon_software.native_engine.EngineSerializerAPI;
import com.paragon_software.native_engine.SearchEngineAPI;
import com.paragon_software.screen_opener_api.ScreenOpenerAPI;
import com.paragon_software.search_manager.MockSearchEngine;
import com.paragon_software.settings_manager.ApplicationSettings;
import com.paragon_software.settings_manager.MockSettingsManager;
import com.paragon_software.settings_manager.SettingsManagerAPI;
import com.paragon_software.settings_manager.exceptions.LocalResourceUnavailableException;
import com.paragon_software.settings_manager.exceptions.ManagerInitException;
import com.paragon_software.settings_manager.exceptions.WrongTypeException;
import com.paragon_software.sound_manager.SoundManagerAPI;
import com.paragon_software.toolbar_manager.ToolbarManager;
import com.paragon_software.utils_slovoed.directory.Directory;
import com.paragon_software.utils_slovoed.directory.MockDirectory;
import com.paragon_software.utils_slovoed.directory.MockDirectoryAbstract;

import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.junit.MockitoJUnit;
import org.mockito.junit.MockitoRule;
import org.robolectric.RobolectricTestRunner;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.List;

import androidx.fragment.app.DialogFragment;
import androidx.fragment.app.FragmentManager;

import io.reactivex.Observable;
import io.reactivex.android.plugins.RxAndroidPlugins;
import io.reactivex.disposables.Disposable;
import io.reactivex.plugins.RxJavaPlugins;
import io.reactivex.schedulers.Schedulers;

import static com.paragon_software.favorites_manager.BaseFavoritesManager.FAVORITES_SAVE_KEY;
import static com.paragon_software.favorites_manager.mock.StaticConsts.ARTICLE1;
import static com.paragon_software.favorites_manager.mock.StaticConsts.ARTICLE2;
import static com.paragon_software.favorites_manager.mock.StaticConsts.ARTICLE3;
import static com.paragon_software.favorites_manager.mock.StaticConsts.DELTA;
import static com.paragon_software.favorites_manager.mock.StaticConsts.DICT1_ID;
import static com.paragon_software.favorites_manager.mock.StaticConsts.DIRECT1;
import static junit.framework.TestCase.assertNotSame;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertSame;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.BDDMockito.given;
import static org.mockito.Mockito.CALLS_REAL_METHODS;
import static org.mockito.Mockito.any;
import static org.mockito.Mockito.atLeast;
import static org.mockito.Mockito.doAnswer;
import static org.mockito.Mockito.doThrow;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;
import static org.mockito.Mockito.withSettings;

@RunWith( RobolectricTestRunner.class )
public class BaseFavoritesManagerTest
{
  @Rule
  public MockitoRule rule = MockitoJUnit.rule();

  @Mock
  private ScreenOpenerAPI screenOpener;
  @Mock
  private EngineSerializerAPI  engineSerializer;
  @Mock
  private SearchEngineAPI      engineSearcher;
  @Mock
  private SettingsManagerAPI   settingsManager;
  @Mock
  private ToolbarManager       toolbarManager;
  @Mock
  private DictionaryManagerAPI dictionaryManager;
  @Mock
  private SoundManagerAPI      soundManager;
  @Mock
  private HintManagerAPI mHintManager;
  @Mock
  private ApplicationSettings  applicationSettings;
  @Mock
  private Activity mActivity;
  @Mock
  private DialogFragment mDialogFragment;

  private BaseFavoritesManager favoritesManager;
  @Mock
  private Context mContext;

  @Mock
  private FragmentManager mFragmentManager;

  static {
    AnalyticsHelper.initialize();
  }

  @BeforeClass
  public static void setUpClass() {
    RxAndroidPlugins.reset();
    RxAndroidPlugins.setInitMainThreadSchedulerHandler(scheduler -> Schedulers.trampoline());
  }

  @Before
  public void setUp() throws ManagerInitException, WrongTypeException
  {
    when(dictionaryManager.getDictionaryAndDirectionChangeObservable()).thenReturn(Observable.empty());
    doAnswer(invocation -> FavoritesSorting.BY_DATE_DESCENDING).when(settingsManager)
            .load(any(String.class), any(FavoritesSorting.class));
    doAnswer(invocation -> applicationSettings).when(settingsManager).loadApplicationSettings();
    doAnswer(invocation -> {
      MockDirectoryAbstract mockDirectory = mock(MockDirectoryAbstract.class, withSettings()
              .useConstructor(null, FAVORITES_SAVE_KEY).defaultAnswer(CALLS_REAL_METHODS));
      return new PersistentDirectory(mockDirectory);
    }).when(settingsManager).load(any(String.class), any(PersistentDirectory.class));
    doAnswer(invocation -> invocation.getArgument(1)).when(settingsManager)
            .load(any(String.class), any(Serializable[].class));

    BaseFavoritesManagerFactory factory = new BaseFavoritesManagerFactory();
    favoritesManager = (BaseFavoritesManager) factory.registerSearchEngine(engineSearcher)
        .registerHintManager(mHintManager)
        .registerUI(mActivity.getClass())
        .registerAddInDirectoryUI(mDialogFragment)
        .registerFileProviderAuthorities(BuildConfig.APPLICATION_ID + ".favorites_manager.fileprovider")
        .create(screenOpener, engineSerializer, settingsManager
            , dictionaryManager, toolbarManager, soundManager, new BaseFavoritesSorter()
            , null, 1000);
  }

  @AfterClass
  public static void cleanUpClass() {
    RxAndroidPlugins.reset();
  }

  @Test
  public void testGetController()
  {
    FavoritesControllerAPI controller = favoritesManager.getController("test");
    FavoritesControllerAPI controller2 = favoritesManager.getController(FavoritesControllerType.DEFAULT_CONTROLLER);
    assertNotNull(controller);
    assertNotNull(controller2);
    assertNotSame(controller, controller2);
    assertSame(controller, favoritesManager.getController("test") );

    verify(toolbarManager, times(2)).registerNotifier((BaseFavoritesController)controller);
    verify(settingsManager, times(2)).registerNotifier((BaseFavoritesController)controller);

    favoritesManager.freeController("test");
    verify(toolbarManager, times(1)).unRegisterNotifier((BaseFavoritesController)controller);
    verify(settingsManager, times(1)).unregisterNotifier((BaseFavoritesController)controller);
  }

  @Test
  public void testHasAddRemoveWord()
  {
    assertFalse(favoritesManager.hasWord(ARTICLE1));
    assertTrue(favoritesManager.addWord(ARTICLE1));
    assertFalse(favoritesManager.addWord(ARTICLE1));
    assertTrue(favoritesManager.hasWord(ARTICLE1));
    assertTrue(favoritesManager.removeWord(ARTICLE1));
    assertFalse(favoritesManager.hasWord(ARTICLE1));
    assertTrue(favoritesManager.addWord(ARTICLE1));
    assertTrue(favoritesManager.addWord(ARTICLE2));
    assertTrue(favoritesManager.removeWords(new ArrayList< ArticleItem >()
      {{ add(ARTICLE1); add(ARTICLE3); }}));
    assertFalse(favoritesManager.removeWords(new ArrayList< ArticleItem >()
      {{ add(ARTICLE1); add(ARTICLE3); }}));
    assertTrue(favoritesManager.hasWord(ARTICLE2));
    assertTrue(favoritesManager.removeAllWords());
    assertFalse(favoritesManager.removeAllWords());
  }

  @Test
  public void testSettingsManagerUsage()
      throws ManagerInitException, LocalResourceUnavailableException
  {
    given(applicationSettings.getEntryListFontSize()).willReturn(ApplicationSettings.getMaxFontSize());
    doAnswer(invocation -> applicationSettings).when(settingsManager).loadApplicationSettings();
    FavoritesControllerAPI controller = favoritesManager.getController
            (FavoritesControllerType.DEFAULT_CONTROLLER);
    verify(settingsManager, times(1)).loadApplicationSettings();
    assertEquals(controller.getEntryListFontSize(), ApplicationSettings.getMaxFontSize(), DELTA);
    assertTrue(favoritesManager.addWord(ARTICLE1));
    assertFalse(favoritesManager.addWord(ARTICLE1));
    verify(settingsManager, times(1)).save(any(String.class)
            , any(Serializable[].class), any(Boolean.class));
    verify(settingsManager, times(2)).loadApplicationSettings();
    assertTrue(favoritesManager.removeWord(ARTICLE1));
    assertFalse(favoritesManager.removeWord(ARTICLE1));
    verify(settingsManager, times(2)).save(any(String.class)
            , any(Serializable[].class), any(Boolean.class));
    verify(settingsManager, times(3)).loadApplicationSettings();
    assertTrue(favoritesManager.addWord(ARTICLE1));
    assertTrue(favoritesManager.addWord(ARTICLE2));
    verify(settingsManager, times(4)).save(any(String.class)
            , any(Serializable[].class), any(Boolean.class));
    verify(settingsManager, times(5)).loadApplicationSettings();
    assertTrue(favoritesManager.removeWords(new ArrayList< ArticleItem >()
      {{ add(ARTICLE1); add(ARTICLE3); }}));
    assertFalse(favoritesManager.removeWords(new ArrayList< ArticleItem >()
      {{ add(ARTICLE1); add(ARTICLE3); }}));
    verify(settingsManager, times(5)).save(any(String.class)
            , any(Serializable[].class), any(Boolean.class));
    verify(settingsManager, times(6)).loadApplicationSettings();
    assertTrue(favoritesManager.removeAllWords());
    assertFalse(favoritesManager.removeAllWords());
    verify(settingsManager, times(6)).save(any(String.class)
            , any(Serializable[].class), any(Boolean.class));
    verify(settingsManager, times(7)).loadApplicationSettings();
  }

  @Test
  public void testEngineArticleUsage()
  {
    favoritesManager.getController(FavoritesControllerType.DEFAULT_CONTROLLER);
    assertTrue(favoritesManager.addWord(ARTICLE1));
    assertFalse(favoritesManager.addWord(ARTICLE1));
    verify(engineSerializer, times(1)).serializeArticleItem(ARTICLE1);
    assertTrue(favoritesManager.removeWord(ARTICLE1));
    assertFalse(favoritesManager.removeWord(ARTICLE1));
    assertTrue(favoritesManager.addWord(ARTICLE1));
    verify(engineSerializer, times(2)).serializeArticleItem(ARTICLE1);
    assertTrue(favoritesManager.addWord(ARTICLE2));
    verify(engineSerializer, times(3)).serializeArticleItem(ARTICLE1);
    verify(engineSerializer, times(1)).serializeArticleItem(ARTICLE2);
    assertTrue(favoritesManager.removeWords(new ArrayList< ArticleItem >()
      {{ add(ARTICLE1); add(ARTICLE3); }}));
    assertFalse(favoritesManager.removeWords(new ArrayList< ArticleItem >()
      {{ add(ARTICLE1); add(ARTICLE3); }}));
    verify(engineSerializer, times(2)).serializeArticleItem(ARTICLE2);
    assertTrue(favoritesManager.removeAllWords());
    assertFalse(favoritesManager.removeAllWords());
    verify(engineSerializer, times(3)).serializeArticleItem(ARTICLE1);
    verify(engineSerializer, times(2)).serializeArticleItem(ARTICLE2);
  }

  @Test
  public void testToolbarManagerUsage()
  {
    favoritesManager.getController(FavoritesControllerType.DEFAULT_CONTROLLER);
    verify(toolbarManager, atLeast(1)).showDictionaryListNoDirection();
    assertTrue(favoritesManager.addWord(ARTICLE1));
    assertFalse(favoritesManager.addWord(ARTICLE1));
    verify(toolbarManager, atLeast(2)).showDictionaryListNoDirection();
    assertTrue(favoritesManager.removeWord(ARTICLE1));
    assertFalse(favoritesManager.removeWord(ARTICLE1));
    verify(toolbarManager, atLeast(3)).showDictionaryListNoDirection();
    assertTrue(favoritesManager.addWord(ARTICLE1));
    assertTrue(favoritesManager.addWord(ARTICLE2));
    verify(toolbarManager, atLeast(4)).showDictionaryListNoDirection();
    assertTrue(favoritesManager.removeWords(new ArrayList< ArticleItem >()
      {{ add(ARTICLE1); add(ARTICLE3); }}));
    assertFalse(favoritesManager.removeWords(new ArrayList< ArticleItem >()
      {{ add(ARTICLE1); add(ARTICLE3); }}));
    verify(toolbarManager, atLeast(5)).showDictionaryListNoDirection();
    assertTrue(favoritesManager.removeAllWords());
    assertFalse(favoritesManager.removeAllWords());
    verify(toolbarManager, atLeast(6)).showDictionaryListNoDirection();
  }

  @Test
  public void testOnErrorListener() throws ManagerInitException
          , LocalResourceUnavailableException, WrongTypeException
  {
    favoritesManager.addWord(ARTICLE1);
    assertEquals(favoritesManager.getWords().size(), 1);
    MockForManager_ErrorListener errorListener = mock(MockForManager_ErrorListener.class, withSettings()
            .useConstructor(7).defaultAnswer(CALLS_REAL_METHODS));
    favoritesManager.registerNotifier(errorListener);
    favoritesManager.getController(FavoritesControllerType.DEFAULT_CONTROLLER);

    // settings manager save exceptions
    ManagerInitException addWordException = new ManagerInitException("addWordException");
    errorListener.setExpectedException(addWordException);
    doThrow(addWordException).when(settingsManager).save(any(String.class)
            , any(Serializable[].class), any(Boolean.class));
    favoritesManager.addWord(ARTICLE2);
    assertEquals(favoritesManager.getWords().size(), 2);
    LocalResourceUnavailableException removeWordException
            = new LocalResourceUnavailableException("removeWordException");
    errorListener.setExpectedException(removeWordException);
    doThrow(removeWordException).when(settingsManager).save(any(String.class)
            , any(Serializable[].class), any(Boolean.class));
    favoritesManager.removeWord(ARTICLE1);
    assertEquals(favoritesManager.getWords().size(), 1);
    ManagerInitException removeWordsException = new ManagerInitException("removeWordsException");
    errorListener.setExpectedException(removeWordsException);
    doThrow(removeWordsException).when(settingsManager).save(any(String.class)
            , any(Serializable[].class), any(Boolean.class));
    favoritesManager.removeWords(new ArrayList< ArticleItem >() {{ add(ARTICLE1); add(ARTICLE3); }});
    assertEquals(favoritesManager.getWords().size(), 1);
    LocalResourceUnavailableException removeAllWordsException
            = new LocalResourceUnavailableException("removeAllWordsException");
    errorListener.setExpectedException(removeAllWordsException);
    doThrow(removeAllWordsException).when(settingsManager).save(any(String.class)
            , any(Serializable[].class), any(Boolean.class));
    favoritesManager.removeAllWords();
    assertEquals(favoritesManager.getWords().size(), 0);

    // settings manager load exceptions
    ManagerInitException firstLoadApplicationSettingsException
            = new ManagerInitException("firstLoadApplicationSettingsException");
    errorListener.setExpectedException(firstLoadApplicationSettingsException);
    doThrow(firstLoadApplicationSettingsException).when(settingsManager).load(any(String.class)
            , any(ApplicationSettings.class));
    favoritesManager.getController(FavoritesControllerType.DEFAULT_CONTROLLER);
    WrongTypeException secondLoadApplicationSettingsException
            = new WrongTypeException("secondLoadApplicationSettingsException");
    errorListener.setExpectedException(secondLoadApplicationSettingsException);
    doThrow(secondLoadApplicationSettingsException).when(settingsManager).load(any(String.class)
            , any(ApplicationSettings.class));
    favoritesManager.getController(FavoritesControllerType.DEFAULT_CONTROLLER);
  }

  @Test
  public void testFavoritesListChangesListener()
  {
    MockForManager_FavoritesListChangesListener favoritesListChangesListener
            = mock(MockForManager_FavoritesListChangesListener.class, withSettings()
          .useConstructor(6).defaultAnswer(CALLS_REAL_METHODS));
    favoritesManager.registerNotifier(favoritesListChangesListener);
    favoritesListChangesListener.setExpectedWords(new ArrayList< ArticleItem >() {{ add(ARTICLE1); }});
    assertTrue(favoritesManager.addWord(ARTICLE1));
    assertFalse(favoritesManager.addWord(ARTICLE1));
    assertEquals(favoritesListChangesListener.getNumberOfCalls(), 1);
    favoritesListChangesListener.setExpectedWords(new ArrayList<>());
    assertTrue(favoritesManager.removeWord(ARTICLE1));
    assertFalse(favoritesManager.removeWord(ARTICLE1));
    assertEquals(favoritesListChangesListener.getNumberOfCalls(), 2);
    favoritesListChangesListener.setExpectedWords(new ArrayList< ArticleItem >() {{ add(ARTICLE1); }});
    assertTrue(favoritesManager.addWord(ARTICLE1));
    assertEquals(favoritesListChangesListener.getNumberOfCalls(), 3);
    favoritesListChangesListener.setExpectedWords(new ArrayList< ArticleItem >()
      {{ add(ARTICLE2); add(ARTICLE1); }});
    assertTrue(favoritesManager.addWord(ARTICLE2));
    assertEquals(favoritesListChangesListener.getNumberOfCalls(), 4);
    favoritesListChangesListener.setExpectedWords(new ArrayList< ArticleItem >() {{ add(ARTICLE2); }});
    assertTrue(favoritesManager.removeWords(new ArrayList< ArticleItem >()
      {{ add(ARTICLE1); add(ARTICLE3); }}));
    assertFalse(favoritesManager.removeWords(new ArrayList< ArticleItem >()
      {{ add(ARTICLE1); add(ARTICLE3); }}));
    assertEquals(favoritesListChangesListener.getNumberOfCalls(), 5);
    favoritesListChangesListener.setExpectedWords(new ArrayList<>());
    assertTrue(favoritesManager.removeAllWords());
    assertFalse(favoritesManager.removeAllWords());
    assertEquals(favoritesListChangesListener.getNumberOfCalls(), 6);
  }

  @Test
  public void testGetWords()
  {
    assertEquals(favoritesManager.getWords().size(), 0);
    favoritesManager.addWord(ARTICLE1);
    assertEquals(favoritesManager.getWords().size(), 1);
    assertEquals(favoritesManager.getWords().toArray()[0], ARTICLE1);
  }

  @Test
  public void testOnStateChangedListener()
  {
    MockForManager_StateChangedListener listener = mock(MockForManager_StateChangedListener.class, withSettings()
          .useConstructor(2, favoritesManager).defaultAnswer(CALLS_REAL_METHODS));
    favoritesManager.registerNotifier(listener);
    favoritesManager.addWord(ARTICLE1);
    favoritesManager.removeWord(ARTICLE1);
    assertEquals(listener.getNumberOfCalls(), 2);
    favoritesManager.unregisterNotifier(listener);
    favoritesManager.addWord(ARTICLE1);
  }

  @Test
  public void testOnDictionaryListChangedListener() throws ManagerInitException, WrongTypeException
  {
    verify(settingsManager, times(1)).load(any(String.class), any(PersistentDirectory.class));
    MockForManager_DictionaryListChangedNotifier dictionaryListChangedNotifier
            = Mockito.spy(MockForManager_DictionaryListChangedNotifier.class);
    dictionaryListChangedNotifier.registerNotifier(favoritesManager);
    verify(settingsManager, times(2)).load(any(String.class), any(Serializable[].class));
    dictionaryListChangedNotifier.onDictionaryListChanged();
    verify(settingsManager, times(4)).load(any(String.class), any(Serializable[].class));
  }



  @Test
  public void testAddRemoveHasDirectory()
  {
    final List< Directory< ArticleItem >> rootDirectory
            = new ArrayList<Directory< ArticleItem >>(){{ add(null); }};
    Disposable disposable = favoritesManager.getRootDirectoryObservable().subscribe(
        directory -> rootDirectory.set(0, directory));
    assertFalse(favoritesManager.hasChildDirectory(rootDirectory.get(0), "TEST"));
    assertTrue(favoritesManager.addDirectory(rootDirectory.get(0), "TEST"));
    assertFalse(favoritesManager.addDirectory(rootDirectory.get(0), "TEST"));
    assertTrue(favoritesManager.hasChildDirectory(rootDirectory.get(0), "TEST"));
    Directory< ArticleItem > toDelete = rootDirectory.get(0).getChildList().get(0);
    assertTrue(favoritesManager.removeDirectory(toDelete));
    assertFalse(favoritesManager.removeDirectory(toDelete));
    assertFalse(favoritesManager.hasChildDirectory(rootDirectory.get(0), "TEST"));
    disposable.dispose();
  }

  @Test
  public void testAddRemoveHasArticleItemInDirectory()
  {
    final List< Directory< ArticleItem >> rootDirectory
            = new ArrayList<Directory< ArticleItem >>(){{ add(null); }};
    Disposable disposable = favoritesManager.getRootDirectoryObservable().subscribe(
        directory -> rootDirectory.set(0, directory));
    assertTrue(favoritesManager.addDirectory(rootDirectory.get(0), "TEST1"));
    assertTrue(favoritesManager.addDirectory(rootDirectory.get(0), "TEST2"));
    Directory< ArticleItem > test1ChildDir = rootDirectory.get(0).getChildList().get(0);
    Directory< ArticleItem > test2ChildDir = rootDirectory.get(0).getChildList().get(1);
    assertTrue(favoritesManager.addDirectory(test2ChildDir, "TEST22"));

    Directory< ArticleItem > test22ChildDir = rootDirectory.get(0)
            .getChildList().get(1)
            .getChildList().get(0);

    assertFalse(favoritesManager.hasWord(ARTICLE1, rootDirectory.get(0)));
    assertFalse(favoritesManager.hasWord(ARTICLE1, test1ChildDir));
    assertFalse(favoritesManager.hasWord(ARTICLE2, test2ChildDir));
    assertFalse(favoritesManager.hasWord(ARTICLE3, test22ChildDir));

    // add items
    assertTrue(favoritesManager.addWord(ARTICLE1, rootDirectory.get(0)));
    assertFalse(favoritesManager.addWord(ARTICLE1, rootDirectory.get(0)));
    assertTrue(favoritesManager.addWord(ARTICLE1, test1ChildDir));
    assertFalse(favoritesManager.addWord(ARTICLE1, test1ChildDir));
    assertTrue(favoritesManager.addWord(ARTICLE2, test2ChildDir));
    assertFalse(favoritesManager.addWord(ARTICLE2, test2ChildDir));
    assertTrue(favoritesManager.addWord(ARTICLE3, test22ChildDir));
    assertFalse(favoritesManager.addWord(ARTICLE3, test22ChildDir));

    assertTrue(favoritesManager.hasWord(ARTICLE1, rootDirectory.get(0)));
    assertTrue(favoritesManager.hasWord(ARTICLE1, test1ChildDir));
    assertTrue(favoritesManager.hasWord(ARTICLE2, test2ChildDir));
    assertTrue(favoritesManager.hasWord(ARTICLE3, test22ChildDir));

    // remove items
    assertTrue(favoritesManager.removeWord(ARTICLE1, rootDirectory.get(0)));
    assertFalse(favoritesManager.removeWord(ARTICLE1, rootDirectory.get(0)));
    assertTrue(favoritesManager.removeWords(new ArrayList< ArticleItem >() {{ add(ARTICLE1); }}
      , test1ChildDir));
    assertFalse(favoritesManager.removeWords(new ArrayList< ArticleItem >() {{ add(ARTICLE1); }}
      , test1ChildDir));
    assertTrue(favoritesManager.removeAllWords(test2ChildDir));
    assertFalse(favoritesManager.removeAllWords(test2ChildDir));
    assertTrue(favoritesManager.removeWord(ARTICLE3, test22ChildDir));
    assertFalse(favoritesManager.removeWord(ARTICLE3, test22ChildDir));

    assertFalse(favoritesManager.hasWord(ARTICLE1, rootDirectory.get(0)));
    assertFalse(favoritesManager.hasWord(ARTICLE1, test1ChildDir));
    assertFalse(favoritesManager.hasWord(ARTICLE2, test2ChildDir));
    assertFalse(favoritesManager.hasWord(ARTICLE3, test22ChildDir));

    disposable.dispose();
  }

  @Test
  public void testPredefinedFavorites() throws ManagerInitException, LocalResourceUnavailableException {

    RxJavaPlugins.reset();
    RxJavaPlugins.setComputationSchedulerHandler(scheduler -> Schedulers.trampoline());
    SettingsManagerAPI settingsManager = Mockito.spy(MockSettingsManager.class);
    DictionaryManagerAPI dictionaryManager = Mockito.spy(MockDictionaryManager.class);
    dictionaryManager.setDictionaryAndDirectionSelectedByUser(new DictionaryAndDirection(DICT1_ID, DIRECT1));

    MockDirectory predefinedFavorites = new MockDirectory(null, "test");
    predefinedFavorites.setArticle(new ArrayList<ArticleItem>() {{
      add(ARTICLE1);
      add(ARTICLE2);
    }});

    MockSearchEngine engineSearcher = mock(MockSearchEngine.class, withSettings()
        .useConstructor(predefinedFavorites).defaultAnswer(CALLS_REAL_METHODS));

    FavoritesManagerAPI favoritesManager = new BaseFavoritesManager(screenOpener, engineSerializer, engineSearcher
        , settingsManager, dictionaryManager, toolbarManager, soundManager, null
        , null, null, null, null
        , "provider", 1000);

    Disposable disposable = favoritesManager.getRootDirectoryObservable().subscribe(directory ->
    {
      assertEquals(FAVORITES_SAVE_KEY, directory.getName());
    });

    assertEquals(ARTICLE1, favoritesManager.getWords().get(0));
    assertEquals(ARTICLE2, favoritesManager.getWords().get(1));

    verify(settingsManager, times(2)).save(anyString(), any(Serializable.class), eq(true));

    RxJavaPlugins.reset();
    disposable.dispose();
  }

  @Test
  public void testShowScreen()
  {
    assertTrue(favoritesManager.showFavoritesScreen(mContext));
    verify(mContext, times(1)).startActivity(new Intent(mContext, mActivity.getClass()));

    assertTrue(favoritesManager.showAddArticleInDirectoryScreen(mFragmentManager, ARTICLE1));
    verify(mDialogFragment, times(1)).show(mFragmentManager, mDialogFragment.getClass().getSimpleName());
  }

  @Test
  public void testFavoritesManagerHolder()
  {
    FavoritesManagerHolder.setManager(favoritesManager);
    assertSame(favoritesManager, FavoritesManagerHolder.getManager());
  }
}
