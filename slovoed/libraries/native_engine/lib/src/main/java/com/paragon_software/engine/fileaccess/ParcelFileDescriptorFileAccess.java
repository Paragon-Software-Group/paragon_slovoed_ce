package com.paragon_software.engine.fileaccess;

import android.os.ParcelFileDescriptor;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.io.File;
import java.io.IOException;

public class ParcelFileDescriptorFileAccess implements FileAccess
{
  @NonNull
  private final ParcelFileDescriptor mParcelFileDescriptor;

  @Nullable
  public static ParcelFileDescriptorFileAccess openFile( File file )
  {
    ParcelFileDescriptorFileAccess res = null;
    if ( file != null )
    {
      try
      {
        ParcelFileDescriptor parcelFileDescriptor =
            ParcelFileDescriptor.open(file, ParcelFileDescriptor.MODE_READ_ONLY);
        if ( parcelFileDescriptor != null )
        {
          res = new ParcelFileDescriptorFileAccess(parcelFileDescriptor);
        }
      }
      catch ( IOException ignore )
      {

      }
    }
    return res;
  }

  ParcelFileDescriptorFileAccess( @NonNull ParcelFileDescriptor parcelFileDescriptor )
  {
    mParcelFileDescriptor = parcelFileDescriptor;
  }

  public int getFd()
  {
    return mParcelFileDescriptor.getFd();
  }

  @Override
  public void close()
  {
    try
    {
      mParcelFileDescriptor.close();
    }
    catch ( IOException ignore )
    {

    }
  }
}
