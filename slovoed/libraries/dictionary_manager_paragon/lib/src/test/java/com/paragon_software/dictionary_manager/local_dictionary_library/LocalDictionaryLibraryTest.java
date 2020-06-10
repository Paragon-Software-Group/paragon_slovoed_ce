package com.paragon_software.dictionary_manager.local_dictionary_library;

import android.content.Context;
import androidx.annotation.NonNull;

import com.paragon_software.dictionary_manager.file_operations.DefaultFileOperations;
import com.paragon_software.dictionary_manager.file_operations.IFileOperations;
import com.paragon_software.dictionary_manager.file_operations.exceptions.CantCreateFileException;
import com.paragon_software.dictionary_manager.file_operations.exceptions.CantDeleteFileException;
import com.paragon_software.dictionary_manager.local_dictionary_library.exceptions.InitDictionaryLibraryException;
import com.paragon_software.dictionary_manager.local_dictionary_library.exceptions.WriteToDictionaryLibraryException;

import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.junit.MockitoJUnit;
import org.mockito.junit.MockitoJUnitRunner;
import org.mockito.junit.MockitoRule;

import java.io.File;
import java.util.Collection;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.Mockito.doReturn;
import static org.mockito.Mockito.doThrow;
import static org.mockito.Mockito.spy;

@RunWith( MockitoJUnitRunner.Silent.class )
public class LocalDictionaryLibraryTest
{
  @Rule
  public MockitoRule rule = MockitoJUnit.rule();

  private File    envDir      = new File("./");
  private String  testLibName = "testLib";
  private File    testLibDir  = new File(envDir, testLibName);
  @Mock
  private Context context;

  @Before
  public void initMocks()
  {
    doReturn(envDir).when(context).getFilesDir();
  }

  private void deleteDir( @NonNull final File file ) throws CantDeleteFileException
  {
    if ( !file.exists() )
    {
      return;
    }
    File[] contents = file.listFiles();
    if ( contents != null )
    {
      for ( File f : contents )
      {
        deleteDir(f);
      }
    }
    if ( !file.delete() )
    {
      throw new CantDeleteFileException(file.getName());
    }
  }

  @Before
  public void setUp() throws Exception
  {
    deleteDir(testLibDir);
    deleteDir(new File(envDir, LocalDictionaryLibrary.getDefaultDictionaryDirectoryName()));
  }

  IFileOperations fileOperations = new DefaultFileOperations();

  @Test
  public void initLibrary() throws Exception
  {
    assertFalse(testLibDir.exists());
    LocalDictionaryLibrary library = new LocalDictionaryLibrary(context, testLibName);
    assertTrue(testLibDir.exists());
  }

  @Test( expected = InitDictionaryLibraryException.class )
  public void initLibraryCreateDirFails() throws Exception
  {
    assertFalse(testLibDir.exists());
    IFileOperations spyFileOperations = spy(fileOperations);
    doThrow(new CantCreateFileException(testLibName)).when(spyFileOperations).createDir(anyString());
    LocalDictionaryLibrary library = new LocalDictionaryLibrary(context, testLibName, spyFileOperations);
  }

  @Test
  public void initLibraryWithDefaultName() throws Exception
  {
    String defaultDirName = LocalDictionaryLibrary.getDefaultDictionaryDirectoryName();
    File defaultTestDir = new File(envDir, defaultDirName);
    assertFalse(defaultTestDir.exists());
    LocalDictionaryLibrary library = new LocalDictionaryLibrary(context);
    assertTrue(defaultTestDir.exists());
    assertTrue(defaultTestDir.delete());
  }

  @Test
  public void initLibraryWithDefaultNameCheck() throws Exception
  {
    String newTestDirName = "newTestDir";
    File newTestDir = new File(envDir, newTestDirName);
    assertFalse(newTestDir.exists());
    LocalDictionaryLibrary library = new LocalDictionaryLibrary(context, newTestDirName);
    assertTrue(newTestDir.exists());
    assertTrue(newTestDir.delete());
  }

  @Test
  public void initLibraryWhenFolderExists() throws Exception
  {
    assertTrue(testLibDir.mkdir());
    LocalDictionaryLibrary library = new LocalDictionaryLibrary(context, testLibName);
    assertTrue(testLibDir.exists());
  }

  @Test
  public void initLibraryWhenFileWithThisNameExists() throws Exception
  {
    assertTrue(testLibDir.createNewFile());
    LocalDictionaryLibrary library = new LocalDictionaryLibrary(context, testLibName);
    assertTrue(testLibDir.exists());
    assertTrue(testLibDir.isDirectory());
  }

  @Test( expected = InitDictionaryLibraryException.class )
  public void initLibraryWhenFileWithThisNameExistsAndDeleteFails() throws Exception
  {
    assertTrue(testLibDir.createNewFile());
    IFileOperations spyFileOperations = spy(fileOperations);
    doThrow(new CantDeleteFileException(testLibName)).when(spyFileOperations).delete(anyString());
    LocalDictionaryLibrary library = new LocalDictionaryLibrary(context, testLibName, spyFileOperations);
  }

  @Test
  public void initLibraryWhenFolderExistsWithDictionariesIsDictionaryPresent() throws Exception
  {
    File dict1 = new File(testLibDir, "dict1");
    File dict2 = new File(testLibDir, "dict2");
    File dict3 = new File(testLibDir, "dict3");

    assertTrue(testLibDir.mkdir());
    assertTrue(dict1.createNewFile());
    assertTrue(dict2.createNewFile());
    assertTrue(dict3.createNewFile());
    LocalDictionaryLibrary library = new LocalDictionaryLibrary(context, testLibName);

    assertTrue(library.isDictionaryPresent("dict1"));
    assertTrue(library.isDictionaryPresent("dict2"));
    assertTrue(library.isDictionaryPresent("dict3"));

  }

  @Test
  public void initLibraryWhenFolderExistsWithDictionariesGetDictionaries() throws Exception
  {
    File dict1 = new File(testLibDir, "dict1");
    File dict2 = new File(testLibDir, "dict2");
    File dict3 = new File(testLibDir, "dict3");

    assertTrue(testLibDir.mkdir());
    assertTrue(dict1.createNewFile());
    assertTrue(dict2.createNewFile());
    assertTrue(dict3.createNewFile());
    LocalDictionaryLibrary library = new LocalDictionaryLibrary(context, testLibName);
    Collection< String > dictList = library.getLocalDictionariesList();
    assertEquals(3, dictList.size());
    assertTrue(dictList.contains("dict1"));
    assertTrue(dictList.contains("dict2"));
    assertTrue(dictList.contains("dict3"));

  }

  @Test
  public void getDictionaryContainerName() throws Exception
  {
    assertFalse(testLibDir.exists());
    LocalDictionaryLibrary library = new LocalDictionaryLibrary(context, testLibName);
    assertEquals(library.getDictionaryContainerName(), testLibName);
    assertTrue(testLibDir.delete());
  }

  @Test
  public void initLibraryWhenFolderExistsWithSubdirs() throws Exception
  {
    File dict1 = new File(testLibDir, "dict1");
    File dict2 = new File(testLibDir, "dict2");
    File internalDir = new File(testLibDir, "dir3");

    assertTrue(testLibDir.mkdir());
    assertTrue(dict1.createNewFile());
    assertTrue(dict2.createNewFile());
    assertTrue(internalDir.mkdir());
    LocalDictionaryLibrary library = new LocalDictionaryLibrary(context, testLibName);

    assertTrue(library.isDictionaryPresent("dict1"));
    assertTrue(library.isDictionaryPresent("dict2"));
    assertFalse(library.isDictionaryPresent("dir3"));
  }

  @Test( expected = WriteToDictionaryLibraryException.class )
  public void getDictionaryWriterForDir() throws Exception
  {
    File internalDir = new File(testLibDir, "dir3");
    assertTrue(internalDir.mkdirs());

    LocalDictionaryLibrary library = new LocalDictionaryLibrary(context, testLibName);

    library.getDictionaryWriter(internalDir.getName());
  }

  @Test( expected = WriteToDictionaryLibraryException.class )
  public void getDictionaryWriterWhenFileExistNoOverwrite() throws Exception
  {
    File internalDir = new File(testLibDir, "dict");
    assertTrue(testLibDir.mkdir());
    assertTrue(internalDir.createNewFile());

    LocalDictionaryLibrary library = new LocalDictionaryLibrary(context, testLibName);
    library.getDictionaryWriter(internalDir.getName(), false).finish();
  }

  @Test
  public void getDictionaryWriterWhenFileExistWithOverwrite() throws Exception
  {
    File internalDir = new File(testLibDir, "dict");
    assertTrue(testLibDir.mkdir());
    assertTrue(internalDir.createNewFile());

    LocalDictionaryLibrary library = new LocalDictionaryLibrary(context, testLibName);
    library.getDictionaryWriter(internalDir.getName(), true).finish();
  }

  @Test
  public void getDictionaryWriterWhenFileNotExist() throws Exception
  {
    File internalDir = new File(testLibDir, "dict");
    assertTrue(testLibDir.mkdir());

    LocalDictionaryLibrary library = new LocalDictionaryLibrary(context, testLibName);
    library.getDictionaryWriter(internalDir.getName(), true).finish();
  }

  @Test
  public void getDictionaryWriterWhenFileNotExistCheckRescan() throws Exception
  {
    final String dictName = "dict";
    File internalDir = new File(testLibDir, dictName);
    assertTrue(testLibDir.mkdir());

    LocalDictionaryLibrary library = new LocalDictionaryLibrary(context, testLibName);
    assertFalse(library.isDictionaryPresent(dictName));
    IDictionaryWriter writer = library.getDictionaryWriter(internalDir.getName(), true);
    assertFalse(library.isDictionaryPresent(dictName));
    writer.write(new byte[3], 3);
    assertFalse(library.isDictionaryPresent(dictName));
    writer.finish();
    assertTrue(library.isDictionaryPresent(dictName));
  }

}
