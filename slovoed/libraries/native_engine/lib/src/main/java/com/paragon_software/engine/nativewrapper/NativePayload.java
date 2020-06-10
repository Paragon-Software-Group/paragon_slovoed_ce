package com.paragon_software.engine.nativewrapper;

import android.content.Context;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.dictionary_manager.extbase.MorphoInfo;
import com.paragon_software.dictionary_manager.location.DictionaryAssetsLocation;
import com.paragon_software.dictionary_manager.location.DictionaryFileLocation;
import com.paragon_software.dictionary_manager.location.DictionaryLocation;
import com.paragon_software.engine.destructionqueue.ManagedResource;
import com.paragon_software.engine.fileaccess.AssetFileDescriptorFileAccess;
import com.paragon_software.engine.fileaccess.FileAccess;
import com.paragon_software.engine.fileaccess.InputStreamFileAccess;
import com.paragon_software.engine.fileaccess.ParcelFileDescriptorFileAccess;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

class NativePayload implements ManagedResource
{

  @NonNull
  private final NativeFunctions mFunctions;
  @NonNull
  private final FileAccess      mFileAccess;

  private List<FileAccess> mMorphoFileAccessList = new ArrayList<>();

  private volatile int mCloseResult = -1;

  @Nullable
  public static NativePayload open( Context context, DictionaryLocation location, List<MorphoInfo> morphoInfoList  ,boolean openMorpho)
  {
    FileAccess fileAccess = getFileAccess(context,location);

    String cacheDir = context.getCacheDir().toString()+"/image";
    File directory = new File(cacheDir);
    if (! directory.exists()){
      directory.mkdir();
    }

    NativeFunctions nativeFunctions = open(fileAccess, cacheDir);

    NativePayload res = null;

    if (null != fileAccess)
    {
      if (null != nativeFunctions)
      {
        res = new NativePayload(nativeFunctions, fileAccess);
        if (openMorpho)
        {
          res.addMorphoBases(context, morphoInfoList);
        }
      }
      else
      {
        fileAccess.close();
      }
    }

    return res;
  }

  private void addMorphoBases(Context context,List<MorphoInfo> morphoInfoList)
  {
    if (null == morphoInfoList)
    {
      return;
    }

    for (MorphoInfo morphoInfo : morphoInfoList)
    {
      FileAccess fileAccess = getFileAccess(context,morphoInfo.getLocation());
      if (fileAccess instanceof ParcelFileDescriptorFileAccess)
      {
        ParcelFileDescriptorFileAccess access = (ParcelFileDescriptorFileAccess)fileAccess;
        int success  = mFunctions.addMorphoFromDescriptor(access.getFd(), 0L, -1L);
        if (0 != success)
        {
          mMorphoFileAccessList.add(fileAccess);
        }
        else
        {
          fileAccess.close();
        }
      }
      else if (fileAccess instanceof AssetFileDescriptorFileAccess)
      {
        AssetFileDescriptorFileAccess access = (AssetFileDescriptorFileAccess)fileAccess;
        int success = mFunctions.addMorphoFromDescriptor(access.getFd(),access.getOffset(),access.getSize());
        if (0 != success)
        {
          mMorphoFileAccessList.add(fileAccess);
        }
        else
        {
          fileAccess.close();
        }
      }
      else if (fileAccess instanceof InputStreamFileAccess)
      {
        InputStreamFileAccess access = (InputStreamFileAccess)fileAccess;
        int success = mFunctions.addMorphoFromReader(new InputStreamWrapper(access.getInputStream()));
        if (0 != success)
        {
          mMorphoFileAccessList.add(fileAccess);
        }
        else
        {
          fileAccess.close();
        }
      }
    }
  }

  @Nullable
  private static NativeFunctions open(FileAccess fileAccess, String cacheDir)
  {
    NativeFunctions nativeFunctions = null;
    if (fileAccess instanceof ParcelFileDescriptorFileAccess)
    {
      ParcelFileDescriptorFileAccess access = (ParcelFileDescriptorFileAccess)fileAccess;
      nativeFunctions = NativeFunctions.openFromDescriptor(access.getFd(), 0L, -1L ,cacheDir);
    }
    else if (fileAccess instanceof AssetFileDescriptorFileAccess)
    {
      AssetFileDescriptorFileAccess access = (AssetFileDescriptorFileAccess)fileAccess;
      nativeFunctions = NativeFunctions.openFromDescriptor(access.getFd(),access.getOffset(),access.getSize(), cacheDir);
    }
    else if (fileAccess instanceof InputStreamFileAccess)
    {
      InputStreamFileAccess access = (InputStreamFileAccess)fileAccess;
      nativeFunctions = NativeFunctions.openFromJavaReader(new InputStreamWrapper(access.getInputStream()), cacheDir);
    }
    return nativeFunctions;
  }

  @Nullable
  public static FileAccess getFileAccess( Context context, DictionaryLocation location)
  {
    FileAccess res = null;
    if ( location instanceof DictionaryAssetsLocation )
    {
      res = getFileAccessFromAssets(context, (DictionaryAssetsLocation) location);
    }
    else if ( location instanceof DictionaryFileLocation )
    {
      res = getFileAccessFromFile((DictionaryFileLocation) location);
    }
    return res;
  }

  @Nullable
  private static FileAccess getFileAccessFromAssets(Context context, DictionaryAssetsLocation location)
  {
    FileAccess fileAccess = getFileAccessViaDescriptor(context,location);
    if (null == fileAccess)
    {
      fileAccess = getFileAccessFromStream(context,location);
    }
    return fileAccess;
  }

  @Nullable
  private static FileAccess getFileAccessFromFile(DictionaryFileLocation location)
  {
    return ParcelFileDescriptorFileAccess.openFile(location.getFile());
  }

  @Nullable
  private static FileAccess getFileAccessViaDescriptor(Context context, DictionaryAssetsLocation location)
  {
    return AssetFileDescriptorFileAccess.openAsset(context, location.getFileName());
  }

  @Nullable
  private static FileAccess getFileAccessFromStream(Context context, DictionaryAssetsLocation location)
  {
    return InputStreamFileAccess.openStreamFromAssets(context, location.getFileName());
  }

  private NativePayload( @NonNull NativeFunctions functions, @NonNull FileAccess fileAccess )
  {
    mFunctions = functions;
    mFileAccess = fileAccess;
  }

  @Override
  public void free()
  {
    mCloseResult = mFunctions.close();
    mFileAccess.close();
    for (FileAccess morphoFileAccess : mMorphoFileAccessList)
    {
      morphoFileAccess.close();
    }
  }

  NativeFunctions getFunctions()
  {
    return mFunctions;
  }

  int getCloseResult()
  {
    return mCloseResult;
  }
}
