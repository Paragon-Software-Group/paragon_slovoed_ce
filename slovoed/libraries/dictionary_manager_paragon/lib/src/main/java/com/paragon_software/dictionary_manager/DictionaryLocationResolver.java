package com.paragon_software.dictionary_manager;

import android.content.Context;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.io.File;

import com.paragon_software.dictionary_manager.components.DictionaryComponent;
import com.paragon_software.dictionary_manager.file_operations.exceptions.CantDeleteFileException;
import com.paragon_software.dictionary_manager.local_dictionary_library.ILocalDictionaryLibrary;
import com.paragon_software.dictionary_manager.location.DictionaryAssetsLocation;
import com.paragon_software.dictionary_manager.location.DictionaryFileLocation;
import com.paragon_software.dictionary_manager.location.DictionaryLocation;

import static com.paragon_software.dictionary_manager.components.DictionaryComponent.Type.MORPHO;

class DictionaryLocationResolver
{
  private static final String FULL_BASES_ASSET_PATH           = "full";
  private static final String DEMO_BASES_ASSET_PATH           = "demo";
  private static final String MORPHO_BASES_ASSET_PATH         = "morpho";
  private static final String DICTIONARY_BASES_FILE_EXTENSION = ".sdc";

  @NonNull
  private ILocalDictionaryLibrary mDictionaryLibrary;
  private Context                 mContext;

  public DictionaryLocationResolver( @NonNull ILocalDictionaryLibrary dictionaryLibrary, @NonNull Context context )
  {
    this.mDictionaryLibrary = dictionaryLibrary;
    this.mContext = context;
  }

  public Context getContext()
  {
    return mContext;
  }

  @NonNull
  public ILocalDictionaryLibrary getDictionaryLibrary()
  {
    return mDictionaryLibrary;
  }

  public DictionaryLocation getLocation( @NonNull DictionaryComponent component )
  {
    if ( isDownloaded(component) )
    {
      return getDictionaryComponentFileLocation(component);
    }
    else if ( isInAssets(component) )
    {
      return getDictionaryComponentAssetLocation(component);
    }
    else
    {
      return null;
    }
  }

  public boolean isDownloaded( DictionaryComponent component )
  {
    return mDictionaryLibrary.getLocalDictionariesList().contains(getComponentFileName(component));
  }

  @Nullable
  private DictionaryFileLocation getDictionaryComponentFileLocation( DictionaryComponent component )
  {
    DictionaryFileLocation fileLocation = null;
    try
    {
      String path = getDictionaryComponentPath(component);
      File file = new File(path);
      if ( file.exists() )
      {
        fileLocation = new DictionaryFileLocation(file);
      }
    }
    catch ( Exception e )
    {
      e.printStackTrace();
    }
    return fileLocation;
  }

  boolean isInAssets( DictionaryComponent component )
  {
    String path;
    if (MORPHO == component.getType())
    {
      path = MORPHO_BASES_ASSET_PATH;
    }
    else
    {
      path = component.isDemo() ? DEMO_BASES_ASSET_PATH : FULL_BASES_ASSET_PATH;
    }

    String name = component.getSdcId() + DICTIONARY_BASES_FILE_EXTENSION;
    return null != Utils.findAssetsFile(mContext, path, name);
  }

  private DictionaryAssetsLocation getDictionaryComponentAssetLocation( DictionaryComponent component )
  {
    String path;
    if (MORPHO == component.getType())
    {
      path = MORPHO_BASES_ASSET_PATH;
    }
    else
    {
      path = component.isDemo() ? DEMO_BASES_ASSET_PATH : FULL_BASES_ASSET_PATH;
    }
    return new DictionaryAssetsLocation(path + "/" + component.getSdcId() + DICTIONARY_BASES_FILE_EXTENSION);
  }

  public boolean remove (@NonNull DictionaryComponent component) throws CantDeleteFileException
  {
    final String componentFileName = getComponentFileName(component);
    return mDictionaryLibrary.remove(componentFileName);
  }

  public String getComponentFileName( @NonNull DictionaryComponent component )
  {
    String demo = component.isDemo() ? "_demo" : "";
    String version = null == component.getVersion() ? "" : component.getVersion();
    version = "_" + version;
    return component.getSdcId() + version + demo + DICTIONARY_BASES_FILE_EXTENSION;
  }

  private String getDictionaryComponentPath( DictionaryComponent component ) throws Exception
  {
    final String componentFileName = getComponentFileName(component);
    return mDictionaryLibrary.getPath(componentFileName);
  }

  public void rescanDictionaries()
  {
    mDictionaryLibrary.rescanDictionaries();
  }

  public boolean renameTmpToDownloadedComponent( @NonNull String tmpFileName, DictionaryComponent component )
  {
    try
    {
      return mDictionaryLibrary.rename(tmpFileName, getComponentFileName(component));
    }
    catch ( Exception e )
    {
      e.printStackTrace();
    }
    return false;
  }
}
