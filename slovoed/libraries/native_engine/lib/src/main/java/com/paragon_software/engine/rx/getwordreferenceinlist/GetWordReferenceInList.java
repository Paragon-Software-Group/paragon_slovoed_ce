package com.paragon_software.engine.rx.getwordreferenceinlist;

import androidx.annotation.NonNull;
import android.util.SparseArray;

import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.engine.nativewrapper.ListInfo;
import com.paragon_software.engine.nativewrapper.ListType;
import com.paragon_software.engine.nativewrapper.NativeDictionary;
import com.paragon_software.engine.rx.RunOn;
import com.paragon_software.engine.rx.Task;
import com.paragon_software.engine.rx.base.BaseEngineTaskRunner;

import java.util.ArrayList;

import io.reactivex.disposables.Disposable;
import io.reactivex.functions.Consumer;
import io.reactivex.functions.Function;

public class GetWordReferenceInList extends BaseEngineTaskRunner< Params, Result, ResultContainer >
{
  @NonNull
  @Override
  protected ResultContainer createResultContainer()
  {
    return new ResultContainer();
  }

  @Override
  protected Disposable subscribe(@NonNull ResultContainer resultContainer )
  {
    return source.doOnNext(new SaveCurrentTask())
            .compose(new RunOn<>(RunOn.What.Observe, RunOn.Where.Computation))
            .map(new SearchImplementation())
            .compose(new RunOn<>(RunOn.What.Observe, RunOn.Where.Caller))
            .filter(new OnlyUncanceled())
            .doOnNext(getDiscardCurrentTask())
            .subscribe(resultTask ->
            {
              resultContainer.onResult(resultTask.get());
            }, Throwable::printStackTrace);
  }

  private class SearchImplementation implements Function< Task< Params >, Task< Result > >
  {
    @Override
    public Task< Result > apply( Task< Params > paramsTask ) throws Exception
    {
      Result result = null;
      Params params = paramsTask.get();
      Dictionary dictionary = findDictionary(params.dictionaryId);
      if ( null != dictionary )
      {
        NativeDictionary nativeDictionary = NativeDictionary.open(applicationContext, dictionary, true, false);
        if ( null != nativeDictionary )
        {
          int listIndex = getListIndex(params.direction, nativeDictionary);
          if ( listIndex >= 0 )
          {
            int[] wordRefs = nativeDictionary.getWordReferenceInList(listIndex, params.query);
            TransformWordReferences transformWordReferences =
                new TransformWordReferences(dictionary.getId(), nativeDictionary, wordRefs, listIndex);
            result = new Result(transformWordReferences.get());
          }
          nativeDictionary.close();
        }
      }
      if ( null == result )
      {
        result = new Result(new ArrayList<>());
      }
      return paramsTask.spawn(result);
    }


    /**
     * @return negative value if list index with specified direction not found
     */
    private int getListIndex( Dictionary.Direction direction, NativeDictionary nativeDictionary )
    {
      int listIndex = Integer.MIN_VALUE;
      SparseArray< ListInfo > lists = nativeDictionary.getLists(ListType.Group.Main);
      int i, n = lists.size();
      for ( i = 0; i < n ; i++ )
      {
        ListInfo info = lists.valueAt(i);
        if ( ( info.languageFrom == direction.getLanguageFrom() ))
        {
          listIndex = lists.keyAt(i);
        }
      }
      return listIndex;
    }
  }

  private class SaveCurrentTask implements Consumer< Task< Params > >
  {
    @Override
    public void accept( Task< Params > paramsTask ) throws Exception
    {
      if ( currentTask != null )
      {
        currentTask.cancel();
      }
      currentTask = paramsTask;

      ResultContainer resultContainer = null;
      if ( weakResultContainer != null && null != ( resultContainer = weakResultContainer.get() ) )
      {
        resultContainer.beforeSearch();
      }
    }
  }
}
