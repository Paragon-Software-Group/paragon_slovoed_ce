package com.paragon_software.settings_manager;

import android.content.Context;

import androidx.test.core.app.ApplicationProvider;

import com.paragon_software.screen_opener_api.ScreenOpenerAPI;
import com.paragon_software.screen_opener_api.ScreenType;
import com.paragon_software.settings_manager.exceptions.AlreadyInProgressException;
import com.paragon_software.settings_manager.exceptions.LocalResourceUnavailableException;
import com.paragon_software.settings_manager.exceptions.ManagerInitException;
import com.paragon_software.settings_manager.exceptions.NoSuchStorageException;
import com.paragon_software.settings_manager.exceptions.StorageTransferException;
import com.paragon_software.settings_manager.exceptions.UnsavedDataException;
import com.paragon_software.settings_manager.exceptions.WrongTypeException;
import com.paragon_software.settings_manager.mock.MockForManager_AppSettingsSaveListener;
import com.paragon_software.settings_manager.mock.MockForManager_SettingsSaveListener;
import com.paragon_software.settings_manager.mock.MockStorage;
import com.paragon_software.settings_manager.mock.MockStorageWithLoad;

import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.junit.MockitoJUnit;
import org.mockito.junit.MockitoRule;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.RuntimeEnvironment;

import java.io.Serializable;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.List;
import java.util.concurrent.atomic.AtomicInteger;

import static junit.framework.TestCase.assertSame;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.Mockito.CALLS_REAL_METHODS;
import static org.mockito.Mockito.doNothing;
import static org.mockito.Mockito.doReturn;
import static org.mockito.Mockito.doThrow;
import static org.mockito.Mockito.spy;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;
import static org.mockito.Mockito.withSettings;

/**
 * Created by belyshov on 07/02/2018.
 */
@RunWith( RobolectricTestRunner.class )
public class SettingsManagerTest implements Serializable
{
  @Rule
  public MockitoRule rule = MockitoJUnit.rule();

  @Mock
  private ISettingsStorage  mockStorage;
  @Mock
  private ISettingsStorage  mockStorage2;
  @Mock
  private SettingsContainer mockContainer;
  private MockContainer container;

  @Mock
  private ScreenOpenerAPI mScreenOpener;

  @Mock
  private Context mContext;

  @Before
  public void initMocks()
  {
    container = Mockito.mock(MockContainer.class, withSettings()
            .useConstructor(null, null).defaultAnswer(CALLS_REAL_METHODS));
  }

  @Test( expected = ManagerInitException.class )
  public void constructorWithSameStorages() throws Exception
  {
    List< String > names = Arrays.asList("first", "second", "first");
    ISettingsStorage[] settingsStorages = getStorageArray("first", "second", "first");

    BaseSettingsManager settingsManager = new BaseSettingsManager(settingsStorages, null);
    Collection< String > settingsList = settingsManager.storageList();
    assertTrue(settingsList.containsAll(names) && names.containsAll(settingsList));
  }

  private ISettingsStorage[] getStorageArray( String... names )
  {
    ISettingsStorage[] settingsStorages = new ISettingsStorage[names.length];
    for ( int i = 0 ; i < names.length ; ++i )
    {
      settingsStorages[i] = Mockito.mock(MockStorage.class, withSettings()
                      .useConstructor(names[i]).defaultAnswer(CALLS_REAL_METHODS));
    }
    return settingsStorages;
  }

  @Test
  public void getStorageList() throws Exception
  {
    List< String > names = Arrays.asList("first", "second", "third");
    ISettingsStorage[] settingsStorages = getStorageArray("first", "second", "third");

    BaseSettingsManager settingsManager = new BaseSettingsManager(settingsStorages, null);
    Collection< String > settingsList = settingsManager.storageList();
    assertTrue(settingsList.containsAll(names) && names.containsAll(settingsList));
  }

  //sendToStorage negative scenario
  @Test( expected = NoSuchStorageException.class )
  public void sendToUnexistingStorage() throws Exception
  {
    when(mockStorage.getName()).thenReturn("storage");
    BaseSettingsManager settingsManager = new BaseSettingsManager(new ISettingsStorage[]{ mockStorage }, null);
    settingsManager.sendToStorage("ololol");
  }

  @Test( expected = ManagerInitException.class )
  public void sendToStorageManagerNotInitialized() throws Exception
  {
    when(mockStorage.getName()).thenReturn("storage");
    BaseSettingsManager settingsManager = new BaseSettingsManager(new ISettingsStorage[]{ mockStorage }, null);
    settingsManager.sendToStorage("storage");
  }

  @Test( expected = AlreadyInProgressException.class )
  public void sendToStorageWhenManagerBusy() throws Exception
  {
    when(mockStorage.getName()).thenReturn("storage");
    BaseSettingsManager settingsManager = new BaseSettingsManager(new ISettingsStorage[]{ mockStorage }, mockContainer);
    settingsManager.setFakeStorageOperationInProgress(true);
    settingsManager.sendToStorage("storage");
  }

  @Test( expected = LocalResourceUnavailableException.class )
  public void sendToStorageWhenContainerThrowsLocalResourceUnavailable() throws Exception
  {
    when(mockStorage.getName()).thenReturn("storage");
    doThrow(new LocalResourceUnavailableException("")).when(mockContainer).confirmBackupFinished();
    BaseSettingsManager settingsManager = new BaseSettingsManager(new ISettingsStorage[]{ mockStorage }, mockContainer);
    settingsManager.sendToStorage("storage");
  }

  @Test( expected = StorageTransferException.class )
  public void sendToStorageWhenStorageThrowsStorageTransferException() throws Exception
  {
    when(mockStorage.getName()).thenReturn("storage");
    doThrow(new StorageTransferException("")).when(mockStorage).save(any(byte[].class));
    doReturn(new byte[0]).when(mockContainer).serializePersistentData();
    BaseSettingsManager settingsManager = new BaseSettingsManager(new ISettingsStorage[]{ mockStorage }, mockContainer);
    settingsManager.sendToStorage("storage");
  }

  @Test
  public void sendToStorageSuccess() throws Exception
  {
    when(mockStorage.getName()).thenReturn("storage");
    doNothing().when(mockContainer).confirmBackupFinished();
    doNothing().when(mockStorage).save(any(byte[].class));
    doReturn(new byte[0]).when(mockContainer).serializePersistentData();
    BaseSettingsManager settingsManager = new BaseSettingsManager(new ISettingsStorage[]{ mockStorage }, mockContainer);
    settingsManager.sendToStorage("storage");
  }

  @Test( expected = AlreadyInProgressException.class )
  public void sendToStoragesWhenManagerBusy() throws Exception
  {
    ISettingsStorage[] settingsStorages = getStorageArray("first", "second", "third");

    BaseSettingsManager settingsManager = new BaseSettingsManager(settingsStorages, null);
    settingsManager.setFakeStorageOperationInProgress(true);
    settingsManager.sendToStorages();
  }

  @Test( expected = ManagerInitException.class )
  public void sendToStoragesManagerNotInitialized() throws Exception
  {
    ISettingsStorage[] settingsStorages = getStorageArray("first", "second", "third");
    BaseSettingsManager settingsManager = new BaseSettingsManager(settingsStorages, null);
    settingsManager.sendToStorages();
  }

  //sendToStorages negative scenario

  @Test( expected = StorageTransferException.class )
  public void sendToStoragesWhenStorageThrowsStorageTransferException() throws Exception
  {
    when(mockStorage.getName()).thenReturn("storage");
    doThrow(new StorageTransferException("")).when(mockStorage).save(any(byte[].class));
    doThrow(new StorageTransferException("")).when(mockContainer).serializePersistentData();
    BaseSettingsManager settingsManager = new BaseSettingsManager(new ISettingsStorage[]{ mockStorage }, mockContainer);
    settingsManager.sendToStorages();
  }

  //save negative scenarios
  @Test( expected = ManagerInitException.class )
  public void saveNoBackupNoManager() throws Exception
  {
    when(mockStorage.getName()).thenReturn("storage");
    BaseSettingsManager settingsManager = new BaseSettingsManager(new ISettingsStorage[]{ mockStorage }, null);
    DataIntStringByteList data = DataIntStringByteList.getStubData();
    settingsManager.save("data", data, false);
  }

  @Test( expected = LocalResourceUnavailableException.class )
  public void saveNoBackupLocalResourceUnavailable() throws Exception
  {
    when(mockStorage.getName()).thenReturn("storage");
    doThrow(new LocalResourceUnavailableException("")).when(mockContainer).putStringData(anyString(), anyString(),
                                                                                         any(SettingsContainer.DATA_TYPE.class));
    BaseSettingsManager settingsManager = new BaseSettingsManager(new ISettingsStorage[]{ mockStorage }, mockContainer);
    DataIntStringByteList data = DataIntStringByteList.getStubData();
    settingsManager.save("data", data, false);
  }

  @Test
  public void sendToStoragesSuccess() throws Exception
  {
    when(mockStorage.getName()).thenReturn("storage");
    doNothing().when(mockContainer).confirmBackupFinished();
    doNothing().when(mockStorage).save(any(byte[].class));
    doReturn(new byte[0]).when(mockContainer).serializePersistentData();
    BaseSettingsManager settingsManager = new BaseSettingsManager(new ISettingsStorage[]{ mockStorage }, mockContainer);
    settingsManager.sendToStorages();
  }

  @Test
  public void sendToStoragesOneSaveFails() throws Exception
  {
    when(mockStorage.getName()).thenReturn("storage");
    doNothing().when(mockStorage).save(any(byte[].class));

    when(mockStorage2.getName()).thenReturn("storage2");
    doThrow(new StorageTransferException("")).when(mockStorage2).save(any(byte[].class));

    doNothing().when(mockContainer).confirmBackupFinished();
    doReturn(new byte[0]).when(mockContainer).serializePersistentData();
    BaseSettingsManager settingsManager =
        new BaseSettingsManager(new ISettingsStorage[]{ mockStorage, mockStorage2 }, mockContainer);
    settingsManager.sendToStorages();
  }

  //load negative scenarios
  @Test( expected = ManagerInitException.class )
  public void loadNoManager() throws Exception
  {
    when(mockStorage.getName()).thenReturn("storage");
    BaseSettingsManager settingsManager = new BaseSettingsManager(new ISettingsStorage[]{ mockStorage }, null);
    DataIntStringByteList data = DataIntStringByteList.getStubData();
    settingsManager.load("data", "value");
  }

  @Test
  public void saveNoBackup() throws Exception
  {
    when(mockStorage.getName()).thenReturn("storage");
    BaseSettingsManager settingsManager = new BaseSettingsManager(new ISettingsStorage[]{ mockStorage }, container);
    DataIntStringByteList data = DataIntStringByteList.getStubData();
    settingsManager.save("data", data, false);
    DataIntStringByteList def = new DataIntStringByteList();
    DataIntStringByteList ret = settingsManager.load("data", def);
    assertTrue(data.equals(ret));

  }

  @Test( expected = WrongTypeException.class )
  public void loadWrongType() throws Exception
  {
    when(mockStorage.getName()).thenReturn("storage");
    BaseSettingsManager settingsManager = new BaseSettingsManager(new ISettingsStorage[]{ mockStorage }, container);
    DataIntStringByteList data = DataIntStringByteList.getStubData();
    settingsManager.save("data", data, false);
    Integer def = new Integer(8);
    Integer ret = settingsManager.load("data", def);
  }

  @Test
  public void loadDefaultValue() throws Exception
  {
    when(mockStorage.getName()).thenReturn("storage");
    BaseSettingsManager settingsManager = new BaseSettingsManager(new ISettingsStorage[]{ mockStorage }, container);
    DataIntStringByteList data = DataIntStringByteList.getStubData();
    settingsManager.save("data", data, false);
    DataIntStringByteList def = new DataIntStringByteList();
    DataIntStringByteList ret = settingsManager.load("newdata", def);
    assertTrue(def.equals(ret));
  }

  //receiveFromStorage negative scenarios
  @Test( expected = NoSuchStorageException.class )
  public void receiveFromStorageNoSuchStorage() throws Exception
  {
    when(mockStorage.getName()).thenReturn("storage");
    BaseSettingsManager settingsManager = new BaseSettingsManager(new ISettingsStorage[]{ mockStorage }, container);
    settingsManager.receiveFromStorage("SStor", null, false);
  }

  @Test( expected = ManagerInitException.class )
  public void receiveFromStorageNoManager() throws Exception
  {
    when(mockStorage.getName()).thenReturn("storage");
    BaseSettingsManager settingsManager = new BaseSettingsManager(new ISettingsStorage[]{ mockStorage }, null);
    settingsManager.receiveFromStorage("storage", null, false);
  }

  @Test( expected = AlreadyInProgressException.class )
  public void receiveFromStorageBusy() throws Exception
  {
    when(mockStorage.getName()).thenReturn("storage");
    MockContainer spyContainer = spy(container);
    doReturn(false).when(spyContainer).isDirty();

    BaseSettingsManager settingsManager = new BaseSettingsManager(new ISettingsStorage[]{ mockStorage }, spyContainer);

    settingsManager.setFakeStorageOperationInProgress(true);
    settingsManager.receiveFromStorage("storage", null, false);
  }

  @Test( expected = UnsavedDataException.class )
  public void receiveFromStorageUnsavedData() throws Exception
  {
    when(mockStorage.getName()).thenReturn("storage");
    MockContainer spyContainer = spy(container);
    doReturn(true).when(spyContainer).isDirty();

    BaseSettingsManager settingsManager = new BaseSettingsManager(new ISettingsStorage[]{ mockStorage }, spyContainer);

    settingsManager.setFakeStorageOperationInProgress(false);
    settingsManager.receiveFromStorage("storage", null, false);
  }

  //receiveFromStorages negative scenarios
  @Test( expected = ManagerInitException.class )
  public void receiveFromStoragesNoManager() throws Exception
  {
    when(mockStorage.getName()).thenReturn("storage");
    BaseSettingsManager settingsManager = new BaseSettingsManager(new ISettingsStorage[]{ mockStorage }, null);
    settingsManager.receiveFromStorages(null, false);
  }

  @Test( expected = AlreadyInProgressException.class )
  public void receiveFromStoragesBusy() throws Exception
  {
    when(mockStorage.getName()).thenReturn("storage");
    MockContainer spyContainer = spy(container);
    doReturn(false).when(spyContainer).isDirty();

    BaseSettingsManager settingsManager = new BaseSettingsManager(new ISettingsStorage[]{ mockStorage }, spyContainer);

    settingsManager.setFakeStorageOperationInProgress(true);
    settingsManager.receiveFromStorages(null, false);
  }

  @Test( expected = UnsavedDataException.class )
  public void receiveFromStoragesUnsavedData() throws Exception
  {
    when(mockStorage.getName()).thenReturn("storage");
    MockContainer spyContainer = spy(container);
    doReturn(true).when(spyContainer).isDirty();

    BaseSettingsManager settingsManager = new BaseSettingsManager(new ISettingsStorage[]{ mockStorage }, spyContainer);

    settingsManager.setFakeStorageOperationInProgress(false);
    settingsManager.receiveFromStorages(null, false);
  }

  @Test
  public void checkInitAndDirty() throws Exception
  {
    when(mockStorage.getName()).thenReturn("storage");
    MockContainer spyContainer = spy(container);
    doReturn(true).when(spyContainer).isDirty();

    BaseSettingsManager settingsManager = new BaseSettingsManager(new ISettingsStorage[]{ mockStorage }, spyContainer);
    settingsManager.checkInitAndDirty(true);
  }

  @Test
  public void initDataSet() throws Exception
  {
    when(mockStorage.getName()).thenReturn("storage");
    MockContainer spyContainer = spy(container);
    doReturn(true).when(spyContainer).isDirty();

    BaseSettingsManager settingsManager = new BaseSettingsManager(new ISettingsStorage[]{ mockStorage }, spyContainer);
    Context context = ApplicationProvider.getApplicationContext();
    settingsManager.initDataSet(context);
    try
    {
      settingsManager.checkInitAndDirty(false);
    }
    catch ( ManagerInitException e )
    {
      assertTrue(e.getCause() instanceof LocalResourceUnavailableException);
    }
  }

  @Test
  public void isRestoredNotInitialized() throws Exception
  {
    when(mockStorage.getName()).thenReturn("storage");
    BaseSettingsManager settingsManager = new BaseSettingsManager(new ISettingsStorage[]{ mockStorage }, null);
    assertFalse(settingsManager.isRestored());
  }

  @Test
  public void isRestoredTrueInitialized() throws Exception
  {
    when(mockStorage.getName()).thenReturn("storage");
    MockContainer spyContainer = spy(container);
    doReturn(0L).when(spyContainer).version();
    BaseSettingsManager settingsManager = new BaseSettingsManager(new ISettingsStorage[]{ mockStorage }, spyContainer);
    assertFalse(settingsManager.isRestored());
  }

  @Test
  public void isRestoredFalseInitialized() throws Exception
  {
    when(mockStorage.getName()).thenReturn("storage");
    MockContainer spyContainer = spy(container);
    doReturn(10L).when(spyContainer).version();
    BaseSettingsManager settingsManager = new BaseSettingsManager(new ISettingsStorage[]{ mockStorage }, spyContainer);
    assertTrue(settingsManager.isRestored());
  }

  @Test
  public void updateDataSetEmptyData() throws Exception
  {
    when(mockStorage.getName()).thenReturn("storage");
    MockContainer spyContainer = spy(container);
    SettingsContainer.BackupContainerView containerView = new SettingsContainer.BackupContainerView(null, 0L);
    BaseSettingsManager settingsManager = new BaseSettingsManager(new ISettingsStorage[]{ mockStorage }, spyContainer);
    assertFalse(settingsManager.updateDataSet(containerView, true));
  }

  @Test( expected = ManagerInitException.class )
  public void updateDataSetNotInited() throws Exception
  {
    when(mockStorage.getName()).thenReturn("storage");
    SettingsContainer.BackupContainerView containerView = new SettingsContainer.BackupContainerView(null, 10L);
    BaseSettingsManager settingsManager = new BaseSettingsManager(new ISettingsStorage[]{ mockStorage }, null);
    assertFalse(settingsManager.updateDataSet(containerView, true));
  }

  @Test( expected = LocalResourceUnavailableException.class )
  public void updateDataSetLocalUnavailable() throws Exception
  {
    when(mockStorage.getName()).thenReturn("storage");
    MockContainer spyContainer = spy(container);
    doThrow(new LocalResourceUnavailableException("")).when(spyContainer).restorePersistentData(
        any(SettingsContainer.BackupContainerView.class));
    SettingsContainer.BackupContainerView containerView = new SettingsContainer.BackupContainerView(null, 10L);
    BaseSettingsManager settingsManager = new BaseSettingsManager(new ISettingsStorage[]{ mockStorage }, spyContainer);
    assertFalse(settingsManager.updateDataSet(containerView, true));
  }

  @Test
  public void updateDataSetOnLowestVersion() throws Exception
  {
    when(mockStorage.getName()).thenReturn("storage");
    MockContainer spyContainer = spy(container);
    doNothing().when(spyContainer).restorePersistentData(any(SettingsContainer.BackupContainerView.class));
    doReturn(42L).when(spyContainer).version();
    SettingsContainer.BackupContainerView containerView = new SettingsContainer.BackupContainerView(null, 10L);
    BaseSettingsManager settingsManager = new BaseSettingsManager(new ISettingsStorage[]{ mockStorage }, spyContainer);
    assertFalse(settingsManager.updateDataSet(containerView, false));
  }

  @Test
  public void updateDataSetOnLowestVersionWithForce() throws Exception
  {
    when(mockStorage.getName()).thenReturn("storage");
    MockContainer spyContainer = spy(container);
    doNothing().when(spyContainer).restorePersistentData(any(SettingsContainer.BackupContainerView.class));
    doReturn(42L).when(spyContainer).version();
    SettingsContainer.BackupContainerView containerView = new SettingsContainer.BackupContainerView(null, 10L);
    BaseSettingsManager settingsManager = new BaseSettingsManager(new ISettingsStorage[]{ mockStorage }, spyContainer);
    assertTrue(settingsManager.updateDataSet(containerView, true));
  }

  @Test
  public void loadSynchronousCheckNoHang() throws Exception
  {
    MockStorage storage = Mockito.mock(MockStorage.class, withSettings()
            .useConstructor("storage").defaultAnswer(CALLS_REAL_METHODS));
    MockContainer spyContainer = spy(container);
    doNothing().when(spyContainer).restorePersistentData(any(SettingsContainer.BackupContainerView.class));
    doReturn(42L).when(spyContainer).version();
    BaseSettingsManager settingsManager = new BaseSettingsManager(new ISettingsStorage[]{ storage }, spyContainer);
    SettingsContainer.BackupContainerView backupContainerView =
        settingsManager.loadSynchronousWithTimeout(Collections.singleton("storage"), null, 400);
    assertEquals(0L, backupContainerView.version());
  }

  @Test
  public void loadSynchronousWithNoDataLoaded() throws Exception
  {
    MockStorageWithLoad storage = Mockito.mock(MockStorageWithLoad.class, withSettings()
            .useConstructor("storage").defaultAnswer(CALLS_REAL_METHODS));
    MockContainer spyContainer = spy(container);
    doNothing().when(spyContainer).restorePersistentData(any(SettingsContainer.BackupContainerView.class));
    doReturn(42L).when(spyContainer).version();
    BaseSettingsManager settingsManager = new BaseSettingsManager(new ISettingsStorage[]{ storage }, spyContainer);
    SettingsContainer.BackupContainerView backupContainerView =
        settingsManager.loadSynchronous(Collections.singleton("storage"), null);
    assertEquals(0L, backupContainerView.version());
  }

  @Test
  public void loadSynchronousUnexistingStorage() throws Exception
  {
    MockStorageWithLoad storage = Mockito.mock(MockStorageWithLoad.class, withSettings()
            .useConstructor("storage").defaultAnswer(CALLS_REAL_METHODS));
    MockContainer spyContainer = spy(container);
    doNothing().when(spyContainer).restorePersistentData(any(SettingsContainer.BackupContainerView.class));
    doReturn(42L).when(spyContainer).version();
    BaseSettingsManager settingsManager = new BaseSettingsManager(new ISettingsStorage[]{ storage }, spyContainer);
    SettingsContainer.BackupContainerView backupContainerView =
        settingsManager.loadSynchronous(Collections.singleton("stor"), null);
    assertEquals(0L, backupContainerView.version());
  }

  // load synchronous

  @Test
  public void loadSynchronousProgress() throws Exception
  {
    when(mockStorage.getName()).thenReturn("storage");
    MockStorageWithLoad storage = Mockito.mock(MockStorageWithLoad.class, withSettings()
            .useConstructor("storage1").defaultAnswer(CALLS_REAL_METHODS));
    MockStorageWithLoad storage1 = Mockito.mock(MockStorageWithLoad.class, withSettings()
          .useConstructor("storage2").defaultAnswer(CALLS_REAL_METHODS));
    doThrow(new StorageTransferException("")).when(mockStorage).load(any(ISettingsStorage.ILoadSettingsResult.class));
    MockContainer spyContainer = spy(container);
    doNothing().when(spyContainer).restorePersistentData(any(SettingsContainer.BackupContainerView.class));
    doReturn(42L).when(spyContainer).version();
    BaseSettingsManager settingsManager =
        new BaseSettingsManager(new ISettingsStorage[]{ storage, storage1, mockStorage }, spyContainer);
    final AtomicInteger counter = new AtomicInteger(0);
    SettingsContainer.BackupContainerView backupContainerView = settingsManager
        .loadSynchronous(Arrays.asList("storage", "storage1", "storage2", "OLOLOL"),
                         new SettingsManagerAPI.IReceiveSettingsResult()
                         {
                           @Override
                           public void onProgress( int i, int i1 )
                           {
                             assertEquals(4, i1);
                             counter.incrementAndGet();
                           }

                           @Override
                           public void onFinished( boolean b )
                           {
                             assertFalse(b);
                           }
                         });
    assertEquals(0L, backupContainerView.version());
  }

  @Test
  public void testGetController() throws Exception
  {
    when(mockStorage.getName()).thenReturn("storage");
    doNothing().when(mockContainer).confirmBackupFinished();
    doNothing().when(mockStorage).save(any(byte[].class));
    BaseSettingsManager settingsManager = new BaseSettingsManager(new ISettingsStorage[]{ mockStorage }, mockContainer);
    assertNull(settingsManager.getController("test"));
    assertNotNull(settingsManager.getController(SettingsControllerType.DEFAULT_CONTROLLER));
  }

  @Test
  public void testFreeController() throws Exception
  {
    when(mockStorage.getName()).thenReturn("storage");
    doNothing().when(mockContainer).confirmBackupFinished();
    doNothing().when(mockStorage).save(any(byte[].class));
    BaseSettingsManager settingsManager = new BaseSettingsManager(new ISettingsStorage[]{ mockStorage }, mockContainer);
    SettingsControllerAPI controller = settingsManager.getController(SettingsControllerType.DEFAULT_CONTROLLER);
    settingsManager.freeController(SettingsControllerType.DEFAULT_CONTROLLER);
    assertTrue(controller == settingsManager.getController(SettingsControllerType.DEFAULT_CONTROLLER));
  }

  @Test
  public void testSettingsSaveListener() throws Exception
  {
    when(mockStorage.getName()).thenReturn("storage");
    doNothing().when(mockContainer).confirmBackupFinished();
    doNothing().when(mockStorage).save(any(byte[].class));
    BaseSettingsManager settingsManager = new BaseSettingsManager(new ISettingsStorage[]{ mockStorage }, mockContainer);
    MockForManager_SettingsSaveListener settingsSaveListener
            = Mockito.mock(MockForManager_SettingsSaveListener.class, withSettings()
            .useConstructor(1).defaultAnswer(CALLS_REAL_METHODS));
    settingsManager.registerNotifier(settingsSaveListener);
    String data = "test_data";
    settingsSaveListener.setExpectedSetting("test_key", data);
    settingsManager.save("test_key", data, true);
    assertEquals(settingsSaveListener.getNumberOfCalls(), 1);
    settingsManager.unregisterNotifier(settingsSaveListener);
    settingsManager.save("test", "test_data2", true);
    assertEquals(settingsSaveListener.getNumberOfCalls(), 1);
  }

  @Test
  public void testApplicationSettingsSaveListener() throws Exception
  {
    when(mockStorage.getName()).thenReturn("storage");
    doNothing().when(mockContainer).confirmBackupFinished();
    doNothing().when(mockStorage).save(any(byte[].class));
    BaseSettingsManager settingsManager = new BaseSettingsManager(new ISettingsStorage[]{ mockStorage }, mockContainer);
    MockForManager_AppSettingsSaveListener applicationSettingsChangeListener
            = Mockito.mock(MockForManager_AppSettingsSaveListener.class, withSettings()
            .useConstructor(1).defaultAnswer(CALLS_REAL_METHODS));
    settingsManager.registerNotifier(applicationSettingsChangeListener);
    ApplicationSettings applicationSettings = settingsManager.loadApplicationSettings();
    applicationSettings.setNewsEnabled(!applicationSettings.isNewsEnabled());
    applicationSettingsChangeListener.setExpectedApplicationSettings(applicationSettings);
    settingsManager.saveApplicationSettings(applicationSettings);
    assertEquals(applicationSettingsChangeListener.getNumberOfCalls(), 1);
    settingsManager.unregisterNotifier(applicationSettingsChangeListener);
    applicationSettings.setStatisticsEnabled(!applicationSettings.isStatisticsEnabled());
    settingsManager.saveApplicationSettings(applicationSettings);
    assertEquals(applicationSettingsChangeListener.getNumberOfCalls(), 1);
  }

  @Test
  public void screenOpenerTest() throws ManagerInitException, WrongTypeException {
    when(mockStorage.getName()).thenReturn("storage");
    BaseSettingsManager settingsManager = new BaseSettingsManager(mContext, new ISettingsStorage[]{mockStorage}, null);
    settingsManager.registerScreenOpener(mScreenOpener);
    SettingsControllerAPI controller = settingsManager.getController(SettingsControllerType.DEFAULT_CONTROLLER);
    controller.openScreen(ScreenType.Settings);
    verify(mScreenOpener, times(1)).openScreen(ScreenType.Settings);
  }

  @Test
  public void testSettingManagerHolder() throws ManagerInitException {
    BaseSettingsManager settingsManager = new BaseSettingsManager(mContext, new ISettingsStorage[]{mockStorage}, null);
    SettingsManagerHolder.setManager(settingsManager);
    assertNotNull(SettingsManagerHolder.getManager());
    assertSame(settingsManager, SettingsManagerHolder.getManager());
  }
}