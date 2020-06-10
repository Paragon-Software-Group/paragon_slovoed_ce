package com.paragon_software.settings_manager;

import com.paragon_software.settings_manager.mock.MockStorageLocal;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.mockito.junit.MockitoJUnitRunner;

import java.util.HashMap;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.mockito.Matchers.anyString;
import static org.mockito.Mockito.doNothing;
import static org.mockito.Mockito.doReturn;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;

/**
 * Created by belyshov on 14/02/2018.
 */
@RunWith( MockitoJUnitRunner.class )
public class PersistentSettingsContainerTest
{
  private MockStorageLocal mockStorage;

  @Before
  public void initMocks()
  {
    mockStorage = Mockito.spy(MockStorageLocal.class);
  }

  @Test
  public void constructorWithNoData() throws Exception
  {
    PersistentSettingsContainer container = new PersistentSettingsContainer(mockStorage);
    SettingsContainer.BackupContainerView backupContainerView = container.getContainerView();
    assertEquals(backupContainerView.version(), 0);
    assertFalse(container.isDirty());
    assertEquals(container.getDataMap().size(), 0);
  }

  @Test
  public void constructorWithDataNoVersion() throws Exception
  {
    final String key = "key";
    final String value = "value";
    final String key2 = "key2";
    final String value2 = "value2";
    final String key3 = "key3";
    final String value3 = "value3";

    HashMap< String, String > map = new HashMap<>();
    map.put(key, value);
    map.put(key2, value2);
    map.put(key3, value3);

    doReturn(map).when(mockStorage).load();

    PersistentSettingsContainer container = new PersistentSettingsContainer(mockStorage);
    assertEquals(container.version(), 0);
    assertFalse(container.isDirty());

    assertEquals(container.getString(key), value);
    assertEquals(container.getString(key2), value2);
    assertEquals(container.getString(key3), value3);
  }

  @Test
  public void constructorWithDataAndVersion() throws Exception
  {
    final String key = "key";
    final String value = "value";
    final String key2 = PersistentSettingsContainer.VersionKey();
    final String value2 = "458";

    HashMap< String, String > map = new HashMap<>();
    map.put(key, value);
    map.put(key2, value2);

    doReturn(map).when(mockStorage).load();

    PersistentSettingsContainer container = new PersistentSettingsContainer(mockStorage);
    assertEquals(458, container.version());
    assertFalse(container.isDirty());

    assertEquals(value, container.getString(key));
    assertEquals(null, container.getString(key2));
  }

  @Test
  public void constructorWithDataAndWrongVersion() throws Exception
  {
    final String key = "key";
    final String value = "value";
    final String key2 = PersistentSettingsContainer.VersionKey();
    final String value2 = "ololol";

    HashMap< String, String > map = new HashMap<>();
    map.put(key, value);
    map.put(key2, value2);

    doReturn(map).when(mockStorage).load();

    PersistentSettingsContainer container = new PersistentSettingsContainer(mockStorage);
    assertEquals(0, container.version());
    assertFalse(container.isDirty());

    assertEquals(value, container.getString(key));
    assertEquals(null, container.getString(key2));
  }

  @Test
  public void putStringAndGet() throws Exception
  {
    final String key = "key";
    final String value = "value";

    PersistentSettingsContainer container = new PersistentSettingsContainer(mockStorage);
    container.putString(key, value);
    assertEquals(container.getString(key), value);
  }

  @Test
  public void getUnexistingString() throws Exception
  {
    final String key = "key";
    PersistentSettingsContainer container = new PersistentSettingsContainer(mockStorage);
    assertEquals(container.getString(key), null);
  }

  @Test
  public void checkSaveThrough() throws Exception
  {
    final String key = "key";
    final String value = "value";
    final String key2 = "key2";
    final String value2 = "value2";
    
    doNothing().when(mockStorage).save(anyString(), anyString());
    PersistentSettingsContainer container = new PersistentSettingsContainer(mockStorage);
    container.putString(key, value);
    verify(mockStorage, times(2)).save(anyString(), anyString()); //dirty and string

    container.putString(key2, value2);
    verify(mockStorage, times(3)).save(anyString(), anyString()); //only string
  }

  @Test
  public void checkDirtyAfterSave() throws Exception
  {
    final String key = "key";
    final String value = "value";

    PersistentSettingsContainer container = new PersistentSettingsContainer(mockStorage);
    container.putString(key, value);
    assertTrue(container.isDirty());
  }

  @Test
  public void upVersion() throws Exception
  {
    PersistentSettingsContainer container = new PersistentSettingsContainer(mockStorage);
    long version = container.version();
    container.upVersion();
    assertEquals(version + 1, container.version());
  }

  @Test
  public void upVersionOnlyInMemory() throws Exception
  {
    PersistentSettingsContainer container = new PersistentSettingsContainer(mockStorage);
    container.upVersion();
    verify(mockStorage, times(0)).save(anyString(), anyString());
  }

  @Test
  public void restoreFromBackup() throws Exception
  {
    final String key = "key";
    final String value = "value";
    final String key2 = "key2";
    final String value2 = "value2";
    final String key3 = "key3";
    final String value3 = "value3";

    HashMap< String, String > map = new HashMap<>();
    map.put(key, value);
    map.put(key2, value2);
    map.put(key3, value3);

    mockStorage.setNewMap(map);

    PersistentSettingsContainer container = new PersistentSettingsContainer(mockStorage);

    final String new_key = "new_key";
    final String new_value = "new_value";
    final String new_key2 = "new_key2";
    final String new_value2 = "new_value2";
    final String new_key3 = "new_key3";
    final String new_value3 = "new_value3";

    HashMap< String, String > new_map = new HashMap<>();
    new_map.put(new_key, new_value);
    new_map.put(new_key2, new_value2);
    new_map.put(new_key3, new_value3);
    long newVersion = 0xbaba;

    container.restoreFromBackup(newVersion, new_map);
    assertEquals(null, container.getString(key));
    assertEquals(null, container.getString(key2));
    assertEquals(null, container.getString(key3));
    assertEquals(null, container.getString(PersistentSettingsContainer.VersionKey()));

    assertEquals(new_value, container.getString(new_key));
    assertEquals(new_value2, container.getString(new_key2));
    assertEquals(new_value3, container.getString(new_key3));
  }

  @Test
  public void checkDirtyAfterRecreate() throws Exception
  {
    final String key = "key";
    final String value = "value";

    PersistentSettingsContainer container = new PersistentSettingsContainer(mockStorage);
    container.putString(key, value);
    assertTrue(container.isDirty());

    PersistentSettingsContainer otherContainer = new PersistentSettingsContainer(mockStorage);
    assertTrue(otherContainer.isDirty());
  }

  @Test
  public void checkConfirmBackupFinished() throws Exception
  {
    final String key = "key";
    final String value = "value";

    PersistentSettingsContainer container = new PersistentSettingsContainer(mockStorage);
    container.putString(key, value);
    assertTrue(container.isDirty());

    PersistentSettingsContainer otherContainer = new PersistentSettingsContainer(mockStorage);
    otherContainer.confirmBackupFinished();
    assertFalse(otherContainer.isDirty());
    assertEquals(null, otherContainer.getString(PersistentSettingsContainer.dirtyKey()));
  }
}