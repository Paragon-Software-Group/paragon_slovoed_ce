package com.paragon_software.history_manager;

import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.net.Uri;
import android.text.Html;
import android.text.Spanned;
import android.view.View;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.article_manager.ShowArticleOptions;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryAndDirection;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.dictionary_manager.MockDictionaryIcon;
import com.paragon_software.hint_manager.HintManagerAPI;
import com.paragon_software.hint_manager.HintType;
import com.paragon_software.history_manager.mock.MockForController_EntryListFontSizeChangeListener;
import com.paragon_software.history_manager.mock.MockForController_ErrorListener;
import com.paragon_software.history_manager.mock.MockForController_HistoryListChangedListener;
import com.paragon_software.history_manager.mock.MockForController_SelectionModeChangedListener;
import com.paragon_software.history_manager.mock.MockForController_TransitionStateChangedListener;
import com.paragon_software.history_manager.mock.MockForController_onControllerShowDeleteSelectedDialogListener;
import com.paragon_software.screen_opener_api.ScreenOpenerAPI;
import com.paragon_software.settings_manager.ApplicationSettings;
import com.paragon_software.settings_manager.OnApplicationSettingsSaveListener;
import com.paragon_software.settings_manager.SettingsManagerAPI;
import com.paragon_software.settings_manager.exceptions.ManagerInitException;
import com.paragon_software.sound_manager.SoundManagerAPI;
import com.paragon_software.toolbar_manager.Notifiers.MockForController_OnDeleteSelectedActionClickNotifier;
import com.paragon_software.toolbar_manager.Notifiers.OnBackActionClick;
import com.paragon_software.toolbar_manager.Notifiers.OnSelectAllActionClick;
import com.paragon_software.toolbar_manager.ToolbarManager;
import com.paragon_software.utils_slovoed.text.LocalizedString;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.invocation.InvocationOnMock;
import org.mockito.stubbing.Answer;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.io.File;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import androidx.core.content.FileProvider;
import androidx.fragment.app.FragmentManager;

import io.reactivex.android.plugins.RxAndroidPlugins;
import io.reactivex.plugins.RxJavaPlugins;
import io.reactivex.schedulers.Schedulers;
import io.reactivex.subjects.PublishSubject;

import static com.paragon_software.history_manager.BaseHistoryController.HISTORY_MANAGER_EXPORT_FILE_PATH;
import static com.paragon_software.history_manager.BaseHistoryController.MIME_TYPE_HTML;
import static com.paragon_software.history_manager.BaseHistoryController.MIME_TYPE_TEXT;
import static com.paragon_software.history_manager.MockExportHTMLBuilder.ARTICLE_LIST;
import static com.paragon_software.history_manager.MockExportHTMLBuilder.CHOOSER_TITLE;
import static com.paragon_software.history_manager.MockExportHTMLBuilder.EXPORT_FILE_DESCRIPTION;
import static com.paragon_software.history_manager.MockExportHTMLBuilder.FOOTER;
import static com.paragon_software.history_manager.MockExportHTMLBuilder.HEADER;
import static com.paragon_software.history_manager.MockExportHTMLBuilder.PART_OF_SPEECH_COLOR;
import static com.paragon_software.history_manager.mock.StaticConsts.ARTICLE1;
import static com.paragon_software.history_manager.mock.StaticConsts.ARTICLE2;
import static com.paragon_software.history_manager.mock.StaticConsts.ARTICLE3;
import static com.paragon_software.history_manager.mock.StaticConsts.DELTA;
import static com.paragon_software.history_manager.mock.StaticConsts.DICT1_ID;
import static com.paragon_software.history_manager.mock.StaticConsts.DICT2_ID;
import static com.paragon_software.history_manager.mock.StaticConsts.DICT3_ID;
import static com.paragon_software.history_manager.mock.StaticConsts.FILE_PROVIDER;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.ArgumentMatchers.isA;
import static org.mockito.ArgumentMatchers.isNull;
import static org.mockito.Mockito.CALLS_REAL_METHODS;
import static org.mockito.Mockito.when;
import static org.mockito.Mockito.doAnswer;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.withSettings;

@RunWith(PowerMockRunner.class)
@PrepareForTest({Intent.class, FileProvider.class, Html.class})
public class BaseHistoryControllerTest {
  @Mock
  private HistoryManagerAPI historyManager;
  @Mock
  private ScreenOpenerAPI screenOpener;

  @Mock
  private ToolbarManager toolbarManager;
  @Mock
  private SettingsManagerAPI settingsManager;
  @Mock
  private SoundManagerAPI soundManager;
  @Mock
  private DictionaryManagerAPI dictionaryManager;
  @Mock
  private HintManagerAPI hintManager;

  @Mock
  private ApplicationSettings applicationSettings;
  @Mock
  private Bitmap BITMAP1;
  @Mock
  private Bitmap BITMAP2;
  @Mock
  private FragmentManager fragmentManager;
  @Mock
  private Intent mIntent;
  @Mock
  private Context mContext;

  private PublishSubject<Boolean> dictionaryAndDirectionSelectNotifier = PublishSubject.create();
  @Mock
  private DictionaryAndDirection dictionaryAndDirection;


  private BaseHistoryController historyController;
  private Class mExportHtmlBuilderClass = MockExportHTMLBuilder.class;


  @BeforeClass
  public static void setUpClass() {
    RxAndroidPlugins.reset();
    RxAndroidPlugins.setInitMainThreadSchedulerHandler(scheduler -> Schedulers.trampoline());
  }

  @Before
  public void setUp() {
    when(applicationSettings.getEntryListFontSize()).thenReturn(ApplicationSettings.getDefaultFontSize());
    doAnswer(new Answer() {
      @Override
      public Object answer(InvocationOnMock invocation) {
        return applicationSettings;
      }
    }).when(settingsManager).loadApplicationSettings();
    when(toolbarManager.getCurrentDictionary()).thenReturn(null);
    when(dictionaryManager.isSelectAllDictionaries()).thenReturn(true);
    when(dictionaryManager.getDictionaryAndDirectionChangeObservable()).thenReturn(dictionaryAndDirectionSelectNotifier);
    when(dictionaryManager.getDictionaryAndDirectionSelectedByUser()).thenReturn(dictionaryAndDirection);
    historyController = new BaseHistoryController(historyManager, screenOpener, settingsManager
        , dictionaryManager, toolbarManager, soundManager, hintManager
        , mExportHtmlBuilderClass, FILE_PROVIDER);
    historyController.activate();
  }

  @After
  public void cleanUp() {
    historyController.deactivate();
  }

  @AfterClass
  public static void cleanUpClass() {
    RxAndroidPlugins.reset();
  }

  @Test
  public void testSetGetWords() {
    assertEquals(0, historyController.getWords().size());
    historyController.setWords(new ArrayList<ArticleItem>() {{
      add(ARTICLE1);
    }});
    assertEquals(1, historyController.getWords().size());
    assertEquals(ARTICLE1, historyController.getWords().get(0));
  }

  @Test
  public void testOpenArticle() {
    historyController.setWords(new ArrayList<ArticleItem>() {{
      add(ARTICLE1);
    }});
    historyController.openArticle(0, null);
    verify(screenOpener, times(1)).showArticle(eq(ARTICLE1)
        , isA(ShowArticleOptions.class), (Context) isNull());
  }

  @Test
  public void testOnHistoryListChangedListener() {
    OnControllerHistoryListChangedListener listener
        = mock(MockForController_HistoryListChangedListener.class
        , withSettings().useConstructor(1).defaultAnswer(CALLS_REAL_METHODS));
    historyController.registerNotifier(listener);
    historyController.setWords(new ArrayList<ArticleItem>() {{
      add(ARTICLE1);
    }});
    historyController.unregisterNotifier(listener);
    historyController.setWords(new ArrayList<ArticleItem>() {{
      add(ARTICLE1);
    }});
  }

  @Test
  public void testSelectUnselectItem() {
    MockForController_SelectionModeChangedListener listener
        = mock(MockForController_SelectionModeChangedListener.class
        , withSettings().useConstructor(1, 2).defaultAnswer(CALLS_REAL_METHODS));
    historyController.setWords(new ArrayList<ArticleItem>() {{
      add(ARTICLE1);
    }});
    assertEquals(historyController.getWords().size(), 1);
    assertEquals(historyController.getSelectedWords().size(), 0);
    historyController.registerNotifier(listener);
    listener.setExpectedListSize(1);
    historyController.selectItem(0);
    assertTrue(historyController.isInSelectionMode());
    assertEquals(historyController.getSelectedWords().size(), 1);
    assertEquals(historyController.getSelectedWords().get(0), ARTICLE1);
    listener.setExpectedListSize(0);
    historyController.unselectItem(0);
    assertEquals(historyController.getSelectedWords().size(), 0);
  }

  @Test
  public void testSetSelectionMode() {
    MockForController_SelectionModeChangedListener listener
        = mock(MockForController_SelectionModeChangedListener.class
        , withSettings().useConstructor(1, 0).defaultAnswer(CALLS_REAL_METHODS));

    MockForController_OnDeleteSelectedActionClickNotifier deleteSelectionClickNotifier = Mockito.spy(MockForController_OnDeleteSelectedActionClickNotifier.class);
    MockForController_onControllerShowDeleteSelectedDialogListener deleteDialogListener =
        Mockito.mock(MockForController_onControllerShowDeleteSelectedDialogListener.class,
            withSettings().useConstructor(1).defaultAnswer(CALLS_REAL_METHODS));

    deleteSelectionClickNotifier.registerNotifier(historyController);
    historyController.registerNotifier(deleteDialogListener);

    historyController.registerNotifier(listener);
    historyController.setSelectionMode(true);
    assertTrue(historyController.isInSelectionMode());
    assertEquals(View.GONE, historyController.getExportActionVisibilityStatus());
    assertFalse(historyController.isExportActionEnable());
    assertEquals(View.GONE, historyController.getDeleteAllActionVisibilityStatus());
    assertFalse(historyController.isDeleteAllActionEnable());
    historyController.unregisterNotifier(listener);
    historyController.setWords(new ArrayList<ArticleItem>() {{
      add(ARTICLE1);
    }});
    historyController.selectItem(0);
    assertEquals(historyController.getSelectedWords().size(), 1);
    deleteSelectionClickNotifier.onDeleteSelectedActionClick();
    historyController.unregisterNotifier(deleteDialogListener);
    deleteSelectionClickNotifier.onDeleteSelectedActionClick();
    historyController.registerNotifier(deleteDialogListener);
    historyController.setSelectionMode(false);
    deleteSelectionClickNotifier.onDeleteSelectedActionClick();
    assertFalse(historyController.isInSelectionMode());
    assertEquals(View.VISIBLE, historyController.getExportActionVisibilityStatus());
    assertTrue(historyController.isExportActionEnable());
    assertEquals(View.VISIBLE, historyController.getDeleteAllActionVisibilityStatus());
    assertTrue(historyController.isDeleteAllActionEnable());
    assertEquals(historyController.getSelectedWords().size(), 0);
  }

  @Test
  public void testSetSelectedWords() {
    MockForController_SelectionModeChangedListener listener
        = mock(MockForController_SelectionModeChangedListener.class
        , withSettings().useConstructor(0, 1).defaultAnswer(CALLS_REAL_METHODS));
    historyController.registerNotifier(listener);
    listener.setExpectedListSize(1);
    assertEquals(historyController.getSelectedWords().size(), 0);
    historyController.setSelectedWords(new ArrayList<ArticleItem>() {{
      add(ARTICLE1);
    }});
    assertEquals(historyController.getSelectedWords().size(), 1);
  }

  @Test
  public void testDeleteSelectedWords() {
    historyController.setSelectionMode(true);
    assertTrue(historyController.isInSelectionMode());
    List<ArticleItem> selectedWords = new ArrayList<ArticleItem>() {{
      add(ARTICLE1);
      add(ARTICLE2);
    }};
    historyController.setSelectedWords(selectedWords);
    historyController.deleteSelectedWords();
    verify(historyManager, times(1)).removeWords(any(List.class));
    assertFalse(historyController.isInSelectionMode());
  }

  @Test
  public void testSelectionModeOff() {
    historyController.setSelectionMode(true);
    historyController.selectionModeOff();
    assertFalse(historyController.isInSelectionMode());
  }

  @Test
  public void testOnErrorListener() {
    MockForController_ErrorListener controllerErrorListener
        = mock(MockForController_ErrorListener.class
        , withSettings().useConstructor(1).defaultAnswer(CALLS_REAL_METHODS));
    historyController.registerNotifier(controllerErrorListener);
    TestOnHistoryErrorNotifier errorNotifier = new TestOnHistoryErrorNotifier();
    errorNotifier.registerNotifier(historyController);
    ManagerInitException removeWordsException = new ManagerInitException("removeWordsException");
    controllerErrorListener.setExpectedException(removeWordsException);
    errorNotifier.onHistoryError(removeWordsException);
    assertEquals(controllerErrorListener.getNumberOfCalls(), 1);
  }

  @Test
  public void testSetGetEntryListFontSize() {
    assertEquals(historyController.getEntryListFontSize(), ApplicationSettings.getDefaultFontSize(), DELTA);
    float initialEntryListFontSize = historyController.getEntryListFontSize();
    float newEntryListFontSize = initialEntryListFontSize;
    if (initialEntryListFontSize + 1 < ApplicationSettings.getMaxFontSize()) {
      newEntryListFontSize = initialEntryListFontSize + 1;
    } else if (initialEntryListFontSize - 1 > ApplicationSettings.getMinFontSize()) {
      newEntryListFontSize = initialEntryListFontSize - 1;
    }
    if (newEntryListFontSize != initialEntryListFontSize) {
      MockForController_EntryListFontSizeChangeListener entryListFontSizeChangeListener
          = mock(MockForController_EntryListFontSizeChangeListener.class
          , withSettings().useConstructor(1, historyController).defaultAnswer(CALLS_REAL_METHODS));
      historyController.registerNotifier(entryListFontSizeChangeListener);
      entryListFontSizeChangeListener.setExpectedEntryListFontSize(newEntryListFontSize);
      historyController.setEntryListFontSize(newEntryListFontSize);
      assertEquals(historyController.getEntryListFontSize(), newEntryListFontSize, DELTA);
      assertEquals(entryListFontSizeChangeListener.getNumberOfCalls(), 1);
      historyController.unregisterNotifier(entryListFontSizeChangeListener);
      historyController.setEntryListFontSize(initialEntryListFontSize);
      assertEquals(historyController.getEntryListFontSize(), initialEntryListFontSize, DELTA);
    }
  }

  @Test
  public void testHasSound() {
    when(soundManager.itemHasSound(eq(ARTICLE1))).thenReturn(true);
    when(soundManager.itemHasSound(eq(ARTICLE2))).thenReturn(false);
    historyController.setWords(new ArrayList<ArticleItem>() {{
      add(ARTICLE1);
      add(ARTICLE2);
    }});
    assertTrue(historyController.hasSound(0));
    assertFalse(historyController.hasSound(1));
    assertFalse(historyController.hasSound(2));
    verify(soundManager, times(2)).itemHasSound(any(ArticleItem.class));

    BaseHistoryController controllerNoSound = new BaseHistoryController(historyManager, screenOpener
        , settingsManager, dictionaryManager, toolbarManager, null, null
        , null, "provider");
    controllerNoSound.setWords(new ArrayList<ArticleItem>() {{
      add(ARTICLE1);
      add(ARTICLE2);
    }});
    assertFalse(controllerNoSound.hasSound(0));
    assertFalse(controllerNoSound.hasSound(1));
  }

  @Test
  public void testPlaySound() {
    historyController.setWords(new ArrayList<ArticleItem>() {{
      add(ARTICLE1);
    }});
    historyController.playSound(0);
    historyController.playSound(1);
    verify(soundManager, times(1)).playSound(any(ArticleItem.class));
  }

  @Test
  public void testOnApplicationSettingsSaveListener() {
    float initialEntryListFontSize = historyController.getEntryListFontSize();
    TestOnApplicationSettingsSaveNotifier applicationSettingsSaveNotifier
        = new TestOnApplicationSettingsSaveNotifier();
    applicationSettingsSaveNotifier.registerNotifier(historyController);
    float newEntryListFontSize = historyController.getEntryListFontSize();
    if (initialEntryListFontSize + 1 < ApplicationSettings.getMaxFontSize()) {
      newEntryListFontSize = initialEntryListFontSize + 1;
    } else if (initialEntryListFontSize - 1 > ApplicationSettings.getMinFontSize()) {
      newEntryListFontSize = initialEntryListFontSize - 1;
    }
    if (newEntryListFontSize != initialEntryListFontSize) {
      when(applicationSettings.getEntryListFontSize()).thenReturn(newEntryListFontSize);
      applicationSettingsSaveNotifier.onApplicationSettingsSave(applicationSettings);
      assertEquals(historyController.getEntryListFontSize(), newEntryListFontSize, DELTA);
    }
  }

  @Test
  public void testOnStateChanged() {
    MockForController_TransitionStateChangedListener controllerTransitionStateListener
        = mock(MockForController_TransitionStateChangedListener.class
        , withSettings().useConstructor(2, historyController).defaultAnswer(CALLS_REAL_METHODS));
    historyController.registerNotifier(controllerTransitionStateListener);
    TestOnHistoryStateChangedNotifier stateChangedNotifier = new TestOnHistoryStateChangedNotifier();
    stateChangedNotifier.registerNotifier(historyController);
    controllerTransitionStateListener.setExpectedState(true);
    when(historyManager.isInTransition()).thenReturn(true);
    stateChangedNotifier.onStateChanged();
    assertEquals(controllerTransitionStateListener.getNumberOfCalls(), 1);
    controllerTransitionStateListener.setExpectedState(false);
    when(historyManager.isInTransition()).thenReturn(false);
    stateChangedNotifier.onStateChanged();
    assertEquals(controllerTransitionStateListener.getNumberOfCalls(), 2);
  }

  @Test
  public void testToolbarSelectAllActionClick() {
    when(historyManager.getWords()).thenReturn(new ArrayList<ArticleItem>() {{
      add(ARTICLE1);
      add(ARTICLE2);
    }});
    historyController.setWords(new ArrayList<ArticleItem>() {{
      add(ARTICLE1);
      add(ARTICLE2);
    }});
    assertEquals(historyController.getWords().size(), 2);
    historyController.setSelectionMode(true);
    assertEquals(historyController.getSelectedWords().size(), 0);
    TestOnSelectAllActionClickNotifier selectAllActionClickNotifier
        = new TestOnSelectAllActionClickNotifier();
    selectAllActionClickNotifier.registerNotifier(historyController);
    selectAllActionClickNotifier.selectAllActionClick();
    assertEquals(historyController.getSelectedWords().size(), 2);
    verify(toolbarManager, times(1)).showSelectionMode(2);
  }

  @Test
  public void testSelectionMode() {
    verify(toolbarManager, times(1)).showDictionaryListNoDirection();
    historyController.setWords(new ArrayList<ArticleItem>() {{
      add(ARTICLE1);
      add(ARTICLE2);
    }});
    assertFalse(historyController.isInSelectionMode());
    historyController.setSelectedWords(new ArrayList<ArticleItem>() {{
      add(ARTICLE1);
    }});
    historyController.setSelectionMode(true);
    verify(toolbarManager, times(1)).showSelectionMode(1);
    assertTrue(historyController.isInSelectionMode());
    historyController.selectItem(1);
    verify(toolbarManager, times(1)).showSelectionMode(2);
    historyController.unselectItem(0);
    verify(toolbarManager, times(2)).showSelectionMode(1);
    historyController.setSelectionMode(false);
    assertFalse(historyController.isInSelectionMode());
    verify(toolbarManager, times(2)).showDictionaryListNoDirection();
  }

  @Test
  public void testToolbarBackActionClick() {
    historyController.setSelectionMode(true);
    assertTrue(historyController.isInSelectionMode());
    TestOnBackActionClickNotifier backActionClickNotifier = new TestOnBackActionClickNotifier();
    backActionClickNotifier.registerNotifier(historyController);
    backActionClickNotifier.backActionClick();
    assertFalse(historyController.isInSelectionMode());
  }

  @Test
  public void checkDeleteAllWordsActionClick() {
    historyController.deleteAllWords();
    verify(historyManager, times(1)).removeWords(any(List.class));
  }

  @Test
  public void testFilterHistoryWordsByDictionaryId() {
    verify(toolbarManager, times(1)).showDictionaryListNoDirection();
    assertEquals(historyController.getWords().size(), 0);
    historyController.setWords(new ArrayList<ArticleItem>() {{
      add(ARTICLE1);
      add(ARTICLE2);
      add(ARTICLE3);
    }});
    when(historyManager.getWords()).thenReturn(new ArrayList<ArticleItem>() {{
      add(ARTICLE1);
      add(ARTICLE2);
      add(ARTICLE3);
    }});
    assertEquals(historyController.getWords().size(), 3);
    when(dictionaryManager.isSelectAllDictionaries()).thenReturn(false);
    when(dictionaryAndDirection.getDictionaryId()).thenReturn(DICT1_ID);
    dictionaryAndDirectionSelectNotifier.onNext(true);
    assertEquals(historyController.getWords().size(), 1);
    when(dictionaryAndDirection.getDictionaryId()).thenReturn(DICT2_ID);
    dictionaryAndDirectionSelectNotifier.onNext(true);
    assertEquals(historyController.getWords().size(), 2);
    verify(toolbarManager, times(3)).showDictionaryListNoDirection();
    when(dictionaryAndDirection.getDictionaryId()).thenReturn(DICT3_ID);
    dictionaryAndDirectionSelectNotifier.onNext(true);
    assertEquals(historyController.getWords().size(), 0);
    verify(toolbarManager, times(4)).showDictionaryListNoDirection();
    when(dictionaryManager.isSelectAllDictionaries()).thenReturn(true);
    dictionaryAndDirectionSelectNotifier.onNext(true);
    assertEquals(historyController.getWords().size(), 3);
    when(dictionaryManager.isSelectAllDictionaries()).thenReturn(false);
    when(dictionaryAndDirection.getDictionaryId()).thenReturn(DICT2_ID);
    dictionaryAndDirectionSelectNotifier.onNext(true);
    historyController.deleteAllWords();
    verify(historyManager, times(1)).removeWords
        (eq(new ArrayList<ArticleItem>() {{
          add(ARTICLE2);
          add(ARTICLE3);
        }}));
  }

  @Test
  public void testGetDictionaryIcon() {
    MockDictionaryIcon dictionaryIcon1 = mock(MockDictionaryIcon.class,
        withSettings().useConstructor(1, BITMAP1).defaultAnswer(CALLS_REAL_METHODS));
    Dictionary dictionary = new Dictionary.Builder(DICT1_ID, LocalizedString.from(""), LocalizedString.from(""), dictionaryIcon1).build();
    MockDictionaryIcon dictionaryIcon2 = mock(MockDictionaryIcon.class,
        withSettings().useConstructor(1, BITMAP2).defaultAnswer(CALLS_REAL_METHODS));
    Dictionary dictionary2 = new Dictionary.Builder(DICT2_ID, LocalizedString.from(""), LocalizedString.from(""), dictionaryIcon2).build();
    when(dictionaryManager.getDictionaries()).thenReturn(Arrays.asList(dictionary, dictionary2));

    assertEquals(dictionaryIcon1, historyController.getDictionaryIcon(DICT1_ID));
    assertEquals(dictionaryIcon2, historyController.getDictionaryIcon(DICT2_ID));

    assertNull(historyController.getDictionaryIcon(DICT3_ID));
  }

  @Test
  public void testShowHintDialog() {
    historyController.showHintManagerDialog(HintType.Favorites, fragmentManager, null);
    verify(hintManager, times(1)).showHintDialog(HintType.Favorites, fragmentManager, null);
  }

  @Test
  public void testExport() {
    MockHistoryController mockController = new MockHistoryController(historyManager, screenOpener
        , settingsManager, dictionaryManager, toolbarManager, soundManager, hintManager
        , mExportHtmlBuilderClass,  FILE_PROVIDER);

    mockController.setIntent(mIntent);

    mockController.setWords(ARTICLE_LIST);

    PowerMockito.mockStatic(Intent.class);
    PowerMockito.mockStatic(FileProvider.class);
    Uri uri = mock(Uri.class);
    File file = new File(mContext.getFilesDir(), HISTORY_MANAGER_EXPORT_FILE_PATH);
    Mockito.when(Intent.createChooser(mIntent, CHOOSER_TITLE)).thenReturn(mIntent);
    Mockito.when(FileProvider.getUriForFile(mContext,  FILE_PROVIDER, file)).thenReturn(uri);

    ExportParams exportParams = new ExportParams(
        HEADER,
        CHOOSER_TITLE,
        FOOTER,
        EXPORT_FILE_DESCRIPTION,
        PART_OF_SPEECH_COLOR);

    mockController.exportHistory(mContext, exportParams);
    verify(mIntent, times(1)).setType(MIME_TYPE_HTML);
    verify(mContext, times(1)).startActivity(mIntent);

    PowerMockito.mockStatic(Html.class);
    Spanned spanned = mock(Spanned.class);
    Mockito.when(Html.fromHtml("")).thenReturn(spanned);

    mockController.exportHistoryAsString(mContext, exportParams);

    verify(mIntent, times(1)).setType(MIME_TYPE_TEXT);
    verify(mContext, times(2)).startActivity(mIntent);
  }


  private static class TestOnApplicationSettingsSaveNotifier {

    private final ArrayList<OnApplicationSettingsSaveListener>
        mOnApplicationSettingsSaveListeners = new ArrayList<>();

    void registerNotifier(OnApplicationSettingsSaveListener notifier) {
      if (!mOnApplicationSettingsSaveListeners.contains(notifier)) {
        mOnApplicationSettingsSaveListeners.add(notifier);
      }
    }

    void onApplicationSettingsSave(ApplicationSettings applicationSettings) {
      for (final OnApplicationSettingsSaveListener listener : mOnApplicationSettingsSaveListeners) {
        listener.onApplicationSettingsSaved(applicationSettings);
      }
    }
  }

  private static class TestOnHistoryErrorNotifier {

    private final ArrayList<OnErrorListener> mOnHistoryErrorListeners = new ArrayList<>();

    void registerNotifier(OnErrorListener notifier) {
      if (!mOnHistoryErrorListeners.contains(notifier)) {
        mOnHistoryErrorListeners.add(notifier);
      }
    }

    void onHistoryError(Exception exception) {
      for (final OnErrorListener listener : mOnHistoryErrorListeners) {
        listener.onHistoryManagerError(exception);
      }
    }
  }

  private static class TestOnHistoryStateChangedNotifier {

    private final ArrayList<OnStateChangedListener> mOnHistoryStateChangedListeners = new ArrayList<>();

    void registerNotifier(OnStateChangedListener notifier) {
      if (!mOnHistoryStateChangedListeners.contains(notifier)) {
        mOnHistoryStateChangedListeners.add(notifier);
      }
    }

    void onStateChanged() {
      for (final OnStateChangedListener listener : mOnHistoryStateChangedListeners) {
        listener.onHistoryStateChanged();
      }
    }
  }

  private static class TestOnSelectAllActionClickNotifier {

    private final ArrayList<OnSelectAllActionClick> mSelectAllActionClicks = new ArrayList<>();

    void registerNotifier(OnSelectAllActionClick notifier) {
      if (!mSelectAllActionClicks.contains(notifier)) {
        mSelectAllActionClicks.add(notifier);
      }
    }

    void selectAllActionClick() {
      for (final OnSelectAllActionClick listener : mSelectAllActionClicks) {
        listener.onSelectAllActionClick();
      }
    }
  }

  private static class TestOnBackActionClickNotifier {

    private final ArrayList<OnBackActionClick> mBackActionClicks = new ArrayList<>();

    void registerNotifier(OnBackActionClick notifier) {
      if (!mBackActionClicks.contains(notifier)) {
        mBackActionClicks.add(notifier);
      }
    }

    void backActionClick() {
      for (final OnBackActionClick listener : mBackActionClicks) {
        listener.onBackActionClick();
      }
    }
  }
}