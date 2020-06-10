package com.paragon_software.engine.rx.translatearticle;

import com.paragon_software.engine.rx.Task;
import io.reactivex.Observer;
import io.reactivex.disposables.Disposable;

class TranslateObserver implements Observer< Task< Result > >
{
  @Override
  public void onSubscribe( Disposable d )
  {

  }

  @Override
  public void onNext( Task< Result > task )
  {
    Result result = task.get();
    if ( result.articleItem != null )
    {
      result.callback.onTranslationReady(result.articleItem, result.translation, result.hasHideOrSwitchBlocks,result.articleSearcher);
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
