package com.paragon_software.engine.rx.deserializearticle;

import androidx.annotation.NonNull;

import java.io.Serializable;
import java.lang.ref.WeakReference;
import java.util.*;

import com.paragon_software.engine.rx.EngineTaskRunner;
import com.paragon_software.engine.rx.RunOn;
import com.paragon_software.engine.rx.Task;
import com.paragon_software.native_engine.EngineSerializerAPI;
import io.reactivex.Observer;
import io.reactivex.functions.Consumer;
import io.reactivex.functions.Predicate;
import io.reactivex.subjects.PublishSubject;
import io.reactivex.subjects.Subject;


public class DeserializeArticlesTaskRunner extends EngineTaskRunner< Params >
{
  @NonNull
  private Subject< Task< Params > > source = PublishSubject.create();

  @NonNull
  private Map< EngineSerializerAPI.OnDeserializedArticlesReadyCallback, Task< Params > > currentTaskMap = new HashMap<>();

  @NonNull
  private Map< EngineSerializerAPI.OnDeserializedArticlesReadyCallback, ResultContainer > resultContainerMap = new HashMap<>();

  @NonNull
  private Map< EngineSerializerAPI.OnDeserializedArticlesReadyCallback, WeakReference< ResultContainer > > weakResultContainerMap = new HashMap<>();

  public void deserializeArticleItems( @NonNull Serializable[] serialized,
                                       @NonNull EngineSerializerAPI.OnDeserializedArticlesReadyCallback callback,
                                       boolean openSoundBases )
  {
    assertInitCalled();
    if ( weakResultContainerMap.get(callback) != null )
    {
      resultContainerMap.put(callback, weakResultContainerMap.get(callback).get());
    }
    if ( resultContainerMap.get(callback) == null )
    {
      ResultContainer resultContainer = new ResultContainer();
      resultContainerMap.put(callback, resultContainer);
      weakResultContainerMap.put(callback, new WeakReference<>(resultContainer));
      subscribe(resultContainer.getObserver(), callback);
    }
    emitNewTask(new Params(serialized, callback, openSoundBases));
  }

  private void subscribe( @NonNull Observer< Task< Result > > resultContainer,
                          EngineSerializerAPI.OnDeserializedArticlesReadyCallback forCallback )
  {
    RunOn< Task< Params > > split = new RunOn<>(RunOn.What.Observe, RunOn.Where.Computation, 200L);
    RunOn< Task< Result > > split2 = new RunOn<>(RunOn.What.Subscribe, RunOn.Where.Computation);
    RunOn< Task< Result > > result = new RunOn<>(RunOn.What.Observe, RunOn.Where.Caller);
    source.filter(new ForCallback(forCallback)).doOnNext(new SaveCurrentTask()).compose(split)
          .switchMap(new SplitByDictionaries(applicationContext, dictionaryManager, split2))
          .compose(result).filter(new OnlyUncanceled()).doOnNext(new DiscardCurrentTask())
          .subscribe(resultContainer);
  }

  private final class SaveCurrentTask implements Consumer< Task< Params > >
  {
    @Override
    public void accept( Task< Params > newTask )
    {
      if ( currentTaskMap.get(newTask.get().callback) != null )
      {
        currentTaskMap.get(newTask.get().callback).cancel();
      }
      currentTaskMap.put(newTask.get().callback, newTask);
    }
  }

  @Override
  protected void emitTask( Task< Params > task )
  {
    source.onNext(task);
  }

  @Override
  protected Collection< Task< Params > > getCurrentTasks()
  {
    List< Task< Params > > res = new ArrayList<>();
    for ( Task< Params > task : currentTaskMap.values())
    {
      if ( task != null)
      {
        res.add(task);
      }
    }
    return res;
  }

  private final class DiscardCurrentTask implements Consumer< Task< Result > >
  {
    @Override
    public void accept( Task< Result > task )
    {
      currentTaskMap.put(task.get().callback, null);
    }
  }

  private class ForCallback implements Predicate< Task< Params > >
  {
    private final EngineSerializerAPI.OnDeserializedArticlesReadyCallback forCallback;

    private ForCallback( EngineSerializerAPI.OnDeserializedArticlesReadyCallback _forCallback )
    {
      forCallback = _forCallback;
    }

    @Override
    public boolean test( Task< Params > task )
    {
      return forCallback == task.get().callback;
    }
  }
}
