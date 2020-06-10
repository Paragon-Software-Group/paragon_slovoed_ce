package com.paragon_software.favorites_manager;

import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.net.Uri;
import android.text.Html;
import android.text.Spanned;
import android.view.View;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.DictionaryAndDirection;
import com.paragon_software.dictionary_manager.MockDictionaryIcon;
import com.paragon_software.dictionary_manager.MockDictionaryManager;
import com.paragon_software.favorites_manager.mock.MockForController_EntryListFontSizeChangeListener;
import com.paragon_software.favorites_manager.mock.MockForController_ErrorListener;
import com.paragon_software.favorites_manager.mock.MockForController_FavoritesErrorNotifier;
import com.paragon_software.favorites_manager.mock.MockForController_FavoritesListChangedListener;
import com.paragon_software.favorites_manager.mock.MockForController_FavoritesSortingChangeListener;
import com.paragon_software.favorites_manager.mock.MockForController_FavoritesStateChangedNotifier;
import com.paragon_software.favorites_manager.mock.MockForController_SelectionModeChangedListener;
import com.paragon_software.favorites_manager.mock.MockForController_TransitionStateChangedListener;
import com.paragon_software.favorites_manager.mock.MockForController_onControlleShareActionChanged;
import com.paragon_software.favorites_manager.mock.MockForController_onControllerDeleteAllActionChanged;
import com.paragon_software.favorites_manager.mock.MockForController_onControllerShowDeleteSelectedDialogListener;
import com.paragon_software.favorites_manager.mock.MockForController_onControllerSotringActionChanged;
import com.paragon_software.hint_manager.HintManagerAPI;
import com.paragon_software.hint_manager.HintParams;
import com.paragon_software.hint_manager.HintType;
import com.paragon_software.screen_opener_api.ScreenOpenerAPI;
import com.paragon_software.settings_manager.ApplicationSettings;
import com.paragon_software.settings_manager.MockForController_AppSettingsSaveNotifier;
import com.paragon_software.settings_manager.SettingsManagerAPI;
import com.paragon_software.settings_manager.exceptions.LocalResourceUnavailableException;
import com.paragon_software.settings_manager.exceptions.ManagerInitException;
import com.paragon_software.settings_manager.exceptions.WrongTypeException;
import com.paragon_software.sound_manager.SoundManagerAPI;
import com.paragon_software.toolbar_manager.Notifiers.MockForController_BackActionClickNotifier;
import com.paragon_software.toolbar_manager.Notifiers.MockForController_DictionarySelectNotifier;
import com.paragon_software.toolbar_manager.Notifiers.MockForController_OnDeleteSelectedActionClickNotifier;
import com.paragon_software.toolbar_manager.Notifiers.MockForController_SelectAllActionClickNotifier;
import com.paragon_software.toolbar_manager.ToolbarManager;
import com.paragon_software.utils_slovoed.directory.Directory;
import com.paragon_software.utils_slovoed.directory.MockDirectory;
import com.paragon_software.utils_slovoed.directory.MockDirectoryAbstract;
import com.paragon_software.utils_slovoed.text.LocalizedString;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.io.File;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.atomic.AtomicBoolean;

import androidx.core.content.FileProvider;
import androidx.fragment.app.FragmentManager;

import io.reactivex.android.plugins.RxAndroidPlugins;
import io.reactivex.observers.TestObserver;
import io.reactivex.schedulers.Schedulers;
import io.reactivex.subjects.BehaviorSubject;
import io.reactivex.subjects.PublishSubject;

import static com.paragon_software.dictionary_manager.Dictionary.DICTIONARY_STATUS.DEMO;
import static com.paragon_software.favorites_manager.BaseFavoritesController.FAVORITE_MANAGER_EXPORT_FILE_PATH;
import static com.paragon_software.favorites_manager.BaseFavoritesController.MIME_TYPE_HTML;
import static com.paragon_software.favorites_manager.BaseFavoritesController.MIME_TYPE_TEXT;
import static com.paragon_software.favorites_manager.MockExportHtmlBuilder.CHOOSER_TITLE;
import static com.paragon_software.favorites_manager.MockExportHtmlBuilder.EXPORT_FILE_DESCRIPTION;
import static com.paragon_software.favorites_manager.MockExportHtmlBuilder.FOOTER;
import static com.paragon_software.favorites_manager.MockExportHtmlBuilder.HEADER;
import static com.paragon_software.favorites_manager.MockExportHtmlBuilder.PART_OF_SPEECH_COLOR;
import static com.paragon_software.favorites_manager.MockExportHtmlBuilder.SUBJECT;
import static com.paragon_software.favorites_manager.mock.StaticConsts.ARTICLE1;
import static com.paragon_software.favorites_manager.mock.StaticConsts.ARTICLE2;
import static com.paragon_software.favorites_manager.mock.StaticConsts.ARTICLE3;
import static com.paragon_software.favorites_manager.mock.StaticConsts.DELTA;
import static com.paragon_software.favorites_manager.mock.StaticConsts.DICT1_ID;
import static com.paragon_software.favorites_manager.mock.StaticConsts.DICT2_ID;
import static com.paragon_software.favorites_manager.mock.StaticConsts.DICT3_ID;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.BDDMockito.given;
import static org.mockito.Mockito.CALLS_REAL_METHODS;
import static org.mockito.Mockito.atLeast;
import static org.mockito.Mockito.doAnswer;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;
import static org.mockito.Mockito.withSettings;
import static org.powermock.api.mockito.PowerMockito.mock;

@RunWith(PowerMockRunner.class)
@PrepareForTest({Intent.class, FileProvider.class, Html.class})
public class BaseFavoritesControllerTest
{
  private MockDirectory rootDirectory;
  @Mock
  private FavoritesManagerAPI favoritesManager;
  @Mock
  private ScreenOpenerAPI screenOpener;

  @Mock
  private ToolbarManager       toolbarManager;
  @Mock
  private SettingsManagerAPI   settingsManager;
  @Mock
  private SoundManagerAPI      soundManager;
  @Mock
  private FavoritesSorterAPI   favoritesSorter;
  @Mock
  private HintManagerAPI mHintManager;

  private MockDictionaryManager dictionaryManager;

  @Mock
  private ApplicationSettings applicationSettings;
  @Mock
  private Intent mIntent;
  @Mock
  private Context mContext;

  private PublishSubject<Boolean> dictionaryAndDirectionSelectNotifier = PublishSubject.create();
  @Mock
  private DictionaryAndDirection dictionaryAndDirection;

  private Class mExportHtmlBuilderClass = MockExportHtmlBuilder.class;

  @Mock
  private Bitmap BITMAP1;
  @Mock
  private Bitmap BITMAP2;

  private BaseFavoritesController favoritesController;
  private String FILE_PROVIDER = "fileProvider";


  @BeforeClass
  public static void setUpClass() {
    RxAndroidPlugins.reset();
    RxAndroidPlugins.setInitMainThreadSchedulerHandler(scheduler -> Schedulers.trampoline());
  }

  @Before
  public void setUp() throws ManagerInitException, WrongTypeException
  {
    given(applicationSettings.getEntryListFontSize()).willReturn(ApplicationSettings.getDefaultFontSize());

    rootDirectory = new MockDirectory(null, "root");

    dictionaryManager = Mockito.spy(MockDictionaryManager.class);
    when(dictionaryManager.isSelectAllDictionaries()).thenReturn(true);
    when(dictionaryManager.getDictionaryAndDirectionChangeObservable()).thenReturn(dictionaryAndDirectionSelectNotifier);
    when(dictionaryManager.getDictionaryAndDirectionSelectedByUser()).thenReturn(dictionaryAndDirection);

    List<Directory< ArticleItem >> childList = new ArrayList<Directory< ArticleItem >>() {{
      add(Mockito.mock(MockDirectoryAbstract.class, withSettings().useConstructor(rootDirectory, "TEST1")
              .defaultAnswer(CALLS_REAL_METHODS)));
      add(Mockito.mock(MockDirectoryAbstract.class, withSettings().useConstructor(rootDirectory, "TEST2")
              .defaultAnswer(CALLS_REAL_METHODS)));
    }};
    rootDirectory.setChildList(childList);

    doAnswer(invocation -> applicationSettings).when(settingsManager).loadApplicationSettings();
    doAnswer(invocation -> BehaviorSubject.createDefault(rootDirectory)).when(favoritesManager)
            .getRootDirectoryObservable();
    given(settingsManager.load(any(String.class), eq(FavoritesSorting.BY_DATE_DESCENDING)))
            .willReturn(FavoritesSorting.BY_DATE_DESCENDING);
    favoritesController = new BaseFavoritesController(favoritesManager, screenOpener
            , settingsManager, dictionaryManager, toolbarManager, soundManager, mHintManager
            , favoritesSorter, mExportHtmlBuilderClass,"test", FILE_PROVIDER);
    favoritesController.activate();
  }

  @After
  public void cleanUp() {
    favoritesController.deactivate();
  }

  @AfterClass
  public static void cleanUpClass() {
    RxAndroidPlugins.reset();
  }

  @Test
  public void testSetGetWords()
  {
    assertEquals(0, favoritesController.getWords().size());
    favoritesController.setWords(new ArrayList< ArticleItem >() {{ add(ARTICLE1); }});
    assertEquals(1, favoritesController.getWords().size());
    assertEquals(ARTICLE1, favoritesController.getWords().get(0));
  }

  @Test
  public void testDeleteWords()
  {
    favoritesController.setWords(new ArrayList< ArticleItem >() {{ add(ARTICLE1); }});
    favoritesController.deleteWord(0);
    verify(favoritesManager, times(1)).removeWord(eq(ARTICLE1), any(Directory.class));
  }

  @Test
  public void testOpenArticle()
  {
    ArrayList<ArticleItem> wordsListForTest = new ArrayList<ArticleItem>() {{
      add(ARTICLE1);
    }};
    favoritesController.setWords(wordsListForTest);
    favoritesController.openArticle(0, null);
    verify(screenOpener, times(1)).showArticleFromSeparateList
            (wordsListForTest, 0, null);
  }

  @Test
  public void testOnFavoritesListChangedListener()
  {
    OnControllerFavoritesListChangedListener listener =
            Mockito.mock(MockForController_FavoritesListChangedListener.class,
                    withSettings().useConstructor(1).defaultAnswer(CALLS_REAL_METHODS));
    favoritesController.registerNotifier(listener);
    favoritesController.setWords(new ArrayList< ArticleItem >() {{ add(ARTICLE1); }});
    favoritesController.unregisterNotifier(listener);
    favoritesController.setWords(new ArrayList< ArticleItem >() {{ add(ARTICLE1); }});
  }

  @Test
  public void testSelectUnselectItem()
  {
    MockForController_SelectionModeChangedListener listener =
            Mockito.mock(MockForController_SelectionModeChangedListener.class,
                    withSettings().useConstructor(1, 2).defaultAnswer(CALLS_REAL_METHODS));
    favoritesController.setWords(new ArrayList< ArticleItem >() {{ add(ARTICLE1); }});
    assertEquals(favoritesController.getWords().size(), 1);
    assertEquals(favoritesController.getSelectedWords().size(), 0);
    favoritesController.registerNotifier(listener);
    listener.setExpectedListSize(1);
    favoritesController.selectItem(0);
    assertTrue(favoritesController.isInSelectionMode());
    assertEquals(favoritesController.getSelectedWords().size(), 1);
    assertEquals(favoritesController.getSelectedWords().get(0), ARTICLE1);
    listener.setExpectedListSize(0);
    favoritesController.unselectItem(0);
    assertEquals(favoritesController.getSelectedWords().size(), 0);
  }

  @Test
  public void testSetSelectionMode()
  {
    MockForController_SelectionModeChangedListener selectionListener =
            Mockito.mock(MockForController_SelectionModeChangedListener.class,
                    withSettings().useConstructor(1, 0).defaultAnswer(CALLS_REAL_METHODS));

    MockForController_onControllerShowDeleteSelectedDialogListener deleteDialogListener =
            Mockito.mock(MockForController_onControllerShowDeleteSelectedDialogListener.class,
                    withSettings().useConstructor(1).defaultAnswer(CALLS_REAL_METHODS));

    MockForController_OnDeleteSelectedActionClickNotifier deleteSelectionClickNotifier = Mockito.spy(MockForController_OnDeleteSelectedActionClickNotifier.class);

    deleteSelectionClickNotifier.registerNotifier(favoritesController);

    favoritesController.registerNotifier(selectionListener);
    favoritesController.registerNotifier(deleteDialogListener);
    favoritesController.setSelectionMode(true);
    assertTrue(favoritesController.isInSelectionMode());
    favoritesController.unregisterNotifier(selectionListener);
    favoritesController.setWords(new ArrayList< ArticleItem >() {{ add(ARTICLE1); }});
    favoritesController.selectItem(0);

    deleteSelectionClickNotifier.onDeleteSelectedActionClick();
    favoritesController.unregisterNotifier(deleteDialogListener);
    deleteSelectionClickNotifier.onDeleteSelectedActionClick();

    assertEquals(favoritesController.getSelectedWords().size(), 1);
    favoritesController.registerNotifier(deleteDialogListener);
    favoritesController.setSelectionMode(false);
    deleteSelectionClickNotifier.onDeleteSelectedActionClick();
    assertFalse(favoritesController.isInSelectionMode());
    assertEquals(favoritesController.getSelectedWords().size(), 0);
  }

  @Test
  public void testSetSelectedWords()
  {
    MockForController_SelectionModeChangedListener listener =
            Mockito.mock(MockForController_SelectionModeChangedListener.class,
                    withSettings().useConstructor(0, 1).defaultAnswer(CALLS_REAL_METHODS));
    favoritesController.registerNotifier(listener);
    listener.setExpectedListSize(1);
    assertEquals(favoritesController.getSelectedWords().size(), 0);
    favoritesController.setSelectedWords(new ArrayList< ArticleItem >() {{ add(ARTICLE1); }});
    assertEquals(favoritesController.getSelectedWords().size(), 1);
  }

  @Test
  public void testDeleteSelectedWords()
  {
    favoritesController.setSelectionMode(true);
    assertTrue(favoritesController.isInSelectionMode());
    List<ArticleItem> selectedWords = new ArrayList< ArticleItem >() {{ add(ARTICLE1); add(ARTICLE2); }};
    favoritesController.setSelectedWords(selectedWords);
    favoritesController.deleteSelectedWords();
    verify(favoritesManager, times(1)).removeWords(any(List.class), any(Directory.class));
    assertFalse(favoritesController.isInSelectionMode());
  }

  @Test
  public void testSelectionModeOff()
  {
    favoritesController.setSelectionMode(true);
    favoritesController.selectionModeOff();
    assertFalse(favoritesController.isInSelectionMode());
  }

  @Test
  public void testOnErrorListener()
  {
    MockForController_ErrorListener controllerErrorListener =
            Mockito.mock(MockForController_ErrorListener.class,
                    withSettings().useConstructor(1).defaultAnswer(CALLS_REAL_METHODS));
    favoritesController.registerNotifier(controllerErrorListener);
    MockForController_FavoritesErrorNotifier errorNotifier = Mockito.spy(MockForController_FavoritesErrorNotifier.class);
    errorNotifier.registerNotifier(favoritesController);
    ManagerInitException removeWordsException = new ManagerInitException("removeWordsException");
    controllerErrorListener.setExpectedException(removeWordsException);
    errorNotifier.onFavoritesError(removeWordsException);
    assertEquals(controllerErrorListener.getNumberOfCalls(), 1);
  }

  @Test
  public void testSetGetEntryListFontSize()
  {
    assertEquals(favoritesController.getEntryListFontSize(), ApplicationSettings.getDefaultFontSize(), DELTA);
    float initialEntryListFontSize = favoritesController.getEntryListFontSize();
    float newEntryListFontSize = initialEntryListFontSize;
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
      MockForController_EntryListFontSizeChangeListener entryListFontSizeChangeListener
          = Mockito.mock(MockForController_EntryListFontSizeChangeListener.class,
              withSettings().useConstructor(1, favoritesController).defaultAnswer(CALLS_REAL_METHODS));
      favoritesController.registerNotifier(entryListFontSizeChangeListener);
      entryListFontSizeChangeListener.setExpectedEntryListFontSize(newEntryListFontSize);
      favoritesController.setEntryListFontSize(newEntryListFontSize);
      assertEquals(favoritesController.getEntryListFontSize(), newEntryListFontSize, DELTA);
      assertEquals(entryListFontSizeChangeListener.getNumberOfCalls(), 1);
      favoritesController.unregisterNotifier(entryListFontSizeChangeListener);
      favoritesController.setEntryListFontSize(initialEntryListFontSize);
      assertEquals(favoritesController.getEntryListFontSize(), initialEntryListFontSize, DELTA);
    }
  }

  @Test
  public void testSetGetFavoritesSorting()
  {
    assertEquals(FavoritesSorting.BY_DATE_DESCENDING, favoritesController.getFavoritesSorting());
    FavoritesSorting initialFavoritesSorting = favoritesController.getFavoritesSorting();
    FavoritesSorting newFavoritesSorting = FavoritesSorting.ALPHABETICALLY_ASCENDING;
    if ( newFavoritesSorting == initialFavoritesSorting )
    {
      newFavoritesSorting = FavoritesSorting.ALPHABETICALLY_DESCENDING;
    }
    if ( newFavoritesSorting != initialFavoritesSorting )
    {
      MockForController_FavoritesSortingChangeListener sortingChangeListener
          = Mockito.mock(MockForController_FavoritesSortingChangeListener.class,
              withSettings().useConstructor(1, favoritesController).defaultAnswer(CALLS_REAL_METHODS));
      favoritesController.registerNotifier(sortingChangeListener);
      sortingChangeListener.setExpectedFavoritesSorting(newFavoritesSorting);
      favoritesController.selectSorting(newFavoritesSorting.ordinal());
      assertEquals(favoritesController.getFavoritesSorting(), newFavoritesSorting);
      assertEquals(sortingChangeListener.getNumberOfCalls(), 1);
      favoritesController.selectSorting(newFavoritesSorting.ordinal());
      assertEquals(sortingChangeListener.getNumberOfCalls(), 1);
      favoritesController.unregisterNotifier(sortingChangeListener);
      favoritesController.selectSorting(initialFavoritesSorting.ordinal());
      assertEquals(favoritesController.getFavoritesSorting(), initialFavoritesSorting);
    }
  }

  @Test
  public void testHasSound()
  {
    given(soundManager.itemHasSound(eq(ARTICLE1))).willReturn(true);
    given(soundManager.itemHasSound(eq(ARTICLE2))).willReturn(false);
    favoritesController.setWords(new ArrayList< ArticleItem >() {{ add(ARTICLE1); add(ARTICLE2); }});
    assertTrue(favoritesController.hasSound(0));
    assertFalse(favoritesController.hasSound(1));
    assertFalse(favoritesController.hasSound(2));
    verify(soundManager, times(2)).itemHasSound(any(ArticleItem.class));

    BaseFavoritesController controllerNoSound = new BaseFavoritesController(
        favoritesManager, screenOpener, settingsManager, dictionaryManager, toolbarManager
            , null, null, favoritesSorter, null
            ,"test", "provider");
    controllerNoSound.setWords(new ArrayList< ArticleItem >() {{ add(ARTICLE1); add(ARTICLE2); }});
    assertFalse(controllerNoSound.hasSound(0));
    assertFalse(controllerNoSound.hasSound(1));
  }

  @Test
  public void testPlaySound()
  {
    favoritesController.setWords(new ArrayList< ArticleItem >() {{ add(ARTICLE1); }});
    favoritesController.playSound(0);
    favoritesController.playSound(1);
    verify(soundManager, times(1)).playSound(any(ArticleItem.class));
  }

  @Test
  public void testSelectFavoritesSorting() throws ManagerInitException,
                                                 LocalResourceUnavailableException
  {
    favoritesController.selectSorting(FavoritesSorting.values().length - 4);
    verify(settingsManager, times(1))
        .save(any(String.class), eq(FavoritesSorting.ALPHABETICALLY_ASCENDING), any(Boolean.class));
    favoritesController.selectSorting(FavoritesSorting.values().length - 3);
    verify(settingsManager, times(1))
        .save(any(String.class), eq(FavoritesSorting.ALPHABETICALLY_DESCENDING), any(Boolean.class));
    favoritesController.selectSorting(FavoritesSorting.values().length - 2);
    verify(settingsManager, times(1))
        .save(any(String.class), eq(FavoritesSorting.BY_DATE_ASCENDING), any(Boolean.class));
    favoritesController.selectSorting(FavoritesSorting.values().length - 1);
    verify(settingsManager, times(1))
        .save(any(String.class), eq(FavoritesSorting.BY_DATE_DESCENDING), any(Boolean.class));
  }

  @Test
  public void testFavoritesWordsSortingSorterUsage()
  {
    favoritesController.setWords(new ArrayList< ArticleItem >()
    {{ add(ARTICLE1); add(ARTICLE2); add(ARTICLE3); }});
    favoritesController.selectSorting(FavoritesSorting.values().length - 4);
    verify(favoritesSorter, times(1)).sort(any(List.class)
            , eq(FavoritesSorting.ALPHABETICALLY_ASCENDING));
    favoritesController.selectSorting(FavoritesSorting.values().length - 3);
    verify(favoritesSorter, times(1)).sort(any(List.class)
            , eq(FavoritesSorting.ALPHABETICALLY_DESCENDING));
    favoritesController.selectSorting(FavoritesSorting.values().length - 2);
    verify(favoritesSorter, times(1)).sort(any(List.class)
            , eq(FavoritesSorting.BY_DATE_ASCENDING));
    favoritesController.selectSorting(FavoritesSorting.values().length - 1);
    verify(favoritesSorter, times(1)).sort(any(List.class)
            , eq(FavoritesSorting.BY_DATE_DESCENDING));
  }

  @Test
  public void testOnApplicationSettingsSaveListener()
  {
    float initialEntryListFontSize = favoritesController.getEntryListFontSize();
    MockForController_AppSettingsSaveNotifier applicationSettingsSaveNotifier
            = Mockito.spy(MockForController_AppSettingsSaveNotifier.class);
    applicationSettingsSaveNotifier.registerNotifier(favoritesController);
    float newEntryListFontSize = favoritesController.getEntryListFontSize();
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
      assertEquals(favoritesController.getEntryListFontSize(), newEntryListFontSize, 0.01f);
    }
  }

  @Test
  public void testOnStateChanged()
  {
    MockForController_TransitionStateChangedListener controllerTransitionStateListener
            = Mockito.mock(MockForController_TransitionStateChangedListener.class,
            withSettings().useConstructor(2, favoritesController).defaultAnswer(CALLS_REAL_METHODS));
    favoritesController.registerNotifier(controllerTransitionStateListener);
    MockForController_FavoritesStateChangedNotifier stateChangedNotifier
            = Mockito.spy(MockForController_FavoritesStateChangedNotifier.class);
    stateChangedNotifier.registerNotifier(favoritesController);
    controllerTransitionStateListener.setExpectedState(true);
    given(favoritesManager.isInTransition()).willReturn(true);
    stateChangedNotifier.onStateChanged();
    assertEquals(controllerTransitionStateListener.getNumberOfCalls(), 1);
    controllerTransitionStateListener.setExpectedState(false);
    given(favoritesManager.isInTransition()).willReturn(false);
    stateChangedNotifier.onStateChanged();
    assertEquals(controllerTransitionStateListener.getNumberOfCalls(), 2);
  }

  @Test
  public void testToolbarSelectAllActionClick()
  {
    favoritesController.setWords(new ArrayList< ArticleItem >() {{ add(ARTICLE1); add(ARTICLE2); }});
    assertEquals(favoritesController.getWords().size(), 2);
    favoritesController.setSelectionMode(true);
    assertEquals(favoritesController.getSelectedWords().size(), 0);
    MockForController_SelectAllActionClickNotifier selectAllActionClickNotifier
        = Mockito.spy(MockForController_SelectAllActionClickNotifier.class);
    selectAllActionClickNotifier.registerNotifier(favoritesController);
    selectAllActionClickNotifier.selectAllActionClick();
    assertEquals(favoritesController.getSelectedWords().size(), 2);
    verify(toolbarManager, times(1)).showSelectionMode(2);
  }

  @Test
  public void testSelectionMode()
  {
    MockForController_onControllerSotringActionChanged listener =
            Mockito.mock(MockForController_onControllerSotringActionChanged.class,
                    withSettings().useConstructor(2, 1).defaultAnswer(CALLS_REAL_METHODS));

    MockForController_onControllerDeleteAllActionChanged listener2 =
            Mockito.mock(MockForController_onControllerDeleteAllActionChanged.class,
                    withSettings().useConstructor(2, 1).defaultAnswer(CALLS_REAL_METHODS));

    MockForController_onControlleShareActionChanged listener3 =
            Mockito.mock(MockForController_onControlleShareActionChanged.class,
                    withSettings().useConstructor(2, 0).defaultAnswer(CALLS_REAL_METHODS));

    favoritesController.registerNotifier(listener);
    favoritesController.registerNotifier(listener2);
    favoritesController.registerNotifier(listener3);

    assertFalse(favoritesController.isSortingActionEnable());
    assertFalse(favoritesController.isDeleteAllActionEnable());
    assertTrue(favoritesController.isShareActionEnable());
    favoritesController.setWords(new ArrayList< ArticleItem >() {{ add(ARTICLE1); add(ARTICLE2); }});
    assertFalse(favoritesController.isInSelectionMode());
    assertEquals(View.VISIBLE,favoritesController.getSortingActionVisibilityStatus());
    assertEquals(View.VISIBLE,favoritesController.getDeleteAllActionVisibilityStatus());
    assertEquals(View.VISIBLE,favoritesController.getShareActionVisibilityStatus());
    favoritesController.setSelectedWords(new ArrayList< ArticleItem >() {{ add(ARTICLE1); }});

    favoritesController.setSelectionMode(true);
    verify(toolbarManager, times(1)).showSelectionMode(1);
    assertTrue(favoritesController.isInSelectionMode());
    assertEquals(View.GONE,favoritesController.getSortingActionVisibilityStatus());
    assertEquals(View.GONE,favoritesController.getDeleteAllActionVisibilityStatus());
    assertEquals(View.GONE,favoritesController.getShareActionVisibilityStatus());
    favoritesController.selectItem(1);
    verify(toolbarManager, times(1)).showSelectionMode(2);
    favoritesController.unselectItem(0);
    verify(toolbarManager, times(2)).showSelectionMode(1);

    favoritesController.setSelectionMode(false);
    assertFalse(favoritesController.isInSelectionMode());
    assertTrue(favoritesController.isSortingActionEnable());
    assertTrue(favoritesController.isDeleteAllActionEnable());
    assertEquals(View.VISIBLE,favoritesController.getSortingActionVisibilityStatus());
    assertEquals(View.VISIBLE,favoritesController.getDeleteAllActionVisibilityStatus());
    assertEquals(View.VISIBLE,favoritesController.getShareActionVisibilityStatus());
    verify(toolbarManager, atLeast(1)).showDictionaryListNoDirection();

    favoritesController.unregisterNotifier(listener);
    favoritesController.unregisterNotifier(listener2);
    favoritesController.unregisterNotifier(listener3);

    favoritesController.setSelectionMode(true);
  }

  @Test
  public void testToolbarBackActionClick()
  {
    favoritesController.setSelectionMode(true);
    assertTrue(favoritesController.isInSelectionMode());
    MockForController_BackActionClickNotifier backActionClickNotifier
            = Mockito.spy(MockForController_BackActionClickNotifier.class);
    backActionClickNotifier.registerNotifier(favoritesController);
    backActionClickNotifier.backActionClick();
    assertFalse(favoritesController.isInSelectionMode());
  }

  @Test
  public void checkDeleteAllWordsActionClick()
  {
    favoritesController.deleteAllWords();
    verify(favoritesManager, times(1)).removeWords(any(List.class), any(Directory.class));
  }

  @Test
  public void testFilterFavoritesWordsByDictionaryId()
  {
    verify(toolbarManager, atLeast(1)).showDictionaryListNoDirection();
    favoritesController = new BaseFavoritesController(favoritesManager, screenOpener
            , settingsManager, dictionaryManager, toolbarManager, soundManager, null
            , null, null ,"test", "provider");
    favoritesController.activate();
    assertEquals(favoritesController.getWords().size(), 0);
    favoritesController.setWords(new ArrayList< ArticleItem >()
    {{ add(ARTICLE1); add(ARTICLE2); add(ARTICLE3); }});
    assertEquals(favoritesController.getWords().size(), 3);
    when(dictionaryManager.isSelectAllDictionaries()).thenReturn(false);
    when(dictionaryAndDirection.getDictionaryId()).thenReturn(DICT1_ID);
    dictionaryAndDirectionSelectNotifier.onNext(true);
    assertEquals(favoritesController.getWords().size(), 1);
    when(dictionaryAndDirection.getDictionaryId()).thenReturn(DICT2_ID);
    dictionaryAndDirectionSelectNotifier.onNext(true);
    assertEquals(favoritesController.getWords().size(), 2);
    verify(toolbarManager, atLeast(3)).showDictionaryListNoDirection();
    when(dictionaryAndDirection.getDictionaryId()).thenReturn(DICT3_ID);
    dictionaryAndDirectionSelectNotifier.onNext(true);
    assertEquals(favoritesController.getWords().size(), 0);
    verify(toolbarManager, atLeast(4)).showDictionaryListNoDirection();
    when(dictionaryManager.isSelectAllDictionaries()).thenReturn(true);
    dictionaryAndDirectionSelectNotifier.onNext(true);
    assertEquals(favoritesController.getWords().size(), 3);
    when(dictionaryManager.isSelectAllDictionaries()).thenReturn(false);
    when(dictionaryAndDirection.getDictionaryId()).thenReturn(DICT2_ID);
    dictionaryAndDirectionSelectNotifier.onNext(true);
    favoritesController.deleteAllWords();
    verify(favoritesManager, times(1)).removeWords
            (eq(new ArrayList< ArticleItem >() {{ add(ARTICLE2); add(ARTICLE3); }}), any(Directory.class));
    favoritesController.deactivate();
  }



  @Test
  public void testDirectoryNavigation()
  {
    final String[] expectedPath = new String[1];
    expectedPath[0] = rootDirectory.getPath();
    favoritesController.getCurrentDirectoryObservable().subscribe(new TestObserver<Directory< ArticleItem >>() {
      @Override
      public void onNext( Directory< ArticleItem > directory )
      {
        assertEquals(expectedPath[0], directory.getPath());
      }
    });

    expectedPath[0] = rootDirectory.getChildList().get(0).getPath();
    favoritesController.goToDirectory(rootDirectory.getChildList().get(0));
    expectedPath[0] = rootDirectory.getPath();
    favoritesController.goToDirectory(rootDirectory.getChildList().get(0).getParent());
    expectedPath[0] = rootDirectory.getChildList().get(1).getPath();
    favoritesController.goToDirectory(rootDirectory.getChildList().get(1));

    assertEquals(expectedPath[0], favoritesController.getCurrentDirectory().getPath());

    expectedPath[0] = rootDirectory.getPath();
    favoritesController.goToRootDirectory();
    assertEquals(rootDirectory.getPath(), favoritesController.getCurrentDirectory().getPath());
  }

  @Test
  public void testHasChildDirectory()
  {
    favoritesController.hasChildDirectory(rootDirectory.getChildList().get(0).getName());
    verify(favoritesManager, atLeast(1)).hasChildDirectory(
        eq(favoritesController.getCurrentDirectory()), eq(rootDirectory.getChildList().get(0).getName()));

    favoritesController.hasChildDirectory(rootDirectory.getChildList().get(1).getName());
    verify(favoritesManager, atLeast(1)).hasChildDirectory(
        eq(favoritesController.getCurrentDirectory()), eq(rootDirectory.getChildList().get(1).getName()));
  }

  @Test
  public void testAddRemoveDirectory()
  {
    favoritesController.setDirs(new ArrayList< Directory<ArticleItem >>() {{ add(rootDirectory); }});
    favoritesController.addDirectory("new directory");
    verify(favoritesManager, atLeast(1)).addDirectory(
        eq(favoritesController.getCurrentDirectory()), eq("new directory"));

    favoritesController.deleteDirectory(favoritesController.getCurrentDirectory().getChildList().get(0));
    verify(favoritesManager, atLeast(1)).removeDirectory(
        eq(favoritesController.getCurrentDirectory().getChildList().get(0)));
  }

  @Test
  public void testDictionaryIcon() {
    MockDictionaryIcon dictionaryIcon1 = Mockito.mock(MockDictionaryIcon.class,
            withSettings().useConstructor(1, BITMAP1).defaultAnswer(CALLS_REAL_METHODS));
    MockDictionaryIcon dictionaryIcon2 = Mockito.mock(MockDictionaryIcon.class,
            withSettings().useConstructor(1, BITMAP2).defaultAnswer(CALLS_REAL_METHODS));

    dictionaryManager.registerDictionary(DICT1_ID, LocalizedString.from("")
            , LocalizedString.from(""), DEMO, dictionaryIcon1);

    assertEquals(dictionaryIcon1, favoritesController.getDictionaryIcon(DICT1_ID));

    dictionaryManager.registerDictionary(DICT2_ID, LocalizedString.from("")
            , LocalizedString.from(""), DEMO, dictionaryIcon2);

    assertEquals(dictionaryIcon2, favoritesController.getDictionaryIcon(DICT2_ID));

    assertNull(favoritesController.getDictionaryIcon(DICT3_ID));
  }

  @Test
  public void testEditModeStatus() {
    AtomicBoolean expectedResult = new AtomicBoolean();
    favoritesController.getEditModeStatusObservable().subscribe(new TestObserver<Boolean>() {
      @Override
      public void onNext(Boolean result) {
        assertEquals(expectedResult.get(), result);
      }
    });
    assertFalse(favoritesController.isInEditMode());
    expectedResult.set(true);
    favoritesController.setEditModeStatus(true);
    assertTrue(favoritesController.isInEditMode());
    expectedResult.set(false);
    favoritesController.setEditModeStatus(false);
    assertFalse(favoritesController.isInEditMode());
  }

  @Test
  public void testExport() {
    MockBaseFavoritesController mockController = new MockBaseFavoritesController(favoritesManager, screenOpener
            , settingsManager, dictionaryManager, toolbarManager, soundManager, null
            , favoritesSorter, mExportHtmlBuilderClass, "test", FILE_PROVIDER);

    mockController.setIntentForExport(mIntent);
    PowerMockito.mockStatic(Intent.class);
    PowerMockito.mockStatic(FileProvider.class);
    Uri uri = mock(Uri.class);
    File file = new File(mContext.getFilesDir(), FAVORITE_MANAGER_EXPORT_FILE_PATH);
    Mockito.when(Intent.createChooser(mIntent, CHOOSER_TITLE)).thenReturn(mIntent);
    Mockito.when(FileProvider.getUriForFile(mContext,  FILE_PROVIDER, file)).thenReturn(uri);

    ExportParams exportParams = new ExportParams(
            HEADER,
            CHOOSER_TITLE,
            FOOTER,
            SUBJECT,
            EXPORT_FILE_DESCRIPTION,
            PART_OF_SPEECH_COLOR);

    mockController.exportFavorites(mContext, exportParams);
    verify(mIntent, times(1)).setType(MIME_TYPE_HTML);
    verify(mContext, times(1)).startActivity(mIntent);

    PowerMockito.mockStatic(Html.class);
    Spanned spanned = mock(Spanned.class);
    Mockito.when(Html.fromHtml("")).thenReturn(spanned);

    mockController.exportFavoritesAsString(mContext, exportParams);

    verify(mIntent, times(1)).setType(MIME_TYPE_TEXT);
    verify(mContext, times(2)).startActivity(mIntent);
  }

  @Test
  public void testHint() {
    FragmentManager fragmentManager = mock(FragmentManager.class);
    HintParams hintParams = mock(HintParams.class);
    favoritesController.showHintManagerDialog(HintType.Favorites, fragmentManager, hintParams);
    verify(mHintManager, times(1)).showHintDialog(HintType.Favorites, fragmentManager, hintParams);
    favoritesController.showHintManagerDialog(HintType.FavoritesSort, fragmentManager, hintParams);
    verify(mHintManager, times(1)).showHintDialog(HintType.FavoritesSort, fragmentManager, hintParams);
  }

  @Test
  public void testDeactivate()
  {
    verify(favoritesManager, times(1)).registerNotifier(favoritesController);
    verify(toolbarManager, times(1)).registerNotifier(favoritesController);
    verify(settingsManager, times(1)).registerNotifier(favoritesController);

    favoritesController.deactivate();
    verify(favoritesManager, times(1)).unregisterNotifier(favoritesController);
    verify(toolbarManager, times(1)).unRegisterNotifier(favoritesController);
    verify(settingsManager, times(1)).unregisterNotifier(favoritesController);
  }
}
