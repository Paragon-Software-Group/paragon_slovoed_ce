package com.paragon_software.engine.rx.sound;

import com.paragon_software.engine.rx.Task;
import io.reactivex.Observer;
import io.reactivex.disposables.Disposable;

public class SoundObserver implements Observer< Task< Result > >
{
  @Override
  public void onSubscribe( Disposable d )
  {

  }

  @Override
  public void onNext( Task< Result > resultTask )
  {
    Result result = resultTask.get();
    if ( result.soundData != null )
    {
      result.callback.onSoundReady(result.soundData);
    }
  }

  @Override
  public void onError( Throwable e )
  {

  }

  @Override
  public void onComplete()
  {

  }
}
