package com.paragon_software.settings_manager;

import com.paragon_software.screen_opener_api.ScreenOpenerAPI;
import com.paragon_software.screen_opener_api.ScreenType;
import com.paragon_software.settings_manager.exceptions.LocalResourceUnavailableException;
import com.paragon_software.settings_manager.exceptions.ManagerInitException;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.invocation.InvocationOnMock;
import org.mockito.junit.MockitoJUnitRunner;
import org.mockito.stubbing.Answer;

import static org.junit.Assert.assertEquals;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.BDDMockito.given;
import static org.mockito.Mockito.CALLS_REAL_METHODS;
import static org.mockito.Mockito.doAnswer;
import static org.mockito.Mockito.doThrow;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.withSettings;

@RunWith( MockitoJUnitRunner.class )
public class SettingsControllerTest
{
  public static final float DELTA = 0.01f;

  @Mock
  private SettingsManagerAPI settingsManager;

  @Mock
  private ScreenOpenerAPI screenOpener;

  private BaseSettingsController settingsController;

  private ApplicationSettings applicationSettings;
  private MockForController_AppSettingsChangeListener settingsChangeListener;

  @Before
  public void setUp() throws ManagerInitException, LocalResourceUnavailableException
  {
    applicationSettings = new ApplicationSettings();
    given(settingsManager.loadApplicationSettings()).willReturn(applicationSettings);
    doAnswer(new Answer()
    {
      @Override
      public Object answer( InvocationOnMock invocation )
      {
        applicationSettings = invocation.getArgument(0);
        settingsController.onApplicationSettingsSaved(applicationSettings);
        return null;
      }
    }).when(settingsManager).saveApplicationSettings(any(ApplicationSettings.class));
    settingsController = new BaseSettingsController(settingsManager, screenOpener);

    settingsChangeListener = Mockito.mock(MockForController_AppSettingsChangeListener.class,
            withSettings().useConstructor(settingsController, 1).defaultAnswer(CALLS_REAL_METHODS));
  }

  @Test
  public void testChangeShowKeyboardForSearch() throws ManagerInitException, LocalResourceUnavailableException
  {
    boolean initShowKeyboardForSearch = applicationSettings.isShowKeyboardForSearchEnabled();
    assertEquals(settingsController.getApplicationSettings().isShowKeyboardForSearchEnabled(), initShowKeyboardForSearch);
    ApplicationSettings newSettings = settingsController.getApplicationSettings();
    newSettings.setShowKeyboardForSearchEnabled(!initShowKeyboardForSearch);
    settingsController.saveNewApplicationSettings(newSettings);
    verify(settingsManager, times(1)).saveApplicationSettings(any(ApplicationSettings.class));
    assertEquals(settingsController.getApplicationSettings().isShowKeyboardForSearchEnabled(), !initShowKeyboardForSearch);
  }

  @Test
  public void testOnApplicationSettingsChangeListenerShowKeyboardForSearch()
  {
    boolean initShowKeyboardForSearch = applicationSettings.isShowKeyboardForSearchEnabled();
    assertEquals(settingsController.getApplicationSettings().isShowKeyboardForSearchEnabled(), initShowKeyboardForSearch);
    settingsController.registerNotifier(settingsChangeListener);
    settingsChangeListener .setExpectedShowKeyboardForSearch(!initShowKeyboardForSearch);
    ApplicationSettings newSettings = settingsController.getApplicationSettings();
    newSettings.setShowKeyboardForSearchEnabled(!initShowKeyboardForSearch);
    settingsController.saveNewApplicationSettings(newSettings);
    assertEquals(settingsChangeListener .getNumberOfCalls(), 1);
    newSettings = settingsController.getApplicationSettings();
    newSettings.setShowKeyboardForSearchEnabled(!initShowKeyboardForSearch);
    settingsController.saveNewApplicationSettings(newSettings);
    assertEquals(settingsChangeListener .getNumberOfCalls(), 1);
    settingsController.unregisterNotifier(settingsChangeListener );
    newSettings = settingsController.getApplicationSettings();
    newSettings.setShowKeyboardForSearchEnabled(initShowKeyboardForSearch);
    settingsController.saveNewApplicationSettings(newSettings);
  }

  @Test
  public void testChangeUsePinchToZoom() throws ManagerInitException, LocalResourceUnavailableException
  {
    boolean initUsePinchToZoom = applicationSettings.isPinchToZoomEnabled();
    assertEquals(settingsController.getApplicationSettings().isPinchToZoomEnabled(), initUsePinchToZoom);
    ApplicationSettings newSettings = settingsController.getApplicationSettings();
    newSettings.setPinchToZoomEnabled(!initUsePinchToZoom);
    settingsController.saveNewApplicationSettings(newSettings);
    verify(settingsManager, times(1)).saveApplicationSettings(any(ApplicationSettings.class));
    assertEquals(settingsController.getApplicationSettings().isPinchToZoomEnabled(), !initUsePinchToZoom);
  }

  @Test
  public void testOnApplicationSettingsChangeListenerUsePinchToZoom()
  {
    boolean initUsePinchToZoom = applicationSettings.isPinchToZoomEnabled();
    assertEquals(settingsController.getApplicationSettings().isPinchToZoomEnabled(), initUsePinchToZoom);
    settingsController.registerNotifier(settingsChangeListener );
    settingsChangeListener .setExpectedUsePinchToZoom(!initUsePinchToZoom);
    ApplicationSettings newSettings = settingsController.getApplicationSettings();
    newSettings.setPinchToZoomEnabled(!initUsePinchToZoom);
    settingsController.saveNewApplicationSettings(newSettings);
    assertEquals(settingsChangeListener .getNumberOfCalls(), 1);
    newSettings = settingsController.getApplicationSettings();
    newSettings.setPinchToZoomEnabled(!initUsePinchToZoom);
    settingsController.saveNewApplicationSettings(newSettings);
    assertEquals(settingsChangeListener .getNumberOfCalls(), 1);
    settingsController.unregisterNotifier(settingsChangeListener );
    newSettings = settingsController.getApplicationSettings();
    newSettings.setPinchToZoomEnabled(initUsePinchToZoom);
    settingsController.saveNewApplicationSettings(newSettings);
  }

  @Test
  public void testChangeEntryListFontSize() throws ManagerInitException, LocalResourceUnavailableException
  {
    float initEntryListFontSize = applicationSettings.getEntryListFontSize();
    assertEquals(settingsController.getApplicationSettings().getEntryListFontSize(), initEntryListFontSize, DELTA);
    ApplicationSettings newSettings = settingsController.getApplicationSettings();
    newSettings.setEntryListFontSize(initEntryListFontSize + 1);
    settingsController.saveNewApplicationSettings(newSettings);
    verify(settingsManager, times(1)).saveApplicationSettings(any(ApplicationSettings.class));
    assertEquals(settingsController.getApplicationSettings().getEntryListFontSize(), initEntryListFontSize + 1, DELTA);
  }

  @Test
  public void testOnApplicationSettingsChangeListenerEntryListFontSize()
  {
    float initEntryListFontSize = applicationSettings.getEntryListFontSize();
    assertEquals(settingsController.getApplicationSettings().getEntryListFontSize(), initEntryListFontSize, DELTA);
    settingsController.registerNotifier(settingsChangeListener );
    settingsChangeListener .setExpectedEntryListFontSize(initEntryListFontSize + 1);
    ApplicationSettings newSettings = settingsController.getApplicationSettings();
    newSettings.setEntryListFontSize(initEntryListFontSize + 1);
    settingsController.saveNewApplicationSettings(newSettings);
    assertEquals(settingsChangeListener .getNumberOfCalls(), 1);
    newSettings = settingsController.getApplicationSettings();
    newSettings.setEntryListFontSize(initEntryListFontSize + 1);
    settingsController.saveNewApplicationSettings(newSettings);
    assertEquals(settingsChangeListener .getNumberOfCalls(), 1);
    settingsController.unregisterNotifier(settingsChangeListener );
    newSettings = settingsController.getApplicationSettings();
    newSettings.setEntryListFontSize(initEntryListFontSize);
    settingsController.saveNewApplicationSettings(newSettings);
  }

  @Test
  public void testChangeSendStatistics() throws ManagerInitException, LocalResourceUnavailableException
  {
    boolean initSendStatistics = applicationSettings.isStatisticsEnabled();
    assertEquals(settingsController.getApplicationSettings().isStatisticsEnabled(), initSendStatistics);
    ApplicationSettings newSettings = settingsController.getApplicationSettings();
    newSettings.setStatisticsEnabled(!initSendStatistics);
    settingsController.saveNewApplicationSettings(newSettings);
    verify(settingsManager, times(1)).saveApplicationSettings(any(ApplicationSettings.class));
    assertEquals(settingsController.getApplicationSettings().isStatisticsEnabled(), !initSendStatistics);
  }

  @Test
  public void testOnApplicationSettingsChangeListenerSendStatistics()
  {
    boolean initSendStatistics = applicationSettings.isStatisticsEnabled();
    assertEquals(settingsController.getApplicationSettings().isStatisticsEnabled(), initSendStatistics);
    settingsController.registerNotifier(settingsChangeListener);
    settingsChangeListener.setExpectedSendStatistics(!initSendStatistics);
    ApplicationSettings newSettings = settingsController.getApplicationSettings();
    newSettings.setStatisticsEnabled(!initSendStatistics);
    settingsController.saveNewApplicationSettings(newSettings);
    assertEquals(settingsChangeListener.getNumberOfCalls(), 1);
    newSettings = settingsController.getApplicationSettings();
    newSettings.setStatisticsEnabled(!initSendStatistics);
    settingsController.saveNewApplicationSettings(newSettings);
    assertEquals(settingsChangeListener.getNumberOfCalls(), 1);
    settingsController.unregisterNotifier(settingsChangeListener);
    newSettings = settingsController.getApplicationSettings();
    newSettings.setStatisticsEnabled(initSendStatistics);
    settingsController.saveNewApplicationSettings(newSettings);
  }

  @Test
  public void testChangeReceiveNews() throws ManagerInitException, LocalResourceUnavailableException
  {
    boolean initReceiveNews = applicationSettings.isNewsEnabled();
    assertEquals(settingsController.getApplicationSettings().isNewsEnabled(), initReceiveNews);
    ApplicationSettings newSettings = settingsController.getApplicationSettings();
    newSettings.setNewsEnabled(!initReceiveNews);
    settingsController.saveNewApplicationSettings(newSettings);
    verify(settingsManager, times(1)).saveApplicationSettings(any(ApplicationSettings.class));
    assertEquals(settingsController.getApplicationSettings().isNewsEnabled(), !initReceiveNews);
  }

  @Test
  public void testOnApplicationSettingsChangeListenerReceiveNews()
  {
    boolean initReceiveNews = applicationSettings.isNewsEnabled();
    assertEquals(settingsController.getApplicationSettings().isNewsEnabled(), initReceiveNews);
    settingsController.registerNotifier(settingsChangeListener );
    settingsChangeListener .setExpectedReceiveNews(!initReceiveNews);
    ApplicationSettings newSettings = settingsController.getApplicationSettings();
    newSettings.setNewsEnabled(!initReceiveNews);
    settingsController.saveNewApplicationSettings(newSettings);
    assertEquals(settingsChangeListener .getNumberOfCalls(), 1);
    newSettings = settingsController.getApplicationSettings();
    newSettings.setNewsEnabled(!initReceiveNews);
    settingsController.saveNewApplicationSettings(newSettings);
    assertEquals(settingsChangeListener .getNumberOfCalls(), 1);
    settingsController.unregisterNotifier(settingsChangeListener );
    newSettings = settingsController.getApplicationSettings();
    newSettings.setNewsEnabled(initReceiveNews);
    settingsController.saveNewApplicationSettings(newSettings);
  }

  @Test
  public void testOnErrorListener() throws ManagerInitException, LocalResourceUnavailableException
  {
    float oldEntryListFontSize = settingsController.getApplicationSettings().getEntryListFontSize();
    MockForController_ErrorListener controllerErrorListener
            = Mockito.mock(MockForController_ErrorListener.class
            , withSettings().useConstructor(2).defaultAnswer(CALLS_REAL_METHODS));
    settingsController.registerNotifier(controllerErrorListener);
    Exception firstSaveException = new ManagerInitException("firstSaveException");
    controllerErrorListener.setExpectedException(firstSaveException);
    doThrow(firstSaveException).when(settingsManager).saveApplicationSettings(any(ApplicationSettings.class));
    ApplicationSettings newSettings = settingsController.getApplicationSettings();
    newSettings.setEntryListFontSize(settingsController.getApplicationSettings().getEntryListFontSize() + 1);
    settingsController.saveNewApplicationSettings(newSettings);
    assertEquals(controllerErrorListener.getNumberOfCalls(), 1);
    assertEquals(oldEntryListFontSize, settingsController.getApplicationSettings().getEntryListFontSize(), DELTA);
    Exception secondSaveException = new LocalResourceUnavailableException("secondSaveException");
    controllerErrorListener.setExpectedException(secondSaveException);
    doThrow(secondSaveException).when(settingsManager).saveApplicationSettings(any(ApplicationSettings.class));
    newSettings = settingsController.getApplicationSettings();
    newSettings.setEntryListFontSize(settingsController.getApplicationSettings().getEntryListFontSize() + 1);
    settingsController.saveNewApplicationSettings(newSettings);
    assertEquals(controllerErrorListener.getNumberOfCalls(), 2);
    assertEquals(oldEntryListFontSize, settingsController.getApplicationSettings().getEntryListFontSize(), DELTA);
  }

  @Test
  public void testScreenOpener()
  {
    settingsController.openScreen(ScreenType.Settings);
    settingsController.openScreen(ScreenType.SettingsGeneral);
    settingsController.openScreen(ScreenType.SettingsNotifications);
    settingsController.openScreen(ScreenType.SettingsMyView);
    verify(screenOpener, times(1)).openScreen(ScreenType.Settings);
    verify(screenOpener, times(1)).openScreen(ScreenType.SettingsGeneral);
    verify(screenOpener, times(1)).openScreen(ScreenType.SettingsNotifications);
    verify(screenOpener, times(1)).openScreen(ScreenType.SettingsMyView);
  }
}
