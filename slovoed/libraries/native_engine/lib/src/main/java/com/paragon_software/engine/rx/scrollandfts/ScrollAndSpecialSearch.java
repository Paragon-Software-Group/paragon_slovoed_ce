package com.paragon_software.engine.rx.scrollandfts;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.location.DictionaryLocation;
import com.paragon_software.engine.rx.EngineTaskRunner;
import com.paragon_software.engine.rx.RunOn;
import com.paragon_software.engine.rx.Task;
import com.paragon_software.engine.rx.scrollandfts.params.AbstractParams;
import com.paragon_software.engine.rx.scrollandfts.params.ScrollParams;
import com.paragon_software.engine.rx.scrollandfts.params.SpecialSearchParam;
import com.paragon_software.engine.rx.scrollandfts.result.AbstractResult;
import com.paragon_software.engine.rx.scrollandfts.result.NoResult;
import com.paragon_software.native_engine.GroupHeader;
import com.paragon_software.native_engine.ScrollResult;
import com.paragon_software.native_engine.SearchType;
import com.paragon_software.native_engine.SortType;
import com.paragon_software.utils_slovoed.collections.CollectionView;

import java.lang.ref.WeakReference;
import java.util.Collection;
import java.util.Collections;

import io.reactivex.Observer;
import io.reactivex.functions.Consumer;
import io.reactivex.functions.Function;
import io.reactivex.subjects.PublishSubject;
import io.reactivex.subjects.Subject;

public final class ScrollAndSpecialSearch extends EngineTaskRunner< AbstractParams >
{
  @NonNull
  private Subject< Task< AbstractParams > > source = PublishSubject.create();

  @NonNull
  private final SearchProcessor searchProcessor = new SearchProcessor();

  @Nullable
  private Task< AbstractParams > currentTask = null;

  @Nullable
  private WeakReference< ResultContainer > weakResultContainer = null;

  @NonNull
  public ScrollResult scroll(Dictionary.DictionaryId dictionaryId,
                             Dictionary.Direction direction, String word,
                             @Nullable Collection<Dictionary.Direction> availableDirections, boolean exactly )
  {
    assertInitCalled();
    return search(new ScrollParams(dictionaryId, direction, word, availableDirections, exactly)).getScrollResult();
  }

  @NonNull
  public CollectionView<CollectionView<ArticleItem, GroupHeader>, Dictionary.Direction>
    search(Dictionary.DictionaryId dictionaryId,
           Dictionary.Direction direction,
           String word,
           @Nullable Collection<Dictionary.Direction> availableDirections,
           @NonNull SearchType searchType,
           @NonNull SortType sortType,
           @NonNull Boolean needRunSearch)
  {
    assertInitCalled();
    return search(new SpecialSearchParam(dictionaryId, direction, word, availableDirections, searchType, sortType, needRunSearch)).getSpecialSearchCollectionView();
  }

  @NonNull
  private ResultContainer search( @NonNull AbstractParams params )
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
    emitNewTask(params);
    return resultContainer;
  }

  @Override
  protected void emitTask( Task< AbstractParams > task )
  {
    source.onNext(task);
  }

  @Override
  protected Collection< Task< AbstractParams > > getCurrentTasks()
  {
    Collection< Task< AbstractParams > > res = Collections.emptySet();
    if ( currentTask != null )
    {
      res = Collections.singleton(currentTask);
    }
    return res;
  }

  private void subscribe( @NonNull Observer< Task< AbstractResult > > resultContainer )
  {
    source.doOnNext(new SaveCurrentTaskAndNotifyResultContainer())
          .compose(new RunOn< Task< AbstractParams > >(RunOn.What.Observe, RunOn.Where.Computation, 200L))
          .map(new DoSearch()).compose(new RunOn< Task< AbstractResult > >(RunOn.What.Observe, RunOn.Where.Caller))
          .filter(new OnlyUncanceled()).doOnNext(new DiscardCurrentTask()).subscribe(resultContainer);

  }

  private class SaveCurrentTaskAndNotifyResultContainer implements Consumer< Task< AbstractParams > >
  {
    @Override
    public void accept( Task< AbstractParams > task )
    {
      if ( currentTask != null )
      {
        currentTask.cancel();
      }
      currentTask = task;
      ResultContainer resultContainer = null;
      if ( weakResultContainer != null )
      {
        resultContainer = weakResultContainer.get();
      }
      if ( resultContainer != null )
      {
        AbstractParams params = task.get();
        DictionaryLocation dictionaryLocation = null;
        Dictionary dictionary = findDictionary(params.dictionaryId);
        if (null != dictionary)
        {
          dictionaryLocation = dictionary.getDictionaryLocation();
        }
        resultContainer.beforeSearch(dictionaryLocation, params);
      }
    }
  }

  private class DoSearch implements Function< Task< AbstractParams >, Task< AbstractResult > >
  {
    @Override
    public Task< AbstractResult > apply( Task< AbstractParams > task )
    {
      AbstractResult result = new NoResult();
      if ( !task.isCanceled() )
      {
        AbstractParams params = task.get();
        result = searchProcessor.process(applicationContext, findDictionary(params.dictionaryId), params);
      }
      return task.spawn(result);
    }
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

  private class DiscardCurrentTask implements Consumer< Task< AbstractResult > >
  {
    @Override
    public void accept( Task< AbstractResult > abstractResultTask )
    {
      currentTask = null;
    }
  }
}
