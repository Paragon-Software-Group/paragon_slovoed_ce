package com.paragon_software.engine.rx;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

public class Task< T > implements Cancelable
{
  @NonNull
  private final Cancelable cancelableImpl;

  @NonNull
  private final T params;

  public static < T > Task< T > createInitial( @NonNull T params )
  {
    return new Task<>(new CancelableImpl(), params);
  }

  private Task( @NonNull Cancelable cancelable, @NonNull T _params )
  {
    cancelableImpl = cancelable;
    params = _params;
  }

  public < T2 > Task< T2 > spawn( @NonNull T2 params )
  {
    return new Task<>(cancelableImpl, params);
  }

  //  public Cancelable getOrigin()
  //  {
  //    return cancelableImpl;
  //  }

  public boolean isRelatedTo( @Nullable Task< T > other )
  {
    boolean res = false;
    if ( other != null )
    {
      res = cancelableImpl.equals(other.cancelableImpl);
    }
    return res;
  }

  @Override
  public void cancel()
  {
    cancelableImpl.cancel();
  }

  @Override
  public boolean isCanceled()
  {
    return cancelableImpl.isCanceled();
  }

  @NonNull
  public T get()
  {
    return params;
  }

  private static class CancelableImpl implements Cancelable
  {
    private volatile boolean cancelFlag = false;

    //  @NonNull
    //  final AtomicBoolean completeFlag = new AtomicBoolean(false);

    @Override
    public void cancel()
    {
      cancelFlag = true;
    }

    //  @Override
    //  public void complete()
    //  {
    //    completeFlag.set(true);
    //  }

    @Override
    public boolean isCanceled()
    {
      return cancelFlag;
    }

    //  @Override
    //  public boolean isComplete()
    //  {
    //    return completeFlag.get();
    //  }
  }
}
