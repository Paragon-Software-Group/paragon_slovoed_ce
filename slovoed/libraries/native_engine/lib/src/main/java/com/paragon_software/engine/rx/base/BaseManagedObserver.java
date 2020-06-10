package com.paragon_software.engine.rx.base;

import com.paragon_software.engine.destructionqueue.ManagedResource;
import com.paragon_software.engine.rx.Task;
import io.reactivex.Observer;
import io.reactivex.disposables.Disposable;

public abstract class BaseManagedObserver< TResult > implements Observer< Task< TResult > >, ManagedResource
{

  protected Disposable disposable;

  @Override
  public void free()
  {
    if ( disposable != null )
    {
      if ( !disposable.isDisposed() )
      {
        disposable.dispose();
      }
    }
  }

  @Override
  public void onSubscribe( Disposable d )
  {
    disposable = d;
  }

  @Override
  public abstract void onNext( Task< TResult > resultTask );

  @Override
  public void onError( Throwable e )
  {

  }

  @Override
  public void onComplete()
  {

  }
}
