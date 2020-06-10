package com.paragon_software.engine.nativewrapper;

import androidx.annotation.NonNull;

import java.io.IOException;
import java.io.InputStream;

class InputStreamWrapper extends JavaReader
{
  @NonNull
  private final InputStream mStream;
  private volatile long mOffset = 0L;
  private final long mSize;
  private volatile boolean ok = true;

  InputStreamWrapper( @NonNull InputStream stream )
  {
    mStream = stream;
    mStream.mark(0);
    long size;
    try
    {
      size = mStream.skip(Long.MAX_VALUE);
      mStream.reset();
    }
    catch ( IOException ignore )
    {
      size = 0L;
      ok = false;
    }
    mStream.mark(0);
    mSize = size;
  }

  @Override
  boolean isOpened()
  {
    return ok;
  }

  @Override
  int read( long offset, @NonNull byte[] buffer )
  {
    int result = 0;
    if ( ok )
    {
      try
      {
        if ( offset < mOffset )
        {
          mStream.reset();
          mOffset = 0L;
          mStream.mark(0);
          mOffset += mStream.skip(offset);
        }
        else if ( offset > mOffset )
        {
          mOffset += mStream.skip(offset - mOffset);
        }
        result = mStream.read(buffer);
        mOffset += result;
      }
      catch ( IOException ignore )
      {
        ok = false;
      }
    }
    return result;
  }

  @Override
  long getSize()
  {
    return mSize;
  }
}
