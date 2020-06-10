package com.paragon_software.engine.fileaccess;

import android.content.Context;
import android.content.res.AssetFileDescriptor;
import android.content.res.AssetManager;
import android.os.ParcelFileDescriptor;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.io.IOException;

public class AssetFileDescriptorFileAccess implements FileAccess
{
  @NonNull
  private final ParcelFileDescriptorFileAccess mParcelFileDescriptorFileAccess;

  @NonNull
  private final AssetFileDescriptor mAssetFileDescriptor;

  @Nullable
  public static AssetFileDescriptorFileAccess openAsset( Context context, String fileName )
  {
    AssetFileDescriptorFileAccess res = null;
    if ( ( context != null ) && ( fileName != null ) )
    {
      AssetManager assetManager = context.getAssets();
      if ( assetManager != null )
      {
        try
        {
          AssetFileDescriptor assetFileDescriptor = assetManager.openFd(fileName);
          if ( assetFileDescriptor != null )
          {
            ParcelFileDescriptor parcelFileDescriptor = assetFileDescriptor.getParcelFileDescriptor();
            if ( parcelFileDescriptor != null )
            {
              res = new AssetFileDescriptorFileAccess(assetFileDescriptor, parcelFileDescriptor);
            }
            else
            {
              try
              {
                assetFileDescriptor.close();
              }
              catch ( IOException ignore )
              {

              }
            }
          }
        }
        catch ( IOException ignore )
        {

        }
      }
    }
    return res;
  }

  AssetFileDescriptorFileAccess( @NonNull AssetFileDescriptor assetFileDescriptor,
                                 @NonNull ParcelFileDescriptor parcelFileDescriptor )
  {
    mAssetFileDescriptor = assetFileDescriptor;
    mParcelFileDescriptorFileAccess = new ParcelFileDescriptorFileAccess(parcelFileDescriptor);
  }

  public int getFd()
  {
    return mParcelFileDescriptorFileAccess.getFd();
  }

  public long getOffset()
  {
    return mAssetFileDescriptor.getStartOffset();
  }

  public long getSize()
  {
    return mAssetFileDescriptor.getLength();
  }

  @Override
  public void close()
  {
    mParcelFileDescriptorFileAccess.close();
    try
    {
      mAssetFileDescriptor.close();
    }
    catch ( IOException ignore )
    {

    }
  }
}
