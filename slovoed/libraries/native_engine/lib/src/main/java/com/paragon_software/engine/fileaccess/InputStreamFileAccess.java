package com.paragon_software.engine.fileaccess;

import android.content.Context;
import android.content.res.AssetManager;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.io.IOException;
import java.io.InputStream;

public class InputStreamFileAccess implements FileAccess
{
  @NonNull
  private final InputStream mInputStream;

  @Nullable
  public static InputStreamFileAccess openStreamFromAssets( Context context, String fileName )
  {
    InputStreamFileAccess res = null;
    if ( ( context != null ) && ( fileName != null ) )
    {
      AssetManager assetManager = context.getAssets();
      if ( assetManager != null )
      {
        try
        {
          InputStream inputStream = assetManager.open(fileName);
          if ( inputStream != null )
          {
            res = new InputStreamFileAccess(inputStream);
          }
        }
        catch ( IOException ignore )
        {

        }
      }
    }
    return res;
  }

  InputStreamFileAccess( @NonNull InputStream inputStream )
  {
    mInputStream = inputStream;
  }

  @NonNull
  public InputStream getInputStream()
  {
    return mInputStream;
  }

  @Override
  public void close()
  {
    try
    {
      mInputStream.close();
    }
    catch ( IOException ignore )
    {

    }
  }
}
