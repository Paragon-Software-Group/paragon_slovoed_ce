package com.paragon_software.favorites_manager;

import android.app.Activity;
import android.content.Context;

import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.hint_manager.HintManagerAPI;
import com.paragon_software.hint_manager.HintType;
import com.paragon_software.native_engine.EngineSerializerAPI;
import com.paragon_software.native_engine.SearchEngineAPI;
import com.paragon_software.screen_opener_api.ScreenOpenerAPI;
import com.paragon_software.settings_manager.SettingsManagerAPI;
import com.paragon_software.settings_manager.exceptions.ManagerInitException;
import com.paragon_software.settings_manager.exceptions.WrongTypeException;
import com.paragon_software.sound_manager.SoundManagerAPI;
import com.paragon_software.toolbar_manager.ToolbarManager;
import com.paragon_software.utils_slovoed.directory.MockDirectory;
import com.paragon_software.utils_slovoed.directory.MockDirectoryAbstract;

import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.junit.MockitoJUnit;
import org.mockito.junit.MockitoRule;

import java.io.Serializable;

import androidx.fragment.app.DialogFragment;
import androidx.fragment.app.FragmentManager;

import static com.paragon_software.favorites_manager.BaseFavoritesManager.FAVORITES_SAVE_KEY;
import static com.paragon_software.favorites_manager.mock.StaticConsts.ARTICLE1;
import static junit.framework.TestCase.assertNotSame;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertSame;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.CALLS_REAL_METHODS;
import static org.mockito.Mockito.doAnswer;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.withSettings;

public class AddDirectoryControllerTest {

  @Rule
  public MockitoRule rule = MockitoJUnit.rule();

  @Mock
  private ScreenOpenerAPI screenOpener;
  @Mock
  private EngineSerializerAPI engineSerializer;
  @Mock
  private SearchEngineAPI engineSearcher;
  @Mock
  private SettingsManagerAPI settingsManager;
  @Mock
  private ToolbarManager toolbarManager;
  @Mock
  private DictionaryManagerAPI dictionaryManager;
  @Mock
  private SoundManagerAPI soundManager;
  @Mock
  private HintManagerAPI mHintManager;
  @Mock
  private FragmentManager mFragmentManager;

  private MockFavoritesManager mockFavoritesManager;

  private FavoritesControllerAPI mockController;

  @Before
  public void setUp() throws ManagerInitException, WrongTypeException {
    doAnswer(invocation -> {
      MockDirectoryAbstract mockDirectory = mock(MockDirectoryAbstract.class, withSettings()
          .useConstructor(null, FAVORITES_SAVE_KEY).defaultAnswer(CALLS_REAL_METHODS));
      return new PersistentDirectory(mockDirectory);
    }).when(settingsManager).load(any(String.class), any(PersistentDirectory.class));
    doAnswer(invocation -> invocation.getArgument(1)).when(settingsManager)
        .load(any(String.class), any(Serializable[].class));

    mockFavoritesManager = new MockFavoritesManager(screenOpener, engineSerializer, engineSearcher
        , settingsManager, dictionaryManager, toolbarManager, soundManager, mHintManager
        , null, null, null, null
        , "provider", 1000);

    mockController = Mockito.spy(FavoritesControllerAPI.class);
    mockFavoritesManager.setController(mockController);
  }

  @Test
  public void testGetAddInDirectoryController() {
    FavoritesControllerAddInDirectoryAPI controller = mockFavoritesManager.getAddInDirectoryController("test");
    FavoritesControllerAddInDirectoryAPI controller2 = mockFavoritesManager.getAddInDirectoryController(FavoritesControllerType.DEFAULT_CONTROLLER);
    assertNotNull(controller);
    assertNotNull(controller2);
    assertNotSame(controller, controller2);
    assertSame(controller, mockFavoritesManager.getAddInDirectoryController("test"));
  }

  @Test
  public void testAddInDirectoryController() {
    FavoritesControllerAddInDirectoryAPI addInDirectoryController = mockFavoritesManager.getAddInDirectoryController("test");
    verify(mockController, times(1)).activate();

    addInDirectoryController.getEntryListFontSize();
    verify(mockController, times(1)).getEntryListFontSize();

    OnControllerTransitionStateChangedListener notifier = Mockito.spy(OnControllerTransitionStateChangedListener.class);
    addInDirectoryController.registerNotifier(notifier);
    verify(mockController, times(1)).registerNotifier(notifier);

    addInDirectoryController.unregisterNotifier(notifier);
    verify(mockController, times(1)).unregisterNotifier(notifier);

    addInDirectoryController.isInTransition();
    verify(mockController, times(1)).isInTransition();

    addInDirectoryController.getCurrentDirectoryObservable();
    verify(mockController, times(1)).getCurrentDirectoryObservable();

    addInDirectoryController.getCurrentDirectory();
    verify(mockController, times(1)).getCurrentDirectory();

    MockDirectory mockDirectory = new MockDirectory(null, "test");
    addInDirectoryController.goToDirectory(mockDirectory);
    verify(mockController, times(1)).goToDirectory(mockDirectory);

    addInDirectoryController.goToRootDirectory();
    verify(mockController, times(1)).goToRootDirectory();

    addInDirectoryController.hasChildDirectory("test");
    verify(mockController, times(1)).hasChildDirectory("test");

    addInDirectoryController.addDirectory("test");
    verify(mockController, times(1)).addDirectory("test");

    addInDirectoryController.setArticleItemToSave(ARTICLE1);
    assertTrue(addInDirectoryController.save());
    verify(mockController, times(2)).getCurrentDirectory();
    assertTrue(addInDirectoryController.hasWord());
    verify(mockController, times(3)).getCurrentDirectory();

    addInDirectoryController.showHintManagerDialog(HintType.Favorites, mFragmentManager, null);
    verify(mockController, times(1)).showHintManagerDialog(HintType.Favorites, mFragmentManager, null);

    addInDirectoryController.deactivate();
    verify(mockController, times(1)).deactivate();
  }
}
