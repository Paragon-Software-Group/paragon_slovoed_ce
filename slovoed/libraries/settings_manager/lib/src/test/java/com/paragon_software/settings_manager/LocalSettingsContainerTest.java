package com.paragon_software.settings_manager;

import com.paragon_software.settings_manager.mock.MockStorageLocal;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.mockito.junit.MockitoJUnitRunner;

import java.util.HashMap;

import static org.junit.Assert.assertEquals;
import static org.mockito.Matchers.anyString;
import static org.mockito.Mockito.doNothing;
import static org.mockito.Mockito.doReturn;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;

/**
 * Created by belyshov on 14/02/2018.
 */
@RunWith( MockitoJUnitRunner.class )
public class LocalSettingsContainerTest
{

  private MockStorageLocal mockStorage;

  @Before
  public void initMocks()
  {
    mockStorage = Mockito.spy(MockStorageLocal.class);
  }

  @Test
  public void putStringAndGet() throws Exception
  {
    final String key = "key";
    final String value = "value";

    LocalSettingsContainer container = new LocalSettingsContainer(mockStorage);
    container.putString(key, value);
    assertEquals(container.getString(key), value);
  }

  @Test
  public void putStringAndGetUnexisting() throws Exception
  {
    final String key = "key";
    final String value = "value";
    final String falsekey = "falsekey";

    LocalSettingsContainer container = new LocalSettingsContainer(mockStorage);
    container.putString(key, value);
    assertEquals(null, container.getString(falsekey));
  }

  @Test
  public void putStringThrough() throws Exception
  {
    final String key = "key";
    final String value = "value";

    doNothing().when(mockStorage).save(anyString(), anyString());
    LocalSettingsContainer container = new LocalSettingsContainer(mockStorage);
    container.putString(key, value);
    verify(mockStorage, times(1)).save(anyString(), anyString());
  }

  @Test
  public void putStringCache() throws Exception
  {
    final String key = "key";
    final String value = "value";

    doNothing().when(mockStorage).save(anyString(), anyString());
    LocalSettingsContainer container = new LocalSettingsContainer(mockStorage);
    container.putString(key, value);
    assertEquals(container.getString(key), value);
  }

  @Test
  public void loadSucess() throws Exception
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

    doNothing().when(mockStorage).save(anyString(), anyString());
    doReturn(map).when(mockStorage).load();

    LocalSettingsContainer container = new LocalSettingsContainer(mockStorage);
    container.putString(key, value);
    assertEquals(container.getString(key), value);
    assertEquals(container.getString(key2), value2);
    assertEquals(container.getString(key3), value3);
  }

  @Test
  public void putStringTwice() throws Exception
  {
    final String key = "key";
    final String value = "value";
    final String value2 = "value2";

    doNothing().when(mockStorage).save(anyString(), anyString());

    LocalSettingsContainer container = new LocalSettingsContainer(mockStorage);
    container.putString(key, value);
    container.putString(key, value2);
    assertEquals(container.getString(key), value2);
  }
}