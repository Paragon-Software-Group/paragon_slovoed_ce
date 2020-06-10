package com.paragon_software.history_manager;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.hint_manager.HintManagerAPI;
import com.paragon_software.history_manager.mock.MockForManager_ErrorListener;
import com.paragon_software.history_manager.mock.MockForManager_HistoryListChangesListener;
import com.paragon_software.history_manager.mock.MockForManager_StateChangedListener;
import com.paragon_software.native_engine.EngineSerializerAPI;
import com.paragon_software.native_engine.MockEngineSerializer;
import com.paragon_software.native_engine.PersistentArticle;
import com.paragon_software.screen_opener_api.ScreenOpenerAPI;
import com.paragon_software.settings_manager.ApplicationSettings;
import com.paragon_software.settings_manager.OnApplicationSettingsSaveListener;
import com.paragon_software.settings_manager.SettingsManagerAPI;
import com.paragon_software.settings_manager.exceptions.LocalResourceUnavailableException;
import com.paragon_software.settings_manager.exceptions.ManagerInitException;
import com.paragon_software.settings_manager.exceptions.WrongTypeException;
import com.paragon_software.sound_manager.SoundManagerAPI;
import com.paragon_software.toolbar_manager.ToolbarManager;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.invocation.InvocationOnMock;
import org.mockito.junit.MockitoJUnit;
import org.mockito.junit.MockitoRule;
import org.mockito.stubbing.Answer;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.shadows.ShadowLooper;

import java.io.Serializable;
import java.util.ArrayList;

import edu.emory.mathcs.backport.java.util.Arrays;
import io.reactivex.Observable;
import io.reactivex.android.plugins.RxAndroidPlugins;
import io.reactivex.schedulers.Schedulers;

import static com.paragon_software.history_manager.mock.StaticConsts.ARTICLE1;
import static com.paragon_software.history_manager.mock.StaticConsts.ARTICLE2;
import static com.paragon_software.history_manager.mock.StaticConsts.ARTICLE3;
import static com.paragon_software.history_manager.mock.StaticConsts.ARTICLE_ITEMS;
import static com.paragon_software.history_manager.mock.StaticConsts.DELTA;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.BDDMockito.given;
import static org.mockito.Mockito.CALLS_REAL_METHODS;
import static org.mockito.Mockito.doAnswer;
import static org.mockito.Mockito.doThrow;
import static org.mockito.Mockito.eq;
import static org.mockito.Mockito.spy;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;
import static org.mockito.Mockito.withSettings;

@RunWith( RobolectricTestRunner.class )
public class BaseHistoryManagerTest
{
  @Rule
  public MockitoRule rule = MockitoJUnit.rule();

  @Mock
  private ScreenOpenerAPI screenOpener;
  @Mock
  private EngineSerializerAPI  engineSerializer;
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
  private IExportHTMLBuilder exportHTMLBuilder;

  private BaseHistoryManager historyManager;
  @Mock
  private Context mContext;

  @BeforeClass
  public static void setUpClass() {
    RxAndroidPlugins.reset();
    RxAndroidPlugins.setInitMainThreadSchedulerHandler(scheduler -> Schedulers.trampoline());
  }

  @Before
  public void setUp()
  {
    doAnswer(new Answer()
    {
      @Override
      public Object answer( InvocationOnMock invocation )
      {
        return applicationSettings;
      }
    }).when(settingsManager).loadApplicationSettings();
    when(dictionaryManager.getDictionaryAndDirectionChangeObservable()).thenReturn(Observable.empty());

    historyManager = (BaseHistoryManager) new BaseHistoryManagerFactory().registerScreenOpener(screenOpener)
        .registerEngineSerializer(engineSerializer)
        .registerSettingsManager(settingsManager)
        .registerDictionaryManager(dictionaryManager)
        .registerSoundManager(soundManager)
        .registerToolbarManager(toolbarManager)
        .registerHintManager(mHintManager)
        .registerUI(mActivity.getClass())
        .registerExportHTMLBuilderClass((Class<IExportHTMLBuilder>) exportHTMLBuilder.getClass())
        .setMaxNumberOfWords(1000)
        .registerFileProviderAuthorities(BuildConfig.APPLICATION_ID + ".history_manager.fileprovider")
        .create();
  }

  @AfterClass
  public static void cleanUpClass() {
    RxAndroidPlugins.reset();
  }

  @Test
  public void testGetController()
  {
    assertNotNull(historyManager.getController("test"));
    assertNotNull(historyManager.getController(HistoryControllerType.DEFAULT_CONTROLLER));
  }

  @Test
  public void testHasAddRemoveWord()
  {
    assertFalse(historyManager.getWords().contains(ARTICLE1));
    assertTrue(historyManager.addWord(ARTICLE1));
    assertTrue(historyManager.addWord(ARTICLE1));
    assertTrue(historyManager.getWords().contains(ARTICLE1));
    assertTrue(historyManager.addWord(ARTICLE2));
    assertTrue(historyManager.removeWords(new ArrayList< ArticleItem >() {{ add(ARTICLE1); add(ARTICLE3); }}));
    assertFalse(historyManager.removeWords(new ArrayList< ArticleItem >() {{ add(ARTICLE1); add(ARTICLE3); }}));
    assertFalse(historyManager.getWords().contains(ARTICLE1));
    assertTrue(historyManager.getWords().contains(ARTICLE2));
    assertTrue(historyManager.addWords(new ArrayList< ArticleItem >() {{ add(ARTICLE1); add(ARTICLE3); }}));
    assertFalse(historyManager.addWords(new ArrayList< ArticleItem >() {{ add(ARTICLE1); add(ARTICLE3); }}));
    assertTrue(historyManager.removeAllWords());
    assertFalse(historyManager.removeAllWords());
  }

  @Test
  public void testSettingsManagerUsage()
      throws ManagerInitException, LocalResourceUnavailableException
  {
    given(applicationSettings.getEntryListFontSize()).willReturn(ApplicationSettings.getMaxFontSize());
    doAnswer(new Answer()
    {
      @Override
      public Object answer( InvocationOnMock invocation )
      {
        return applicationSettings;
      }
    }).when(settingsManager).loadApplicationSettings();
    HistoryControllerAPI controller = historyManager.getController(HistoryControllerType.DEFAULT_CONTROLLER);
    verify(settingsManager, times(1)).loadApplicationSettings();
    assertEquals(controller.getEntryListFontSize(), ApplicationSettings.getMaxFontSize(), DELTA);
    assertTrue(historyManager.addWord(ARTICLE1));
    assertTrue(historyManager.addWord(ARTICLE1));
    verify(settingsManager, times(2)).save(any(String.class), any(Serializable[].class), any(Boolean.class));
    verify(settingsManager, times(3)).loadApplicationSettings();
    assertTrue(historyManager.addWord(ARTICLE2));
    verify(settingsManager, times(3)).save(any(String.class), any(Serializable[].class), any(Boolean.class));
    verify(settingsManager, times(4)).loadApplicationSettings();
    assertTrue(historyManager.removeWords(new ArrayList< ArticleItem >() {{ add(ARTICLE1); add(ARTICLE3); }}));
    assertFalse(historyManager.removeWords(new ArrayList< ArticleItem >() {{ add(ARTICLE1); add(ARTICLE3); }}));
    verify(settingsManager, times(4)).save(any(String.class), any(Serializable[].class), any(Boolean.class));
    verify(settingsManager, times(5)).loadApplicationSettings();
    assertTrue(historyManager.addWords(new ArrayList< ArticleItem >() {{ add(ARTICLE1); add(ARTICLE3); }}));
    assertFalse(historyManager.addWords(new ArrayList< ArticleItem >() {{ add(ARTICLE1); add(ARTICLE3); }}));
    verify(settingsManager, times(5)).save(any(String.class), any(Serializable[].class), any(Boolean.class));
    verify(settingsManager, times(6)).loadApplicationSettings();
    assertTrue(historyManager.removeAllWords());
    assertFalse(historyManager.removeAllWords());
    verify(settingsManager, times(6)).save(any(String.class), any(Serializable[].class), any(Boolean.class));
    verify(settingsManager, times(7)).loadApplicationSettings();
  }

  @Test
  public void testEngineArticleUsage()
  {
    historyManager.getController(HistoryControllerType.DEFAULT_CONTROLLER);
    assertTrue(historyManager.addWord(ARTICLE1));
    assertTrue(historyManager.addWord(ARTICLE1));
    verify(engineSerializer, times(2)).serializeArticleItem(ARTICLE1);
    assertTrue(historyManager.addWord(ARTICLE2));
    verify(engineSerializer, times(3)).serializeArticleItem(ARTICLE1);
    verify(engineSerializer, times(1)).serializeArticleItem(ARTICLE2);
    assertTrue(historyManager.removeWords(new ArrayList< ArticleItem >() {{ add(ARTICLE1); add(ARTICLE3); }}));
    assertFalse(historyManager.removeWords(new ArrayList< ArticleItem >() {{ add(ARTICLE1); add(ARTICLE3); }}));
    verify(engineSerializer, times(2)).serializeArticleItem(ARTICLE2);
    assertTrue(historyManager.addWords(new ArrayList< ArticleItem >() {{ add(ARTICLE1); add(ARTICLE3); }}));
    assertFalse(historyManager.addWords(new ArrayList< ArticleItem >() {{ add(ARTICLE1); add(ARTICLE3); }}));
    verify(engineSerializer, times(4)).serializeArticleItem(ARTICLE1);
    verify(engineSerializer, times(3)).serializeArticleItem(ARTICLE2);
    assertTrue(historyManager.removeAllWords());
    assertFalse(historyManager.removeAllWords());
    verify(engineSerializer, times(4)).serializeArticleItem(ARTICLE1);
    verify(engineSerializer, times(3)).serializeArticleItem(ARTICLE2);
  }

  @Test
  public void testToolbarManagerUsage()
  {
    historyManager.getController(HistoryControllerType.DEFAULT_CONTROLLER);
    verify(toolbarManager, times(1)).showDictionaryListNoDirection();
    assertTrue(historyManager.addWord(ARTICLE1));
    assertTrue(historyManager.addWord(ARTICLE1));
    verify(toolbarManager, times(7)).showDictionaryListNoDirection();
    assertTrue(historyManager.addWord(ARTICLE2));
    verify(toolbarManager, times(10)).showDictionaryListNoDirection();
    assertTrue(historyManager.removeWords(new ArrayList< ArticleItem >() {{ add(ARTICLE1); add(ARTICLE3); }}));
    assertFalse(historyManager.removeWords(new ArrayList< ArticleItem >() {{ add(ARTICLE1); add(ARTICLE3); }}));
    verify(toolbarManager, times(13)).showDictionaryListNoDirection();
    assertTrue(historyManager.addWords(new ArrayList< ArticleItem >() {{ add(ARTICLE1); add(ARTICLE3); }}));
    assertFalse(historyManager.addWords(new ArrayList< ArticleItem >() {{ add(ARTICLE1); add(ARTICLE3); }}));
    verify(toolbarManager, times(16)).showDictionaryListNoDirection();
    assertTrue(historyManager.removeAllWords());
    assertFalse(historyManager.removeAllWords());
    verify(toolbarManager, times(19)).showDictionaryListNoDirection();
  }

  @Test
  public void testOnErrorListener() throws ManagerInitException, LocalResourceUnavailableException, WrongTypeException
  {
    historyManager.addWord(ARTICLE1);
    assertEquals(historyManager.getWords().size(), 1);
    MockForManager_ErrorListener errorListener
            = Mockito.mock(MockForManager_ErrorListener.class
            , withSettings().useConstructor(6).defaultAnswer(CALLS_REAL_METHODS));
    historyManager.registerNotifier(errorListener);
    historyManager.getController(HistoryControllerType.DEFAULT_CONTROLLER);

    // settings manager save exceptions
    ManagerInitException addWordException = new ManagerInitException("addWordException");
    errorListener.setExpectedException(addWordException);
    doThrow(addWordException).when(settingsManager).save(any(String.class), any(Serializable[].class), any(Boolean.class));
    historyManager.addWord(ARTICLE2);
    assertEquals(historyManager.getWords().size(), 1);
    ManagerInitException removeWordsException = new ManagerInitException("removeWordsException");
    errorListener.setExpectedException(removeWordsException);
    doThrow(removeWordsException).when(settingsManager).save(any(String.class), any(Serializable[].class), any(Boolean.class));
    historyManager.removeWords(new ArrayList< ArticleItem >() {{ add(ARTICLE1); add(ARTICLE3); }});
    assertEquals(historyManager.getWords().size(), 1);
    ManagerInitException addWordsException = new ManagerInitException("addWordsException");
    errorListener.setExpectedException(addWordsException);
    doThrow(addWordsException).when(settingsManager).save(any(String.class), any(Serializable[].class), any(Boolean.class));
    historyManager.addWords(new ArrayList< ArticleItem >() {{ add(ARTICLE1); add(ARTICLE3); }});
    assertEquals(historyManager.getWords().size(), 1);
    LocalResourceUnavailableException removeAllWordsException = new LocalResourceUnavailableException("removeAllWordsException");
    errorListener.setExpectedException(removeAllWordsException);
    doThrow(removeAllWordsException).when(settingsManager).save(any(String.class), any(Serializable[].class), any(Boolean.class));
    historyManager.removeAllWords();
    assertEquals(historyManager.getWords().size(), 1);

    // settings manager load exceptions
    ManagerInitException firstLoadApplicationSettingsException = new ManagerInitException("firstLoadApplicationSettingsException");
    errorListener.setExpectedException(firstLoadApplicationSettingsException);
    doThrow(firstLoadApplicationSettingsException).when(settingsManager).load(any(String.class), any(ApplicationSettings.class));
    historyManager.getController(HistoryControllerType.DEFAULT_CONTROLLER);
    WrongTypeException secondLoadApplicationSettingsException = new WrongTypeException("secondLoadApplicationSettingsException");
    errorListener.setExpectedException(secondLoadApplicationSettingsException);
    doThrow(secondLoadApplicationSettingsException).when(settingsManager).load(any(String.class), any(ApplicationSettings.class));
    historyManager.getController(HistoryControllerType.DEFAULT_CONTROLLER);
  }

  @Test
  public void testHistoryListChangesListener()
  {
    MockForManager_HistoryListChangesListener historyListChangesListener
            = Mockito.mock(MockForManager_HistoryListChangesListener.class
            , withSettings().useConstructor(6).defaultAnswer(CALLS_REAL_METHODS));
    historyManager.registerNotifier(historyListChangesListener);
    historyListChangesListener.setExpectedWords(new ArrayList< ArticleItem >() {{ add(ARTICLE1); }});
    assertTrue(historyManager.addWord(ARTICLE1));
    assertTrue(historyManager.addWord(ARTICLE1));
    assertEquals(historyListChangesListener.getNumberOfCalls(), 2);
    historyListChangesListener.setExpectedWords(new ArrayList< ArticleItem >() {{ add(ARTICLE2); add(ARTICLE1); }});
    assertTrue(historyManager.addWord(ARTICLE2));
    assertEquals(historyListChangesListener.getNumberOfCalls(), 3);
    historyListChangesListener.setExpectedWords(new ArrayList< ArticleItem >() {{ add(ARTICLE2); }});
    assertTrue(historyManager.removeWords(new ArrayList< ArticleItem >() {{ add(ARTICLE1); add(ARTICLE3); }}));
    assertFalse(historyManager.removeWords(new ArrayList< ArticleItem >() {{ add(ARTICLE1); add(ARTICLE3); }}));
    assertEquals(historyListChangesListener.getNumberOfCalls(), 4);
    historyListChangesListener.setExpectedWords(new ArrayList< ArticleItem >() {{ add(ARTICLE1); add(ARTICLE3); add(ARTICLE2); }});
    assertTrue(historyManager.addWords(new ArrayList< ArticleItem >() {{ add(ARTICLE1); add(ARTICLE3); }}));
    assertFalse(historyManager.addWords(new ArrayList< ArticleItem >() {{ add(ARTICLE1); add(ARTICLE3); }}));
    assertEquals(historyListChangesListener.getNumberOfCalls(), 5);
    historyListChangesListener.setExpectedWords(new ArrayList< ArticleItem >());
    assertTrue(historyManager.removeAllWords());
    assertFalse(historyManager.removeAllWords());
    assertEquals(historyListChangesListener.getNumberOfCalls(), 6);
  }

  @Test
  public void testGetWords()
  {
    assertEquals(historyManager.getWords().size(), 0);
    historyManager.addWord(ARTICLE1);
    assertEquals(historyManager.getWords().size(), 1);
    assertEquals(historyManager.getWords().get(0), ARTICLE1);
  }

  @Test
  public void testOnStateChangedListener()
  {
    MockForManager_StateChangedListener listener
            = Mockito.mock(MockForManager_StateChangedListener.class
            , withSettings().useConstructor(2, historyManager).defaultAnswer(CALLS_REAL_METHODS));
    historyManager.registerNotifier(listener);
    historyManager.addWord(ARTICLE1);
    historyManager.removeAllWords();
    assertEquals(listener.getNumberOfCalls(), 2);
    historyManager.unregisterNotifier(listener);
    historyManager.addWord(ARTICLE1);
  }

  @Test
  public void testOnDictionaryListChangedListener() throws ManagerInitException, WrongTypeException
  {
    verify(settingsManager, times(1)).load(any(String.class), any(PersistentArticle[].class));
    doAnswer(new Answer()
    {
      @Override
      public Object answer( InvocationOnMock invocation )
      {
        return new PersistentArticle[]{ new PersistentArticle(ARTICLE1) };
      }
    }).when(settingsManager).load(any(String.class), any(PersistentArticle[].class));
    TestOnDictionaryListChangedNotifier dictionaryListChangedNotifier = new TestOnDictionaryListChangedNotifier();
    dictionaryListChangedNotifier.registerNotifier(historyManager);
    dictionaryListChangedNotifier.onDictionaryListChanged();
    verify(settingsManager, times(2)).load(any(String.class), any(PersistentArticle[].class));
    ShadowLooper.runUiThreadTasksIncludingDelayedTasks();
    verify(engineSerializer, times(1)).deserializeArticleItems(
        any(Serializable[].class), any(EngineSerializerAPI.OnDeserializedArticlesReadyCallback.class), eq(true));
  }

  private class TestOnDictionaryListChangedNotifier
  {

    private final ArrayList< DictionaryManagerAPI.IDictionaryListObserver > mOnDictionaryListListeners = new ArrayList<>();

    public void registerNotifier( DictionaryManagerAPI.IDictionaryListObserver notifier )
    {
      if (!mOnDictionaryListListeners.contains(notifier))
      {
        mOnDictionaryListListeners.add(notifier);
      }
    }

    public void onDictionaryListChanged()
    {
      for (final DictionaryManagerAPI.IDictionaryListObserver listener : mOnDictionaryListListeners)
      {
        listener.onDictionaryListChanged();
      }
    }
  }

  @Test
  public void testShowHistoryScreen() {
    historyManager.showHistoryScreen(mContext);
    verify(mContext, times(1)).startActivity(new Intent(mContext, mActivity.getClass()));
  }

  @Test
  public void testDeserializedArticles() throws ManagerInitException, WrongTypeException {
    engineSerializer = spy(MockEngineSerializer.class);
    doAnswer(new Answer() {
      @Override
      public Object answer(InvocationOnMock invocation) {
        return new PersistentArticle[]{new PersistentArticle(ARTICLE1)};
      }
    }).when(settingsManager).load(any(String.class), any(PersistentArticle[].class));

    historyManager = new BaseHistoryManager(screenOpener, engineSerializer, settingsManager
        , dictionaryManager, null, "provider", 1000);

    assertEquals(Arrays.asList(ARTICLE_ITEMS), historyManager.getWords());
    assertFalse(historyManager.isInTransition());
  }

  @Test
  public void testFreeController() {
    HistoryControllerAPI controller = historyManager.getController("test");
    verify(toolbarManager, times(1)).registerNotifier((ToolbarManager.Notifier) controller);
    verify(settingsManager, times(1)).registerNotifier((OnApplicationSettingsSaveListener) controller);
    historyManager.freeController("test");
    verify(toolbarManager, times(1)).unRegisterNotifier((ToolbarManager.Notifier) controller);
    verify(settingsManager, times(1)).unregisterNotifier((OnApplicationSettingsSaveListener) controller);
  }

  @Test
  public void testHistoryManagerHolder() {
    assertNull(HistoryManagerHolder.getManager());
    HistoryManagerHolder.setManager(historyManager);
    assertEquals(historyManager, HistoryManagerHolder.getManager());
  }
}