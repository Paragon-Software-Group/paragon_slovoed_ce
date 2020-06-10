package com.paragon_software.search_manager;


import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryAndDirection;
import com.paragon_software.dictionary_manager.MockDictionaryManager;
import com.paragon_software.mock.MockForManager_ErrorListener;
import com.paragon_software.mock.MockSearchEngine;
import com.paragon_software.mock.MockSoundManager;
import com.paragon_software.native_engine.SortType;
import com.paragon_software.settings_manager.ApplicationSettings;
import com.paragon_software.settings_manager.OnApplicationSettingsSaveListener;
import com.paragon_software.settings_manager.SettingsManagerAPI;
import com.paragon_software.settings_manager.exceptions.ManagerInitException;
import com.paragon_software.toolbar_manager.MockToolbarManager;
import com.paragon_software.utils_slovoed.text.LocalizedString;

import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.invocation.InvocationOnMock;
import org.mockito.junit.MockitoJUnitRunner;
import org.mockito.stubbing.Answer;

import java.util.ArrayList;
import java.util.List;

import io.reactivex.android.plugins.RxAndroidPlugins;
import io.reactivex.schedulers.Schedulers;

import static com.paragon_software.mock.StaticConsts.DELTA;
import static com.paragon_software.native_engine.SearchType.SEARCH_TYPE_DID_YOU_MEAN;
import static com.paragon_software.native_engine.SearchType.SEARCH_TYPE_FTS;
import static com.paragon_software.native_engine.SearchType.SEARCH_TYPE_WILD_CARD;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.Answers.CALLS_REAL_METHODS;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.BDDMockito.given;
import static org.mockito.Mockito.doAnswer;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;

@RunWith( MockitoJUnitRunner.class )
public class SearchManagerTest
{
  @Mock
  private SettingsManagerAPI settingsManager;
  @Mock
  private ApplicationSettings applicationSettings;
  
  private ParagonSearchManager paragonSearchManager;


  @BeforeClass
  public static void setUpClass() {
    RxAndroidPlugins.reset();
    RxAndroidPlugins.setInitMainThreadSchedulerHandler(scheduler -> Schedulers.trampoline());
  }

  @Before
  public void setUp()
  {
    paragonSearchManager = new ParagonSearchManager();
    given(applicationSettings.getEntryListFontSize()).willReturn(ApplicationSettings.getDefaultFontSize());
    doAnswer(new Answer()
    {
      @Override
      public Object answer( InvocationOnMock invocation )
      {
        return applicationSettings;
      }
    }).when(settingsManager).loadApplicationSettings();
  }

  @AfterClass
  public static void cleanUpClass() {
    RxAndroidPlugins.reset();
  }

  @Test
  public void checkRegisterSearchEngine()
  {
    MockSearchEngine searchEngine = Mockito.spy(MockSearchEngine.class);

    assertNull(searchEngine.mSearchString);

    paragonSearchManager.registerSearchEngine(searchEngine);

    String TEST_STRING = "TEST_STRING";

    paragonSearchManager.scroll(0, TEST_STRING,false, false);

    assertEquals(searchEngine.mSearchString, TEST_STRING);
  }

  @Test
  public void testSettingsManagerUsage()
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
    paragonSearchManager.registerSettingsManager(settingsManager);
    SearchController controller = paragonSearchManager.getController(SearchController.CONTROLLER_TYPE_DEFAULT);
    verify(settingsManager, times(1)).loadApplicationSettings();
    assertEquals(controller.getEntryListFontSize(), ApplicationSettings.getMaxFontSize(), DELTA);
  }

  @Test
  public void checkRegisterSoundManager()
  {
    MockSoundManager soundManager = Mockito.spy(MockSoundManager.class);
    assertEquals(soundManager.itemHasSoundCounter, 0);

    paragonSearchManager.registerSoundManager(soundManager);
    assertEquals(soundManager.itemHasSoundCounter, 0);

    ArticleItem item = new ArticleItem.Builder(new Dictionary.DictionaryId("test"),
            0,
            0).
            build();

    paragonSearchManager.hasSound(item);
    assertEquals(soundManager.itemHasSoundCounter, 1);
  }

  @Test
  public void testOnErrorListener()
  {
    MockForManager_ErrorListener controllerErrorListener
            = Mockito.mock(MockForManager_ErrorListener.class, Mockito.withSettings()
            .useConstructor(1).defaultAnswer(CALLS_REAL_METHODS));
    paragonSearchManager.registerNotifier(controllerErrorListener);
    ManagerInitException managerInitException = new ManagerInitException("ManagerInitException");
    controllerErrorListener.setExpectedException(managerInitException);
    paragonSearchManager.onError(managerInitException);
    assertEquals(controllerErrorListener.getNumberOfCalls(), 1);
  }

  @Test
  public void checkRegisterDictionaryManager()
  {
    MockDictionaryManager dictionaryManager = Mockito.spy(MockDictionaryManager.class);
    assertEquals(dictionaryManager.mGetDictionariesCounter, 0 );

    paragonSearchManager.registerDictionaryManager(dictionaryManager);
    assertEquals(dictionaryManager.mGetDictionariesCounter, 0 );

    paragonSearchManager.getDictionaries();
    assertEquals(dictionaryManager.mGetDictionariesCounter, 1 );
  }

  @Test
  public void checkSetSelectedDirection()
  {
    MockDictionaryManager dictionaryManager = Mockito.spy(MockDictionaryManager.class);
    assertEquals( new DictionaryAndDirection(MockDictionaryManager.dictId_1, MockDictionaryManager.direction_1),
            dictionaryManager.mDictionaryAndDirection);

    paragonSearchManager.registerDictionaryManager(dictionaryManager);
    assertEquals( new DictionaryAndDirection(MockDictionaryManager.dictId_1, MockDictionaryManager.direction_1),
            dictionaryManager.mDictionaryAndDirection);

    paragonSearchManager.setSelectedDirection(MockDictionaryManager.direction_2);
    verify(dictionaryManager, times(1)).setDictionaryAndDirectionSelectedByUser(
            eq(new DictionaryAndDirection(MockDictionaryManager.dictId_1, MockDictionaryManager.direction_2)));

    paragonSearchManager.setSelectedDirection(MockDictionaryManager.direction_1);
    verify(dictionaryManager, times(1)).setDictionaryAndDirectionSelectedByUser(
            eq(new DictionaryAndDirection(MockDictionaryManager.dictId_1, MockDictionaryManager.direction_1)));
  }

  @Test
  public void checkGetController()
  {
    paragonSearchManager.registerSettingsManager(settingsManager);

    assertNull(paragonSearchManager.getController("test"));
    assertNotNull(paragonSearchManager.getController(SearchController.CONTROLLER_TYPE_DEFAULT));
    assertNull(paragonSearchManager.getController("test"));
    assertNotNull(paragonSearchManager.getController(SearchController.CONTROLLER_TYPE_DEFAULT));
  }

  @Test
  public void checkScroll()
  {
    MockDictionaryManager dictionaryManager = Mockito.spy(MockDictionaryManager.class);
    paragonSearchManager.registerDictionaryManager(dictionaryManager);

    MockSearchEngine searchEngine = Mockito.spy(MockSearchEngine.class);
    paragonSearchManager.registerSearchEngine(searchEngine);

    String testString = "test";
    boolean autoChange = true;

    Dictionary.Direction direction = dictionaryManager.direction_1;
    paragonSearchManager.onDictionarySelect(dictionaryManager.dictId_1);

    assertNull(searchEngine.mSearchDirection);
    assertNull(searchEngine.mSearchString);

    paragonSearchManager.scroll(direction.getLanguageFrom(), testString, autoChange, false);

    assertEquals(searchEngine.mSearchDirection,direction);
    assertEquals(searchEngine.mSearchString,testString);
  }

  @Test
  public void checkSearchAll()
  {
    MockSearchEngine searchEngine = Mockito.spy(MockSearchEngine.class);
    paragonSearchManager.registerSearchEngine(searchEngine);

    String testString = "test";

    assertNull(searchEngine.mSearchString);
    paragonSearchManager.searchAll(testString);

    assertEquals(searchEngine.mSearchString,testString);
  }

  @Test
  public void checkFtsSearch()
  {
    MockDictionaryManager dictionaryManager = Mockito.spy(MockDictionaryManager.class);
    paragonSearchManager.registerDictionaryManager(dictionaryManager);

    MockSearchEngine searchEngine = Mockito.spy(MockSearchEngine.class);
    paragonSearchManager.registerSearchEngine(searchEngine);

    String testString = "test";
    boolean autoChange = true;

    Dictionary.Direction direction = dictionaryManager.direction_1;
    paragonSearchManager.onDictionarySelect(dictionaryManager.dictId_1);

    assertNull(searchEngine.mSearchDirection);
    assertNull(searchEngine.mSearchString);

    paragonSearchManager.search(direction.getLanguageFrom(), testString, autoChange, SEARCH_TYPE_FTS , SortType.Full, false);

    assertEquals(searchEngine.mSearchDirection,direction);
    assertEquals(searchEngine.mSearchString,testString);
  }

  @Test
  public void checkDidYouMeanSearch()
  {
    MockDictionaryManager dictionaryManager = Mockito.spy(MockDictionaryManager.class);
    paragonSearchManager.registerDictionaryManager(dictionaryManager);

    MockSearchEngine searchEngine = Mockito.spy(MockSearchEngine.class);
    paragonSearchManager.registerSearchEngine(searchEngine);

    String testString = "test";
    boolean autoChange = true;

    Dictionary.Direction direction = dictionaryManager.direction_1;
    paragonSearchManager.onDictionarySelect(dictionaryManager.dictId_1);

    assertNull(searchEngine.mSearchDirection);
    assertNull(searchEngine.mSearchString);

    paragonSearchManager.search(direction.getLanguageFrom(), testString, autoChange, SEARCH_TYPE_DID_YOU_MEAN, SortType.Full, false);

    assertEquals(searchEngine.mSearchDirection,direction);
    assertEquals(searchEngine.mSearchString,testString);
  }

  @Test
  public void checkWildCardSearch()
  {
    MockDictionaryManager dictionaryManager = Mockito.spy(MockDictionaryManager.class);
    paragonSearchManager.registerDictionaryManager(dictionaryManager);

    MockSearchEngine searchEngine = Mockito.spy(MockSearchEngine.class);
    paragonSearchManager.registerSearchEngine(searchEngine);

    String testString = "test";
    boolean autoChange = true;

    Dictionary.Direction direction = dictionaryManager.direction_1;
    paragonSearchManager.onDictionarySelect(dictionaryManager.dictId_1);

    assertNull(searchEngine.mSearchDirection);
    assertNull(searchEngine.mSearchString);

    paragonSearchManager.search(direction.getLanguageFrom(), testString, autoChange, SEARCH_TYPE_WILD_CARD, SortType.Full, false);

    assertEquals(searchEngine.mSearchDirection,direction);
    assertEquals(searchEngine.mSearchString,testString);
  }

  @Test
  public void checkPlaySound()
  {
    MockSoundManager soundManager = Mockito.spy(MockSoundManager.class);
    paragonSearchManager.registerSoundManager(soundManager);

    ArticleItem articleItem = new ArticleItem.Builder(null,0,0).build();

    assertEquals(soundManager.playSoundCounter,0);
    paragonSearchManager.playSound(articleItem);
    assertEquals(soundManager.playSoundCounter,1);
  }

  @Test
  public void checkHasSound()
  {
      MockSoundManager soundManager = Mockito.spy(MockSoundManager.class);
      paragonSearchManager.registerSoundManager(soundManager);

      ArticleItem articleItem = new ArticleItem.Builder(null,0,0).build();

      assertEquals(soundManager.itemHasSoundCounter,0);
      paragonSearchManager.hasSound(articleItem);
      assertEquals(soundManager.itemHasSoundCounter,1);
  }

  @Test
  public void checkGetSelectedDictionary()
  {
    MockDictionaryManager dictionaryManager = Mockito.spy(MockDictionaryManager.class);
    paragonSearchManager.registerDictionaryManager(dictionaryManager);

    dictionaryManager.mDictionaryAndDirection =  new DictionaryAndDirection(MockDictionaryManager.dictId_2, MockDictionaryManager.direction_2);
    assertEquals(paragonSearchManager.getSelectedDictionary(),MockDictionaryManager.dictId_2);

    dictionaryManager.mDictionaryAndDirection =  new DictionaryAndDirection(MockDictionaryManager.dictId_1, MockDictionaryManager.direction_1);
    assertEquals(paragonSearchManager.getSelectedDictionary(),MockDictionaryManager.dictId_1);
  }

  @Test
  public void checkGetSelectedDirection()
  {
    MockDictionaryManager dictionaryManager = Mockito.spy(MockDictionaryManager.class);
    paragonSearchManager.registerDictionaryManager(dictionaryManager);

    dictionaryManager.mDictionaryAndDirection =  new DictionaryAndDirection(MockDictionaryManager.dictId_2, MockDictionaryManager.direction_2);
    assertEquals(paragonSearchManager.getSelectedDirection(),dictionaryManager.mDictionaryAndDirection.getDirection().getLanguageFrom());

    dictionaryManager.mDictionaryAndDirection =  new DictionaryAndDirection(MockDictionaryManager.dictId_1, MockDictionaryManager.direction_1);
    assertEquals(paragonSearchManager.getSelectedDirection(),dictionaryManager.mDictionaryAndDirection.getDirection().getLanguageFrom());
  }

  @Test
  public void checkGetDictionaries()
  {
    MockDictionaryManager dictionaryManager = Mockito.spy(MockDictionaryManager.class);
    paragonSearchManager.registerDictionaryManager(dictionaryManager);

    assertEquals(paragonSearchManager.getDictionaries(),dictionaryManager.dictionaries);

    List<Dictionary> dictionaries = new ArrayList<>();
    dictionaries.add(new Dictionary.Builder(
            new Dictionary.DictionaryId("test_1"),
            LocalizedString.from("test_title_1"),
            LocalizedString.from("test_desctiption_1"),
                                 null).
            build());

    dictionaries.add(new Dictionary.Builder(
            new Dictionary.DictionaryId("test_2"),
            LocalizedString.from("test_title_2"),
            LocalizedString.from("test_desctiption_2"),
            null).
            build());

    dictionaryManager.dictionaries = dictionaries;
    assertEquals(paragonSearchManager.getDictionaries(),dictionaryManager.dictionaries);
  }

  @Test
  public void testOnApplicationSettingsChangeListener() {
    paragonSearchManager.registerSettingsManager(settingsManager);
    SearchController controller = paragonSearchManager.getController(SearchController.CONTROLLER_TYPE_DEFAULT);
    float initialEntryListFontSize = controller.getEntryListFontSize();
    TestOnApplicationSettingsSaveNotifier applicationSettingsSaveNotifier = new TestOnApplicationSettingsSaveNotifier();
    applicationSettingsSaveNotifier.registerNotifier(paragonSearchManager);
    float newEntryListFontSize = controller.getEntryListFontSize();
    if (initialEntryListFontSize + 1 < ApplicationSettings.getMaxFontSize())
    {
      newEntryListFontSize = initialEntryListFontSize + 1;
    }
    else if (initialEntryListFontSize - 1 > ApplicationSettings.getMinFontSize())
    {
      newEntryListFontSize = initialEntryListFontSize - 1;
    }
    if (newEntryListFontSize != initialEntryListFontSize)
    {
      given(applicationSettings.getEntryListFontSize()).willReturn(newEntryListFontSize);
      applicationSettingsSaveNotifier.onApplicationSettingsSave(applicationSettings);
      assertEquals(controller.getEntryListFontSize(), newEntryListFontSize, DELTA);
    }
  }

  private class TestOnApplicationSettingsSaveNotifier
  {
    private final ArrayList< OnApplicationSettingsSaveListener > mOnApplicationSettingsSaveListeners 
            = new ArrayList<>();

    public void registerNotifier( OnApplicationSettingsSaveListener notifier )
    {
      if (!mOnApplicationSettingsSaveListeners.contains(notifier))
      {
        mOnApplicationSettingsSaveListeners.add(notifier);
      }
    }

    public void onApplicationSettingsSave(ApplicationSettings applicationSettings)
    {
      for (final OnApplicationSettingsSaveListener listener : mOnApplicationSettingsSaveListeners )
      {
        listener.onApplicationSettingsSaved(applicationSettings);
      }
    }
  }
}
