package com.paragon_software.engine.nativewrapper;

import androidx.annotation.NonNull;

abstract class JavaReader
{
  private final NativeFunctions.Callback mIsOpenCallback = new NativeFunctions.Callback()
  {
    @Override
    @NonNull
    public Boolean function( Object... args )
    {
      return isOpened();
    }
  };

  private final NativeFunctions.Callback mReadCallback = new NativeFunctions.Callback()
  {
    @Override
    @NonNull
    public Integer function( Object... args )
    {
      return read((Long) args[0], (byte[]) args[1]);
    }
  };

  private final NativeFunctions.Callback mGetSizeCallback = new NativeFunctions.Callback()
  {
    @Override
    @NonNull
    public Long function( Object... args )
    {
      return getSize();
    }
  };

  Object[] getCallbacks()
  {
    return new Object[]{ mIsOpenCallback, mReadCallback, mGetSizeCallback };
  }

  abstract boolean isOpened();

  abstract int read( long offset, byte[] buffer );

  abstract long getSize();

}
