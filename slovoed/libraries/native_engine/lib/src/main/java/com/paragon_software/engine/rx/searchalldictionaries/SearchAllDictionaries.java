package com.paragon_software.engine.rx.searchalldictionaries;

import android.content.Context;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.engine.rx.EngineTaskRunner;
import com.paragon_software.engine.rx.RunOn;
import com.paragon_software.engine.rx.Task;
import com.paragon_software.native_engine.SearchAllResult;
import io.reactivex.Observable;
import io.reactivex.ObservableSource;
import io.reactivex.Observer;
import io.reactivex.functions.Consumer;
import io.reactivex.functions.Function;
import io.reactivex.subjects.PublishSubject;
import io.reactivex.subjects.Subject;

public final class SearchAllDictionaries extends EngineTaskRunner< Params >
{
  @NonNull
  private Subject< Task< Params > > source = PublishSubject.create();

  @Nullable
  private Task< Params > currentTask = null;

  @Nullable
  private WeakReference< ResultContainer > weakResultContainer = null;

  @NonNull
  public SearchAllResult search( @NonNull String query, int maxWords )
  {
    assertInitCalled();
    ResultContainer resultContainer = null;
    if ( weakResultContainer != null )
    {
      resultContainer = weakResultContainer.get();
    }
    if ( resultContainer == null )
    {
      resultContainer = new ResultContainer();
      weakResultContainer = new WeakReference<>(resultContainer);
      subscribe(resultContainer.getObserver());
    }

    resultContainer.toggleSearch(true);
    emitNewTask(new Params(dictionaryManager.getDictionaries(), query, maxWords));
    return resultContainer;
  }

  @Override
  protected void emitTask( Task< Params > task )
  {
    source.onNext(task);
  }

  @Override
  protected Collection< Task< Params > > getCurrentTasks()
  {
    Collection< Task< Params > > res = Collections.emptySet();
    if ( currentTask != null )
    {
      res = Collections.singleton(currentTask);
    }
    return res;
  }

  private void subscribe( @NonNull Observer< Task< Result > > resultContainer )
  {
    RunOn< Task< Params > > split1 = new RunOn<>(RunOn.What.Observe, RunOn.Where.Computation, 200L);
    RunOn< Task< Result > > split2 = new RunOn<>(RunOn.What.Subscribe, RunOn.Where.Computation);
    RunOn< Task< Result > > result = new RunOn<>(RunOn.What.Observe, RunOn.Where.Caller);
    source.doOnNext(new SaveCurrentTask()).compose(split1)
          .switchMap(new SplitDictionaryArray(applicationContext, split2)).compose(result).filter(new OnlyUncanceled())
          .doOnNext(new DiscardCurrentTask()).subscribe(resultContainer);
  }

  @Override
  public void onDictionaryListChanged()
  {
    if ( weakResultContainer != null )
    {
      ResultContainer resultContainer = weakResultContainer.get();
      if (resultContainer != null)
      {
        resultContainer.reset();
      }
    }
    super.onDictionaryListChanged();
  }

  private class SaveCurrentTask implements Consumer< Task< Params > >
  {
    @Override
    public void accept( Task< Params > paramsTask )
    {
      //      Log.d("SLOVOED", "search request: " + Thread.currentThread().getName());
      if ( currentTask != null )
      {
        currentTask.cancel();
      }
      currentTask = paramsTask;
    }
  }

  private class DiscardCurrentTask implements Consumer< Task< Result > >
  {
    @Override
    public void accept( Task< Result > resultTask )
    {
      if ( resultTask.get().isTerminator() )
      {
        currentTask = null;
      }
      //      if ( resultTask.isCanceled() )
      //      {
      //        Log.d("SLOVOED", "canceled task squeezed through");
      //      }
    }
  }

  private static class SplitDictionaryArray implements Function< Task< Params >, ObservableSource< Task< Result > > >
  {
    private final Context applicationContext;

    @NonNull
    private final RunOn< Task< Result > > dictionaryRunOn;

    SplitDictionaryArray( Context context, @NonNull RunOn< Task< Result > > _dictionaryRunOn )
    {
      applicationContext = context;
      dictionaryRunOn = _dictionaryRunOn;
    }

    @Override
    public ObservableSource< Task< Result > > apply( Task< Params > task )
    {
      //      Log.d("SLOVOED", "split dicts: " + Thread.currentThread().getName());
      ObservableSource< Task< Result > > res;
      if ( !task.isCanceled() )
      {
        Params params = task.get();
        List< Task< SingleFileParams > > list = new ArrayList<>(params.dictionaries.size());
        for ( Dictionary dictionary : params.dictionaries )
        {
          list.add(task.spawn(new SingleFileParams(dictionary, params.query, params.maxWords)));
        }
        res = Observable.fromIterable(list).flatMap(new SearchInDictionary(applicationContext, dictionaryRunOn))
                        .concatWith(Observable.just(task.spawn(Result.TERMINATOR)));
      }
      else
      {
        //        Log.d("SLOVOED", "split canceled");
        res = Observable.empty();
      }
      return res;
    }
  }

  private static class SearchInDictionary
      implements Function< Task< SingleFileParams >, ObservableSource< Task< Result > > >
  {
    private final Context applicationContext;

    @NonNull
    private final RunOn< Task< Result > > runOn;

    SearchInDictionary( Context _context, @NonNull RunOn< Task< Result > > _runOn )
    {
      applicationContext = _context;
      runOn = _runOn;
    }

    @Override
    public ObservableSource< Task< Result > > apply( Task< SingleFileParams > task )
    {
      return Observable.create(new FtsSource(applicationContext, task)).compose(runOn);
    }
  }
}
