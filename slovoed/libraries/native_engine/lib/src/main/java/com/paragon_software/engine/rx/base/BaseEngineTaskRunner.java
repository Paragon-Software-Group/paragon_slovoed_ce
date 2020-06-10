package com.paragon_software.engine.rx.base;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.lang.ref.WeakReference;
import java.util.Collection;
import java.util.Collections;

import com.paragon_software.engine.rx.EngineTaskRunner;
import com.paragon_software.engine.rx.Task;

import io.reactivex.disposables.Disposable;
import io.reactivex.functions.Consumer;
import io.reactivex.subjects.PublishSubject;
import io.reactivex.subjects.Subject;

public abstract class BaseEngineTaskRunner< TParams, TResult, TResultContainer > extends EngineTaskRunner< TParams >
{
  @NonNull
  protected Subject< Task< TParams > > source = PublishSubject.create();

  @Nullable
  protected Task< TParams > currentTask = null;

  @Nullable
  private Disposable sourceDisposable = null;

  @Nullable
  protected WeakReference< TResultContainer > weakResultContainer = null;

  @NonNull
  public TResultContainer search( @NonNull TParams params )
  {
    assertInitCalled();
    TResultContainer resultContainer = null;
    if ( weakResultContainer != null )
    {
      resultContainer = weakResultContainer.get();
    }
    if ( sourceDisposable == null || resultContainer == null )
    {
      resultContainer = createResultContainer();
      weakResultContainer = new WeakReference<>(resultContainer);
      sourceDisposable = subscribe(resultContainer);
    }
    emitNewTask(params);
    return resultContainer;
  }

  @NonNull
  protected abstract TResultContainer createResultContainer();

  @Override
  protected void emitTask( Task< TParams > task )
  {
    source.onNext(task);
  }

  @Override
  protected Collection< Task< TParams > > getCurrentTasks()
  {
    Collection< Task< TParams > > res = Collections.emptySet();
    if ( currentTask != null )
    {
      res = Collections.singleton(currentTask);
    }
    return res;
  }

  final protected Consumer< Task< TParams > > getDefaultSaveCurrentTask()
  {
    return new SaveCurrentTask();
  }

  protected class SaveCurrentTask implements Consumer< Task< TParams > >
  {
    @Override
    public void accept( Task< TParams > paramsTask )
    {
      if ( currentTask != null )
      {
        currentTask.cancel();
      }
      currentTask = paramsTask;
    }
  }

  final protected Consumer< Task< TResult > > getDiscardCurrentTask()
  {
    return new DiscardCurrentTask();
  }

  protected class DiscardCurrentTask implements Consumer< Task< TResult > >
  {
    @Override
    public void accept( Task< TResult > resultTask )
    {
      currentTask = null;
    }
  }

  protected abstract Disposable subscribe( @NonNull TResultContainer resultContainer );
}
